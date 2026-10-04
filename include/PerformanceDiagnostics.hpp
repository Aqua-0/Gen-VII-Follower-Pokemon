#pragma once

#include "FollowerFeatures.hpp"

#ifndef FOLLOWER_3GX_DIAGNOSTIC
#define FOLLOWER_3GX_DIAGNOSTIC 0
#endif

#ifndef FOLLOWER_3GX_PERFORMANCE_FEATURES
#define FOLLOWER_3GX_PERFORMANCE_FEATURES 1
#endif

#include "PluginRuntime.hpp"

namespace Gen7Follower3gx
{

typedef unsigned long long PerformanceTick;

enum PerformanceZone
{
  PERFORMANCE_ZONE_FOLLOWER,
  PERFORMANCE_ZONE_COLLISION,
  PERFORMANCE_ZONE_MODEL,
  PERFORMANCE_ZONE_TRAVERSAL,
  PERFORMANCE_ZONE_OUTLINE,
  PERFORMANCE_ZONE_OVERLAY,
  PERFORMANCE_ZONE_COUNT,
};

enum PerformanceCounter
{
  PERFORMANCE_COUNTER_GROUND_MESH,
  PERFORMANCE_COUNTER_WALL_MESH,
  PERFORMANCE_COUNTER_WALL_CYLINDER,
  PERFORMANCE_COUNTER_WALL_BOX,
  PERFORMANCE_COUNTER_COUNT,
};

enum PerformanceTestMode
{
  PERFORMANCE_TEST_NORMAL,
  PERFORMANCE_TEST_NO_OCCLUDERS,
  PERFORMANCE_TEST_EMPTY_EDGE,
  PERFORMANCE_TEST_ID_EDGE,
  PERFORMANCE_TEST_ID_EDGE_SOFT,
  PERFORMANCE_TEST_ID_EDGE_MEDIUM,
  PERFORMANCE_TEST_NO_FOLLOWER_OUTLINE,
  PERFORMANCE_TEST_FOLLOWER_HIDDEN,
  PERFORMANCE_TEST_MODE_COUNT,
};

typedef unsigned int PerformanceOptionMask;

enum PerformanceOption
{
  PERFORMANCE_OPTION_LITE_FOLLOWER_MATERIALS,
  PERFORMANCE_OPTION_HIDE_FOLLOWER_SHADOW,
  PERFORMANCE_OPTION_DISABLE_FIELD_OUTLINES,
  PERFORMANCE_OPTION_DISABLE_BLOOM,
  PERFORMANCE_OPTION_DISABLE_EFFECTS,
  PERFORMANCE_OPTION_DISABLE_WEATHER,
  PERFORMANCE_OPTION_DISABLE_CHARACTER_SHADOWS,
  PERFORMANCE_OPTION_DISABLE_SKYBOX,
  PERFORMANCE_OPTION_REDUCED_TRANSFORM_PRECISION,
  PERFORMANCE_OPTION_DISABLE_DEPTH_OF_FIELD,
  PERFORMANCE_OPTION_DISABLE_MOTION_BLUR,
  PERFORMANCE_OPTION_SIMPLE_WORLD_LIGHTING,
  PERFORMANCE_OPTION_COUNT,
};

inline PerformanceOptionMask GetPerformanceOptionBit(PerformanceOption option)
{
  return static_cast<unsigned int>(option) < PERFORMANCE_OPTION_COUNT
    ? 1U << static_cast<unsigned int>(option)
    : 0U;
}

inline bool HasPerformanceOption(
  PerformanceOptionMask options,
  PerformanceOption option
)
{
  const PerformanceOptionMask bit = GetPerformanceOptionBit(option);
  return bit != 0 && (options & bit) != 0;
}

inline PerformanceOptionMask FilterSupportedPerformanceOptions(
  PerformanceOptionMask options
)
{
  const PerformanceOptionMask validMask =
    (1U << static_cast<unsigned int>(PERFORMANCE_OPTION_COUNT)) - 1U;
  options &= validMask;
  if (IsRegularGameRuntime())
  {
    options &= ~GetPerformanceOptionBit(
      PERFORMANCE_OPTION_LITE_FOLLOWER_MATERIALS
      );
  }
  return options;
}

enum BallTransitionDiagnosticKind
{
  BALL_TRANSITION_DIAGNOSTIC_SPAWN,
  BALL_TRANSITION_DIAGNOSTIC_RECALL,
};

enum DynamicEffectHeapSource
{
  DYNAMIC_EFFECT_HEAP_NONE,
  DYNAMIC_EFFECT_HEAP_POKEMON_MODEL,
  DYNAMIC_EFFECT_HEAP_EVENT_DEVICE,
  DYNAMIC_EFFECT_HEAP_EXTERNAL_APPLICATION,
};

enum BallTransitionStartResult
{
  BALL_TRANSITION_START_NOT_ATTEMPTED,
  BALL_TRANSITION_START_STARTED,
  BALL_TRANSITION_START_FOLLOWER_USES_EVENT_HEAP,
  BALL_TRANSITION_START_NO_EFFECT_MANAGER,
  BALL_TRANSITION_START_NO_MODEL,
  BALL_TRANSITION_START_NO_RESOURCE_HEAP,
  BALL_TRANSITION_START_BUSY,
  BALL_TRANSITION_START_RESOURCE_ALREADY_LOADED,
  BALL_TRANSITION_START_RESOURCE_HEAP_LOW,
  BALL_TRANSITION_START_LOAD_FAILED,
  BALL_TRANSITION_START_NO_EFFECT_SLOT,
  BALL_TRANSITION_START_CREATE_FAILED,
};

struct BallTransitionDiagnosticSnapshot
{
  unsigned int attemptCount;
  unsigned int successCount;
  unsigned int kind;
  unsigned int result;
  unsigned int resourceHeapSource;
  unsigned int resourceHeapFree;
  unsigned int minimumHeapFree;
  unsigned int resourceWasLoaded;
  unsigned int systemSlotsFree;
  unsigned int eventSlotsFree;
  unsigned int weatherSlotsFree;
  unsigned int rideSlotsFree;
  unsigned int selectedWorkType;
};

struct FollowerSceneGraphDiagnosticSnapshot
{
  unsigned int totalJoints;
  unsigned int renderJoints;
  unsigned int requiredJoints;
};

struct FollowerGroundDiagnosticSnapshot
{
  unsigned int species;
  unsigned int form;
  int motion;
  int bodyRadiusTenths;
  int rawRootYTenths;
  int appliedRootYTenths;
  int cmHeight;
  int fieldAdjustHeight;
};

struct FollowerLifecycleDiagnosticSnapshot
{
  unsigned int state;
  unsigned int terminateBlocker;
  unsigned int loadFailure;
};

enum FollowerInteractionDiagnosticState
{
  FOLLOWER_INTERACTION_STATE_NONE,
  FOLLOWER_INTERACTION_STATE_LOADING,
  FOLLOWER_INTERACTION_STATE_READY,
  FOLLOWER_INTERACTION_STATE_PLAYING,
  FOLLOWER_INTERACTION_STATE_CANCELED,
};

enum FollowerInteractionDiagnosticResult
{
  FOLLOWER_INTERACTION_RESULT_NOT_ATTEMPTED,
  FOLLOWER_INTERACTION_RESULT_LOAD_STARTED,
  FOLLOWER_INTERACTION_RESULT_RESPOND,
  FOLLOWER_INTERACTION_RESULT_HAPPY_A,
  FOLLOWER_INTERACTION_RESULT_HAPPY_B,
  FOLLOWER_INTERACTION_RESULT_HAPPY_C,
  FOLLOWER_INTERACTION_RESULT_FIELD_FALLBACK,
  FOLLOWER_INTERACTION_RESULT_TOO_FAR,
  FOLLOWER_INTERACTION_RESULT_MOVING,
  FOLLOWER_INTERACTION_RESULT_BUSY,
  FOLLOWER_INTERACTION_RESULT_EVENT_ACTIVE,
  FOLLOWER_INTERACTION_RESULT_NO_HEAP,
  FOLLOWER_INTERACTION_RESULT_NO_SYSTEM,
  FOLLOWER_INTERACTION_RESULT_LOAD_FAILED,
  FOLLOWER_INTERACTION_RESULT_BAD_PACK,
  FOLLOWER_INTERACTION_RESULT_NO_KW_MOTION,
  FOLLOWER_INTERACTION_RESULT_CANCELED,
};

enum FollowerInteractionDiagnosticMotion
{
  FOLLOWER_INTERACTION_MOTION_NONE,
  FOLLOWER_INTERACTION_MOTION_RESPOND,
  FOLLOWER_INTERACTION_MOTION_HAPPY_A,
  FOLLOWER_INTERACTION_MOTION_HAPPY_B,
  FOLLOWER_INTERACTION_MOTION_HAPPY_C,
  FOLLOWER_INTERACTION_MOTION_FIELD,
};

struct FollowerInteractionDiagnosticSnapshot
{
  unsigned int attemptCount;
  unsigned int successCount;
  unsigned int state;
  unsigned int result;
  unsigned int dataId;
  unsigned int heapFree;
  unsigned int bufferSize;
  unsigned int realSize;
  unsigned int resourceCount;
  unsigned int respondAvailable;
  unsigned int happyAvailable; // Bits 0/1/2: Happy A/B/C.
  unsigned int selectedMotion;
};

enum DittoTransformDiagnosticState
{
  DITTO_TRANSFORM_STATE_NONE,
  DITTO_TRANSFORM_STATE_FLASH_IN,
  DITTO_TRANSFORM_STATE_CLONE_HOLD,
  DITTO_TRANSFORM_STATE_FLASH_OUT,
  DITTO_TRANSFORM_STATE_CLEANUP,
};

enum DittoTransformDiagnosticResult
{
  DITTO_TRANSFORM_RESULT_NOT_ATTEMPTED,
  DITTO_TRANSFORM_RESULT_STARTED,
  DITTO_TRANSFORM_RESULT_ACTIVE,
  DITTO_TRANSFORM_RESULT_COMPLETE,
  DITTO_TRANSFORM_RESULT_NO_MANAGER,
  DITTO_TRANSFORM_RESULT_NO_PLAYER,
  DITTO_TRANSFORM_RESULT_NO_DRESS,
  DITTO_TRANSFORM_RESULT_NO_SLOT,
  DITTO_TRANSFORM_RESULT_NO_HEAP,
  DITTO_TRANSFORM_RESULT_HEAP_CREATE_FAILED,
  DITTO_TRANSFORM_RESULT_WORK_FAILED,
  DITTO_TRANSFORM_RESULT_FLASH_FAILED,
  DITTO_TRANSFORM_RESULT_CLONE_FAILED,
  DITTO_TRANSFORM_RESULT_CANCELED,
};

enum DittoCloneMotionPhase
{
  DITTO_CLONE_MOTION_NONE,
  DITTO_CLONE_MOTION_FALLBACK,
  DITTO_CLONE_MOTION_POSE,
  DITTO_CLONE_MOTION_RECOVER,
};

struct DittoTransformDiagnosticSnapshot
{
  unsigned int attemptCount;
  unsigned int successCount;
  unsigned int state;
  unsigned int result;
  unsigned int modelId;
  unsigned int parentHeapFree;
  unsigned int cloneHeapFree;
  unsigned int motionId;
  unsigned int motionPhase;
  unsigned int motionFrame;
  unsigned int motionEndFrame;
};

enum CastformWeatherDiagnosticResult
{
  CASTFORM_WEATHER_RESULT_NOT_ATTEMPTED,
  CASTFORM_WEATHER_RESULT_STARTED,
  CASTFORM_WEATHER_RESULT_PERFORMANCE_DISABLED,
  CASTFORM_WEATHER_RESULT_NOT_OUTDOORS,
  CASTFORM_WEATHER_RESULT_NO_CONTROL,
  CASTFORM_WEATHER_RESULT_INVALID_CONTROL,
  CASTFORM_WEATHER_RESULT_INVALID_KIND,
  CASTFORM_WEATHER_RESULT_BUSY,
  CASTFORM_WEATHER_RESULT_APPLY_FAILED,
  CASTFORM_WEATHER_RESULT_RESTORED,
};

struct CastformWeatherDiagnosticSnapshot
{
  unsigned int attemptCount;
  unsigned int successCount;
  unsigned int result;
  unsigned int requestedWeather;
  int previousWeather;
  int previousForceWeather;
};

enum InteractionEffectStartResult
{
  INTERACTION_EFFECT_START_NOT_ATTEMPTED,
  INTERACTION_EFFECT_START_STARTED,
  INTERACTION_EFFECT_START_PERFORMANCE_DISABLED,
  INTERACTION_EFFECT_START_FOLLOWER_USES_EVENT_HEAP,
  INTERACTION_EFFECT_START_NO_EFFECT_MANAGER,
  INTERACTION_EFFECT_START_NO_MODEL,
  INTERACTION_EFFECT_START_NO_RESOURCE_HEAP,
  INTERACTION_EFFECT_START_BUSY,
  INTERACTION_EFFECT_START_RESOURCE_ALREADY_LOADED,
  INTERACTION_EFFECT_START_RESOURCE_HEAP_LOW,
  INTERACTION_EFFECT_START_LOAD_FAILED,
  INTERACTION_EFFECT_START_NO_EFFECT_SLOT,
  INTERACTION_EFFECT_START_CREATE_FAILED,
  INTERACTION_EFFECT_START_MODULE_UNSUPPORTED,
  INTERACTION_EFFECT_START_MODULE_DLL_HEAP_LOW,
  INTERACTION_EFFECT_START_MODULE_LOAD_FAILED,
};

struct InteractionEffectDiagnosticSnapshot
{
  unsigned int attemptCount;
  unsigned int successCount;
  unsigned int result;
  unsigned int effectType;
  unsigned int resourceHeapSource;
  unsigned int resourceHeapFreeBefore;
  unsigned int resourceHeapFreeAfter;
  unsigned int minimumHeapFree;
  unsigned int resourceWasLoaded;
  unsigned int selectedWorkType;
};

enum FollowerHeapDiagnosticKind
{
  FOLLOWER_HEAP_DIAGNOSTIC_AREA,
  FOLLOWER_HEAP_DIAGNOSTIC_APP_DEVICE,
  FOLLOWER_HEAP_DIAGNOSTIC_EVENT_DEVICE,
  FOLLOWER_HEAP_DIAGNOSTIC_DLL,
  FOLLOWER_HEAP_DIAGNOSTIC_POKEMON_MODEL,
  FOLLOWER_HEAP_DIAGNOSTIC_COUNT,
};

struct FollowerHeapDiagnosticMeasurement
{
  unsigned int totalSize;
  unsigned int totalFree;
  unsigned int maximumAllocatable;
};

struct FollowerHeapDiagnosticSample
{
  FollowerHeapDiagnosticMeasurement heaps[FOLLOWER_HEAP_DIAGNOSTIC_COUNT];
  unsigned int parentSource;
  unsigned int followerUsesEventHeap;
};

struct FollowerHeapDiagnosticSnapshot
{
  FollowerHeapDiagnosticSample beforeFollower;
  FollowerHeapDiagnosticSample current;
  unsigned int captureCount;
};

// Release builds get the presets too. Timing and overlays are only in diagnostic builds.
unsigned int GetSavedPerformanceOptions();
void SetSavedPerformanceOptions(unsigned int options);
PerformanceOptionMask GetPerformanceOptions();
void SetPerformanceOptions(PerformanceOptionMask options);
bool IsPerformanceOptionEnabled(PerformanceOption option);
void SetPerformanceOptionEnabled(PerformanceOption option, bool enabled);
void SetTransientPerformanceOptionEnabled(
  PerformanceOption option,
  bool enabled
  );
void ResetPerformanceOptions();

#if FOLLOWER_3GX_DIAGNOSTIC

PerformanceTick PerformanceReadTick();
void PerformanceRecordTicks(
  PerformanceZone zone,
  PerformanceTick startTick
  );
void PerformanceRecordQuery(
  PerformanceCounter counter,
  PerformanceTick startTick
  );
void InitializePerformanceDiagnostics();
void ShutdownPerformanceDiagnostics();
PerformanceTestMode GetPerformanceTestMode();
void SetPerformanceOverlayVisible(bool visible);
bool IsPerformanceOverlayVisible();
void RecordBallTransitionStart(
  BallTransitionDiagnosticKind kind,
  BallTransitionStartResult result,
  unsigned int resourceHeapSource,
  unsigned int resourceHeapFree,
  unsigned int minimumHeapFree,
  bool resourceWasLoaded,
  unsigned int systemSlotsFree,
  unsigned int eventSlotsFree,
  unsigned int weatherSlotsFree,
  unsigned int rideSlotsFree,
  unsigned int selectedWorkType
  );
BallTransitionDiagnosticSnapshot GetBallTransitionDiagnostics();
void SetFollowerSceneGraphDiagnostics(
  unsigned int totalJoints,
  unsigned int renderJoints,
  unsigned int requiredJoints
  );
FollowerSceneGraphDiagnosticSnapshot GetFollowerSceneGraphDiagnostics();
void UpdateFollowerGroundDiagnostics(
  const FollowerGroundDiagnosticSnapshot& snapshot
  );
FollowerGroundDiagnosticSnapshot GetFollowerGroundDiagnostics();
void UpdateFollowerLifecycleDiagnostics(
  unsigned int state,
  unsigned int terminateBlocker,
  unsigned int loadFailure
  );
FollowerLifecycleDiagnosticSnapshot GetFollowerLifecycleDiagnostics();
void UpdateFollowerInteractionDiagnostics(
  const FollowerInteractionDiagnosticSnapshot& snapshot,
  bool incrementAttempt,
  bool incrementSuccess
  );
FollowerInteractionDiagnosticSnapshot GetFollowerInteractionDiagnostics();
void UpdateDittoTransformDiagnostics(
  const DittoTransformDiagnosticSnapshot& snapshot,
  bool incrementAttempt,
  bool incrementSuccess
  );
DittoTransformDiagnosticSnapshot GetDittoTransformDiagnostics();
void UpdateCastformWeatherDiagnostics(
  const CastformWeatherDiagnosticSnapshot& snapshot,
  bool incrementAttempt,
  bool incrementSuccess
  );
CastformWeatherDiagnosticSnapshot GetCastformWeatherDiagnostics();
void RecordInteractionEffectStart(
  InteractionEffectStartResult result,
  unsigned int effectType,
  unsigned int resourceHeapSource,
  unsigned int resourceHeapFreeBefore,
  unsigned int resourceHeapFreeAfter,
  unsigned int minimumHeapFree,
  bool resourceWasLoaded,
  unsigned int selectedWorkType
  );
InteractionEffectDiagnosticSnapshot GetInteractionEffectDiagnostics();
void RecordFollowerHeapDiagnostics(
  const FollowerHeapDiagnosticSample& sample,
  bool captureBeforeFollower
  );
FollowerHeapDiagnosticSnapshot GetFollowerHeapDiagnostics();

class PerformanceScope
{
public:
  explicit PerformanceScope(PerformanceZone zone);
  ~PerformanceScope();

private:
  PerformanceScope(const PerformanceScope&);
  PerformanceScope& operator=(const PerformanceScope&);

  PerformanceZone m_Zone;
  PerformanceTick m_StartTick;
};

#define FOLLOWER_PERF_SCOPE(name, zone) \
  ::Gen7Follower3gx::PerformanceScope name(zone)
#define FOLLOWER_PERF_QUERY_BEGIN(name) \
  const ::Gen7Follower3gx::PerformanceTick name = \
    ::Gen7Follower3gx::PerformanceReadTick()
#define FOLLOWER_PERF_QUERY_END(counter, name) \
  ::Gen7Follower3gx::PerformanceRecordQuery(counter, name)

#else

PerformanceOptionMask GetTransientPerformanceOptions();
void ClearTransientPerformanceOptions();

inline void InitializePerformanceDiagnostics()
{
  ClearTransientPerformanceOptions();
}

inline void ShutdownPerformanceDiagnostics()
{
  ClearTransientPerformanceOptions();
}

inline PerformanceTestMode GetPerformanceTestMode()
{
  return PERFORMANCE_TEST_NORMAL;
}

inline PerformanceOptionMask GetPerformanceOptions()
{
  return FilterSupportedPerformanceOptions(
    GetSavedPerformanceOptions() | GetTransientPerformanceOptions()
    );
}

inline void SetPerformanceOptions(PerformanceOptionMask options)
{
  SetSavedPerformanceOptions(FilterSupportedPerformanceOptions(options));
}

inline bool IsPerformanceOptionEnabled(PerformanceOption option)
{
  return HasPerformanceOption(GetPerformanceOptions(), option);
}

inline void SetPerformanceOptionEnabled(
  PerformanceOption option,
  bool enabled
)
{
  const PerformanceOptionMask bit = GetPerformanceOptionBit(option);
  if (!bit)
  {
    return;
  }
  const PerformanceOptionMask options = FilterSupportedPerformanceOptions(
    GetSavedPerformanceOptions()
    );
  SetPerformanceOptions(enabled ? options | bit : options & ~bit);
}

inline void ResetPerformanceOptions()
{
  SetPerformanceOptions(0U);
}

inline void SetPerformanceOverlayVisible(bool)
{
}

inline bool IsPerformanceOverlayVisible()
{
  return false;
}

inline void RecordBallTransitionStart(
  BallTransitionDiagnosticKind,
  BallTransitionStartResult,
  unsigned int,
  unsigned int,
  unsigned int,
  bool,
  unsigned int,
  unsigned int,
  unsigned int,
  unsigned int,
  unsigned int
)
{
}

inline void SetFollowerSceneGraphDiagnostics(
  unsigned int,
  unsigned int,
  unsigned int
)
{
}

inline FollowerSceneGraphDiagnosticSnapshot
GetFollowerSceneGraphDiagnostics()
{
  FollowerSceneGraphDiagnosticSnapshot snapshot = {};
  return snapshot;
}

inline void UpdateFollowerGroundDiagnostics(
  const FollowerGroundDiagnosticSnapshot&
  )
{
}

inline FollowerGroundDiagnosticSnapshot GetFollowerGroundDiagnostics()
{
  FollowerGroundDiagnosticSnapshot snapshot = {};
  return snapshot;
}

inline void UpdateFollowerInteractionDiagnostics(
  const FollowerInteractionDiagnosticSnapshot&,
  bool,
  bool
)
{
}

inline FollowerInteractionDiagnosticSnapshot
GetFollowerInteractionDiagnostics()
{
  FollowerInteractionDiagnosticSnapshot snapshot = {};
  return snapshot;
}

inline void UpdateDittoTransformDiagnostics(
  const DittoTransformDiagnosticSnapshot&,
  bool,
  bool
)
{
}

inline DittoTransformDiagnosticSnapshot GetDittoTransformDiagnostics()
{
  DittoTransformDiagnosticSnapshot snapshot = {};
  return snapshot;
}

inline void UpdateCastformWeatherDiagnostics(
  const CastformWeatherDiagnosticSnapshot&,
  bool,
  bool
)
{
}

inline CastformWeatherDiagnosticSnapshot GetCastformWeatherDiagnostics()
{
  CastformWeatherDiagnosticSnapshot snapshot = {};
  return snapshot;
}

inline void RecordInteractionEffectStart(
  InteractionEffectStartResult,
  unsigned int,
  unsigned int,
  unsigned int,
  unsigned int,
  unsigned int,
  bool,
  unsigned int
)
{
}

inline InteractionEffectDiagnosticSnapshot GetInteractionEffectDiagnostics()
{
  InteractionEffectDiagnosticSnapshot snapshot = {};
  return snapshot;
}

#define FOLLOWER_PERF_SCOPE(name, zone) ((void)0)
#define FOLLOWER_PERF_QUERY_BEGIN(name) ((void)0)
#define FOLLOWER_PERF_QUERY_END(counter, name) ((void)0)

#endif

} // namespace Gen7Follower3gx
