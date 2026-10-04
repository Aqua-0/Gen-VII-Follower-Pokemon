#include "FollowerArena.hpp"
#include "Diagnostics.hpp"
#include "PartyFollowerSettings.hpp"
#include "FollowerArenaLifecycle.hpp"
#include "types.h"
#include <3ds.h>
#include <CTRPluginFramework/System/Process.hpp>
#include <CTRPluginFramework/System/System.hpp>
#include <cstdio>
#include <cstring>
namespace Gen7Follower3gx {
namespace {
const u32 MaximumUnregisteredDeviceTail=0x800000;
const u32 MainImageScanBegin=0x00100000, MainImageScanEnd=0x01000000;
const u32 OldLinearHeapAddress=0x14000000, OldLinearHeapEnd=0x1c000000;
const u32 NewLinearHeapAddress=0x30000000, NewLinearHeapEnd=0x40000000;
const u32 kDeviceMemorySizeGetterLoadLiteral=0xe59f0004U;
const u32 kDeviceMemorySizeGetterLoadSize=0xe5900004U;
const u32 kDeviceMemorySizeGetterReturn=0xe12fff1eU;
#define FOLLOWER_ARENA_TRACE_VALUE(label, value) ((void)0)
void* arenaHeap=NULL;
unsigned int arenaCapacity=0;
unsigned int arenaSize=FollowerArenaSize;
bool quarantined=false;
const char* arenaStatus="Not attempted";
Result FreeApplicationFcram(void* memory, u32 size)
{
  if (!memory || size == 0)
  {
    return 0;
  }
  u32 releasedAddress = 0;
  return svcControlMemory(
    &releasedAddress,
    reinterpret_cast<u32>(memory),
    0,
    size,
    MEMOP_FREE,
    static_cast<MemPerm>(0)
    );
}

struct DeviceMemoryRegistration
{
  volatile u32* sizeWord;
  u32 getterAddress;
  u32 address;
  u32 mappedSize;
  u32 registeredSize;

  DeviceMemoryRegistration()
    : sizeWord(NULL),
      getterAddress(0),
      address(0),
      mappedSize(0),
      registeredSize(0)
  {
  }
};

bool QueryMemory(u32 address, MemInfo& memory)
{
  PageInfo page;
  std::memset(&memory, 0, sizeof(memory));
  std::memset(&page, 0, sizeof(page));
  return R_SUCCEEDED(svcQueryMemory(&memory, &page, address)) &&
    memory.size != 0;
}

bool IsReadWriteMemory(const MemInfo& memory)
{
  return (memory.perm & MEMPERM_READWRITE) == MEMPERM_READWRITE;
}

bool IsReadExecuteMemory(const MemInfo& memory)
{
  const u32 required = MEMPERM_READ | MEMPERM_EXECUTE;
  return (memory.perm & required) == required;
}

bool ContainsRange(const MemInfo& memory, u32 address, u32 size)
{
  const u64 memoryEnd =
    static_cast<u64>(memory.base_addr) + memory.size;
  const u64 rangeEnd = static_cast<u64>(address) + size;
  return size != 0 && address >= memory.base_addr &&
    rangeEnd <= memoryEnd;
}

bool FindDeviceMemoryRange(
  DeviceMemoryRegistration& registration,
  u32 requestedSize
  )
{
  const u32 addresses[] =
  {
    NewLinearHeapAddress,
    OldLinearHeapAddress
  };
  const u32 ends[] =
  {
    NewLinearHeapEnd,
    OldLinearHeapEnd
  };
  for (u32 index = 0; index < sizeof(addresses) / sizeof(addresses[0]);
       ++index)
  {
    const u32 deviceAddress = addresses[index];
    const u32 linearEnd = ends[index];
    MemInfo deviceMemory;
    if (!QueryMemory(deviceAddress, deviceMemory) ||
        deviceMemory.base_addr != deviceAddress ||
        deviceMemory.size < 0x100000 ||
        (deviceMemory.size & 0xfffU) != 0 ||
        !IsReadWriteMemory(deviceMemory) ||
        deviceMemory.size > linearEnd - deviceAddress ||
        requestedSize >
          linearEnd - deviceAddress - deviceMemory.size)
    {
      continue;
    }

    const u32 deviceEnd = deviceAddress + deviceMemory.size;
    MemInfo nextMemory;
    if (!QueryMemory(deviceEnd, nextMemory) ||
        nextMemory.state != MEMSTATE_FREE)
    {
      continue;
    }

    registration.address = deviceAddress;
    registration.mappedSize = deviceMemory.size;
    return true;
  }
  return false;
}

bool IsReadable(const void* pointer, u32 size)
{
  const u32 address = reinterpret_cast<u32>(pointer);
  return pointer && size && address + size >= address &&
    CTRPluginFramework::Process::CheckAddress(address, MEMPERM_READ) &&
    CTRPluginFramework::Process::CheckAddress(address + size - 1, MEMPERM_READ);
}

bool FindDeviceMemorySizeWord(
  DeviceMemoryRegistration& registration,
  u32& patternMatches,
  u32& validatedMatches
  )
{
  patternMatches = 0;
  validatedMatches = 0;
  volatile u32* selectedSizeWord = NULL;
  u32 selectedGetterAddress = 0;
  bool ambiguous = false;

  u32 cursor = MainImageScanBegin;
  while (cursor < MainImageScanEnd)
  {
    MemInfo memory;
    if (!QueryMemory(cursor, memory))
    {
      return false;
    }
    const u64 blockEnd64 = static_cast<u64>(memory.base_addr) + memory.size;
    if (blockEnd64 <= cursor)
    {
      return false;
    }
    const u32 blockEnd = blockEnd64 > MainImageScanEnd
      ? static_cast<u32>(MainImageScanEnd)
      : static_cast<u32>(blockEnd64);
    if (IsReadExecuteMemory(memory) && blockEnd > cursor)
    {
      u32 scan = memory.base_addr > cursor ? memory.base_addr : cursor;
      scan = (scan + 3U) & ~3U;
      while (scan < blockEnd &&
             blockEnd - scan >= 4U * sizeof(u32))
      {
        const volatile u32* const words =
          reinterpret_cast<const volatile u32*>(scan);
        if (words[0] == kDeviceMemorySizeGetterLoadLiteral &&
            words[1] == kDeviceMemorySizeGetterLoadSize &&
            words[2] == kDeviceMemorySizeGetterReturn)
        {
          ++patternMatches;
          const u32 globalsAddress = words[3];
          FOLLOWER_ARENA_TRACE_VALUE(
            "external getter candidate index",
            patternMatches - 1
            );
          FOLLOWER_ARENA_TRACE_VALUE(
            "external getter candidate function",
            scan
            );
          FOLLOWER_ARENA_TRACE_VALUE(
            "external getter candidate globals",
            globalsAddress
            );
          MemInfo globalsMemory;
          std::memset(&globalsMemory, 0, sizeof(globalsMemory));
          u32 accessRejectionMask = 0;
          if ((globalsAddress & 3U) != 0)
          {
            accessRejectionMask |= 1U << 0;
          }
          if (globalsAddress < MainImageScanBegin ||
              globalsAddress >= MainImageScanEnd)
          {
            accessRejectionMask |= 1U << 1;
          }
          const bool memoryQueried =
            (accessRejectionMask & ((1U << 0) | (1U << 1))) == 0 &&
            QueryMemory(globalsAddress, globalsMemory);
          if (!memoryQueried)
          {
            accessRejectionMask |= 1U << 2;
          }
          else
          {
            if (!IsReadWriteMemory(globalsMemory))
            {
              accessRejectionMask |= 1U << 3;
            }
            // Hooks can make private data read/write/execute. Allow that, but skip code and read-only data.
            if (globalsMemory.state != MEMSTATE_PRIVATE)
            {
              accessRejectionMask |= 1U << 4;
            }
            if (!ContainsRange(
                  globalsMemory,
                  globalsAddress,
                  2U * sizeof(u32)
                  ))
            {
              accessRejectionMask |= 1U << 5;
            }
            FOLLOWER_ARENA_TRACE_VALUE(
              "external getter mapping base",
              globalsMemory.base_addr
              );
            FOLLOWER_ARENA_TRACE_VALUE(
              "external getter mapping size",
              globalsMemory.size
              );
            FOLLOWER_ARENA_TRACE_VALUE(
              "external getter mapping perm",
              globalsMemory.perm
              );
            FOLLOWER_ARENA_TRACE_VALUE(
              "external getter mapping state",
              globalsMemory.state
              );
          }
          const bool candidateWordsReadable = IsReadable(
            reinterpret_cast<const void*>(globalsAddress),
            2U * sizeof(u32)
            );
          if (!candidateWordsReadable)
          {
            accessRejectionMask |= 1U << 6;
          }
          FOLLOWER_ARENA_TRACE_VALUE(
            "external getter candidate access rejection",
            accessRejectionMask
            );
          if (candidateWordsReadable)
          {
            volatile u32* const globals =
              reinterpret_cast<volatile u32*>(globalsAddress);
            const u32 registeredAddress = globals[0];
            const u32 registeredSize = globals[1];
            u32 rejectionMask = 0;
            if (registeredAddress != registration.address)
            {
              rejectionMask |= 1U << 0;
            }
            if (registeredSize < 0x100000)
            {
              rejectionMask |= 1U << 1;
            }
            if ((registeredSize & 0xfffU) != 0)
            {
              rejectionMask |= 1U << 2;
            }
            if (registeredSize > registration.mappedSize)
            {
              rejectionMask |= 1U << 3;
            }
            else if (registration.mappedSize - registeredSize >
                     MaximumUnregisteredDeviceTail)
            {
              rejectionMask |= 1U << 4;
            }
            FOLLOWER_ARENA_TRACE_VALUE(
              "external getter candidate base",
              registeredAddress
              );
            FOLLOWER_ARENA_TRACE_VALUE(
              "external getter candidate size",
              registeredSize
              );
            FOLLOWER_ARENA_TRACE_VALUE(
              "external getter candidate rejection",
              rejectionMask
              );
            // The mapping may extend past the registered range because of later allocations.
            if (accessRejectionMask == 0 && rejectionMask == 0)
            {
              ++validatedMatches;
              volatile u32* const sizeWord = globals + 1;
              if (!selectedSizeWord)
              {
                selectedSizeWord = sizeWord;
                selectedGetterAddress = scan;
                registration.registeredSize = registeredSize;
              }
              else if (selectedSizeWord != sizeWord)
              {
                ambiguous = true;
              }
            }
          }
          else
          {
            FOLLOWER_ARENA_TRACE_VALUE(
              "external getter candidate rejection",
              0xffffffffU
              );
          }
        }
        scan += sizeof(u32);
      }
    }
    cursor = blockEnd;
  }

  registration.sizeWord = selectedSizeWord;
  registration.getterAddress = selectedGetterAddress;
  if (!selectedSizeWord || ambiguous)
  {
    return false;
  }
  return true;
}

// The getter differs between game versions. Require exactly one matching base/size pair in writable private data.
bool FindDeviceMemorySizeWordByValuePair(
  DeviceMemoryRegistration& registration,
  u32& pairMatches
  )
{
  pairMatches = 0;
  volatile u32* selectedSizeWord = NULL;
  u32 selectedRegisteredSize = 0;

  u32 cursor = MainImageScanBegin;
  while (cursor < MainImageScanEnd)
  {
    MemInfo memory;
    if (!QueryMemory(cursor, memory))
    {
      return false;
    }
    const u64 blockEnd64 = static_cast<u64>(memory.base_addr) + memory.size;
    if (blockEnd64 <= cursor)
    {
      return false;
    }
    const u32 blockEnd = blockEnd64 > MainImageScanEnd
      ? static_cast<u32>(MainImageScanEnd)
      : static_cast<u32>(blockEnd64);
    if (memory.state == MEMSTATE_PRIVATE &&
        IsReadWriteMemory(memory) && blockEnd > cursor)
    {
      u32 scan = memory.base_addr > cursor ? memory.base_addr : cursor;
      scan = (scan + 3U) & ~3U;
      while (scan < blockEnd &&
             blockEnd - scan >= 2U * sizeof(u32))
      {
        volatile u32* const words =
          reinterpret_cast<volatile u32*>(scan);
        if (words[0] == registration.address)
        {
          const u32 registeredSize = words[1];
          if (registeredSize >= 0x100000 &&
              (registeredSize & 0xfffU) == 0 &&
              registeredSize <= registration.mappedSize &&
              registration.mappedSize - registeredSize <=
                MaximumUnregisteredDeviceTail)
          {
            ++pairMatches;
            selectedSizeWord = words + 1;
            selectedRegisteredSize = registeredSize;
            FOLLOWER_ARENA_TRACE_VALUE(
              "external dress value-pair globals",
              scan
              );
            FOLLOWER_ARENA_TRACE_VALUE(
              "external dress value-pair size",
              registeredSize
              );
          }
        }
        scan += sizeof(u32);
      }
    }
    cursor = blockEnd;
  }

  if (pairMatches != 1 || !selectedSizeWord)
  {
    return false;
  }
  registration.sizeWord = selectedSizeWord;
  registration.getterAddress = 0;
  registration.registeredSize = selectedRegisteredSize;
  return true;
}

bool SetDeviceMemoryRegistrationSize(
  volatile u32* sizeWord,
  u32 expectedSize,
  u32 newSize
  )
{
  if (!sizeWord || *sizeWord != expectedSize)
  {
    return false;
  }
  *sizeWord = newSize;
  __dsb();
  return *sizeWord == newSize;
}

bool RestoreDeviceMemoryRegistration(
  volatile u32* sizeWord,
  u32 originalSize,
  u32 extendedSize,
  bool registered
  )
{
  if (!registered)
  {
    return true;
  }
  if (!sizeWord)
  {
    return false;
  }
  if (*sizeWord == originalSize)
  {
    return true;
  }
  return SetDeviceMemoryRegistrationSize(
    sizeWord,
    extendedSize,
    originalSize
    );
}


#undef FOLLOWER_ARENA_TRACE_VALUE
struct ArenaBackend {
  DeviceMemoryRegistration registration;
  u32 allocated=0;
  Result allocationResult=0;
  bool allocationQuarantined=false;
  bool AllocationQuarantined() const { return allocationQuarantined; }
  bool Restore(unsigned int original, unsigned int extended) {
    return RestoreDeviceMemoryRegistration(registration.sizeWord,original,extended,true);
  }
  bool Allocate(unsigned int address, unsigned int size) {
    allocationResult=svcControlMemory(&allocated,address,0,size,MEMOP_ALLOC_LINEAR,MEMPERM_READWRITE);
    if (R_FAILED(allocationResult)) return false;
    if (allocated==address) return true;
    if (allocated) allocationQuarantined=R_FAILED(FreeApplicationFcram(reinterpret_cast<void*>(allocated),size));
    return false;
  }
  bool Register(unsigned int expected, unsigned int size) {
    return SetDeviceMemoryRegistrationSize(registration.sizeWord,expected,size);
  }
  void* CreateHeap(unsigned int address, unsigned int size) {
    typedef void* (*CreateHeapBuffer)(void*,int,int,int,const char*);
    return reinterpret_cast<CreateHeapBuffer>(0x001049a8U)(
      reinterpret_cast<void*>(address),-2,size,0,"FollowerPrivateFcram");
  }
  bool Free(unsigned int address, unsigned int size) {
    return R_SUCCEEDED(FreeApplicationFcram(reinterpret_cast<void*>(address),size));
  }
};
void Report(const char* status, u32 freeBefore, u32 address=0, u32 mapped=0, u32 registered=0, u32 result=0)
{
  arenaStatus=status;
#if FOLLOWER_3GX_DIAGNOSTIC
  char line[320];
  const int length=std::snprintf(line,sizeof(line),
    "[FollowerArena] %s platform=%s capacity=%u size=%u freeBefore=%u freeAfter=%u address=%08lx mapped=%u registered=%u result=%08lx\n",
    status,CTRPluginFramework::System::IsCitra() ? "emulator" :
      CTRPluginFramework::System::IsNew3DS() ? "new3ds" : "old3ds",
    arenaCapacity,arenaSize,static_cast<unsigned int>(freeBefore),static_cast<unsigned int>(osGetMemRegionFree(MEMREGION_APPLICATION)),
    static_cast<unsigned long>(address),static_cast<unsigned int>(mapped),static_cast<unsigned int>(registered),static_cast<unsigned long>(result));
  if (length>0) svcOutputDebugString(line,length<static_cast<int>(sizeof(line)) ? length : sizeof(line)-1);
#else
  (void)freeBefore;
  (void)address;
  (void)mapped;
  (void)registered;
  (void)result;
#endif
}
}
bool EnsureFollowerArena()
{
  if (arenaHeap) return true;
  if (quarantined) return false;
  const u32 freeBefore=osGetMemRegionFree(MEMREGION_APPLICATION);
  unsigned int capacity=GetPartyFollowerSettings().count;
  while (capacity>1 && !CanReserveFollowerArena(freeBefore,FollowerArenaSizeForCount(capacity))) --capacity;
  arenaSize=FollowerArenaSizeForCount(capacity);
  if (!CanReserveFollowerArena(freeBefore,arenaSize)) { Report("Insufficient free application RAM",freeBefore); return false; }
  if (!CTRPluginFramework::Process::CheckAddress(0x001049a8U,MEMPERM_READ|MEMPERM_EXECUTE) ||
      !CTRPluginFramework::Process::CheckAddress(0x001049acU,MEMPERM_READ|MEMPERM_EXECUTE) ||
      *reinterpret_cast<volatile u32*>(0x001049a8U)!=0xe92d4010U ||
      *reinterpret_cast<volatile u32*>(0x001049acU)!=0xe24dd008U) {
    Report("Heap wrapper guard failed",freeBefore); return false;
  }
  ArenaBackend backend;
  if (!FindDeviceMemoryRange(backend.registration,arenaSize)) {
    Report("No contiguous device memory range",freeBefore); return false;
  }
  DeviceMemoryRegistration getter=backend.registration, pair=backend.registration;
  u32 patterns=0, validated=0, pairs=0;
  const bool getterFound=FindDeviceMemorySizeWord(getter,patterns,validated);
  const bool pairFound=FindDeviceMemorySizeWordByValuePair(pair,pairs);
  if ((!getterFound && !pairFound) ||
      (getterFound && pairFound && getter.sizeWord!=pair.sizeWord)) {
    Report("Device registration guard failed",freeBefore); return false;
  }
  backend.registration=getterFound ? getter : pair;
  const u32 address=backend.registration.address+backend.registration.mappedSize;
  const u32 extended=backend.registration.mappedSize+arenaSize;
  const auto created=CreateRegisteredFollowerArena(backend,address,
    backend.registration.registeredSize,extended,arenaSize);
  arenaHeap=created.heap;
  arenaCapacity=arenaHeap ? capacity : 0;
  quarantined=created.quarantined;
  Report(created.status,freeBefore,address,backend.registration.mappedSize,
    backend.registration.registeredSize,static_cast<u32>(backend.allocationResult));
  return arenaHeap!=NULL;
}
const char* GetFollowerArenaStatus() { return arenaStatus; }
unsigned int GetFollowerArenaCapacity() { return arenaCapacity; }
}
extern "C" void* Follower3gx_GetDedicatedArenaHeap()
{ return Gen7Follower3gx::arenaHeap; }
