#include "LayeredFsConflict.hpp"

#include <3ds/types.h>
#include <cstring>
#include <CTRPluginFramework/System/File.hpp>
#include <CTRPluginFramework/System/Process.hpp>
#include <CTRPluginFramework/System/System.hpp>
#include <CTRPluginFramework/Utils/Utils.hpp>

#include "CroModule.hpp"
#include "GameProfile.hpp"

namespace Gen7Follower3gx
{
namespace
{

enum ConflictFlag
{
  CONFLICT_FIELD_RO_FILE = 1U << 0,
  CONFLICT_FIELD_DEBUG_FILE = 1U << 1,
  CONFLICT_STATIC_CRR_FILE = 1U << 2,
  CONFLICT_EMULATOR_FIELD_RO = 1U << 3,
};

u32 g_ConflictFlags = 0;
u32 g_WarningPending = 0;

struct Sha256Context
{
  u32 state[8];
  u64 bitLength;
  u8 block[64];
  u32 blockLength;
};

const u32 kSha256RoundConstants[64] =
{
  0x428a2f98U, 0x71374491U, 0xb5c0fbcfU, 0xe9b5dba5U,
  0x3956c25bU, 0x59f111f1U, 0x923f82a4U, 0xab1c5ed5U,
  0xd807aa98U, 0x12835b01U, 0x243185beU, 0x550c7dc3U,
  0x72be5d74U, 0x80deb1feU, 0x9bdc06a7U, 0xc19bf174U,
  0xe49b69c1U, 0xefbe4786U, 0x0fc19dc6U, 0x240ca1ccU,
  0x2de92c6fU, 0x4a7484aaU, 0x5cb0a9dcU, 0x76f988daU,
  0x983e5152U, 0xa831c66dU, 0xb00327c8U, 0xbf597fc7U,
  0xc6e00bf3U, 0xd5a79147U, 0x06ca6351U, 0x14292967U,
  0x27b70a85U, 0x2e1b2138U, 0x4d2c6dfcU, 0x53380d13U,
  0x650a7354U, 0x766a0abbU, 0x81c2c92eU, 0x92722c85U,
  0xa2bfe8a1U, 0xa81a664bU, 0xc24b8b70U, 0xc76c51a3U,
  0xd192e819U, 0xd6990624U, 0xf40e3585U, 0x106aa070U,
  0x19a4c116U, 0x1e376c08U, 0x2748774cU, 0x34b0bcb5U,
  0x391c0cb3U, 0x4ed8aa4aU, 0x5b9cca4fU, 0x682e6ff3U,
  0x748f82eeU, 0x78a5636fU, 0x84c87814U, 0x8cc70208U,
  0x90befffaU, 0xa4506cebU, 0xbef9a3f7U, 0xc67178f2U,
};

u32 RotateRight(u32 value, u32 count)
{
  return (value >> count) | (value << (32U - count));
}

u32 ReadBigEndianU32(const u8* bytes)
{
  return
    (static_cast<u32>(bytes[0]) << 24) |
    (static_cast<u32>(bytes[1]) << 16) |
    (static_cast<u32>(bytes[2]) << 8) |
    static_cast<u32>(bytes[3]);
}

void WriteBigEndianU32(u8* bytes, u32 value)
{
  bytes[0] = static_cast<u8>(value >> 24);
  bytes[1] = static_cast<u8>(value >> 16);
  bytes[2] = static_cast<u8>(value >> 8);
  bytes[3] = static_cast<u8>(value);
}

void TransformSha256Block(Sha256Context& context, const u8* block)
{
  u32 words[64];
  for (u32 index = 0; index < 16; ++index)
  {
    words[index] = ReadBigEndianU32(block + index * 4U);
  }
  for (u32 index = 16; index < 64; ++index)
  {
    const u32 previous15 = words[index - 15];
    const u32 previous2 = words[index - 2];
    const u32 sigma0 =
      RotateRight(previous15, 7) ^
      RotateRight(previous15, 18) ^
      (previous15 >> 3);
    const u32 sigma1 =
      RotateRight(previous2, 17) ^
      RotateRight(previous2, 19) ^
      (previous2 >> 10);
    words[index] =
      words[index - 16] + sigma0 + words[index - 7] + sigma1;
  }

  u32 a = context.state[0];
  u32 b = context.state[1];
  u32 c = context.state[2];
  u32 d = context.state[3];
  u32 e = context.state[4];
  u32 f = context.state[5];
  u32 g = context.state[6];
  u32 h = context.state[7];

  for (u32 index = 0; index < 64; ++index)
  {
    const u32 sum1 =
      RotateRight(e, 6) ^ RotateRight(e, 11) ^ RotateRight(e, 25);
    const u32 choice = (e & f) ^ ((~e) & g);
    const u32 temporary1 =
      h + sum1 + choice + kSha256RoundConstants[index] + words[index];
    const u32 sum0 =
      RotateRight(a, 2) ^ RotateRight(a, 13) ^ RotateRight(a, 22);
    const u32 majority = (a & b) ^ (a & c) ^ (b & c);
    const u32 temporary2 = sum0 + majority;

    h = g;
    g = f;
    f = e;
    e = d + temporary1;
    d = c;
    c = b;
    b = a;
    a = temporary1 + temporary2;
  }

  context.state[0] += a;
  context.state[1] += b;
  context.state[2] += c;
  context.state[3] += d;
  context.state[4] += e;
  context.state[5] += f;
  context.state[6] += g;
  context.state[7] += h;
}

void InitializeSha256(Sha256Context& context)
{
  context.state[0] = 0x6a09e667U;
  context.state[1] = 0xbb67ae85U;
  context.state[2] = 0x3c6ef372U;
  context.state[3] = 0xa54ff53aU;
  context.state[4] = 0x510e527fU;
  context.state[5] = 0x9b05688cU;
  context.state[6] = 0x1f83d9abU;
  context.state[7] = 0x5be0cd19U;
  context.bitLength = 0;
  context.blockLength = 0;
}

void UpdateSha256(Sha256Context& context, const u8* data, u32 length)
{
  context.bitLength += static_cast<u64>(length) * 8U;
  while (length != 0)
  {
    const u32 available = 64U - context.blockLength;
    const u32 copyLength = length < available ? length : available;
    std::memcpy(
      context.block + context.blockLength,
      data,
      copyLength
      );
    context.blockLength += copyLength;
    data += copyLength;
    length -= copyLength;
    if (context.blockLength == 64U)
    {
      TransformSha256Block(context, context.block);
      context.blockLength = 0;
    }
  }
}

void FinalizeSha256(Sha256Context& context, u8 digest[32])
{
  const u64 messageBitLength = context.bitLength;
  const u8 marker = 0x80U;
  static const u8 zeros[64] = {};
  UpdateSha256(context, &marker, 1);
  const u32 paddingLength = context.blockLength <= 56U
    ? 56U - context.blockLength
    : 64U + 56U - context.blockLength;
  UpdateSha256(context, zeros, paddingLength);

  u8 encodedLength[8];
  for (u32 index = 0; index < 8; ++index)
  {
    encodedLength[7U - index] = static_cast<u8>(
      messageBitLength >> (index * 8U)
      );
  }
  UpdateSha256(context, encodedLength, sizeof(encodedLength));

  for (u32 index = 0; index < 8; ++index)
  {
    WriteBigEndianU32(digest + index * 4U, context.state[index]);
  }
}

int HexNibble(char value)
{
  if (value >= '0' && value <= '9')
  {
    return value - '0';
  }
  if (value >= 'a' && value <= 'f')
  {
    return value - 'a' + 10;
  }
  if (value >= 'A' && value <= 'F')
  {
    return value - 'A' + 10;
  }
  return -1;
}

bool DigestMatchesHex(const u8 digest[32], const char* expectedHex)
{
  if (!expectedHex || std::strlen(expectedHex) != 64U)
  {
    return false;
  }
  for (u32 index = 0; index < 32; ++index)
  {
    const int high = HexNibble(expectedHex[index * 2U]);
    const int low = HexNibble(expectedHex[index * 2U + 1U]);
    if (high < 0 || low < 0 ||
        digest[index] != static_cast<u8>((high << 4) | low))
    {
      return false;
    }
  }
  return true;
}

bool FileMatchesSha256(const std::string& path, const char* expectedHex)
{
  CTRPluginFramework::File file;
  if (CTRPluginFramework::File::Open(
        file,
        path,
        CTRPluginFramework::File::READ
        ) != CTRPluginFramework::File::SUCCESS)
  {
    return false;
  }

  const u64 fileSize = file.GetSize();
  if (fileSize > 0xffffffffULL)
  {
    return false;
  }

  Sha256Context context;
  InitializeSha256(context);
  u8 readBuffer[1024];
  u64 remaining = fileSize;
  while (remaining != 0)
  {
    const u32 readLength = remaining < sizeof(readBuffer)
      ? static_cast<u32>(remaining)
      : static_cast<u32>(sizeof(readBuffer));
    if (file.Read(readBuffer, readLength) != CTRPluginFramework::File::SUCCESS)
    {
      return false;
    }
    UpdateSha256(context, readBuffer, readLength);
    remaining -= readLength;
  }

  u8 digest[32];
  FinalizeSha256(context, digest);
  return DigestMatchesHex(digest, expectedHex);
}

std::string BuildRomFsPath()
{
  return CTRPluginFramework::Utils::Format(
    "/luma/titles/%016llX/romfs",
    CTRPluginFramework::Process::GetTitleID()
    );
}

bool FileExists(const std::string& path)
{
  return CTRPluginFramework::File::Exists(path) == 1;
}

void RecordConflict(u32 flags)
{
  if (flags == 0)
  {
    return;
  }

  const u32 previous = __atomic_fetch_or(
    &g_ConflictFlags,
    flags,
    __ATOMIC_RELAXED
    );
  if (previous == 0)
  {
    __atomic_store_n(&g_WarningPending, 1U, __ATOMIC_RELEASE);
  }
}

bool HookCallsiteMatches(
  const HookProfile& hook,
  const CroModuleView& fieldRo
)
{
  if (hook.callsiteTextOffset + sizeof(u32) > fieldRo.TextSize())
  {
    return false;
  }

  const volatile u32* const callsite = reinterpret_cast<const volatile u32*>(
    fieldRo.TextBase() + hook.callsiteTextOffset
    );
  return *callsite == hook.expectedCallInstruction;
}

void AppendDetectedFile(
  std::string& message,
  u32 flags,
  u32 flag,
  const char* relativePath
)
{
  if ((flags & flag) != 0)
  {
    message += "\n";
    message += relativePath;
  }
}

} // namespace

void InitializeLayeredFsConflictDetection()
{
  __atomic_store_n(&g_ConflictFlags, 0U, __ATOMIC_RELAXED);
  __atomic_store_n(&g_WarningPending, 0U, __ATOMIC_RELAXED);

  // We can't see emulator mod files through the SD archive, so check the loaded CRO.
  if (CTRPluginFramework::System::IsCitra())
  {
    return;
  }

  const GameProfile* const profile = SelectGameProfile(
    CTRPluginFramework::Process::GetTitleID(),
    CTRPluginFramework::Process::GetVersion()
    );
  if (!profile)
  {
    return;
  }

  const std::string romFsPath = BuildRomFsPath();
  u32 flags = 0;
  const std::string fieldRoPath = romFsPath + "/FieldRo.cro";
  if (FileExists(fieldRoPath) &&
      !FileMatchesSha256(fieldRoPath, profile->fieldRoSha256))
  {
    flags |= CONFLICT_FIELD_RO_FILE;
  }
  const std::string fieldDebugPath = romFsPath + "/FieldDebug.cro";
  if (FileExists(fieldDebugPath) &&
      !FileMatchesSha256(fieldDebugPath, profile->fieldDebugSha256))
  {
    flags |= CONFLICT_FIELD_DEBUG_FILE;
  }
  if (FileExists(romFsPath + "/.crr/static.crr"))
  {
    flags |= CONFLICT_STATIC_CRR_FILE;
  }

  // A CRR could belong to another mod. Also check for a modified follower CRO.
  if ((flags & (CONFLICT_FIELD_RO_FILE | CONFLICT_FIELD_DEBUG_FILE)) != 0)
  {
    RecordConflict(flags);
  }
}

void InspectEmulatorFieldRoForConflict(
  const GameProfile& profile,
  const CroModuleView& fieldRo
)
{
  if (!CTRPluginFramework::System::IsCitra())
  {
    return;
  }

  if (!HookCallsiteMatches(profile.updateHook, fieldRo) ||
      !HookCallsiteMatches(profile.terminateHook, fieldRo) ||
      !HookCallsiteMatches(profile.surfHook, fieldRo) ||
      !HookCallsiteMatches(profile.eventCheckHook, fieldRo))
  {
    RecordConflict(CONFLICT_EMULATOR_FIELD_RO);
  }
}

bool HasLayeredFsConflict()
{
  return __atomic_load_n(&g_ConflictFlags, __ATOMIC_RELAXED) != 0;
}

bool ConsumeLayeredFsConflictWarning()
{
  return __atomic_exchange_n(
    &g_WarningPending,
    0U,
    __ATOMIC_ACQUIRE
    ) != 0;
}

std::string BuildLayeredFsConflictMessage()
{
  const u32 flags = __atomic_load_n(&g_ConflictFlags, __ATOMIC_RELAXED);
  const std::string titleId = CTRPluginFramework::Utils::Format(
    "%016llX",
    CTRPluginFramework::Process::GetTitleID()
    );

  if ((flags & CONFLICT_EMULATOR_FIELD_RO) != 0 &&
      (flags & (CONFLICT_FIELD_RO_FILE | CONFLICT_FIELD_DEBUG_FILE)) == 0)
  {
    return
      "An incompatible FieldRo CRO was loaded from emulator LayeredFS.\n\n"
      "Please remove the old follower files from:\n"
      "load/mods/" + titleId + "/romfs/\n\n"
      "Remove FieldRo.cro and FieldDebug.cro. Remove .crr/static.crr only "
      "if it came from the old follower package. Then fully restart the "
      "game.";
  }

  std::string message =
    "Legacy follower LayeredFS files conflict with this 3GX.\n\n"
    "Please delete or move these files from:\n" +
    BuildRomFsPath() + "/";
  AppendDetectedFile(
    message,
    flags,
    CONFLICT_FIELD_RO_FILE,
    "FieldRo.cro"
    );
  AppendDetectedFile(
    message,
    flags,
    CONFLICT_FIELD_DEBUG_FILE,
    "FieldDebug.cro"
    );
  AppendDetectedFile(
    message,
    flags,
    CONFLICT_STATIC_CRR_FILE,
    ".crr/static.crr"
    );
  message += "\n\nThen fully restart the game.";
  return message;
}

} // namespace Gen7Follower3gx
