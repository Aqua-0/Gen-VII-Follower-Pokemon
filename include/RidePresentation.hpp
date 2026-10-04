#pragma once
namespace Gen7Follower3gx {
struct GameProfile;
bool InstallRidePresentationHooks(const GameProfile& profile);
void RemoveRidePresentationHooks();
void BeginRideShadowPass();
void EndRideShadowPass();
}
#if FOLLOWER_CARRIER_DEDICATED_ARENA
extern "C" void Follower3gx_SetRidePresentation(void* playerWork, void* playerModel,
  void* riderModel, bool mounted, float speed);
extern "C" void Follower3gx_ClearRidePresentation();
#else
inline void Follower3gx_SetRidePresentation(void*,void*,void*,bool,float) {}
inline void Follower3gx_ClearRidePresentation() {}
#endif
