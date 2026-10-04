#include <3ds.h>

#include <CTRPluginFramework/System/FwkSettings.hpp>

extern "C" Result Follower3gx_MapProcessMemoryExV1(
  Handle destinationProcess,
  u32 destinationAddress,
  Handle sourceProcess,
  u32 sourceAddress,
  u32 size
);

namespace CTRPluginFramework
{
namespace
{

struct ScreensFramebuffers
{
  u8 topFramebuffer0[400 * 240 * 2];
  u8 topFramebuffer1[400 * 240 * 2];
  u8 bottomFramebuffer0[320 * 240 * 2];
  u8 bottomFramebuffer1[320 * 240 * 2];
} PACKED;

Result MapHookMemory(u32 sourceAddress)
{
  Result result = Follower3gx_MapProcessMemoryExV1(
    CUR_PROCESS_HANDLE,
    0x01e80000,
    CUR_PROCESS_HANDLE,
    sourceAddress,
    0x2000
    );
  if (R_SUCCEEDED(result))
  {
    return result;
  }

  u32 processId = 0;
  Handle processHandle = 0;
  if (R_FAILED(svcGetProcessId(&processId, CUR_PROCESS_HANDLE)) ||
      R_FAILED(svcOpenProcess(&processHandle, processId)))
  {
    return result;
  }

  result = Follower3gx_MapProcessMemoryExV1(
    processHandle,
    0x01e80000,
    processHandle,
    sourceAddress,
    0x2000
    );
  svcCloseHandle(processHandle);
  return result;
}

void FailHeapBootstrap(Result result)
{
  *reinterpret_cast<volatile u32*>(0xdeadc0de) = result;
  for (;;)
  {
  }
}

} // namespace

extern "C" char* fake_heap_start;
extern "C" char* fake_heap_end;
extern "C" u32 __ctru_heap;
extern "C" u32 __ctru_heap_size;

extern "C" void __system_allocateHeaps()
{
  const u32 framebufferSize =
    (sizeof(ScreensFramebuffers) + 0x1000) & ~0xfffU;

  __ctru_heap = FwkSettings::Header->heapVA + framebufferSize;
  __ctru_heap_size =
    FwkSettings::Header->heapSize - 0x2000 - framebufferSize;

  mappableInit(0x11000000, 0x14000000);

  const Result result = MapHookMemory(__ctru_heap + __ctru_heap_size);
  if (R_FAILED(result))
  {
    FailHeapBootstrap(result);
  }

  fake_heap_start = reinterpret_cast<char*>(__ctru_heap);
  fake_heap_end = fake_heap_start + __ctru_heap_size;
}

} // namespace CTRPluginFramework
