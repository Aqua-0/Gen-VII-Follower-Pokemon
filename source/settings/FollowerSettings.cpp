#include "FollowMode.hpp"
#include "FollowerSettings.hpp"
#include "PerformanceDiagnostics.hpp"

#include <3ds.h>
#include <CTRPluginFramework/System/File.hpp>

namespace Gen7Follower3gx
{
namespace
{

const char SETTINGS_PATH[] = "Gen7FieldFollower.cfg";
const u32 SETTINGS_MAGIC = 0x53463747;
const u32 SETTINGS_VERSION = 10;
const u32 SETTINGS_CHECK_VALUE = 0x7f3a91c5;
const u32 DECIMAL_SCALE = 10;
const u32 DEFAULT_POKE_BALL_SOUND_VOLUME_PERCENT = 100;
const u32 MAX_POKE_BALL_SOUND_VOLUME_PERCENT = 100;

const u32 DEFAULT_TRAIL_DELAY = 6;
const u32 DEFAULT_STOP_DISTANCE = 350;
const u32 DEFAULT_START_DISTANCE = 600;
const u32 DEFAULT_RUN_DISTANCE = 2600;
const u32 DEFAULT_WALK_SPEED = 140;
const u32 DEFAULT_RUN_SPEED = 300;
const u32 DEFAULT_CATCH_UP_STEP = 15;
const u32 DEFAULT_WARP_DISTANCE = 9000;

const u32 MIN_TRAIL_DELAY = 0;
const u32 MAX_TRAIL_DELAY = 60;
const u32 MIN_STOP_DISTANCE = 0;
const u32 MAX_STOP_DISTANCE = 1500;
const u32 MIN_START_DISTANCE = 50;
const u32 MAX_START_DISTANCE = 3000;
const u32 MIN_RUN_DISTANCE = 800;
const u32 MAX_RUN_DISTANCE = 7000;
const u32 MIN_WALK_SPEED = 20;
const u32 MAX_WALK_SPEED = 600;
const u32 MIN_RUN_SPEED = 50;
const u32 MAX_RUN_SPEED = 1200;
const u32 MIN_CATCH_UP_STEP = 0;
const u32 MAX_CATCH_UP_STEP = 80;
const u32 MIN_WARP_DISTANCE = 3000;
const u32 MAX_WARP_DISTANCE = 20000;
const u32 MIN_WARP_MARGIN = 500;

struct SettingsRecordV1
{
  u32 magic;
  u32 version;
  u32 size;
  u32 outlineMode;
  u32 check;
};

struct BehaviorState
{
  u32 trailDelay;
  u32 stopDistance;
  u32 startDistance;
  u32 runDistance;
  u32 walkSpeed;
  u32 runSpeed;
  u32 catchUpStep;
  u32 warpDistance;
};

struct SettingsRecordV2
{
  u32 magic;
  u32 version;
  u32 size;
  u32 outlineMode;
  u32 trailDelay;
  u32 stopDistance;
  u32 startDistance;
  u32 runDistance;
  u32 walkSpeed;
  u32 runSpeed;
  u32 catchUpStep;
  u32 warpDistance;
  u32 check;
};

struct SettingsRecordV3
{
  u32 magic;
  u32 version;
  u32 size;
  u32 outlineMode;
  u32 trailDelay;
  u32 stopDistance;
  u32 startDistance;
  u32 runDistance;
  u32 walkSpeed;
  u32 runSpeed;
  u32 catchUpStep;
  u32 warpDistance;
  u32 uncapAnimationPlayback;
  u32 check;
};

struct SettingsRecordV4
{
  u32 magic;
  u32 version;
  u32 size;
  u32 outlineMode;
  u32 trailDelay;
  u32 stopDistance;
  u32 startDistance;
  u32 runDistance;
  u32 walkSpeed;
  u32 runSpeed;
  u32 catchUpStep;
  u32 warpDistance;
  u32 uncapAnimationPlayback;
  u32 performanceOptions;
  u32 check;
};

struct SettingsRecordV5
{
  u32 magic;
  u32 version;
  u32 size;
  u32 outlineMode;
  u32 trailDelay;
  u32 stopDistance;
  u32 startDistance;
  u32 runDistance;
  u32 walkSpeed;
  u32 runSpeed;
  u32 catchUpStep;
  u32 warpDistance;
  u32 uncapAnimationPlayback;
  u32 performanceOptions;
  u32 drawDistanceMode;
  u32 check;
};

struct SettingsRecordV6
{
  u32 magic;
  u32 version;
  u32 size;
  u32 outlineMode;
  u32 trailDelay;
  u32 stopDistance;
  u32 startDistance;
  u32 runDistance;
  u32 walkSpeed;
  u32 runSpeed;
  u32 catchUpStep;
  u32 warpDistance;
  u32 uncapAnimationPlayback;
  u32 performanceOptions;
  u32 drawDistanceMode;
  u32 terrainDetailMode;
  u32 collisionQualityMode;
  u32 animationRateMode;
  u32 check;
};

struct SettingsRecordV7
{
  u32 magic;
  u32 version;
  u32 size;
  u32 outlineMode;
  u32 trailDelay;
  u32 stopDistance;
  u32 startDistance;
  u32 runDistance;
  u32 walkSpeed;
  u32 runSpeed;
  u32 catchUpStep;
  u32 warpDistance;
  u32 uncapAnimationPlayback;
  u32 performanceOptions;
  u32 drawDistanceMode;
  u32 terrainDetailMode;
  u32 collisionQualityMode;
  u32 animationRateMode;
  u32 followerEnabled;
  u32 dynamicFollowerEnabled;
  u32 check;
};

struct SettingsRecordV8
{
  u32 magic;
  u32 version;
  u32 size;
  u32 outlineMode;
  u32 trailDelay;
  u32 stopDistance;
  u32 startDistance;
  u32 runDistance;
  u32 walkSpeed;
  u32 runSpeed;
  u32 catchUpStep;
  u32 warpDistance;
  u32 uncapAnimationPlayback;
  u32 performanceOptions;
  u32 drawDistanceMode;
  u32 terrainDetailMode;
  u32 collisionQualityMode;
  u32 animationRateMode;
  u32 followerEnabled;
  u32 dynamicFollowerEnabled;
  // Keep this so version-8 settings files still have the same layout.
  u32 legacyHoverOffsetEnabled;
  u32 check;
};

struct SettingsRecordV9
{
  u32 magic;
  u32 version;
  u32 size;
  u32 outlineMode;
  u32 trailDelay;
  u32 stopDistance;
  u32 startDistance;
  u32 runDistance;
  u32 walkSpeed;
  u32 runSpeed;
  u32 catchUpStep;
  u32 warpDistance;
  u32 uncapAnimationPlayback;
  u32 performanceOptions;
  u32 drawDistanceMode;
  u32 terrainDetailMode;
  u32 collisionQualityMode;
  u32 animationRateMode;
  u32 followerEnabled;
  u32 dynamicFollowerEnabled;
  // Version 9 uses the old hover slot to store whether sound is muted.
  u32 pokeBallSoundMuted;
  u32 formChangeEnabled;
  u32 specialEffectsEnabled;
  u32 check;
};

struct SettingsRecord
{
  u32 magic;
  u32 version;
  u32 size;
  u32 outlineMode;
  u32 trailDelay;
  u32 stopDistance;
  u32 startDistance;
  u32 runDistance;
  u32 walkSpeed;
  u32 runSpeed;
  u32 catchUpStep;
  u32 warpDistance;
  u32 uncapAnimationPlayback;
  u32 performanceOptions;
  u32 drawDistanceMode;
  u32 terrainDetailMode;
  u32 collisionQualityMode;
  u32 animationRateMode;
  u32 followerEnabled;
  u32 dynamicFollowerEnabled;
  // Keep the old prefix so existing checksums still work. Store the percentage below.
  u32 legacyPokeBallSoundMuted;
  u32 formChangeEnabled;
  u32 specialEffectsEnabled;
  u32 pokeBallSoundVolumePercent;
  u32 check;
};

u32 g_OutlineMode = FOLLOWER_OUTLINE_ID_SOFT;
u32 g_TrailDelay = DEFAULT_TRAIL_DELAY;
u32 g_StopDistance = DEFAULT_STOP_DISTANCE;
u32 g_StartDistance = DEFAULT_START_DISTANCE;
u32 g_RunDistance = DEFAULT_RUN_DISTANCE;
u32 g_WalkSpeed = DEFAULT_WALK_SPEED;
u32 g_RunSpeed = DEFAULT_RUN_SPEED;
u32 g_CatchUpStep = DEFAULT_CATCH_UP_STEP;
u32 g_WarpDistance = DEFAULT_WARP_DISTANCE;
u32 g_UncapAnimationPlayback = 0;
u32 g_SavedPerformanceOptions = 0;
u32 g_DrawDistanceMode = FOLLOWER_DRAW_DISTANCE_FULL;
u32 g_TerrainDetailMode = FOLLOWER_TERRAIN_DETAIL_STOCK;
u32 g_CollisionQualityMode = FOLLOWER_COLLISION_QUALITY_FULL;
u32 g_AnimationRateMode = FOLLOWER_ANIMATION_RATE_30_FPS;
u32 g_FollowerEnabled = 1;
u32 g_DynamicFollowerEnabled = 0;
u32 g_FormChangeEnabled = 1;
u32 g_SpecialEffectsEnabled = 1;
u32 g_PokeBallSoundVolumePercent =
  DEFAULT_POKE_BALL_SOUND_VOLUME_PERCENT;
bool g_Initialized = false;
bool g_SettingsLoadedFromDisk = false;

u32 MixCheck(u32 check, u32 value)
{
  return (check ^ value) * 16777619u;
}

u32 CalculateCheck(const SettingsRecordV1& record)
{
  return record.magic ^ record.version ^ record.size ^ record.outlineMode ^
    SETTINGS_CHECK_VALUE;
}

u32 CalculateCheck(const SettingsRecordV2& record)
{
  u32 check = SETTINGS_CHECK_VALUE;
  check = MixCheck(check, record.magic);
  check = MixCheck(check, record.version);
  check = MixCheck(check, record.size);
  check = MixCheck(check, record.outlineMode);
  check = MixCheck(check, record.trailDelay);
  check = MixCheck(check, record.stopDistance);
  check = MixCheck(check, record.startDistance);
  check = MixCheck(check, record.runDistance);
  check = MixCheck(check, record.walkSpeed);
  check = MixCheck(check, record.runSpeed);
  check = MixCheck(check, record.catchUpStep);
  return MixCheck(check, record.warpDistance);
}

u32 CalculateCheck(const SettingsRecordV3& record)
{
  SettingsRecordV2 previous;
  previous.magic = record.magic;
  previous.version = record.version;
  previous.size = record.size;
  previous.outlineMode = record.outlineMode;
  previous.trailDelay = record.trailDelay;
  previous.stopDistance = record.stopDistance;
  previous.startDistance = record.startDistance;
  previous.runDistance = record.runDistance;
  previous.walkSpeed = record.walkSpeed;
  previous.runSpeed = record.runSpeed;
  previous.catchUpStep = record.catchUpStep;
  previous.warpDistance = record.warpDistance;
  return MixCheck(
    CalculateCheck(previous),
    record.uncapAnimationPlayback
    );
}

u32 CalculateCheck(const SettingsRecordV4& record)
{
  SettingsRecordV3 previous;
  previous.magic = record.magic;
  previous.version = record.version;
  previous.size = record.size;
  previous.outlineMode = record.outlineMode;
  previous.trailDelay = record.trailDelay;
  previous.stopDistance = record.stopDistance;
  previous.startDistance = record.startDistance;
  previous.runDistance = record.runDistance;
  previous.walkSpeed = record.walkSpeed;
  previous.runSpeed = record.runSpeed;
  previous.catchUpStep = record.catchUpStep;
  previous.warpDistance = record.warpDistance;
  previous.uncapAnimationPlayback = record.uncapAnimationPlayback;
  return MixCheck(
    CalculateCheck(previous),
    record.performanceOptions
    );
}

u32 CalculateCheck(const SettingsRecordV5& record)
{
  SettingsRecordV4 previous;
  previous.magic = record.magic;
  previous.version = record.version;
  previous.size = record.size;
  previous.outlineMode = record.outlineMode;
  previous.trailDelay = record.trailDelay;
  previous.stopDistance = record.stopDistance;
  previous.startDistance = record.startDistance;
  previous.runDistance = record.runDistance;
  previous.walkSpeed = record.walkSpeed;
  previous.runSpeed = record.runSpeed;
  previous.catchUpStep = record.catchUpStep;
  previous.warpDistance = record.warpDistance;
  previous.uncapAnimationPlayback = record.uncapAnimationPlayback;
  previous.performanceOptions = record.performanceOptions;
  return MixCheck(
    CalculateCheck(previous),
    record.drawDistanceMode
    );
}

u32 CalculateCheck(const SettingsRecordV6& record)
{
  SettingsRecordV5 previous;
  previous.magic = record.magic;
  previous.version = record.version;
  previous.size = record.size;
  previous.outlineMode = record.outlineMode;
  previous.trailDelay = record.trailDelay;
  previous.stopDistance = record.stopDistance;
  previous.startDistance = record.startDistance;
  previous.runDistance = record.runDistance;
  previous.walkSpeed = record.walkSpeed;
  previous.runSpeed = record.runSpeed;
  previous.catchUpStep = record.catchUpStep;
  previous.warpDistance = record.warpDistance;
  previous.uncapAnimationPlayback = record.uncapAnimationPlayback;
  previous.performanceOptions = record.performanceOptions;
  previous.drawDistanceMode = record.drawDistanceMode;
  u32 check = CalculateCheck(previous);
  check = MixCheck(check, record.terrainDetailMode);
  check = MixCheck(check, record.collisionQualityMode);
  return MixCheck(check, record.animationRateMode);
}

u32 CalculateCheck(const SettingsRecordV7& record)
{
  SettingsRecordV6 previous;
  previous.magic = record.magic;
  previous.version = record.version;
  previous.size = record.size;
  previous.outlineMode = record.outlineMode;
  previous.trailDelay = record.trailDelay;
  previous.stopDistance = record.stopDistance;
  previous.startDistance = record.startDistance;
  previous.runDistance = record.runDistance;
  previous.walkSpeed = record.walkSpeed;
  previous.runSpeed = record.runSpeed;
  previous.catchUpStep = record.catchUpStep;
  previous.warpDistance = record.warpDistance;
  previous.uncapAnimationPlayback = record.uncapAnimationPlayback;
  previous.performanceOptions = record.performanceOptions;
  previous.drawDistanceMode = record.drawDistanceMode;
  previous.terrainDetailMode = record.terrainDetailMode;
  previous.collisionQualityMode = record.collisionQualityMode;
  previous.animationRateMode = record.animationRateMode;
  u32 check = CalculateCheck(previous);
  check = MixCheck(check, record.followerEnabled);
  return MixCheck(check, record.dynamicFollowerEnabled);
}

u32 CalculateCheck(const SettingsRecordV8& record)
{
  SettingsRecordV7 previous;
  previous.magic = record.magic;
  previous.version = record.version;
  previous.size = record.size;
  previous.outlineMode = record.outlineMode;
  previous.trailDelay = record.trailDelay;
  previous.stopDistance = record.stopDistance;
  previous.startDistance = record.startDistance;
  previous.runDistance = record.runDistance;
  previous.walkSpeed = record.walkSpeed;
  previous.runSpeed = record.runSpeed;
  previous.catchUpStep = record.catchUpStep;
  previous.warpDistance = record.warpDistance;
  previous.uncapAnimationPlayback = record.uncapAnimationPlayback;
  previous.performanceOptions = record.performanceOptions;
  previous.drawDistanceMode = record.drawDistanceMode;
  previous.terrainDetailMode = record.terrainDetailMode;
  previous.collisionQualityMode = record.collisionQualityMode;
  previous.animationRateMode = record.animationRateMode;
  previous.followerEnabled = record.followerEnabled;
  previous.dynamicFollowerEnabled = record.dynamicFollowerEnabled;
  return MixCheck(
    CalculateCheck(previous),
    record.legacyHoverOffsetEnabled
    );
}

u32 CalculateCheck(const SettingsRecordV9& record)
{
  SettingsRecordV8 previous;
  previous.magic = record.magic;
  previous.version = record.version;
  previous.size = record.size;
  previous.outlineMode = record.outlineMode;
  previous.trailDelay = record.trailDelay;
  previous.stopDistance = record.stopDistance;
  previous.startDistance = record.startDistance;
  previous.runDistance = record.runDistance;
  previous.walkSpeed = record.walkSpeed;
  previous.runSpeed = record.runSpeed;
  previous.catchUpStep = record.catchUpStep;
  previous.warpDistance = record.warpDistance;
  previous.uncapAnimationPlayback = record.uncapAnimationPlayback;
  previous.performanceOptions = record.performanceOptions;
  previous.drawDistanceMode = record.drawDistanceMode;
  previous.terrainDetailMode = record.terrainDetailMode;
  previous.collisionQualityMode = record.collisionQualityMode;
  previous.animationRateMode = record.animationRateMode;
  previous.followerEnabled = record.followerEnabled;
  previous.dynamicFollowerEnabled = record.dynamicFollowerEnabled;
  previous.legacyHoverOffsetEnabled = record.pokeBallSoundMuted;
  u32 check = CalculateCheck(previous);
  check = MixCheck(check, record.formChangeEnabled);
  return MixCheck(check, record.specialEffectsEnabled);
}

u32 CalculateCheck(const SettingsRecord& record)
{
  SettingsRecordV9 previous;
  previous.magic = record.magic;
  previous.version = record.version;
  previous.size = record.size;
  previous.outlineMode = record.outlineMode;
  previous.trailDelay = record.trailDelay;
  previous.stopDistance = record.stopDistance;
  previous.startDistance = record.startDistance;
  previous.runDistance = record.runDistance;
  previous.walkSpeed = record.walkSpeed;
  previous.runSpeed = record.runSpeed;
  previous.catchUpStep = record.catchUpStep;
  previous.warpDistance = record.warpDistance;
  previous.uncapAnimationPlayback = record.uncapAnimationPlayback;
  previous.performanceOptions = record.performanceOptions;
  previous.drawDistanceMode = record.drawDistanceMode;
  previous.terrainDetailMode = record.terrainDetailMode;
  previous.collisionQualityMode = record.collisionQualityMode;
  previous.animationRateMode = record.animationRateMode;
  previous.followerEnabled = record.followerEnabled;
  previous.dynamicFollowerEnabled = record.dynamicFollowerEnabled;
  previous.pokeBallSoundMuted = record.legacyPokeBallSoundMuted;
  previous.formChangeEnabled = record.formChangeEnabled;
  previous.specialEffectsEnabled = record.specialEffectsEnabled;
  return MixCheck(
    CalculateCheck(previous),
    record.pokeBallSoundVolumePercent
    );
}

bool IsValidMode(u32 mode)
{
  return mode < static_cast<u32>(FOLLOWER_OUTLINE_MODE_COUNT);
}

bool IsValidDrawDistanceMode(u32 mode)
{
  return mode < static_cast<u32>(FOLLOWER_DRAW_DISTANCE_MODE_COUNT);
}

bool IsValidTerrainDetailMode(u32 mode)
{
  return mode < static_cast<u32>(FOLLOWER_TERRAIN_DETAIL_MODE_COUNT);
}

bool IsValidCollisionQualityMode(u32 mode)
{
  return mode < static_cast<u32>(FOLLOWER_COLLISION_QUALITY_MODE_COUNT);
}

bool IsValidAnimationRateMode(u32 mode)
{
  return mode < static_cast<u32>(FOLLOWER_ANIMATION_RATE_MODE_COUNT);
}

bool IsValidLegacyPerformanceOptions(u32 options)
{
  return (options & ~0x3ffU) == 0;
}

u32 MigrateLegacyPerformanceOptions(u32 options)
{
  u32 migrated = 0;
  if (options & (1U << 0))
  {
    migrated |= GetPerformanceOptionBit(
      PERFORMANCE_OPTION_LITE_FOLLOWER_MATERIALS
      );
  }
  for (u32 oldBit = 2; oldBit <= 8; ++oldBit)
  {
    if (options & (1U << oldBit))
    {
      migrated |= 1U << (oldBit - 1);
    }
  }
  return migrated;
}

bool IsValidPerformanceOptions(u32 options)
{
  const u32 validMask =
    (1U << static_cast<u32>(PERFORMANCE_OPTION_COUNT)) - 1U;
  return (options & ~validMask) == 0;
}

bool IsWithin(u32 value, u32 minimum, u32 maximum)
{
  return value >= minimum && value <= maximum;
}

bool IsValidBehavior(const BehaviorState& state)
{
  return
    IsWithin(state.trailDelay, MIN_TRAIL_DELAY, MAX_TRAIL_DELAY) &&
    IsWithin(state.stopDistance, MIN_STOP_DISTANCE, MAX_STOP_DISTANCE) &&
    IsWithin(state.startDistance, MIN_START_DISTANCE, MAX_START_DISTANCE) &&
    IsWithin(state.runDistance, MIN_RUN_DISTANCE, MAX_RUN_DISTANCE) &&
    IsWithin(state.walkSpeed, MIN_WALK_SPEED, MAX_WALK_SPEED) &&
    IsWithin(state.runSpeed, MIN_RUN_SPEED, MAX_RUN_SPEED) &&
    IsWithin(state.catchUpStep, MIN_CATCH_UP_STEP, MAX_CATCH_UP_STEP) &&
    IsWithin(state.warpDistance, MIN_WARP_DISTANCE, MAX_WARP_DISTANCE) &&
    state.stopDistance <= state.startDistance &&
    state.startDistance <= state.runDistance &&
    state.walkSpeed <= state.runSpeed &&
    state.runDistance + MIN_WARP_MARGIN <= state.warpDistance;
}

BehaviorState GetBehaviorState()
{
  BehaviorState state;
  state.trailDelay = __atomic_load_n(&g_TrailDelay, __ATOMIC_RELAXED);
  state.stopDistance = __atomic_load_n(&g_StopDistance, __ATOMIC_RELAXED);
  state.startDistance = __atomic_load_n(&g_StartDistance, __ATOMIC_RELAXED);
  state.runDistance = __atomic_load_n(&g_RunDistance, __ATOMIC_RELAXED);
  state.walkSpeed = __atomic_load_n(&g_WalkSpeed, __ATOMIC_RELAXED);
  state.runSpeed = __atomic_load_n(&g_RunSpeed, __ATOMIC_RELAXED);
  state.catchUpStep = __atomic_load_n(&g_CatchUpStep, __ATOMIC_RELAXED);
  state.warpDistance = __atomic_load_n(&g_WarpDistance, __ATOMIC_RELAXED);
  return state;
}

void StoreBehaviorState(const BehaviorState& state)
{
  __atomic_store_n(&g_TrailDelay, state.trailDelay, __ATOMIC_RELAXED);
  __atomic_store_n(&g_StopDistance, state.stopDistance, __ATOMIC_RELAXED);
  __atomic_store_n(&g_StartDistance, state.startDistance, __ATOMIC_RELAXED);
  __atomic_store_n(&g_RunDistance, state.runDistance, __ATOMIC_RELAXED);
  __atomic_store_n(&g_WalkSpeed, state.walkSpeed, __ATOMIC_RELAXED);
  __atomic_store_n(&g_RunSpeed, state.runSpeed, __ATOMIC_RELAXED);
  __atomic_store_n(&g_CatchUpStep, state.catchUpStep, __ATOMIC_RELAXED);
  __atomic_store_n(&g_WarpDistance, state.warpDistance, __ATOMIC_RELAXED);
}

BehaviorState GetDefaultBehavior()
{
  BehaviorState state;
  state.trailDelay = DEFAULT_TRAIL_DELAY;
  state.stopDistance = DEFAULT_STOP_DISTANCE;
  state.startDistance = DEFAULT_START_DISTANCE;
  state.runDistance = DEFAULT_RUN_DISTANCE;
  state.walkSpeed = DEFAULT_WALK_SPEED;
  state.runSpeed = DEFAULT_RUN_SPEED;
  state.catchUpStep = DEFAULT_CATCH_UP_STEP;
  state.warpDistance = DEFAULT_WARP_DISTANCE;
  return state;
}

u32 ToStoredValue(FollowerBehaviorSetting setting, float value)
{
  if (setting == FOLLOWER_BEHAVIOR_TRAIL_DELAY)
  {
    return static_cast<u32>(value);
  }
  return static_cast<u32>(value * static_cast<float>(DECIMAL_SCALE) + 0.5f);
}

float FromStoredValue(FollowerBehaviorSetting setting, u32 value)
{
  if (setting == FOLLOWER_BEHAVIOR_TRAIL_DELAY)
  {
    return static_cast<float>(value);
  }
  return static_cast<float>(value) / static_cast<float>(DECIMAL_SCALE);
}

u32* GetBehaviorField(BehaviorState* state, FollowerBehaviorSetting setting)
{
  switch (setting)
  {
  case FOLLOWER_BEHAVIOR_TRAIL_DELAY:
    return &state->trailDelay;
  case FOLLOWER_BEHAVIOR_STOP_DISTANCE:
    return &state->stopDistance;
  case FOLLOWER_BEHAVIOR_START_DISTANCE:
    return &state->startDistance;
  case FOLLOWER_BEHAVIOR_RUN_DISTANCE:
    return &state->runDistance;
  case FOLLOWER_BEHAVIOR_WALK_SPEED:
    return &state->walkSpeed;
  case FOLLOWER_BEHAVIOR_RUN_SPEED:
    return &state->runSpeed;
  case FOLLOWER_BEHAVIOR_CATCH_UP_STEP:
    return &state->catchUpStep;
  case FOLLOWER_BEHAVIOR_WARP_DISTANCE:
    return &state->warpDistance;
  default:
    return NULL;
  }
}

u32 GetBehaviorField(const BehaviorState& state, FollowerBehaviorSetting setting)
{
  BehaviorState copy = state;
  u32* const field = GetBehaviorField(&copy, setting);
  return field ? *field : 0;
}

void SaveSettings()
{
  const BehaviorState state = GetBehaviorState();
  SettingsRecord record;
  record.magic = SETTINGS_MAGIC;
  record.version = SETTINGS_VERSION;
  record.size = sizeof(record);
  record.outlineMode = __atomic_load_n(&g_OutlineMode, __ATOMIC_RELAXED);
  record.trailDelay = state.trailDelay;
  record.stopDistance = state.stopDistance;
  record.startDistance = state.startDistance;
  record.runDistance = state.runDistance;
  record.walkSpeed = state.walkSpeed;
  record.runSpeed = state.runSpeed;
  record.catchUpStep = state.catchUpStep;
  record.warpDistance = state.warpDistance;
  record.uncapAnimationPlayback = __atomic_load_n(
    &g_UncapAnimationPlayback,
    __ATOMIC_RELAXED
    );
  record.performanceOptions = __atomic_load_n(
    &g_SavedPerformanceOptions,
    __ATOMIC_RELAXED
    );
  record.drawDistanceMode = __atomic_load_n(
    &g_DrawDistanceMode,
    __ATOMIC_RELAXED
    );
  record.terrainDetailMode = __atomic_load_n(
    &g_TerrainDetailMode,
    __ATOMIC_RELAXED
    );
  record.collisionQualityMode = __atomic_load_n(
    &g_CollisionQualityMode,
    __ATOMIC_RELAXED
    );
  record.animationRateMode = __atomic_load_n(
    &g_AnimationRateMode,
    __ATOMIC_RELAXED
    );
  record.followerEnabled = __atomic_load_n(
    &g_FollowerEnabled,
    __ATOMIC_RELAXED
    );
  record.dynamicFollowerEnabled = __atomic_load_n(
    &g_DynamicFollowerEnabled,
    __ATOMIC_RELAXED
    );
  const u32 pokeBallSoundVolumePercent = __atomic_load_n(
    &g_PokeBallSoundVolumePercent,
    __ATOMIC_RELAXED
    );
  record.legacyPokeBallSoundMuted =
    pokeBallSoundVolumePercent == 0 ? 1U : 0U;
  record.formChangeEnabled = __atomic_load_n(
    &g_FormChangeEnabled,
    __ATOMIC_RELAXED
    );
  record.specialEffectsEnabled = __atomic_load_n(
    &g_SpecialEffectsEnabled,
    __ATOMIC_RELAXED
    );
  record.pokeBallSoundVolumePercent = pokeBallSoundVolumePercent;
  record.check = CalculateCheck(record);

  CTRPluginFramework::File file;
  const int mode = CTRPluginFramework::File::WRITE |
    CTRPluginFramework::File::CREATE |
    CTRPluginFramework::File::TRUNCATE |
    CTRPluginFramework::File::SYNC;
  if (CTRPluginFramework::File::Open(file, SETTINGS_PATH, mode) != 0)
  {
    return;
  }
  file.Write(&record, sizeof(record));
  file.Flush();
}

bool LoadSettings(bool& settingsLoaded)
{
  settingsLoaded = false;
  CTRPluginFramework::File file;
  if (CTRPluginFramework::File::Open(
        file,
        SETTINGS_PATH,
        CTRPluginFramework::File::READ
        ) != 0)
  {
    return false;
  }

  if (file.GetSize() == sizeof(SettingsRecord))
  {
    SettingsRecord record;
    if (file.Read(&record, sizeof(record)) != 0 ||
        record.magic != SETTINGS_MAGIC ||
        record.version != SETTINGS_VERSION ||
        record.size != sizeof(record) ||
        record.check != CalculateCheck(record) ||
        !IsValidMode(record.outlineMode) ||
        !IsValidDrawDistanceMode(record.drawDistanceMode) ||
        !IsValidTerrainDetailMode(record.terrainDetailMode) ||
        !IsValidCollisionQualityMode(record.collisionQualityMode) ||
        !IsValidAnimationRateMode(record.animationRateMode) ||
        record.uncapAnimationPlayback > 1 ||
        record.followerEnabled > 1 ||
        record.dynamicFollowerEnabled > 1 ||
        record.legacyPokeBallSoundMuted > 1 ||
        record.pokeBallSoundVolumePercent >
          MAX_POKE_BALL_SOUND_VOLUME_PERCENT ||
        record.legacyPokeBallSoundMuted !=
          (record.pokeBallSoundVolumePercent == 0 ? 1U : 0U) ||
        record.formChangeEnabled > 1 ||
        record.specialEffectsEnabled > 1 ||
        !IsValidPerformanceOptions(record.performanceOptions))
    {
      return false;
    }

    BehaviorState state;
    state.trailDelay = record.trailDelay;
    state.stopDistance = record.stopDistance;
    state.startDistance = record.startDistance;
    state.runDistance = record.runDistance;
    state.walkSpeed = record.walkSpeed;
    state.runSpeed = record.runSpeed;
    state.catchUpStep = record.catchUpStep;
    state.warpDistance = record.warpDistance;
    if (!IsValidBehavior(state))
    {
      return false;
    }

    __atomic_store_n(&g_OutlineMode, record.outlineMode, __ATOMIC_RELAXED);
    __atomic_store_n(
      &g_UncapAnimationPlayback,
      record.uncapAnimationPlayback,
      __ATOMIC_RELAXED
      );
    __atomic_store_n(
      &g_SavedPerformanceOptions,
      record.performanceOptions,
      __ATOMIC_RELAXED
      );
    __atomic_store_n(
      &g_DrawDistanceMode,
      record.drawDistanceMode,
      __ATOMIC_RELAXED
      );
    __atomic_store_n(
      &g_TerrainDetailMode,
      record.terrainDetailMode,
      __ATOMIC_RELAXED
      );
    __atomic_store_n(
      &g_CollisionQualityMode,
      record.collisionQualityMode,
      __ATOMIC_RELAXED
      );
    __atomic_store_n(
      &g_AnimationRateMode,
      record.animationRateMode,
      __ATOMIC_RELAXED
      );
    __atomic_store_n(
      &g_FollowerEnabled,
      record.followerEnabled,
      __ATOMIC_RELAXED
      );
    __atomic_store_n(
      &g_DynamicFollowerEnabled,
      record.dynamicFollowerEnabled,
      __ATOMIC_RELAXED
      );
    __atomic_store_n(
      &g_FormChangeEnabled,
      record.formChangeEnabled,
      __ATOMIC_RELAXED
      );
    __atomic_store_n(
      &g_SpecialEffectsEnabled,
      record.specialEffectsEnabled,
      __ATOMIC_RELAXED
      );
    __atomic_store_n(
      &g_PokeBallSoundVolumePercent,
      record.pokeBallSoundVolumePercent,
      __ATOMIC_RELAXED
      );
    StoreBehaviorState(state);
    settingsLoaded = true;
    return false;
  }

  if (file.GetSize() == sizeof(SettingsRecordV9))
  {
    SettingsRecordV9 record;
    if (file.Read(&record, sizeof(record)) != 0 ||
        record.magic != SETTINGS_MAGIC ||
        record.version != 9 ||
        record.size != sizeof(record) ||
        record.check != CalculateCheck(record) ||
        !IsValidMode(record.outlineMode) ||
        !IsValidDrawDistanceMode(record.drawDistanceMode) ||
        !IsValidTerrainDetailMode(record.terrainDetailMode) ||
        !IsValidCollisionQualityMode(record.collisionQualityMode) ||
        !IsValidAnimationRateMode(record.animationRateMode) ||
        record.uncapAnimationPlayback > 1 ||
        record.followerEnabled > 1 ||
        record.dynamicFollowerEnabled > 1 ||
        record.pokeBallSoundMuted > 1 ||
        record.formChangeEnabled > 1 ||
        record.specialEffectsEnabled > 1 ||
        !IsValidPerformanceOptions(record.performanceOptions))
    {
      return false;
    }

    BehaviorState state;
    state.trailDelay = record.trailDelay;
    state.stopDistance = record.stopDistance;
    state.startDistance = record.startDistance;
    state.runDistance = record.runDistance;
    state.walkSpeed = record.walkSpeed;
    state.runSpeed = record.runSpeed;
    state.catchUpStep = record.catchUpStep;
    state.warpDistance = record.warpDistance;
    if (!IsValidBehavior(state))
    {
      return false;
    }

    __atomic_store_n(&g_OutlineMode, record.outlineMode, __ATOMIC_RELAXED);
    __atomic_store_n(
      &g_UncapAnimationPlayback,
      record.uncapAnimationPlayback,
      __ATOMIC_RELAXED
      );
    __atomic_store_n(
      &g_SavedPerformanceOptions,
      record.performanceOptions,
      __ATOMIC_RELAXED
      );
    __atomic_store_n(
      &g_DrawDistanceMode,
      record.drawDistanceMode,
      __ATOMIC_RELAXED
      );
    __atomic_store_n(
      &g_TerrainDetailMode,
      record.terrainDetailMode,
      __ATOMIC_RELAXED
      );
    __atomic_store_n(
      &g_CollisionQualityMode,
      record.collisionQualityMode,
      __ATOMIC_RELAXED
      );
    __atomic_store_n(
      &g_AnimationRateMode,
      record.animationRateMode,
      __ATOMIC_RELAXED
      );
    __atomic_store_n(
      &g_FollowerEnabled,
      record.followerEnabled,
      __ATOMIC_RELAXED
      );
    __atomic_store_n(
      &g_DynamicFollowerEnabled,
      record.dynamicFollowerEnabled,
      __ATOMIC_RELAXED
      );
    __atomic_store_n(
      &g_FormChangeEnabled,
      record.formChangeEnabled,
      __ATOMIC_RELAXED
      );
    __atomic_store_n(
      &g_SpecialEffectsEnabled,
      record.specialEffectsEnabled,
      __ATOMIC_RELAXED
      );
    __atomic_store_n(
      &g_PokeBallSoundVolumePercent,
      record.pokeBallSoundMuted == 0
        ? DEFAULT_POKE_BALL_SOUND_VOLUME_PERCENT
        : 0U,
      __ATOMIC_RELAXED
      );
    StoreBehaviorState(state);
    settingsLoaded = true;
    return true;
  }

  if (file.GetSize() == sizeof(SettingsRecordV8))
  {
    SettingsRecordV8 record;
    if (file.Read(&record, sizeof(record)) != 0 ||
        record.magic != SETTINGS_MAGIC ||
        record.version != 8 ||
        record.size != sizeof(record) ||
        record.check != CalculateCheck(record) ||
        !IsValidMode(record.outlineMode) ||
        !IsValidDrawDistanceMode(record.drawDistanceMode) ||
        !IsValidTerrainDetailMode(record.terrainDetailMode) ||
        !IsValidCollisionQualityMode(record.collisionQualityMode) ||
        !IsValidAnimationRateMode(record.animationRateMode) ||
        record.uncapAnimationPlayback > 1 ||
        record.followerEnabled > 1 ||
        record.dynamicFollowerEnabled > 1 ||
        record.legacyHoverOffsetEnabled > 1 ||
        !IsValidPerformanceOptions(record.performanceOptions))
    {
      return false;
    }

    BehaviorState state;
    state.trailDelay = record.trailDelay;
    state.stopDistance = record.stopDistance;
    state.startDistance = record.startDistance;
    state.runDistance = record.runDistance;
    state.walkSpeed = record.walkSpeed;
    state.runSpeed = record.runSpeed;
    state.catchUpStep = record.catchUpStep;
    state.warpDistance = record.warpDistance;
    if (!IsValidBehavior(state))
    {
      return false;
    }

    __atomic_store_n(&g_OutlineMode, record.outlineMode, __ATOMIC_RELAXED);
    __atomic_store_n(
      &g_UncapAnimationPlayback,
      record.uncapAnimationPlayback,
      __ATOMIC_RELAXED
      );
    __atomic_store_n(
      &g_SavedPerformanceOptions,
      record.performanceOptions,
      __ATOMIC_RELAXED
      );
    __atomic_store_n(
      &g_DrawDistanceMode,
      record.drawDistanceMode,
      __ATOMIC_RELAXED
      );
    __atomic_store_n(
      &g_TerrainDetailMode,
      record.terrainDetailMode,
      __ATOMIC_RELAXED
      );
    __atomic_store_n(
      &g_CollisionQualityMode,
      record.collisionQualityMode,
      __ATOMIC_RELAXED
      );
    __atomic_store_n(
      &g_AnimationRateMode,
      record.animationRateMode,
      __ATOMIC_RELAXED
      );
    __atomic_store_n(
      &g_FollowerEnabled,
      record.followerEnabled,
      __ATOMIC_RELAXED
      );
    __atomic_store_n(
      &g_DynamicFollowerEnabled,
      record.dynamicFollowerEnabled,
      __ATOMIC_RELAXED
      );
    StoreBehaviorState(state);
    settingsLoaded = true;
    return true;
  }

  if (file.GetSize() == sizeof(SettingsRecordV7))
  {
    SettingsRecordV7 record;
    if (file.Read(&record, sizeof(record)) != 0 ||
        record.magic != SETTINGS_MAGIC ||
        record.version != 7 ||
        record.size != sizeof(record) ||
        record.check != CalculateCheck(record) ||
        !IsValidMode(record.outlineMode) ||
        !IsValidDrawDistanceMode(record.drawDistanceMode) ||
        !IsValidTerrainDetailMode(record.terrainDetailMode) ||
        !IsValidCollisionQualityMode(record.collisionQualityMode) ||
        !IsValidAnimationRateMode(record.animationRateMode) ||
        record.uncapAnimationPlayback > 1 ||
        record.followerEnabled > 1 ||
        record.dynamicFollowerEnabled > 1 ||
        !IsValidPerformanceOptions(record.performanceOptions))
    {
      return false;
    }

    BehaviorState state;
    state.trailDelay = record.trailDelay;
    state.stopDistance = record.stopDistance;
    state.startDistance = record.startDistance;
    state.runDistance = record.runDistance;
    state.walkSpeed = record.walkSpeed;
    state.runSpeed = record.runSpeed;
    state.catchUpStep = record.catchUpStep;
    state.warpDistance = record.warpDistance;
    if (!IsValidBehavior(state))
    {
      return false;
    }

    __atomic_store_n(&g_OutlineMode, record.outlineMode, __ATOMIC_RELAXED);
    __atomic_store_n(
      &g_UncapAnimationPlayback,
      record.uncapAnimationPlayback,
      __ATOMIC_RELAXED
      );
    __atomic_store_n(
      &g_SavedPerformanceOptions,
      record.performanceOptions,
      __ATOMIC_RELAXED
      );
    __atomic_store_n(
      &g_DrawDistanceMode,
      record.drawDistanceMode,
      __ATOMIC_RELAXED
      );
    __atomic_store_n(
      &g_TerrainDetailMode,
      record.terrainDetailMode,
      __ATOMIC_RELAXED
      );
    __atomic_store_n(
      &g_CollisionQualityMode,
      record.collisionQualityMode,
      __ATOMIC_RELAXED
      );
    __atomic_store_n(
      &g_AnimationRateMode,
      record.animationRateMode,
      __ATOMIC_RELAXED
      );
    __atomic_store_n(
      &g_FollowerEnabled,
      record.followerEnabled,
      __ATOMIC_RELAXED
      );
    __atomic_store_n(
      &g_DynamicFollowerEnabled,
      record.dynamicFollowerEnabled,
      __ATOMIC_RELAXED
      );
    StoreBehaviorState(state);
    settingsLoaded = true;
    return true;
  }

  if (file.GetSize() == sizeof(SettingsRecordV6))
  {
    SettingsRecordV6 record;
    if (file.Read(&record, sizeof(record)) != 0 ||
        record.magic != SETTINGS_MAGIC ||
        record.version != 6 ||
        record.size != sizeof(record) ||
        record.check != CalculateCheck(record) ||
        !IsValidMode(record.outlineMode) ||
        !IsValidDrawDistanceMode(record.drawDistanceMode) ||
        !IsValidTerrainDetailMode(record.terrainDetailMode) ||
        !IsValidCollisionQualityMode(record.collisionQualityMode) ||
        !IsValidAnimationRateMode(record.animationRateMode) ||
        record.uncapAnimationPlayback > 1 ||
        !IsValidPerformanceOptions(record.performanceOptions))
    {
      return false;
    }

    BehaviorState state;
    state.trailDelay = record.trailDelay;
    state.stopDistance = record.stopDistance;
    state.startDistance = record.startDistance;
    state.runDistance = record.runDistance;
    state.walkSpeed = record.walkSpeed;
    state.runSpeed = record.runSpeed;
    state.catchUpStep = record.catchUpStep;
    state.warpDistance = record.warpDistance;
    if (!IsValidBehavior(state))
    {
      return false;
    }

    __atomic_store_n(&g_OutlineMode, record.outlineMode, __ATOMIC_RELAXED);
    __atomic_store_n(
      &g_UncapAnimationPlayback,
      record.uncapAnimationPlayback,
      __ATOMIC_RELAXED
      );
    __atomic_store_n(
      &g_SavedPerformanceOptions,
      record.performanceOptions,
      __ATOMIC_RELAXED
      );
    __atomic_store_n(
      &g_DrawDistanceMode,
      record.drawDistanceMode,
      __ATOMIC_RELAXED
      );
    __atomic_store_n(
      &g_TerrainDetailMode,
      record.terrainDetailMode,
      __ATOMIC_RELAXED
      );
    __atomic_store_n(
      &g_CollisionQualityMode,
      record.collisionQualityMode,
      __ATOMIC_RELAXED
      );
    __atomic_store_n(
      &g_AnimationRateMode,
      record.animationRateMode,
      __ATOMIC_RELAXED
      );
    StoreBehaviorState(state);
    settingsLoaded = true;
    return true;
  }

  if (file.GetSize() == sizeof(SettingsRecordV5))
  {
    SettingsRecordV5 record;
    if (file.Read(&record, sizeof(record)) != 0 ||
        record.magic != SETTINGS_MAGIC ||
        record.version != 5 ||
        record.size != sizeof(record) ||
        record.check != CalculateCheck(record) ||
        !IsValidMode(record.outlineMode) ||
        !IsValidDrawDistanceMode(record.drawDistanceMode) ||
        record.uncapAnimationPlayback > 1 ||
        !IsValidLegacyPerformanceOptions(record.performanceOptions))
    {
      return false;
    }

    BehaviorState state;
    state.trailDelay = record.trailDelay;
    state.stopDistance = record.stopDistance;
    state.startDistance = record.startDistance;
    state.runDistance = record.runDistance;
    state.walkSpeed = record.walkSpeed;
    state.runSpeed = record.runSpeed;
    state.catchUpStep = record.catchUpStep;
    state.warpDistance = record.warpDistance;
    if (!IsValidBehavior(state))
    {
      return false;
    }

    __atomic_store_n(&g_OutlineMode, record.outlineMode, __ATOMIC_RELAXED);
    __atomic_store_n(
      &g_UncapAnimationPlayback,
      record.uncapAnimationPlayback,
      __ATOMIC_RELAXED
      );
    __atomic_store_n(
      &g_SavedPerformanceOptions,
      MigrateLegacyPerformanceOptions(record.performanceOptions),
      __ATOMIC_RELAXED
      );
    __atomic_store_n(
      &g_DrawDistanceMode,
      record.drawDistanceMode,
      __ATOMIC_RELAXED
      );
    __atomic_store_n(
      &g_CollisionQualityMode,
      (record.performanceOptions & (1U << 1))
        ? static_cast<u32>(FOLLOWER_COLLISION_QUALITY_BALANCED)
        : static_cast<u32>(FOLLOWER_COLLISION_QUALITY_FULL),
      __ATOMIC_RELAXED
      );
    StoreBehaviorState(state);
    settingsLoaded = true;
    return true;
  }

  if (file.GetSize() == sizeof(SettingsRecordV4))
  {
    SettingsRecordV4 record;
    if (file.Read(&record, sizeof(record)) != 0 ||
        record.magic != SETTINGS_MAGIC ||
        record.version != 4 ||
        record.size != sizeof(record) ||
        record.check != CalculateCheck(record) ||
        !IsValidMode(record.outlineMode) ||
        record.uncapAnimationPlayback > 1 ||
        !IsValidLegacyPerformanceOptions(record.performanceOptions))
    {
      return false;
    }

    BehaviorState state;
    state.trailDelay = record.trailDelay;
    state.stopDistance = record.stopDistance;
    state.startDistance = record.startDistance;
    state.runDistance = record.runDistance;
    state.walkSpeed = record.walkSpeed;
    state.runSpeed = record.runSpeed;
    state.catchUpStep = record.catchUpStep;
    state.warpDistance = record.warpDistance;
    if (!IsValidBehavior(state))
    {
      return false;
    }

    __atomic_store_n(&g_OutlineMode, record.outlineMode, __ATOMIC_RELAXED);
    __atomic_store_n(
      &g_UncapAnimationPlayback,
      record.uncapAnimationPlayback,
      __ATOMIC_RELAXED
      );
    __atomic_store_n(
      &g_SavedPerformanceOptions,
      MigrateLegacyPerformanceOptions(record.performanceOptions),
      __ATOMIC_RELAXED
      );
    __atomic_store_n(
      &g_CollisionQualityMode,
      (record.performanceOptions & (1U << 1))
        ? static_cast<u32>(FOLLOWER_COLLISION_QUALITY_BALANCED)
        : static_cast<u32>(FOLLOWER_COLLISION_QUALITY_FULL),
      __ATOMIC_RELAXED
      );
    StoreBehaviorState(state);
    settingsLoaded = true;
    return true;
  }

  if (file.GetSize() == sizeof(SettingsRecordV3))
  {
    SettingsRecordV3 record;
    if (file.Read(&record, sizeof(record)) != 0 ||
        record.magic != SETTINGS_MAGIC ||
        record.version != 3 ||
        record.size != sizeof(record) ||
        record.check != CalculateCheck(record) ||
        !IsValidMode(record.outlineMode) ||
        record.uncapAnimationPlayback > 1)
    {
      return false;
    }

    BehaviorState state;
    state.trailDelay = record.trailDelay;
    state.stopDistance = record.stopDistance;
    state.startDistance = record.startDistance;
    state.runDistance = record.runDistance;
    state.walkSpeed = record.walkSpeed;
    state.runSpeed = record.runSpeed;
    state.catchUpStep = record.catchUpStep;
    state.warpDistance = record.warpDistance;
    if (!IsValidBehavior(state))
    {
      return false;
    }

    __atomic_store_n(&g_OutlineMode, record.outlineMode, __ATOMIC_RELAXED);
    __atomic_store_n(
      &g_UncapAnimationPlayback,
      record.uncapAnimationPlayback,
      __ATOMIC_RELAXED
      );
    StoreBehaviorState(state);
    settingsLoaded = true;
    return true;
  }

  if (file.GetSize() == sizeof(SettingsRecordV2))
  {
    SettingsRecordV2 record;
    if (file.Read(&record, sizeof(record)) != 0 ||
        record.magic != SETTINGS_MAGIC ||
        record.version != 2 ||
        record.size != sizeof(record) ||
        record.check != CalculateCheck(record) ||
        !IsValidMode(record.outlineMode))
    {
      return false;
    }

    BehaviorState state;
    state.trailDelay = record.trailDelay;
    state.stopDistance = record.stopDistance;
    state.startDistance = record.startDistance;
    state.runDistance = record.runDistance;
    state.walkSpeed = record.walkSpeed;
    state.runSpeed = record.runSpeed;
    state.catchUpStep = record.catchUpStep;
    state.warpDistance = record.warpDistance;
    if (!IsValidBehavior(state))
    {
      return false;
    }

    __atomic_store_n(&g_OutlineMode, record.outlineMode, __ATOMIC_RELAXED);
    StoreBehaviorState(state);
    settingsLoaded = true;
    return true;
  }

  if (file.GetSize() == sizeof(SettingsRecordV1))
  {
    SettingsRecordV1 record;
    if (file.Read(&record, sizeof(record)) == 0 &&
        record.magic == SETTINGS_MAGIC &&
        record.version == 1 &&
        record.size == sizeof(record) &&
        record.check == CalculateCheck(record) &&
        IsValidMode(record.outlineMode))
    {
      __atomic_store_n(&g_OutlineMode, record.outlineMode, __ATOMIC_RELAXED);
      settingsLoaded = true;
      return true;
    }
  }

  return false;
}

} // namespace

bool InitializeFollowerSettings()
{
  if (g_Initialized)
  {
    return g_SettingsLoadedFromDisk;
  }

  __atomic_store_n(
    &g_OutlineMode,
    static_cast<u32>(FOLLOWER_OUTLINE_ID_SOFT),
    __ATOMIC_RELAXED
    );
  StoreBehaviorState(GetDefaultBehavior());
  __atomic_store_n(&g_UncapAnimationPlayback, 0, __ATOMIC_RELAXED);
  __atomic_store_n(&g_SavedPerformanceOptions, 0, __ATOMIC_RELAXED);
  __atomic_store_n(
    &g_DrawDistanceMode,
    static_cast<u32>(FOLLOWER_DRAW_DISTANCE_FULL),
    __ATOMIC_RELAXED
    );
  __atomic_store_n(
    &g_TerrainDetailMode,
    static_cast<u32>(FOLLOWER_TERRAIN_DETAIL_STOCK),
    __ATOMIC_RELAXED
    );
  __atomic_store_n(
    &g_CollisionQualityMode,
    static_cast<u32>(FOLLOWER_COLLISION_QUALITY_FULL),
    __ATOMIC_RELAXED
    );
  __atomic_store_n(
    &g_AnimationRateMode,
    static_cast<u32>(FOLLOWER_ANIMATION_RATE_30_FPS),
    __ATOMIC_RELAXED
    );
  __atomic_store_n(&g_FollowerEnabled, 1U, __ATOMIC_RELAXED);
  __atomic_store_n(&g_DynamicFollowerEnabled, 0U, __ATOMIC_RELAXED);
  __atomic_store_n(&g_FormChangeEnabled, 1U, __ATOMIC_RELAXED);
  __atomic_store_n(&g_SpecialEffectsEnabled, 1U, __ATOMIC_RELAXED);
  __atomic_store_n(
    &g_PokeBallSoundVolumePercent,
    DEFAULT_POKE_BALL_SOUND_VOLUME_PERCENT,
    __ATOMIC_RELAXED
    );
  bool needsMigration = LoadSettings(g_SettingsLoadedFromDisk);
  if (g_TrailDelay==3 && g_StopDistance==200 && g_StartDistance==300 &&
      g_RunDistance==1600 && g_WalkSpeed==140 && g_RunSpeed==300 &&
      g_CatchUpStep==15 && g_WarpDistance==9000)
  {
    StoreBehaviorState(GetDefaultBehavior());
    needsMigration=true;
  }
  g_Initialized = true;
  if (needsMigration)
  {
    SaveSettings();
  }
  return g_SettingsLoadedFromDisk;
}

void ShutdownFollowerSettings()
{
  if (!g_Initialized)
  {
    return;
  }
  SaveSettings();
  g_Initialized = false;
}

bool IsFollowerEnabled()
{
  return __atomic_load_n(&g_FollowerEnabled, __ATOMIC_RELAXED) != 0;
}

void SetFollowerEnabled(bool enabled)
{
  const u32 value = enabled ? 1U : 0U;
  const u32 previous = __atomic_exchange_n(
    &g_FollowerEnabled,
    value,
    __ATOMIC_RELAXED
    );
  if (g_Initialized && previous != value)
  {
    SaveSettings();
  }
}

bool IsDynamicFollowerEnabled()
{
  return __atomic_load_n(
    &g_DynamicFollowerEnabled,
    __ATOMIC_RELAXED
    ) != 0;
}

void SetDynamicFollowerEnabled(bool enabled)
{
  const u32 value = enabled ? 1U : 0U;
  const u32 previous = __atomic_exchange_n(
    &g_DynamicFollowerEnabled,
    value,
    __ATOMIC_RELAXED
    );
  if (g_Initialized && previous != value)
  {
    SaveSettings();
  }
}

bool IsFollowerFormChangeEnabled()
{
  return __atomic_load_n(&g_FormChangeEnabled, __ATOMIC_RELAXED) != 0;
}

void SetFollowerFormChangeEnabled(bool enabled)
{
  const u32 value = enabled ? 1U : 0U;
  const u32 previous = __atomic_exchange_n(
    &g_FormChangeEnabled,
    value,
    __ATOMIC_RELAXED
    );
  if (g_Initialized && previous != value)
  {
    SaveSettings();
  }
}

bool IsFollowerSpecialEffectsEnabled()
{
  return __atomic_load_n(&g_SpecialEffectsEnabled, __ATOMIC_RELAXED) != 0;
}

void SetFollowerSpecialEffectsEnabled(bool enabled)
{
  const u32 value = enabled ? 1U : 0U;
  const u32 previous = __atomic_exchange_n(
    &g_SpecialEffectsEnabled,
    value,
    __ATOMIC_RELAXED
    );
  if (g_Initialized && previous != value)
  {
    SaveSettings();
  }
}

unsigned int GetFollowerPokeBallSoundVolumePercent()
{
  return __atomic_load_n(
    &g_PokeBallSoundVolumePercent,
    __ATOMIC_RELAXED
    );
}

bool SetFollowerPokeBallSoundVolumePercent(unsigned int volumePercent)
{
  if (volumePercent > MAX_POKE_BALL_SOUND_VOLUME_PERCENT)
  {
    return false;
  }
  const u32 previous = __atomic_exchange_n(
    &g_PokeBallSoundVolumePercent,
    volumePercent,
    __ATOMIC_RELAXED
    );
  if (g_Initialized && previous != volumePercent)
  {
    SaveSettings();
  }
  return true;
}

FollowerOutlineMode GetFollowerOutlineMode()
{
  const u32 mode = __atomic_load_n(&g_OutlineMode, __ATOMIC_RELAXED);
  return IsValidMode(mode)
    ? static_cast<FollowerOutlineMode>(mode)
    : FOLLOWER_OUTLINE_ID_SOFT;
}

void SetFollowerOutlineMode(FollowerOutlineMode mode)
{
  const u32 value = static_cast<u32>(mode);
  if (!IsValidMode(value))
  {
    return;
  }

  const u32 previous = __atomic_exchange_n(
    &g_OutlineMode,
    value,
    __ATOMIC_RELAXED
    );
  if (g_Initialized && previous != value)
  {
    SaveSettings();
  }
}

const char* GetFollowerOutlineModeName(FollowerOutlineMode mode)
{
  switch (mode)
  {
  case FOLLOWER_APPEARANCE_NORMAL_EDGE:
    return "Normal edge (experimental)";
  case FOLLOWER_APPEARANCE_NATIVE_FIELD:
    return "Native field";
  case FOLLOWER_APPEARANCE_SOFT_FIELD:
    return "Field soft";
  case FOLLOWER_OUTLINE_ID_MEDIUM:
    return "ID Medium";
  case FOLLOWER_OUTLINE_ID_ORIGINAL:
    return "ID Original";
  case FOLLOWER_OUTLINE_OFF:
    return "Off";
  default:
    return "ID Soft";
  }
}

FollowerDrawDistanceMode GetFollowerDrawDistanceMode()
{
  const u32 mode = __atomic_load_n(&g_DrawDistanceMode, __ATOMIC_RELAXED);
  return IsValidDrawDistanceMode(mode)
    ? static_cast<FollowerDrawDistanceMode>(mode)
    : FOLLOWER_DRAW_DISTANCE_FULL;
}

void SetFollowerDrawDistanceMode(FollowerDrawDistanceMode mode)
{
  const u32 value = static_cast<u32>(mode);
  if (!IsValidDrawDistanceMode(value))
  {
    return;
  }

  const u32 previous = __atomic_exchange_n(
    &g_DrawDistanceMode,
    value,
    __ATOMIC_RELAXED
    );
  if (g_Initialized && previous != value)
  {
    SaveSettings();
  }
}

const char* GetFollowerDrawDistanceModeName(FollowerDrawDistanceMode mode)
{
  switch (mode)
  {
  case FOLLOWER_DRAW_DISTANCE_HIGH:
    return "High (75%)";
  case FOLLOWER_DRAW_DISTANCE_MEDIUM:
    return "Medium (50%)";
  case FOLLOWER_DRAW_DISTANCE_LOW:
    return "Low (35%)";
  default:
    return "Full (retail)";
  }
}

float GetFollowerDrawDistanceScale(FollowerDrawDistanceMode mode)
{
  switch (mode)
  {
  case FOLLOWER_DRAW_DISTANCE_HIGH:
    return 0.75f;
  case FOLLOWER_DRAW_DISTANCE_MEDIUM:
    return 0.50f;
  case FOLLOWER_DRAW_DISTANCE_LOW:
    return 0.35f;
  default:
    return 1.0f;
  }
}

FollowerTerrainDetailMode GetFollowerTerrainDetailMode()
{
  const u32 mode = __atomic_load_n(&g_TerrainDetailMode, __ATOMIC_RELAXED);
  return IsValidTerrainDetailMode(mode)
    ? static_cast<FollowerTerrainDetailMode>(mode)
    : FOLLOWER_TERRAIN_DETAIL_STOCK;
}

void SetFollowerTerrainDetailMode(FollowerTerrainDetailMode mode)
{
  const u32 value = static_cast<u32>(mode);
  if (!IsValidTerrainDetailMode(value))
  {
    return;
  }
  const u32 previous = __atomic_exchange_n(
    &g_TerrainDetailMode,
    value,
    __ATOMIC_RELAXED
    );
  if (g_Initialized && previous != value)
  {
    SaveSettings();
  }
}

const char* GetFollowerTerrainDetailModeName(FollowerTerrainDetailMode mode)
{
  return mode == FOLLOWER_TERRAIN_DETAIL_MEDIUM
    ? "Medium"
    : "Stock (High)";
}

FollowerCollisionQualityMode GetFollowerCollisionQualityMode()
{
  const u32 mode = __atomic_load_n(&g_CollisionQualityMode, __ATOMIC_RELAXED);
  return IsValidCollisionQualityMode(mode)
    ? static_cast<FollowerCollisionQualityMode>(mode)
    : FOLLOWER_COLLISION_QUALITY_FULL;
}

void SetFollowerCollisionQualityMode(FollowerCollisionQualityMode mode)
{
  const u32 value = static_cast<u32>(mode);
  if (!IsValidCollisionQualityMode(value))
  {
    return;
  }
  const u32 previous = __atomic_exchange_n(
    &g_CollisionQualityMode,
    value,
    __ATOMIC_RELAXED
    );
  if (g_Initialized && previous != value)
  {
    SaveSettings();
  }
}

const char* GetFollowerCollisionQualityModeName(
  FollowerCollisionQualityMode mode
  )
{
  switch (mode)
  {
  case FOLLOWER_COLLISION_QUALITY_BALANCED:
    return "Balanced";
  case FOLLOWER_COLLISION_QUALITY_FAST:
    return "Fast";
  default:
    return "Full";
  }
}

FollowerAnimationRateMode GetFollowerAnimationRateMode()
{
  const u32 mode = __atomic_load_n(&g_AnimationRateMode, __ATOMIC_RELAXED);
  return IsValidAnimationRateMode(mode)
    ? static_cast<FollowerAnimationRateMode>(mode)
    : FOLLOWER_ANIMATION_RATE_30_FPS;
}

void SetFollowerAnimationRateMode(FollowerAnimationRateMode mode)
{
  const u32 value = static_cast<u32>(mode);
  if (!IsValidAnimationRateMode(value))
  {
    return;
  }
  const u32 previous = __atomic_exchange_n(
    &g_AnimationRateMode,
    value,
    __ATOMIC_RELAXED
    );
  if (g_Initialized && previous != value)
  {
    SaveSettings();
  }
}

const char* GetFollowerAnimationRateModeName(FollowerAnimationRateMode mode)
{
  switch (mode)
  {
  case FOLLOWER_ANIMATION_RATE_15_FPS:
    return "15 FPS";
  case FOLLOWER_ANIMATION_RATE_15_FPS_INTERPOLATED:
    return "15 FPS interpolated";
  case FOLLOWER_ANIMATION_RATE_10_FPS_INTERPOLATED:
    return "10 FPS interpolated";
  case FOLLOWER_ANIMATION_RATE_7_5_FPS_INTERPOLATED:
    return "7.5 FPS interpolated";
  case FOLLOWER_ANIMATION_RATE_5_FPS_INTERPOLATED:
    return "5 FPS interpolated";
  default:
    return "30 FPS";
  }
}

float GetFollowerBehaviorValue(FollowerBehaviorSetting setting)
{
  const BehaviorState state = GetBehaviorState();
  return FromStoredValue(setting, GetBehaviorField(state, setting));
}

float GetFollowerBehaviorMinimum(FollowerBehaviorSetting setting)
{
  switch (setting)
  {
  case FOLLOWER_BEHAVIOR_TRAIL_DELAY:
    return static_cast<float>(MIN_TRAIL_DELAY);
  case FOLLOWER_BEHAVIOR_STOP_DISTANCE:
    return FromStoredValue(setting, MIN_STOP_DISTANCE);
  case FOLLOWER_BEHAVIOR_START_DISTANCE:
    return FromStoredValue(setting, MIN_START_DISTANCE);
  case FOLLOWER_BEHAVIOR_RUN_DISTANCE:
    return FromStoredValue(setting, MIN_RUN_DISTANCE);
  case FOLLOWER_BEHAVIOR_WALK_SPEED:
    return FromStoredValue(setting, MIN_WALK_SPEED);
  case FOLLOWER_BEHAVIOR_RUN_SPEED:
    return FromStoredValue(setting, MIN_RUN_SPEED);
  case FOLLOWER_BEHAVIOR_CATCH_UP_STEP:
    return FromStoredValue(setting, MIN_CATCH_UP_STEP);
  case FOLLOWER_BEHAVIOR_WARP_DISTANCE:
    return FromStoredValue(setting, MIN_WARP_DISTANCE);
  default:
    return 0.0f;
  }
}

float GetFollowerBehaviorMaximum(FollowerBehaviorSetting setting)
{
  switch (setting)
  {
  case FOLLOWER_BEHAVIOR_TRAIL_DELAY:
    return static_cast<float>(MAX_TRAIL_DELAY);
  case FOLLOWER_BEHAVIOR_STOP_DISTANCE:
    return FromStoredValue(setting, MAX_STOP_DISTANCE);
  case FOLLOWER_BEHAVIOR_START_DISTANCE:
    return FromStoredValue(setting, MAX_START_DISTANCE);
  case FOLLOWER_BEHAVIOR_RUN_DISTANCE:
    return FromStoredValue(setting, MAX_RUN_DISTANCE);
  case FOLLOWER_BEHAVIOR_WALK_SPEED:
    return FromStoredValue(setting, MAX_WALK_SPEED);
  case FOLLOWER_BEHAVIOR_RUN_SPEED:
    return FromStoredValue(setting, MAX_RUN_SPEED);
  case FOLLOWER_BEHAVIOR_CATCH_UP_STEP:
    return FromStoredValue(setting, MAX_CATCH_UP_STEP);
  case FOLLOWER_BEHAVIOR_WARP_DISTANCE:
    return FromStoredValue(setting, MAX_WARP_DISTANCE);
  default:
    return 0.0f;
  }
}

bool IsFollowerBehaviorValueValid(
  FollowerBehaviorSetting setting,
  float value
  )
{
  if (setting < FOLLOWER_BEHAVIOR_TRAIL_DELAY ||
      setting >= FOLLOWER_BEHAVIOR_SETTING_COUNT ||
      value != value ||
      value < GetFollowerBehaviorMinimum(setting) ||
      value > GetFollowerBehaviorMaximum(setting))
  {
    return false;
  }
  if (setting == FOLLOWER_BEHAVIOR_TRAIL_DELAY &&
      value != static_cast<float>(static_cast<u32>(value)))
  {
    return false;
  }

  BehaviorState state = GetBehaviorState();
  u32* const field = GetBehaviorField(&state, setting);
  if (!field)
  {
    return false;
  }
  *field = ToStoredValue(setting, value);
  return IsValidBehavior(state);
}

bool SetFollowerBehaviorValue(
  FollowerBehaviorSetting setting,
  float value
  )
{
  if (!IsFollowerBehaviorValueValid(setting, value))
  {
    return false;
  }

  BehaviorState state = GetBehaviorState();
  u32* const field = GetBehaviorField(&state, setting);
  const u32 stored = ToStoredValue(setting, value);
  if (!field || *field == stored)
  {
    return true;
  }

  *field = stored;
  StoreBehaviorState(state);
  if (g_Initialized)
  {
    SaveSettings();
  }
  return true;
}

void ResetFollowerBehaviorDefaults()
{
  StoreBehaviorState(GetDefaultBehavior());
  __atomic_store_n(&g_UncapAnimationPlayback, 0, __ATOMIC_RELAXED);
  if (g_Initialized)
  {
    SaveSettings();
  }
}

bool IsFollowerAnimationPlaybackUncapped()
{
  return __atomic_load_n(
    &g_UncapAnimationPlayback,
    __ATOMIC_RELAXED
    ) != 0;
}

void SetFollowerAnimationPlaybackUncapped(bool uncapped)
{
  const u32 value = uncapped ? 1 : 0;
  const u32 previous = __atomic_exchange_n(
    &g_UncapAnimationPlayback,
    value,
    __ATOMIC_RELAXED
    );
  if (g_Initialized && previous != value)
  {
    SaveSettings();
  }
}

unsigned int GetSavedPerformanceOptions()
{
  return __atomic_load_n(
    &g_SavedPerformanceOptions,
    __ATOMIC_RELAXED
    );
}

void SetSavedPerformanceOptions(unsigned int options)
{
  if (!IsValidPerformanceOptions(options))
  {
    return;
  }

  const u32 previous = __atomic_exchange_n(
    &g_SavedPerformanceOptions,
    static_cast<u32>(options),
    __ATOMIC_RELAXED
    );
  if (g_Initialized && previous != options)
  {
    SaveSettings();
  }
}

unsigned int GetFollowerTrailDelayFrames()
{
  if (IsCloseFollowEnabled()) return 3U;
  return __atomic_load_n(&g_TrailDelay, __ATOMIC_RELAXED);
}

float GetFollowerStopDistance()
{
  if (IsCloseFollowEnabled()) return 20.0f;
  return FromStoredValue(
    FOLLOWER_BEHAVIOR_STOP_DISTANCE,
    __atomic_load_n(&g_StopDistance, __ATOMIC_RELAXED)
    );
}

float GetFollowerStartDistance()
{
  if (IsCloseFollowEnabled()) return 30.0f;
  return FromStoredValue(
    FOLLOWER_BEHAVIOR_START_DISTANCE,
    __atomic_load_n(&g_StartDistance, __ATOMIC_RELAXED)
    );
}

float GetFollowerRunDistance()
{
  if (IsCloseFollowEnabled()) return 160.0f;
  return FromStoredValue(
    FOLLOWER_BEHAVIOR_RUN_DISTANCE,
    __atomic_load_n(&g_RunDistance, __ATOMIC_RELAXED)
    );
}

float GetFollowerRunExitDistance()
{
  return GetFollowerRunDistance() * (10.0f / 13.0f);
}

float GetFollowerWalkSpeed()
{
  return FromStoredValue(
    FOLLOWER_BEHAVIOR_WALK_SPEED,
    __atomic_load_n(&g_WalkSpeed, __ATOMIC_RELAXED)
    );
}

float GetFollowerRunSpeed()
{
  return FromStoredValue(
    FOLLOWER_BEHAVIOR_RUN_SPEED,
    __atomic_load_n(&g_RunSpeed, __ATOMIC_RELAXED)
    );
}

float GetFollowerCatchUpStep()
{
  return FromStoredValue(
    FOLLOWER_BEHAVIOR_CATCH_UP_STEP,
    __atomic_load_n(&g_CatchUpStep, __ATOMIC_RELAXED)
    );
}

float GetFollowerWarpDistance()
{
  return FromStoredValue(
    FOLLOWER_BEHAVIOR_WARP_DISTANCE,
    __atomic_load_n(&g_WarpDistance, __ATOMIC_RELAXED)
    );
}

} // namespace Gen7Follower3gx
