#pragma once

#include "GameProfile.hpp"

namespace Gen7Follower3gx
{

bool InitializeFollowerController(GameFamily family);
bool ShutdownFollowerController();
u32 FollowerPreUpdate(void* fieldmap);
void FollowerPostUpdate(void* fieldmap);
void FollowerAfterEventCheck(void* fieldmap);
u32 FollowerHostTerminate(void* fieldmap);
void FollowerSuspendForWaterRide();
bool IsFollowerControllerInitialized();

} // namespace Gen7Follower3gx

extern "C" u32 Follower3gx_PreUpdate(void* fieldmap);
extern "C" void Follower3gx_PostUpdate(void* fieldmap);
extern "C" u32 Follower3gx_HostTerminate(void* fieldmap);
