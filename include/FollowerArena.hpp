#pragma once
namespace Gen7Follower3gx {
bool EnsureFollowerArena();
const char* GetFollowerArenaStatus();
unsigned int GetFollowerArenaCapacity();
}
extern "C" void* Follower3gx_GetDedicatedArenaHeap();
