#pragma once

namespace Gen7Follower3gx
{

enum FollowerOutlineMode
{
  FOLLOWER_OUTLINE_ID_SOFT,
  FOLLOWER_OUTLINE_ID_MEDIUM,
  FOLLOWER_OUTLINE_ID_ORIGINAL,
  FOLLOWER_OUTLINE_OFF,
  FOLLOWER_OUTLINE_MODE_COUNT,
};

enum FollowerDrawDistanceMode
{
  FOLLOWER_DRAW_DISTANCE_FULL,
  FOLLOWER_DRAW_DISTANCE_HIGH,
  FOLLOWER_DRAW_DISTANCE_MEDIUM,
  FOLLOWER_DRAW_DISTANCE_LOW,
  FOLLOWER_DRAW_DISTANCE_MODE_COUNT,
};

enum FollowerTerrainDetailMode
{
  FOLLOWER_TERRAIN_DETAIL_STOCK,
  FOLLOWER_TERRAIN_DETAIL_MEDIUM,
  FOLLOWER_TERRAIN_DETAIL_MODE_COUNT,
};

enum FollowerCollisionQualityMode
{
  FOLLOWER_COLLISION_QUALITY_FULL,
  FOLLOWER_COLLISION_QUALITY_BALANCED,
  FOLLOWER_COLLISION_QUALITY_FAST,
  FOLLOWER_COLLISION_QUALITY_MODE_COUNT,
};

enum FollowerAnimationRateMode
{
  FOLLOWER_ANIMATION_RATE_30_FPS,
  FOLLOWER_ANIMATION_RATE_15_FPS,
  FOLLOWER_ANIMATION_RATE_15_FPS_INTERPOLATED,
  FOLLOWER_ANIMATION_RATE_10_FPS_INTERPOLATED,
  FOLLOWER_ANIMATION_RATE_7_5_FPS_INTERPOLATED,
  FOLLOWER_ANIMATION_RATE_5_FPS_INTERPOLATED,
  FOLLOWER_ANIMATION_RATE_MODE_COUNT,
};

enum FollowerBehaviorSetting
{
  FOLLOWER_BEHAVIOR_TRAIL_DELAY,
  FOLLOWER_BEHAVIOR_STOP_DISTANCE,
  FOLLOWER_BEHAVIOR_START_DISTANCE,
  FOLLOWER_BEHAVIOR_RUN_DISTANCE,
  FOLLOWER_BEHAVIOR_WALK_SPEED,
  FOLLOWER_BEHAVIOR_RUN_SPEED,
  FOLLOWER_BEHAVIOR_CATCH_UP_STEP,
  FOLLOWER_BEHAVIOR_WARP_DISTANCE,
  FOLLOWER_BEHAVIOR_SETTING_COUNT,
};

bool InitializeFollowerSettings();
void ShutdownFollowerSettings();
bool IsFollowerEnabled();
void SetFollowerEnabled(bool enabled);
bool IsDynamicFollowerEnabled();
void SetDynamicFollowerEnabled(bool enabled);
bool IsFollowerFormChangeEnabled();
void SetFollowerFormChangeEnabled(bool enabled);
bool IsFollowerSpecialEffectsEnabled();
void SetFollowerSpecialEffectsEnabled(bool enabled);
unsigned int GetFollowerPokeBallSoundVolumePercent();
bool SetFollowerPokeBallSoundVolumePercent(unsigned int volumePercent);
FollowerOutlineMode GetFollowerOutlineMode();
void SetFollowerOutlineMode(FollowerOutlineMode mode);
const char* GetFollowerOutlineModeName(FollowerOutlineMode mode);
FollowerDrawDistanceMode GetFollowerDrawDistanceMode();
void SetFollowerDrawDistanceMode(FollowerDrawDistanceMode mode);
const char* GetFollowerDrawDistanceModeName(FollowerDrawDistanceMode mode);
float GetFollowerDrawDistanceScale(FollowerDrawDistanceMode mode);
FollowerTerrainDetailMode GetFollowerTerrainDetailMode();
void SetFollowerTerrainDetailMode(FollowerTerrainDetailMode mode);
const char* GetFollowerTerrainDetailModeName(FollowerTerrainDetailMode mode);
FollowerCollisionQualityMode GetFollowerCollisionQualityMode();
void SetFollowerCollisionQualityMode(FollowerCollisionQualityMode mode);
const char* GetFollowerCollisionQualityModeName(
  FollowerCollisionQualityMode mode
  );
FollowerAnimationRateMode GetFollowerAnimationRateMode();
void SetFollowerAnimationRateMode(FollowerAnimationRateMode mode);
const char* GetFollowerAnimationRateModeName(FollowerAnimationRateMode mode);

float GetFollowerBehaviorValue(FollowerBehaviorSetting setting);
float GetFollowerBehaviorMinimum(FollowerBehaviorSetting setting);
float GetFollowerBehaviorMaximum(FollowerBehaviorSetting setting);
bool IsFollowerBehaviorValueValid(
  FollowerBehaviorSetting setting,
  float value
  );
bool SetFollowerBehaviorValue(
  FollowerBehaviorSetting setting,
  float value
  );
void ResetFollowerBehaviorDefaults();
bool IsFollowerAnimationPlaybackUncapped();
void SetFollowerAnimationPlaybackUncapped(bool uncapped);
unsigned int GetSavedPerformanceOptions();
void SetSavedPerformanceOptions(unsigned int options);

unsigned int GetFollowerTrailDelayFrames();
float GetFollowerStopDistance();
float GetFollowerStartDistance();
float GetFollowerRunDistance();
float GetFollowerRunExitDistance();
float GetFollowerWalkSpeed();
float GetFollowerRunSpeed();
float GetFollowerCatchUpStep();
float GetFollowerWarpDistance();

} // namespace Gen7Follower3gx
