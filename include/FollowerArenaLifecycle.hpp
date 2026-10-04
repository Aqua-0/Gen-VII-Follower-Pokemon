#pragma once
namespace Gen7Follower3gx {
const unsigned int FollowerArenaSize=6U*1024U*1024U;
const unsigned int FollowerArenaReserve=8U*1024U*1024U;
inline unsigned int FollowerArenaSizeForCount(unsigned int count)
{ return FollowerArenaSize+(count-1U)*4U*1024U*1024U; }
inline bool CanReserveFollowerArena(unsigned int freeBytes, unsigned int size=FollowerArenaSize)
{ return freeBytes>=FollowerArenaReserve && size<=freeBytes-FollowerArenaReserve; }
struct FollowerArenaResult { void* heap; bool quarantined; const char* status; };
template<class Backend>
FollowerArenaResult CreateRegisteredFollowerArena(Backend& backend,unsigned int address,
  unsigned int originalSize,unsigned int extendedSize,unsigned int size=FollowerArenaSize)
{
  if (!backend.Allocate(address,size))
    return {nullptr,backend.AllocationQuarantined(),"Linear allocation failed"};
  if (!backend.Register(originalSize,extendedSize)) {
    // Registration can change even if the call fails.
    // Put the old size back before freeing the extra memory.
    const bool restored=backend.Restore(originalSize,extendedSize);
    const bool freed=restored && backend.Free(address,size);
    return {nullptr,!freed,"Device registration failed"};
  }
  void* heap=backend.CreateHeap(address,size);
  if (!heap) {
    const bool restored=backend.Restore(originalSize,extendedSize);
    const bool freed=restored && backend.Free(address,size);
    return {nullptr,!freed,"Retail heap creation failed"};
  }
  // Keep the arena registered until the game exits. Child heaps can still be freed normally.
  return {heap,false,"Active"};
}
}
