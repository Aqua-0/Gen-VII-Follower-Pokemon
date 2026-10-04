#pragma once

#if FOLLOWER_3GX_DIAGNOSTIC
extern "C" void Follower3gx_TraceRide(const char* message);
#else
inline void Follower3gx_TraceRide(const char*) {}
#endif

extern "C" void Follower3gx_NotifyRide(const char* message);
extern "C" void* Follower3gx_LoadUltraRidePack(unsigned int characterId, bool lunala);
extern "C" void Follower3gx_FreeUltraRidePack(void* data);

extern "C" void Follower3gx_NotifyEventCleanup(unsigned int reasons,
  unsigned int followerState, unsigned int heapSource, const void* eventVtable);

struct RideEventMemorySnapshot {
  unsigned int species, mounted, modelBorrowsEventHeap;
  unsigned int modelSize, modelFree, systemSize, systemFree;
  unsigned int independentSource, independentLargest, fullFollowerRequired, eventLargest;
};
extern "C" void Follower3gx_LogEventMemory(const RideEventMemorySnapshot* snapshot);
