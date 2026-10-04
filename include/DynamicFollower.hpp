#pragma once

namespace Gen7Follower3gx
{

void InitializeDynamicFollower();
void ShutdownDynamicFollower();
void ResetDynamicFollowerForField();
void SetDynamicFollowerFieldReady(bool ready);
bool ShouldFollowerRun();
bool IsDynamicFollowerSuppressed();
unsigned int GetDynamicFollowerFpsTenths();

} // namespace Gen7Follower3gx
