#include "RetailApi.hpp"

#include "Diagnostics.hpp"
#include "types.h"
#include <CTRPluginFramework/System/Process.hpp>

extern "C" void Follower3gx_SetGfglSingleton(void* implementation);

namespace Gen7Follower3gx
{
namespace
{

bool g_IsRetailApiBound = false;

bool IsReadable(u32 address, u32 size)
{
  if (size == 0 || address + size < address)
  {
    return false;
  }
  return CTRPluginFramework::Process::CheckAddress(address, MEMPERM_READ) &&
    CTRPluginFramework::Process::CheckAddress(
      address + size - 1,
      MEMPERM_READ
      );
}

u32 ReadU32(u32 address)
{
  return *reinterpret_cast<const volatile u32*>(address);
}

bool IsPrevalidatedLifecycleEntry(
  const GameProfile& profile,
  u32 address,
  u32 expectedInstruction
)
{
  const LifecycleProfile& lifecycle = profile.lifecycle;
  return
    (address == lifecycle.startModuleAddress &&
     expectedInstruction == lifecycle.startModuleInstructions[0]) ||
    (address == lifecycle.disposeModuleAddress &&
     expectedInstruction == lifecycle.disposeModuleInstructions[0]);
}

} // namespace

bool BindRetailApi(const GameProfile& profile, const CroModuleView& fieldRo)
{
  if (!fieldRo.Module() || g_IsRetailApiBound)
  {
    return false;
  }

  u32 resolved[RetailFunctionCount];
  for (u32 index = 0; index < RetailFunctionCount; ++index)
  {
    u32 address = profile.retailAddresses[index];
    if (g_RetailFunctionProviders[index] == RetailProvider::FieldRo)
    {
      if (address >= fieldRo.TextSize())
      {
        return false;
      }
      address += fieldRo.TextBase();
    }

    const u32 expectedInstruction =
      profile.retailEntryInstructions[index];
    // These entries were checked before adding hooks. Their first instructions have changed since then.
    const bool prevalidatedLifecycleEntry =
      g_RetailFunctionProviders[index] == RetailProvider::Static &&
      IsPrevalidatedLifecycleEntry(profile, address, expectedInstruction);

    if (!IsReadable(address, sizeof(u32)) ||
        (!prevalidatedLifecycleEntry &&
         ReadU32(address) != expectedInstruction))
    {
      FOLLOWER_3GX_TRACE_VALUE("retail ABI index", index);
      FOLLOWER_3GX_TRACE_VALUE("retail ABI address", address);
      if (IsReadable(address, sizeof(u32)))
      {
        FOLLOWER_3GX_TRACE_VALUE("retail ABI actual", ReadU32(address));
      }
      FOLLOWER_3GX_TRACE_VALUE(
        "retail ABI expected",
        expectedInstruction
        );
      return false;
    }
    resolved[index] = address;
  }

  if (!IsReadable(profile.gfglSingletonAddress, sizeof(u32)))
  {
    FOLLOWER_3GX_TRACE("gfgl singleton address unreadable");
    return false;
  }
  void* gfglSingleton = reinterpret_cast<void*>(
    ReadU32(profile.gfglSingletonAddress)
    );
  if (!gfglSingleton)
  {
    FOLLOWER_3GX_TRACE("gfgl singleton is null");
    return false;
  }

  for (u32 index = 0; index < RetailFunctionCount; ++index)
  {
    g_RetailFunctionPointers[index] = resolved[index];
  }
  Follower3gx_SetGfglSingleton(gfglSingleton);
  g_IsRetailApiBound = true;
  return true;
}

void UnbindRetailApi()
{
  g_IsRetailApiBound = false;
  Follower3gx_SetGfglSingleton(NULL);
  ClearRetailFunctionPointers();
}

bool IsRetailApiBound()
{
  return g_IsRetailApiBound;
}

} // namespace Gen7Follower3gx
