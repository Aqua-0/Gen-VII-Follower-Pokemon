#pragma once

namespace Gen7Follower3gx
{

void FollowGameCamera();
bool IsFollowingGameCamera();
void SetFreeCameraParked(bool parked);
bool IsFreeCameraParked();
void RequestFreeCameraReset();
void SetFreeCameraSpeed(unsigned int preset);
unsigned int GetFreeCameraSpeed();
void InitializeFreeCamera();
void ShutdownFreeCamera();
void SetFreeCameraEnabled(bool enabled);
bool IsFreeCameraEnabled();
bool IsFreeCameraActive();
void PrepareFreeCamera(void* fieldmap, bool freeField);
void UpdateFreeCamera(void* fieldmap);
void TerminateFreeCameraField(void* fieldmap);

} // namespace Gen7Follower3gx
