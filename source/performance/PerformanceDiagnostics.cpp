#include "PerformanceDiagnostics.hpp"

#if FOLLOWER_3GX_DIAGNOSTIC

#include "DiagnosticEffectModule.hpp"
#include "FollowerSettings.hpp"
#include "VictiniLuck.hpp"

#include <CTRPluginFramework/Graphics/Color.hpp>
#include <CTRPluginFramework/Graphics/OSD.hpp>
#include <CTRPluginFramework/Menu/PluginMenu.hpp>
#include <CTRPluginFramework/System/System.hpp>

#include <stdio.h>
#include <string.h>

#include <string>

namespace Gen7Follower3gx
{
namespace
{

const u64 SYSTEM_TICKS_PER_SECOND = 268111856ULL;
const u64 DISPLAY_INTERVAL_TICKS = SYSTEM_TICKS_PER_SECOND / 4;
const u64 SLOW_FRAME_TICKS = SYSTEM_TICKS_PER_SECOND * 38 / 1000;
const unsigned int DISPLAY_LINE_COUNT = 23;
const unsigned int DISPLAY_RECT_HEIGHT = DISPLAY_LINE_COUNT * 10 + 4;

struct LiveMetrics
{
  u32 zoneTicks[PERFORMANCE_ZONE_COUNT];
  u32 counters[PERFORMANCE_COUNTER_COUNT];
};

struct PerformanceWindow
{
  u64 frameTicks;
  u64 zoneTicks[PERFORMANCE_ZONE_COUNT];
  u64 counters[PERFORMANCE_COUNTER_COUNT];
  u32 frameCount;
  u32 slowFrameCount;
  u32 maximumFollowerTicks;
};

LiveMetrics g_LiveMetrics;
PerformanceWindow g_Window;
BallTransitionDiagnosticSnapshot g_BallTransitionDiagnostics;
FollowerSceneGraphDiagnosticSnapshot g_FollowerSceneGraphDiagnostics;
FollowerGroundDiagnosticSnapshot g_FollowerGroundDiagnostics;
FollowerLifecycleDiagnosticSnapshot g_FollowerLifecycleDiagnostics;
FollowerInteractionDiagnosticSnapshot g_FollowerInteractionDiagnostics;
DittoTransformDiagnosticSnapshot g_DittoTransformDiagnostics;
CastformWeatherDiagnosticSnapshot g_CastformWeatherDiagnostics;
InteractionEffectDiagnosticSnapshot g_InteractionEffectDiagnostics;
FollowerHeapDiagnosticSnapshot g_FollowerHeapDiagnostics;
std::string g_Lines[DISPLAY_LINE_COUNT];
CTRPluginFramework::Color g_FpsColor(255, 255, 255);
u64 g_PreviousTopTick = 0;
bool g_Initialized = false;
bool g_OverlayVisible = false;
PerformanceOptionMask g_PerformanceOptions = 0;
PerformanceOptionMask g_TransientPerformanceOptions = 0;

u32 AtomicExchange(u32* value)
{
  return __atomic_exchange_n(value, 0U, __ATOMIC_RELAXED);
}

void AtomicAdd(u32* value, u32 amount)
{
  __atomic_fetch_add(value, amount, __ATOMIC_RELAXED);
}

void ClearLiveMetrics()
{
  for (u32 i = 0; i < PERFORMANCE_ZONE_COUNT; ++i)
  {
    __atomic_store_n(&g_LiveMetrics.zoneTicks[i], 0U, __ATOMIC_RELAXED);
  }
  for (u32 i = 0; i < PERFORMANCE_COUNTER_COUNT; ++i)
  {
    __atomic_store_n(&g_LiveMetrics.counters[i], 0U, __ATOMIC_RELAXED);
  }
}

void ClearWindow()
{
  memset(&g_Window, 0, sizeof(g_Window));
}

u32 TicksToHundredthsMilliseconds(u64 ticks)
{
  return static_cast<u32>(
    (ticks * 100000ULL + SYSTEM_TICKS_PER_SECOND / 2) /
    SYSTEM_TICKS_PER_SECOND
    );
}

u32 AverageTicksAsHundredthsMilliseconds(u64 ticks, u32 count)
{
  return count ? TicksToHundredthsMilliseconds(ticks / count) : 0;
}

u32 AverageCountAsTenths(u64 count, u32 frames)
{
  return frames
    ? static_cast<u32>((count * 10ULL + frames / 2) / frames)
    : 0;
}

void SetLine(unsigned int line, const char* text)
{
  if (line < DISPLAY_LINE_COUNT)
  {
    g_Lines[line].assign(text);
  }
}

const char* GetTestModeName(PerformanceTestMode mode)
{
  switch (mode)
  {
  case PERFORMANCE_TEST_NO_OCCLUDERS:
    return "NO OCCLUDERS";
  case PERFORMANCE_TEST_EMPTY_EDGE:
    return "EMPTY EDGE";
  case PERFORMANCE_TEST_ID_EDGE:
    return "ID ORIGINAL";
  case PERFORMANCE_TEST_ID_EDGE_SOFT:
    return "ID SOFT";
  case PERFORMANCE_TEST_ID_EDGE_MEDIUM:
    return "ID MEDIUM";
  case PERFORMANCE_TEST_NO_FOLLOWER_OUTLINE:
    return "NO EDGE";
  case PERFORMANCE_TEST_FOLLOWER_HIDDEN:
    return "HIDDEN";
  default:
    return "NORMAL";
  }
}

const char* GetBallTransitionKindName(unsigned int kind)
{
  return kind == BALL_TRANSITION_DIAGNOSTIC_RECALL ? "recall" : "spawn";
}

const char* GetDynamicEffectHeapSourceName(unsigned int source)
{
  switch (source)
  {
  case DYNAMIC_EFFECT_HEAP_POKEMON_MODEL:
    return "POKE";
  case DYNAMIC_EFFECT_HEAP_EVENT_DEVICE:
    return "EVT";
  case DYNAMIC_EFFECT_HEAP_EXTERNAL_APPLICATION:
    return "EXT";
  default:
    return "NONE";
  }
}

const char* GetBallTransitionResultName(unsigned int result)
{
  switch (result)
  {
  case BALL_TRANSITION_START_STARTED:
    return "STARTED";
  case BALL_TRANSITION_START_FOLLOWER_USES_EVENT_HEAP:
    return "FOLLOWER_EVT_HEAP";
  case BALL_TRANSITION_START_NO_EFFECT_MANAGER:
    return "NO_EFFECT_MGR";
  case BALL_TRANSITION_START_NO_MODEL:
    return "NO_MODEL";
  case BALL_TRANSITION_START_NO_RESOURCE_HEAP:
    return "NO_HEAP";
  case BALL_TRANSITION_START_BUSY:
    return "BUSY";
  case BALL_TRANSITION_START_RESOURCE_ALREADY_LOADED:
    return "RESOURCE_OWNED";
  case BALL_TRANSITION_START_RESOURCE_HEAP_LOW:
    return "HEAP_LOW";
  case BALL_TRANSITION_START_LOAD_FAILED:
    return "LOAD_FAILED";
  case BALL_TRANSITION_START_NO_EFFECT_SLOT:
    return "NO_EFFECT_SLOT";
  case BALL_TRANSITION_START_CREATE_FAILED:
    return "CREATE_FAILED";
  default:
    return "NOT_ATTEMPTED";
  }
}

const char* GetInteractionEffectResultName(unsigned int result)
{
  switch (result)
  {
  case INTERACTION_EFFECT_START_STARTED:
    return "STARTED";
  case INTERACTION_EFFECT_START_PERFORMANCE_DISABLED:
    return "PERF_OFF";
  case INTERACTION_EFFECT_START_FOLLOWER_USES_EVENT_HEAP:
    return "FOLLOWER_EVT_HEAP";
  case INTERACTION_EFFECT_START_NO_EFFECT_MANAGER:
    return "NO_EFFECT_MGR";
  case INTERACTION_EFFECT_START_NO_MODEL:
    return "NO_MODEL";
  case INTERACTION_EFFECT_START_NO_RESOURCE_HEAP:
    return "NO_HEAP";
  case INTERACTION_EFFECT_START_BUSY:
    return "BUSY";
  case INTERACTION_EFFECT_START_RESOURCE_ALREADY_LOADED:
    return "RESOURCE_OWNED";
  case INTERACTION_EFFECT_START_RESOURCE_HEAP_LOW:
    return "HEAP_LOW";
  case INTERACTION_EFFECT_START_LOAD_FAILED:
    return "LOAD_FAILED";
  case INTERACTION_EFFECT_START_NO_EFFECT_SLOT:
    return "NO_EFFECT_SLOT";
  case INTERACTION_EFFECT_START_CREATE_FAILED:
    return "CREATE_FAILED";
  case INTERACTION_EFFECT_START_MODULE_UNSUPPORTED:
    return "MODULE_UNSUPPORTED";
  case INTERACTION_EFFECT_START_MODULE_DLL_HEAP_LOW:
    return "MODULE_DLL_LOW";
  case INTERACTION_EFFECT_START_MODULE_LOAD_FAILED:
    return "MODULE_LOAD_FAILED";
  default:
    return "NOT_ATTEMPTED";
  }
}

const char* GetEffectModuleRequirementName(unsigned int requirement)
{
  switch (requirement)
  {
  case DIAGNOSTIC_EFFECT_MODULE_FIELD_EFFECT_UNIQUE:
    return "UNIQUE";
  case DIAGNOSTIC_EFFECT_MODULE_FIELD_EFFECT_JOIN_FESTA:
    return "JOIN";
  default:
    return "NONE";
  }
}

const char* GetEffectModuleResultName(unsigned int result)
{
  switch (result)
  {
  case DIAGNOSTIC_EFFECT_MODULE_RESULT_NOT_REQUIRED:
    return "NOT_NEEDED";
  case DIAGNOSTIC_EFFECT_MODULE_RESULT_READY_RETAIL:
    return "RETAIL";
  case DIAGNOSTIC_EFFECT_MODULE_RESULT_READY_PLUGIN:
    return "OWNED";
  case DIAGNOSTIC_EFFECT_MODULE_RESULT_LOADED:
    return "LOADED";
  case DIAGNOSTIC_EFFECT_MODULE_RESULT_UNSUPPORTED:
    return "UNSUPPORTED";
  case DIAGNOSTIC_EFFECT_MODULE_RESULT_NO_FIELDMAP:
    return "NO_FIELD";
  case DIAGNOSTIC_EFFECT_MODULE_RESULT_NO_GAME_MANAGER:
    return "NO_GAME";
  case DIAGNOSTIC_EFFECT_MODULE_RESULT_NO_FILE_MANAGER:
    return "NO_FILE";
  case DIAGNOSTIC_EFFECT_MODULE_RESULT_NO_RO_MANAGER:
    return "NO_RO";
  case DIAGNOSTIC_EFFECT_MODULE_RESULT_NO_DLL_HEAP:
    return "NO_HEAP";
  case DIAGNOSTIC_EFFECT_MODULE_RESULT_DLL_HEAP_LOW:
    return "HEAP_LOW";
  case DIAGNOSTIC_EFFECT_MODULE_RESULT_LOAD_FAILED:
    return "LOAD_FAIL";
  case DIAGNOSTIC_EFFECT_MODULE_RESULT_RELEASED:
    return "RELEASED";
  default:
    return "NONE";
  }
}

const char* GetEffectWorkTypeName(unsigned int workType)
{
  switch (workType)
  {
  case 0:
    return "SYS";
  case 1:
    return "EVT";
  case 2:
    return "WTR";
  case 3:
    return "RID";
  default:
    return "NONE";
  }
}

const char* GetFollowerInteractionStateName(unsigned int state)
{
  switch (state)
  {
  case FOLLOWER_INTERACTION_STATE_LOADING:
    return "LOAD";
  case FOLLOWER_INTERACTION_STATE_READY:
    return "READY";
  case FOLLOWER_INTERACTION_STATE_PLAYING:
    return "PLAY";
  case FOLLOWER_INTERACTION_STATE_CANCELED:
    return "CANCEL";
  default:
    return "NONE";
  }
}

const char* GetFollowerLifecycleStateName(unsigned int state)
{
  switch (state)
  {
  case 0: return "IDLE";
  case 1: return "SUPPRESS";
  case 2: return "INIT";
  case 3: return "CREATE";
  case 4: return "MODEL_LOAD";
  case 5: return "ACTIVE";
  case 6: return "FORM_DELETE";
  case 7: return "DELETE";
  case 8: return "TERM";
  default: return "UNKNOWN";
  }
}

const char* GetFollowerTerminateBlockerName(unsigned int blocker)
{
  switch (blocker)
  {
  case 1: return "EFFECT";
  case 2: return "CAN_DELETE";
  case 3: return "NODE_REF";
  case 4: return "FACTORY";
  case 5: return "MODEL_LOAD";
  default: return "READY";
  }
}

const char* GetFollowerLoadFailureName(unsigned int failure)
{
  switch (failure)
  {
  case 1: return "PARENT_MISSING";
  case 2: return "PARENT_LOW";
  case 3: return "HEAP_CREATE";
  case 4: return "FACTORY_CREATE";
  case 5: return "MODEL_CREATE";
  case 6: return "EVENT_BUSY";
  case 7: return "EVENT_LOW";
  case 8: return "REPEL";
  default: return "NONE";
  }
}

const char* GetFollowerInteractionResultName(unsigned int result)
{
  switch (result)
  {
  case FOLLOWER_INTERACTION_RESULT_LOAD_STARTED:
    return "LOAD_STARTED";
  case FOLLOWER_INTERACTION_RESULT_RESPOND:
    return "RESPOND";
  case FOLLOWER_INTERACTION_RESULT_HAPPY_A:
    return "HAPPY_A";
  case FOLLOWER_INTERACTION_RESULT_HAPPY_B:
    return "HAPPY_B";
  case FOLLOWER_INTERACTION_RESULT_HAPPY_C:
    return "HAPPY_C";
  case FOLLOWER_INTERACTION_RESULT_FIELD_FALLBACK:
    return "FI_FALLBACK";
  case FOLLOWER_INTERACTION_RESULT_TOO_FAR:
    return "TOO_FAR";
  case FOLLOWER_INTERACTION_RESULT_MOVING:
    return "MOVING";
  case FOLLOWER_INTERACTION_RESULT_BUSY:
    return "BUSY";
  case FOLLOWER_INTERACTION_RESULT_EVENT_ACTIVE:
    return "EVENT";
  case FOLLOWER_INTERACTION_RESULT_NO_HEAP:
    return "NO_HEAP";
  case FOLLOWER_INTERACTION_RESULT_NO_SYSTEM:
    return "NO_SYSTEM";
  case FOLLOWER_INTERACTION_RESULT_LOAD_FAILED:
    return "LOAD_FAILED";
  case FOLLOWER_INTERACTION_RESULT_BAD_PACK:
    return "BAD_PACK";
  case FOLLOWER_INTERACTION_RESULT_NO_KW_MOTION:
    return "NO_KW";
  case FOLLOWER_INTERACTION_RESULT_CANCELED:
    return "CANCELED";
  default:
    return "NOT_ATTEMPTED";
  }
}

const char* GetFollowerHeapParentName(unsigned int source)
{
  switch (source)
  {
  case 1:
    return "EXT";
  case 2:
    return "AREA";
  case 3:
    return "FIELD";
  case 4:
    return "APP";
  case 5:
    return "EVENT";
  case 6:
    return "SHARED_APP";
  default:
    return "NONE";
  }
}

const char* GetDittoTransformStateName(unsigned int state)
{
  switch (state)
  {
  case DITTO_TRANSFORM_STATE_FLASH_IN:
    return "FLASH_IN";
  case DITTO_TRANSFORM_STATE_CLONE_HOLD:
    return "HOLD";
  case DITTO_TRANSFORM_STATE_FLASH_OUT:
    return "FLASH_OUT";
  case DITTO_TRANSFORM_STATE_CLEANUP:
    return "CLEANUP";
  default:
    return "NONE";
  }
}

const char* GetDittoTransformResultName(unsigned int result)
{
  switch (result)
  {
  case DITTO_TRANSFORM_RESULT_STARTED:
    return "STARTED";
  case DITTO_TRANSFORM_RESULT_ACTIVE:
    return "ACTIVE";
  case DITTO_TRANSFORM_RESULT_COMPLETE:
    return "COMPLETE";
  case DITTO_TRANSFORM_RESULT_NO_MANAGER:
    return "NO_MANAGER";
  case DITTO_TRANSFORM_RESULT_NO_PLAYER:
    return "NO_PLAYER";
  case DITTO_TRANSFORM_RESULT_NO_DRESS:
    return "NO_DRESS";
  case DITTO_TRANSFORM_RESULT_NO_SLOT:
    return "NO_SLOT";
  case DITTO_TRANSFORM_RESULT_NO_HEAP:
    return "NO_HEAP";
  case DITTO_TRANSFORM_RESULT_HEAP_CREATE_FAILED:
    return "HEAP_FAILED";
  case DITTO_TRANSFORM_RESULT_WORK_FAILED:
    return "WORK_FAILED";
  case DITTO_TRANSFORM_RESULT_FLASH_FAILED:
    return "FLASH_FAILED";
  case DITTO_TRANSFORM_RESULT_CLONE_FAILED:
    return "CLONE_FAILED";
  case DITTO_TRANSFORM_RESULT_CANCELED:
    return "CANCELED";
  default:
    return "NOT_ATTEMPTED";
  }
}

const char* GetDittoCloneMotionPhaseName(unsigned int phase)
{
  switch (phase)
  {
  case DITTO_CLONE_MOTION_FALLBACK:
    return "FALLBACK";
  case DITTO_CLONE_MOTION_POSE:
    return "POSE";
  case DITTO_CLONE_MOTION_RECOVER:
    return "END";
  default:
    return "NONE";
  }
}

const char* GetCastformWeatherResultName(unsigned int result)
{
  switch (result)
  {
  case CASTFORM_WEATHER_RESULT_STARTED:
    return "STARTED";
  case CASTFORM_WEATHER_RESULT_PERFORMANCE_DISABLED:
    return "PERF_OFF";
  case CASTFORM_WEATHER_RESULT_NOT_OUTDOORS:
    return "INDOOR";
  case CASTFORM_WEATHER_RESULT_NO_CONTROL:
    return "NO_CONTROL";
  case CASTFORM_WEATHER_RESULT_INVALID_CONTROL:
    return "BAD_CONTROL";
  case CASTFORM_WEATHER_RESULT_INVALID_KIND:
    return "BAD_KIND";
  case CASTFORM_WEATHER_RESULT_BUSY:
    return "BUSY";
  case CASTFORM_WEATHER_RESULT_APPLY_FAILED:
    return "APPLY_FAIL";
  case CASTFORM_WEATHER_RESULT_RESTORED:
    return "RESTORED";
  default:
    return "NOT_ATTEMPTED";
  }
}

const char* GetFieldWeatherKindName(unsigned int weatherKind)
{
  switch (weatherKind)
  {
  case 0:
    return "SUN";
  case 1:
    return "CLOUD";
  case 2:
    return "RAIN";
  case 3:
    return "STORM";
  case 4:
    return "SNOW";
  case 5:
    return "BLIZZARD";
  case 6:
    return "DRY";
  case 7:
    return "SAND";
  case 8:
    return "MIST";
  case 9:
    return "SUNRAIN";
  case 10:
    return "DIAMOND";
  case 0xffffffffU:
    return "MAP";
  default:
    return "NONE";
  }
}

const char* GetFollowerInteractionMotionName(unsigned int motion)
{
  switch (motion)
  {
  case FOLLOWER_INTERACTION_MOTION_RESPOND:
    return "RESP";
  case FOLLOWER_INTERACTION_MOTION_HAPPY_A:
    return "HAP_A";
  case FOLLOWER_INTERACTION_MOTION_HAPPY_B:
    return "HAP_B";
  case FOLLOWER_INTERACTION_MOTION_HAPPY_C:
    return "HAP_C";
  case FOLLOWER_INTERACTION_MOTION_FIELD:
    return "FI";
  default:
    return "NONE";
  }
}

void PublishWindow()
{
  if (!g_Window.frameCount || !g_Window.frameTicks)
  {
    return;
  }

  const u32 fpsTenths = static_cast<u32>(
    (static_cast<u64>(g_Window.frameCount) *
      SYSTEM_TICKS_PER_SECOND * 10ULL + g_Window.frameTicks / 2) /
    g_Window.frameTicks
    );
  const u32 frameHundredths = AverageTicksAsHundredthsMilliseconds(
    g_Window.frameTicks,
    g_Window.frameCount
    );
  const u32 followerHundredths = AverageTicksAsHundredthsMilliseconds(
    g_Window.zoneTicks[PERFORMANCE_ZONE_FOLLOWER],
    g_Window.frameCount
    );
  const u32 followerMaximumHundredths = TicksToHundredthsMilliseconds(
    g_Window.maximumFollowerTicks
    );
  const u32 collisionHundredths = AverageTicksAsHundredthsMilliseconds(
    g_Window.zoneTicks[PERFORMANCE_ZONE_COLLISION],
    g_Window.frameCount
    );
  const u32 modelHundredths = AverageTicksAsHundredthsMilliseconds(
    g_Window.zoneTicks[PERFORMANCE_ZONE_MODEL],
    g_Window.frameCount
    );
  const u32 outlineHundredths = AverageTicksAsHundredthsMilliseconds(
    g_Window.zoneTicks[PERFORMANCE_ZONE_OUTLINE],
    g_Window.frameCount
    );
  const u32 traversalHundredths = AverageTicksAsHundredthsMilliseconds(
    g_Window.zoneTicks[PERFORMANCE_ZONE_TRAVERSAL],
    g_Window.frameCount
    );
  const u32 overlayHundredths = AverageTicksAsHundredthsMilliseconds(
    g_Window.zoneTicks[PERFORMANCE_ZONE_OVERLAY],
    g_Window.frameCount
    );

  const u64 measuredSubregions =
    g_Window.zoneTicks[PERFORMANCE_ZONE_COLLISION] +
    g_Window.zoneTicks[PERFORMANCE_ZONE_MODEL] +
    g_Window.zoneTicks[PERFORMANCE_ZONE_TRAVERSAL] +
    g_Window.zoneTicks[PERFORMANCE_ZONE_OUTLINE];
  const u64 otherTicks =
    g_Window.zoneTicks[PERFORMANCE_ZONE_FOLLOWER] > measuredSubregions
      ? g_Window.zoneTicks[PERFORMANCE_ZONE_FOLLOWER] - measuredSubregions
      : 0;
  const u32 otherHundredths = AverageTicksAsHundredthsMilliseconds(
    otherTicks,
    g_Window.frameCount
    );

  const u32 groundTenths = AverageCountAsTenths(
    g_Window.counters[PERFORMANCE_COUNTER_GROUND_MESH],
    g_Window.frameCount
    );
  const u64 wallCount =
    g_Window.counters[PERFORMANCE_COUNTER_WALL_MESH] +
    g_Window.counters[PERFORMANCE_COUNTER_WALL_CYLINDER] +
    g_Window.counters[PERFORMANCE_COUNTER_WALL_BOX];
  const u32 wallTenths = AverageCountAsTenths(
    wallCount,
    g_Window.frameCount
    );
  const u32 wallMeshTenths = AverageCountAsTenths(
    g_Window.counters[PERFORMANCE_COUNTER_WALL_MESH],
    g_Window.frameCount
    );
  const u32 wallCylinderTenths = AverageCountAsTenths(
    g_Window.counters[PERFORMANCE_COUNTER_WALL_CYLINDER],
    g_Window.frameCount
    );
  const u32 wallBoxTenths = AverageCountAsTenths(
    g_Window.counters[PERFORMANCE_COUNTER_WALL_BOX],
    g_Window.frameCount
    );

  char line[64];
  snprintf(
    line,
    sizeof(line),
    "%s  FPS %lu.%lu  %lu.%02lums  slow %lu/%lu",
    GetTestModeName(GetPerformanceTestMode()),
    fpsTenths / 10,
    fpsTenths % 10,
    frameHundredths / 100,
    frameHundredths % 100,
    g_Window.slowFrameCount,
    g_Window.frameCount
    );
  SetLine(0, line);

  snprintf(
    line,
    sizeof(line),
    "Follower %lu.%02lums  max %lu.%02lu",
    followerHundredths / 100,
    followerHundredths % 100,
    followerMaximumHundredths / 100,
    followerMaximumHundredths % 100
    );
  SetLine(1, line);

  snprintf(
    line,
    sizeof(line),
    "Collision %lu.%02lums  G %lu.%lu W %lu.%lu",
    collisionHundredths / 100,
    collisionHundredths % 100,
    groundTenths / 10,
    groundTenths % 10,
    wallTenths / 10,
    wallTenths % 10
    );
  SetLine(2, line);

  snprintf(
    line,
    sizeof(line),
    "Queries M %lu.%lu C %lu.%lu B %lu.%lu",
    wallMeshTenths / 10,
    wallMeshTenths % 10,
    wallCylinderTenths / 10,
    wallCylinderTenths % 10,
    wallBoxTenths / 10,
    wallBoxTenths % 10
    );
  SetLine(3, line);

  snprintf(
    line,
    sizeof(line),
    "Model %lu.%02lu  edge %lu.%02lu  other %lu.%02lu",
    modelHundredths / 100,
    modelHundredths % 100,
    outlineHundredths / 100,
    outlineHundredths % 100,
    otherHundredths / 100,
    otherHundredths % 100
    );
  SetLine(4, line);

  const FollowerSceneGraphDiagnosticSnapshot sceneGraph =
    GetFollowerSceneGraphDiagnostics();
  snprintf(
    line,
    sizeof(line),
    "Traverse %lu.%02lums  J%u R%u A%u",
    traversalHundredths / 100,
    traversalHundredths % 100,
    sceneGraph.totalJoints,
    sceneGraph.renderJoints,
    sceneGraph.requiredJoints
    );
  SetLine(5, line);

  snprintf(
    line,
    sizeof(line),
    "%s  options %03lx  profiler %lu.%02lums",
    CTRPluginFramework::System::IsNew3DS() ? "N3DS" : "O3DS",
    static_cast<unsigned long>(GetPerformanceOptions()),
    overlayHundredths / 100,
    overlayHundredths % 100
    );
  SetLine(6, line);

  const BallTransitionDiagnosticSnapshot ball =
    GetBallTransitionDiagnostics();
  const VictiniLuckDiagnosticSnapshot luck =
    GetVictiniLuckDiagnostics();
  if (luck.active)
  {
    snprintf(
      line,
      sizeof(line),
      "Luck %u.%us R%u H%u E%u P%u S%u %u>%u",
      luck.remainingTenths / 10,
      luck.remainingTenths % 10,
      luck.targetRolls,
      luck.hooksEnabled,
      luck.encounterCount,
      luck.pokemonCount,
      luck.intruderCount,
      luck.previousRolls,
      luck.boostedRolls
      );
  }
  else
  {
    snprintf(
      line,
      sizeof(line),
      "Ball %s: %s  try %u ok %u",
      GetBallTransitionKindName(ball.kind),
      GetBallTransitionResultName(ball.result),
      ball.attemptCount,
      ball.successCount
      );
  }
  SetLine(7, line);

  snprintf(
    line,
    sizeof(line),
    "Heap %s free %08x guard %x pre%u",
    GetDynamicEffectHeapSourceName(ball.resourceHeapSource),
    ball.resourceHeapFree,
    ball.minimumHeapFree,
    ball.resourceWasLoaded
    );
  SetLine(8, line);

  snprintf(
    line,
    sizeof(line),
    "Slots S%u E%u W%u R%u  use %s",
    ball.systemSlotsFree,
    ball.eventSlotsFree,
    ball.weatherSlotsFree,
    ball.rideSlotsFree,
    GetEffectWorkTypeName(ball.selectedWorkType)
    );
  SetLine(9, line);

  const FollowerInteractionDiagnosticSnapshot interaction =
    GetFollowerInteractionDiagnostics();
  snprintf(
    line,
    sizeof(line),
    "Interact %s: %s  try %u ok %u",
    GetFollowerInteractionStateName(interaction.state),
    GetFollowerInteractionResultName(interaction.result),
    interaction.attemptCount,
    interaction.successCount
    );
  SetLine(10, line);

  snprintf(
    line,
    sizeof(line),
    "KW d%u h%x %x/%x n%u R%u H%c%c%c %s",
    interaction.dataId,
    interaction.heapFree,
    interaction.bufferSize,
    interaction.realSize,
    interaction.resourceCount,
    interaction.respondAvailable,
    interaction.happyAvailable & 1U ? 'A' : '-',
    interaction.happyAvailable & 2U ? 'B' : '-',
    interaction.happyAvailable & 4U ? 'C' : '-',
    GetFollowerInteractionMotionName(interaction.selectedMotion)
    );
  SetLine(11, line);

  const DittoTransformDiagnosticSnapshot ditto =
    GetDittoTransformDiagnostics();
  snprintf(
    line,
    sizeof(line),
    "Ditto %s %s m%u %s %u/%u h%x/%x",
    GetDittoTransformStateName(ditto.state),
    GetDittoTransformResultName(ditto.result),
    ditto.motionId,
    GetDittoCloneMotionPhaseName(ditto.motionPhase),
    ditto.motionFrame,
    ditto.motionEndFrame,
    ditto.parentHeapFree,
    ditto.cloneHeapFree
    );
  SetLine(12, line);

  const CastformWeatherDiagnosticSnapshot castform =
    GetCastformWeatherDiagnostics();
  snprintf(
    line,
    sizeof(line),
    "Weather %s %s p%d f%d try%u ok%u",
    GetFieldWeatherKindName(castform.requestedWeather),
    GetCastformWeatherResultName(castform.result),
    castform.previousWeather,
    castform.previousForceWeather,
    castform.attemptCount,
    castform.successCount
    );
  SetLine(13, line);

  const InteractionEffectDiagnosticSnapshot effect =
    GetInteractionEffectDiagnostics();
  snprintf(
    line,
    sizeof(line),
    "Fx%u %s %s h%x/%x g%x p%u %s %u/%u",
    effect.effectType,
    GetInteractionEffectResultName(effect.result),
    GetDynamicEffectHeapSourceName(effect.resourceHeapSource),
    effect.resourceHeapFreeBefore,
    effect.resourceHeapFreeAfter,
    effect.minimumHeapFree,
    effect.resourceWasLoaded,
    GetEffectWorkTypeName(effect.selectedWorkType),
    effect.attemptCount,
    effect.successCount
    );
  SetLine(14, line);

  const DiagnosticEffectModuleSnapshot effectModule =
    GetDiagnosticEffectModuleSnapshot();
  snprintf(
    line,
    sizeof(line),
    "Module %s %s d%x/%x own%u %u/%u",
    GetEffectModuleRequirementName(effectModule.requirement),
    GetEffectModuleResultName(effectModule.result),
    effectModule.dllHeapFreeBefore,
    effectModule.dllHeapFreeAfter,
    effectModule.ownsModule,
    effectModule.attemptCount,
    effectModule.successCount
    );
  SetLine(15, line);

  const FollowerHeapDiagnosticSnapshot heaps =
    GetFollowerHeapDiagnostics();
  snprintf(
    line,
    sizeof(line),
    "Heap parent %s model %s capture %u",
    GetFollowerHeapParentName(heaps.current.parentSource),
    heaps.current.followerUsesEventHeap ? "EVT" : "LOCAL",
    heaps.captureCount
    );
  SetLine(16, line);

  static const char* const heapNames[FOLLOWER_HEAP_DIAGNOSTIC_COUNT] =
  {
    "Area",
    "App",
    "Event",
    "DLL",
    "Poke",
  };
  for (u32 i = 0; i < FOLLOWER_HEAP_DIAGNOSTIC_COUNT; ++i)
  {
    if (i == FOLLOWER_HEAP_DIAGNOSTIC_POKEMON_MODEL)
    {
      const FollowerHeapDiagnosticMeasurement& model =
        heaps.current.heaps[i];
      const unsigned int used = model.totalSize >= model.totalFree
        ? model.totalSize - model.totalFree
        : 0;
      snprintf(
        line,
        sizeof(line),
        "%s T%x U%x F%x M%x",
        heapNames[i],
        model.totalSize,
        used,
        model.totalFree,
        model.maximumAllocatable
        );
    }
    else
    {
      snprintf(
        line,
        sizeof(line),
        "%s F%x>%x M%x>%x",
        heapNames[i],
        heaps.beforeFollower.heaps[i].totalFree,
        heaps.current.heaps[i].totalFree,
        heaps.beforeFollower.heaps[i].maximumAllocatable,
        heaps.current.heaps[i].maximumAllocatable
        );
    }
    SetLine(17 + i, line);
  }

  const FollowerGroundDiagnosticSnapshot ground =
    GetFollowerGroundDiagnostics();
  const FollowerLifecycleDiagnosticSnapshot lifecycle =
    GetFollowerLifecycleDiagnostics();
  if (lifecycle.state != 5U || lifecycle.terminateBlocker != 0U)
  {
    snprintf(
      line,
      sizeof(line),
      "Lifecycle %s fail %s wait %s",
      GetFollowerLifecycleStateName(lifecycle.state),
      GetFollowerLoadFailureName(lifecycle.loadFailure),
      GetFollowerTerminateBlockerName(lifecycle.terminateBlocker)
      );
  }
  else
  {
    snprintf(
      line,
      sizeof(line),
      "Yx10 S%u/%u M%d B%d R%d A%d C%d F%d",
      ground.species,
      ground.form,
      ground.motion,
      ground.bodyRadiusTenths,
      ground.rawRootYTenths,
      ground.appliedRootYTenths,
      ground.cmHeight,
      ground.fieldAdjustHeight
      );
  }
  SetLine(22, line);

  if (fpsTenths >= 295)
  {
    g_FpsColor = CTRPluginFramework::Color(128, 255, 160);
  }
  else if (fpsTenths >= 270)
  {
    g_FpsColor = CTRPluginFramework::Color(255, 224, 96);
  }
  else
  {
    g_FpsColor = CTRPluginFramework::Color(255, 112, 112);
  }
}

void AccumulateFrame(u64 frameTicks)
{
  u32 frameZoneTicks[PERFORMANCE_ZONE_COUNT];
  for (u32 i = 0; i < PERFORMANCE_ZONE_COUNT; ++i)
  {
    frameZoneTicks[i] = AtomicExchange(&g_LiveMetrics.zoneTicks[i]);
    g_Window.zoneTicks[i] += frameZoneTicks[i];
  }
  for (u32 i = 0; i < PERFORMANCE_COUNTER_COUNT; ++i)
  {
    g_Window.counters[i] += AtomicExchange(&g_LiveMetrics.counters[i]);
  }

  g_Window.frameTicks += frameTicks;
  ++g_Window.frameCount;
  if (frameTicks > SLOW_FRAME_TICKS)
  {
    ++g_Window.slowFrameCount;
  }
  if (frameZoneTicks[PERFORMANCE_ZONE_FOLLOWER] >
      g_Window.maximumFollowerTicks)
  {
    g_Window.maximumFollowerTicks =
      frameZoneTicks[PERFORMANCE_ZONE_FOLLOWER];
  }

  if (!g_OverlayVisible &&
      g_Window.frameTicks >= DISPLAY_INTERVAL_TICKS)
  {
    PublishWindow();
    ClearWindow();
  }
}

bool PerformanceOverlayCallback(const CTRPluginFramework::Screen& screen)
{
  if (!screen.IsTop || !g_Initialized)
  {
    return false;
  }

  const u64 overlayStartTick = svcGetSystemTick();
  CTRPluginFramework::PluginMenu* menu =
    CTRPluginFramework::PluginMenu::GetRunningInstance();
  if (menu && menu->IsOpen())
  {
    g_PreviousTopTick = 0;
    ClearLiveMetrics();
    return false;
  }

  if (g_PreviousTopTick)
  {
    AccumulateFrame(overlayStartTick - g_PreviousTopTick);
  }
  else
  {
    ClearLiveMetrics();
  }
  g_PreviousTopTick = overlayStartTick;

  if (!g_OverlayVisible)
  {
    PerformanceRecordTicks(PERFORMANCE_ZONE_OVERLAY, overlayStartTick);
    return false;
  }

  screen.DrawRect(
    2,
    2,
    316,
    DISPLAY_RECT_HEIGHT,
    CTRPluginFramework::Color(0, 0, 0, 176),
    true
    );
  const CTRPluginFramework::Color transparent(0, 0, 0, 0);
  for (u32 i = 0; i < DISPLAY_LINE_COUNT; ++i)
  {
    const CTRPluginFramework::Color& foreground =
      i == 0 ? g_FpsColor : CTRPluginFramework::Color::White;
    screen.Draw(g_Lines[i], 5, 4 + i * 10, foreground, transparent);
  }

  PerformanceRecordTicks(PERFORMANCE_ZONE_OVERLAY, overlayStartTick);
  return false;
}

} // namespace

PerformanceTick PerformanceReadTick()
{
  return svcGetSystemTick();
}

void PerformanceRecordTicks(
  PerformanceZone zone,
  PerformanceTick startTick
)
{
  if (static_cast<u32>(zone) >= PERFORMANCE_ZONE_COUNT)
  {
    return;
  }
  AtomicAdd(
    &g_LiveMetrics.zoneTicks[zone],
    static_cast<u32>(svcGetSystemTick() - startTick)
    );
}

void PerformanceRecordQuery(
  PerformanceCounter counter,
  PerformanceTick startTick
)
{
  if (static_cast<u32>(counter) >= PERFORMANCE_COUNTER_COUNT)
  {
    return;
  }
  PerformanceRecordTicks(PERFORMANCE_ZONE_COLLISION, startTick);
  AtomicAdd(&g_LiveMetrics.counters[counter], 1U);
}

PerformanceScope::PerformanceScope(PerformanceZone zone)
: m_Zone(zone)
, m_StartTick(svcGetSystemTick())
{
}

PerformanceScope::~PerformanceScope()
{
  PerformanceRecordTicks(m_Zone, m_StartTick);
}

void InitializePerformanceDiagnostics()
{
  if (g_Initialized)
  {
    return;
  }

  ClearLiveMetrics();
  ClearWindow();
  memset(
    &g_BallTransitionDiagnostics,
    0,
    sizeof(g_BallTransitionDiagnostics)
    );
  memset(
    &g_FollowerSceneGraphDiagnostics,
    0,
    sizeof(g_FollowerSceneGraphDiagnostics)
    );
  memset(
    &g_FollowerGroundDiagnostics,
    0,
    sizeof(g_FollowerGroundDiagnostics)
    );
  memset(
    &g_FollowerLifecycleDiagnostics,
    0,
    sizeof(g_FollowerLifecycleDiagnostics)
    );
  memset(
    &g_FollowerInteractionDiagnostics,
    0,
    sizeof(g_FollowerInteractionDiagnostics)
    );
  memset(
    &g_DittoTransformDiagnostics,
    0,
    sizeof(g_DittoTransformDiagnostics)
    );
  memset(
    &g_CastformWeatherDiagnostics,
    0,
    sizeof(g_CastformWeatherDiagnostics)
    );
  memset(
    &g_InteractionEffectDiagnostics,
    0,
    sizeof(g_InteractionEffectDiagnostics)
    );
  memset(
    &g_FollowerHeapDiagnostics,
    0,
    sizeof(g_FollowerHeapDiagnostics)
    );
  g_CastformWeatherDiagnostics.requestedWeather = 0xffffffffU;
  g_CastformWeatherDiagnostics.previousWeather = -1;
  g_CastformWeatherDiagnostics.previousForceWeather = -1;
  g_InteractionEffectDiagnostics.effectType = 79;
  g_InteractionEffectDiagnostics.minimumHeapFree = 0x20000;
  g_DittoTransformDiagnostics.modelId = 35;
  g_PreviousTopTick = 0;
  g_OverlayVisible = false;
  __atomic_store_n(
    &g_PerformanceOptions,
    FilterSupportedPerformanceOptions(GetSavedPerformanceOptions()),
    __ATOMIC_RELAXED
    );
  __atomic_store_n(
    &g_TransientPerformanceOptions,
    0U,
    __ATOMIC_RELAXED
    );
  for (u32 i = 0; i < DISPLAY_LINE_COUNT; ++i)
  {
    g_Lines[i].reserve(64);
    g_Lines[i].clear();
  }
  SetLine(0, "Follower profiler warming up...");
  SetLine(1, "Follower 0.00ms  max 0.00");
  SetLine(2, "Collision 0.00ms  G 0.0 W 0.0");
  SetLine(3, "Queries M 0.0 C 0.0 B 0.0");
  SetLine(4, "Model 0.00  edge 0.00  other 0.00");
  SetLine(5, "Traverse 0.00ms  J0 R0 A0");
  SetLine(6, "Profiler 0.00ms");
  SetLine(7, "Ball spawn: NOT_ATTEMPTED  try 0 ok 0");
  SetLine(8, "Heap NONE free 00000000 guard 0 pre0");
  SetLine(9, "Slots S0 E0 W0 R0  use NONE");
  SetLine(10, "Interact NONE: NOT_ATTEMPTED  try 0 ok 0");
  SetLine(11, "KW d0 h0 0/0 n0 R0H0 NONE");
  SetLine(12, "Ditto NONE NOT_ATTEMPTED m0 NONE 0/0 h0/0");
  SetLine(13, "Cast NONE NOT_ATTEMPTED p-1 f-1 try0 ok0");
  SetLine(14, "Fx79 NOT_ATTEMPTED NONE h0/0 g20000 p0 NONE 0/0");
  SetLine(15, "Module NONE NONE d0/0 own0 0/0");
  SetLine(16, "Heap parent NONE model LOCAL capture 0");
  SetLine(17, "Area F0>0 M0>0");
  SetLine(18, "App F0>0 M0>0");
  SetLine(19, "Event F0>0 M0>0");
  SetLine(20, "DLL F0>0 M0>0");
  SetLine(21, "Poke T0 U0 F0 M0");
  SetLine(22, "Yx10 S0/0 M-1 B0 R0 A0 C0 F0");
  g_Initialized = true;
  CTRPluginFramework::OSD::Run(PerformanceOverlayCallback);
}

PerformanceTestMode GetPerformanceTestMode()
{
  switch (GetFollowerOutlineMode())
  {
  case FOLLOWER_OUTLINE_ID_MEDIUM:
    return PERFORMANCE_TEST_ID_EDGE_MEDIUM;
  case FOLLOWER_OUTLINE_ID_ORIGINAL:
    return PERFORMANCE_TEST_ID_EDGE;
  case FOLLOWER_OUTLINE_OFF:
    return PERFORMANCE_TEST_NO_FOLLOWER_OUTLINE;
  default:
    return PERFORMANCE_TEST_ID_EDGE_SOFT;
  }
}

PerformanceOptionMask GetPerformanceOptions()
{
  const PerformanceOptionMask saved = __atomic_load_n(
    &g_PerformanceOptions,
    __ATOMIC_RELAXED
    );
  const PerformanceOptionMask transient = __atomic_load_n(
    &g_TransientPerformanceOptions,
    __ATOMIC_RELAXED
    );
  return FilterSupportedPerformanceOptions(saved | transient);
}

void SetPerformanceOptions(PerformanceOptionMask options)
{
  options = FilterSupportedPerformanceOptions(options);
  if (__atomic_exchange_n(
        &g_PerformanceOptions,
        options,
        __ATOMIC_RELAXED
        ) == options)
  {
    return;
  }
  SetSavedPerformanceOptions(options);
  ClearLiveMetrics();
  ClearWindow();
  g_PreviousTopTick = 0;
}

bool IsPerformanceOptionEnabled(PerformanceOption option)
{
  return HasPerformanceOption(GetPerformanceOptions(), option);
}

void SetPerformanceOptionEnabled(PerformanceOption option, bool enabled)
{
  const PerformanceOptionMask bit = GetPerformanceOptionBit(option);
  if (bit == 0)
  {
    return;
  }

  const PerformanceOptionMask options = __atomic_load_n(
    &g_PerformanceOptions,
    __ATOMIC_RELAXED
    );
  SetPerformanceOptions(enabled ? options | bit : options & ~bit);
}

void SetTransientPerformanceOptionEnabled(
  PerformanceOption option,
  bool enabled
  )
{
  const PerformanceOptionMask bit = GetPerformanceOptionBit(option);
  if (bit == 0)
  {
    return;
  }

  if (enabled)
  {
    __atomic_fetch_or(
      &g_TransientPerformanceOptions,
      bit,
      __ATOMIC_RELAXED
      );
  }
  else
  {
    __atomic_fetch_and(
      &g_TransientPerformanceOptions,
      ~bit,
      __ATOMIC_RELAXED
      );
  }
}

void ResetPerformanceOptions()
{
  SetPerformanceOptions(0U);
}

void SetPerformanceOverlayVisible(bool visible)
{
  if (g_OverlayVisible == visible)
  {
    return;
  }
  g_OverlayVisible = visible;
  ClearLiveMetrics();
  ClearWindow();
  g_PreviousTopTick = 0;
}

bool IsPerformanceOverlayVisible()
{
  return g_OverlayVisible;
}

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
)
{
  __atomic_store_n(
    &g_BallTransitionDiagnostics.kind,
    static_cast<unsigned int>(kind),
    __ATOMIC_RELAXED
    );
  __atomic_store_n(
    &g_BallTransitionDiagnostics.result,
    static_cast<unsigned int>(result),
    __ATOMIC_RELAXED
    );
  __atomic_store_n(
    &g_BallTransitionDiagnostics.resourceHeapSource,
    resourceHeapSource,
    __ATOMIC_RELAXED
    );
  __atomic_store_n(
    &g_BallTransitionDiagnostics.resourceHeapFree,
    resourceHeapFree,
    __ATOMIC_RELAXED
    );
  __atomic_store_n(
    &g_BallTransitionDiagnostics.minimumHeapFree,
    minimumHeapFree,
    __ATOMIC_RELAXED
    );
  __atomic_store_n(
    &g_BallTransitionDiagnostics.resourceWasLoaded,
    resourceWasLoaded ? 1U : 0U,
    __ATOMIC_RELAXED
    );
  __atomic_store_n(
    &g_BallTransitionDiagnostics.systemSlotsFree,
    systemSlotsFree,
    __ATOMIC_RELAXED
    );
  __atomic_store_n(
    &g_BallTransitionDiagnostics.eventSlotsFree,
    eventSlotsFree,
    __ATOMIC_RELAXED
    );
  __atomic_store_n(
    &g_BallTransitionDiagnostics.weatherSlotsFree,
    weatherSlotsFree,
    __ATOMIC_RELAXED
    );
  __atomic_store_n(
    &g_BallTransitionDiagnostics.rideSlotsFree,
    rideSlotsFree,
    __ATOMIC_RELAXED
    );
  __atomic_store_n(
    &g_BallTransitionDiagnostics.selectedWorkType,
    selectedWorkType,
    __ATOMIC_RELAXED
    );
  if (result == BALL_TRANSITION_START_STARTED)
  {
    __atomic_fetch_add(
      &g_BallTransitionDiagnostics.successCount,
      1U,
      __ATOMIC_RELAXED
      );
  }
  __atomic_fetch_add(
    &g_BallTransitionDiagnostics.attemptCount,
    1U,
    __ATOMIC_RELAXED
    );
}

BallTransitionDiagnosticSnapshot GetBallTransitionDiagnostics()
{
  BallTransitionDiagnosticSnapshot result;
  result.attemptCount = __atomic_load_n(
    &g_BallTransitionDiagnostics.attemptCount,
    __ATOMIC_RELAXED
    );
  result.successCount = __atomic_load_n(
    &g_BallTransitionDiagnostics.successCount,
    __ATOMIC_RELAXED
    );
  result.kind = __atomic_load_n(
    &g_BallTransitionDiagnostics.kind,
    __ATOMIC_RELAXED
    );
  result.result = __atomic_load_n(
    &g_BallTransitionDiagnostics.result,
    __ATOMIC_RELAXED
    );
  result.resourceHeapSource = __atomic_load_n(
    &g_BallTransitionDiagnostics.resourceHeapSource,
    __ATOMIC_RELAXED
    );
  result.resourceHeapFree = __atomic_load_n(
    &g_BallTransitionDiagnostics.resourceHeapFree,
    __ATOMIC_RELAXED
    );
  result.minimumHeapFree = __atomic_load_n(
    &g_BallTransitionDiagnostics.minimumHeapFree,
    __ATOMIC_RELAXED
    );
  result.resourceWasLoaded = __atomic_load_n(
    &g_BallTransitionDiagnostics.resourceWasLoaded,
    __ATOMIC_RELAXED
    );
  result.systemSlotsFree = __atomic_load_n(
    &g_BallTransitionDiagnostics.systemSlotsFree,
    __ATOMIC_RELAXED
    );
  result.eventSlotsFree = __atomic_load_n(
    &g_BallTransitionDiagnostics.eventSlotsFree,
    __ATOMIC_RELAXED
    );
  result.weatherSlotsFree = __atomic_load_n(
    &g_BallTransitionDiagnostics.weatherSlotsFree,
    __ATOMIC_RELAXED
    );
  result.rideSlotsFree = __atomic_load_n(
    &g_BallTransitionDiagnostics.rideSlotsFree,
    __ATOMIC_RELAXED
    );
  result.selectedWorkType = __atomic_load_n(
    &g_BallTransitionDiagnostics.selectedWorkType,
    __ATOMIC_RELAXED
    );
  return result;
}

void RecordInteractionEffectStart(
  InteractionEffectStartResult result,
  unsigned int effectType,
  unsigned int resourceHeapSource,
  unsigned int resourceHeapFreeBefore,
  unsigned int resourceHeapFreeAfter,
  unsigned int minimumHeapFree,
  bool resourceWasLoaded,
  unsigned int selectedWorkType
)
{
  __atomic_store_n(
    &g_InteractionEffectDiagnostics.result,
    static_cast<unsigned int>(result),
    __ATOMIC_RELAXED
    );
  __atomic_store_n(
    &g_InteractionEffectDiagnostics.effectType,
    effectType,
    __ATOMIC_RELAXED
    );
  __atomic_store_n(
    &g_InteractionEffectDiagnostics.resourceHeapSource,
    resourceHeapSource,
    __ATOMIC_RELAXED
    );
  __atomic_store_n(
    &g_InteractionEffectDiagnostics.resourceHeapFreeBefore,
    resourceHeapFreeBefore,
    __ATOMIC_RELAXED
    );
  __atomic_store_n(
    &g_InteractionEffectDiagnostics.resourceHeapFreeAfter,
    resourceHeapFreeAfter,
    __ATOMIC_RELAXED
    );
  __atomic_store_n(
    &g_InteractionEffectDiagnostics.minimumHeapFree,
    minimumHeapFree,
    __ATOMIC_RELAXED
    );
  __atomic_store_n(
    &g_InteractionEffectDiagnostics.resourceWasLoaded,
    resourceWasLoaded ? 1U : 0U,
    __ATOMIC_RELAXED
    );
  __atomic_store_n(
    &g_InteractionEffectDiagnostics.selectedWorkType,
    selectedWorkType,
    __ATOMIC_RELAXED
    );
  if (result == INTERACTION_EFFECT_START_STARTED)
  {
    __atomic_fetch_add(
      &g_InteractionEffectDiagnostics.successCount,
      1U,
      __ATOMIC_RELAXED
      );
  }
  __atomic_fetch_add(
    &g_InteractionEffectDiagnostics.attemptCount,
    1U,
    __ATOMIC_RELAXED
    );
}

InteractionEffectDiagnosticSnapshot GetInteractionEffectDiagnostics()
{
  InteractionEffectDiagnosticSnapshot result;
  result.attemptCount = __atomic_load_n(
    &g_InteractionEffectDiagnostics.attemptCount,
    __ATOMIC_RELAXED
    );
  result.successCount = __atomic_load_n(
    &g_InteractionEffectDiagnostics.successCount,
    __ATOMIC_RELAXED
    );
  result.result = __atomic_load_n(
    &g_InteractionEffectDiagnostics.result,
    __ATOMIC_RELAXED
    );
  result.effectType = __atomic_load_n(
    &g_InteractionEffectDiagnostics.effectType,
    __ATOMIC_RELAXED
    );
  result.resourceHeapSource = __atomic_load_n(
    &g_InteractionEffectDiagnostics.resourceHeapSource,
    __ATOMIC_RELAXED
    );
  result.resourceHeapFreeBefore = __atomic_load_n(
    &g_InteractionEffectDiagnostics.resourceHeapFreeBefore,
    __ATOMIC_RELAXED
    );
  result.resourceHeapFreeAfter = __atomic_load_n(
    &g_InteractionEffectDiagnostics.resourceHeapFreeAfter,
    __ATOMIC_RELAXED
    );
  result.minimumHeapFree = __atomic_load_n(
    &g_InteractionEffectDiagnostics.minimumHeapFree,
    __ATOMIC_RELAXED
    );
  result.resourceWasLoaded = __atomic_load_n(
    &g_InteractionEffectDiagnostics.resourceWasLoaded,
    __ATOMIC_RELAXED
    );
  result.selectedWorkType = __atomic_load_n(
    &g_InteractionEffectDiagnostics.selectedWorkType,
    __ATOMIC_RELAXED
    );
  return result;
}

void RecordFollowerHeapDiagnostics(
  const FollowerHeapDiagnosticSample& sample,
  bool captureBeforeFollower
)
{
  for (u32 i = 0; i < FOLLOWER_HEAP_DIAGNOSTIC_COUNT; ++i)
  {
    if (captureBeforeFollower)
    {
      __atomic_store_n(
        &g_FollowerHeapDiagnostics.beforeFollower.heaps[i].totalSize,
        sample.heaps[i].totalSize,
        __ATOMIC_RELAXED
        );
      __atomic_store_n(
        &g_FollowerHeapDiagnostics.beforeFollower.heaps[i].totalFree,
        sample.heaps[i].totalFree,
        __ATOMIC_RELAXED
        );
      __atomic_store_n(
        &g_FollowerHeapDiagnostics.beforeFollower.heaps[i].maximumAllocatable,
        sample.heaps[i].maximumAllocatable,
        __ATOMIC_RELAXED
        );
    }
    __atomic_store_n(
      &g_FollowerHeapDiagnostics.current.heaps[i].totalSize,
      sample.heaps[i].totalSize,
      __ATOMIC_RELAXED
      );
    __atomic_store_n(
      &g_FollowerHeapDiagnostics.current.heaps[i].totalFree,
      sample.heaps[i].totalFree,
      __ATOMIC_RELAXED
      );
    __atomic_store_n(
      &g_FollowerHeapDiagnostics.current.heaps[i].maximumAllocatable,
      sample.heaps[i].maximumAllocatable,
      __ATOMIC_RELAXED
      );
  }

  if (captureBeforeFollower)
  {
    __atomic_store_n(
      &g_FollowerHeapDiagnostics.beforeFollower.parentSource,
      sample.parentSource,
      __ATOMIC_RELAXED
      );
    __atomic_store_n(
      &g_FollowerHeapDiagnostics.beforeFollower.followerUsesEventHeap,
      sample.followerUsesEventHeap,
      __ATOMIC_RELAXED
      );
    __atomic_fetch_add(
      &g_FollowerHeapDiagnostics.captureCount,
      1U,
      __ATOMIC_RELAXED
      );
  }
  __atomic_store_n(
    &g_FollowerHeapDiagnostics.current.parentSource,
    sample.parentSource,
    __ATOMIC_RELAXED
    );
  __atomic_store_n(
    &g_FollowerHeapDiagnostics.current.followerUsesEventHeap,
    sample.followerUsesEventHeap,
    __ATOMIC_RELAXED
    );
}

FollowerHeapDiagnosticSnapshot GetFollowerHeapDiagnostics()
{
  FollowerHeapDiagnosticSnapshot result;
  for (u32 i = 0; i < FOLLOWER_HEAP_DIAGNOSTIC_COUNT; ++i)
  {
    result.beforeFollower.heaps[i].totalSize = __atomic_load_n(
      &g_FollowerHeapDiagnostics.beforeFollower.heaps[i].totalSize,
      __ATOMIC_RELAXED
      );
    result.beforeFollower.heaps[i].totalFree = __atomic_load_n(
      &g_FollowerHeapDiagnostics.beforeFollower.heaps[i].totalFree,
      __ATOMIC_RELAXED
      );
    result.beforeFollower.heaps[i].maximumAllocatable = __atomic_load_n(
      &g_FollowerHeapDiagnostics.beforeFollower.heaps[i].maximumAllocatable,
      __ATOMIC_RELAXED
      );
    result.current.heaps[i].totalSize = __atomic_load_n(
      &g_FollowerHeapDiagnostics.current.heaps[i].totalSize,
      __ATOMIC_RELAXED
      );
    result.current.heaps[i].totalFree = __atomic_load_n(
      &g_FollowerHeapDiagnostics.current.heaps[i].totalFree,
      __ATOMIC_RELAXED
      );
    result.current.heaps[i].maximumAllocatable = __atomic_load_n(
      &g_FollowerHeapDiagnostics.current.heaps[i].maximumAllocatable,
      __ATOMIC_RELAXED
      );
  }
  result.beforeFollower.parentSource = __atomic_load_n(
    &g_FollowerHeapDiagnostics.beforeFollower.parentSource,
    __ATOMIC_RELAXED
    );
  result.beforeFollower.followerUsesEventHeap = __atomic_load_n(
    &g_FollowerHeapDiagnostics.beforeFollower.followerUsesEventHeap,
    __ATOMIC_RELAXED
    );
  result.current.parentSource = __atomic_load_n(
    &g_FollowerHeapDiagnostics.current.parentSource,
    __ATOMIC_RELAXED
    );
  result.current.followerUsesEventHeap = __atomic_load_n(
    &g_FollowerHeapDiagnostics.current.followerUsesEventHeap,
    __ATOMIC_RELAXED
    );
  result.captureCount = __atomic_load_n(
    &g_FollowerHeapDiagnostics.captureCount,
    __ATOMIC_RELAXED
    );
  return result;
}

void SetFollowerSceneGraphDiagnostics(
  unsigned int totalJoints,
  unsigned int renderJoints,
  unsigned int requiredJoints
)
{
  __atomic_store_n(
    &g_FollowerSceneGraphDiagnostics.totalJoints,
    totalJoints,
    __ATOMIC_RELAXED
    );
  __atomic_store_n(
    &g_FollowerSceneGraphDiagnostics.renderJoints,
    renderJoints,
    __ATOMIC_RELAXED
    );
  __atomic_store_n(
    &g_FollowerSceneGraphDiagnostics.requiredJoints,
    requiredJoints,
    __ATOMIC_RELAXED
    );
}

FollowerSceneGraphDiagnosticSnapshot GetFollowerSceneGraphDiagnostics()
{
  FollowerSceneGraphDiagnosticSnapshot result;
  result.totalJoints = __atomic_load_n(
    &g_FollowerSceneGraphDiagnostics.totalJoints,
    __ATOMIC_RELAXED
    );
  result.renderJoints = __atomic_load_n(
    &g_FollowerSceneGraphDiagnostics.renderJoints,
    __ATOMIC_RELAXED
    );
  result.requiredJoints = __atomic_load_n(
    &g_FollowerSceneGraphDiagnostics.requiredJoints,
    __ATOMIC_RELAXED
    );
  return result;
}

void UpdateFollowerGroundDiagnostics(
  const FollowerGroundDiagnosticSnapshot& snapshot
  )
{
#define FOLLOWER_STORE_GROUND_FIELD(field) \
  __atomic_store_n( \
    &g_FollowerGroundDiagnostics.field, \
    snapshot.field, \
    __ATOMIC_RELAXED \
    )
  FOLLOWER_STORE_GROUND_FIELD(species);
  FOLLOWER_STORE_GROUND_FIELD(form);
  FOLLOWER_STORE_GROUND_FIELD(motion);
  FOLLOWER_STORE_GROUND_FIELD(bodyRadiusTenths);
  FOLLOWER_STORE_GROUND_FIELD(rawRootYTenths);
  FOLLOWER_STORE_GROUND_FIELD(appliedRootYTenths);
  FOLLOWER_STORE_GROUND_FIELD(cmHeight);
  FOLLOWER_STORE_GROUND_FIELD(fieldAdjustHeight);
#undef FOLLOWER_STORE_GROUND_FIELD
}

FollowerGroundDiagnosticSnapshot GetFollowerGroundDiagnostics()
{
  FollowerGroundDiagnosticSnapshot result;
#define FOLLOWER_LOAD_GROUND_FIELD(field) \
  result.field = __atomic_load_n( \
    &g_FollowerGroundDiagnostics.field, \
    __ATOMIC_RELAXED \
    )
  FOLLOWER_LOAD_GROUND_FIELD(species);
  FOLLOWER_LOAD_GROUND_FIELD(form);
  FOLLOWER_LOAD_GROUND_FIELD(motion);
  FOLLOWER_LOAD_GROUND_FIELD(bodyRadiusTenths);
  FOLLOWER_LOAD_GROUND_FIELD(rawRootYTenths);
  FOLLOWER_LOAD_GROUND_FIELD(appliedRootYTenths);
  FOLLOWER_LOAD_GROUND_FIELD(cmHeight);
  FOLLOWER_LOAD_GROUND_FIELD(fieldAdjustHeight);
#undef FOLLOWER_LOAD_GROUND_FIELD
  return result;
}

void UpdateFollowerLifecycleDiagnostics(
  unsigned int state,
  unsigned int terminateBlocker,
  unsigned int loadFailure
  )
{
  __atomic_store_n(
    &g_FollowerLifecycleDiagnostics.state,
    state,
    __ATOMIC_RELAXED
    );
  __atomic_store_n(
    &g_FollowerLifecycleDiagnostics.terminateBlocker,
    terminateBlocker,
    __ATOMIC_RELAXED
    );
  __atomic_store_n(
    &g_FollowerLifecycleDiagnostics.loadFailure,
    loadFailure,
    __ATOMIC_RELAXED
    );
}

FollowerLifecycleDiagnosticSnapshot GetFollowerLifecycleDiagnostics()
{
  FollowerLifecycleDiagnosticSnapshot result;
  result.state = __atomic_load_n(
    &g_FollowerLifecycleDiagnostics.state,
    __ATOMIC_RELAXED
    );
  result.terminateBlocker = __atomic_load_n(
    &g_FollowerLifecycleDiagnostics.terminateBlocker,
    __ATOMIC_RELAXED
    );
  result.loadFailure = __atomic_load_n(
    &g_FollowerLifecycleDiagnostics.loadFailure,
    __ATOMIC_RELAXED
    );
  return result;
}

void UpdateFollowerInteractionDiagnostics(
  const FollowerInteractionDiagnosticSnapshot& snapshot,
  bool incrementAttempt,
  bool incrementSuccess
)
{
#define FOLLOWER_STORE_INTERACTION_FIELD(field) \
  __atomic_store_n( \
    &g_FollowerInteractionDiagnostics.field, \
    snapshot.field, \
    __ATOMIC_RELAXED \
    )
  FOLLOWER_STORE_INTERACTION_FIELD(state);
  FOLLOWER_STORE_INTERACTION_FIELD(result);
  FOLLOWER_STORE_INTERACTION_FIELD(dataId);
  FOLLOWER_STORE_INTERACTION_FIELD(heapFree);
  FOLLOWER_STORE_INTERACTION_FIELD(bufferSize);
  FOLLOWER_STORE_INTERACTION_FIELD(realSize);
  FOLLOWER_STORE_INTERACTION_FIELD(resourceCount);
  FOLLOWER_STORE_INTERACTION_FIELD(respondAvailable);
  FOLLOWER_STORE_INTERACTION_FIELD(happyAvailable);
  FOLLOWER_STORE_INTERACTION_FIELD(selectedMotion);
#undef FOLLOWER_STORE_INTERACTION_FIELD
  if (incrementAttempt)
  {
    __atomic_fetch_add(
      &g_FollowerInteractionDiagnostics.attemptCount,
      1U,
      __ATOMIC_RELAXED
      );
  }
  if (incrementSuccess)
  {
    __atomic_fetch_add(
      &g_FollowerInteractionDiagnostics.successCount,
      1U,
      __ATOMIC_RELAXED
      );
  }
}

FollowerInteractionDiagnosticSnapshot GetFollowerInteractionDiagnostics()
{
  FollowerInteractionDiagnosticSnapshot result;
#define FOLLOWER_LOAD_INTERACTION_FIELD(field) \
  result.field = __atomic_load_n( \
    &g_FollowerInteractionDiagnostics.field, \
    __ATOMIC_RELAXED \
    )
  FOLLOWER_LOAD_INTERACTION_FIELD(attemptCount);
  FOLLOWER_LOAD_INTERACTION_FIELD(successCount);
  FOLLOWER_LOAD_INTERACTION_FIELD(state);
  FOLLOWER_LOAD_INTERACTION_FIELD(result);
  FOLLOWER_LOAD_INTERACTION_FIELD(dataId);
  FOLLOWER_LOAD_INTERACTION_FIELD(heapFree);
  FOLLOWER_LOAD_INTERACTION_FIELD(bufferSize);
  FOLLOWER_LOAD_INTERACTION_FIELD(realSize);
  FOLLOWER_LOAD_INTERACTION_FIELD(resourceCount);
  FOLLOWER_LOAD_INTERACTION_FIELD(respondAvailable);
  FOLLOWER_LOAD_INTERACTION_FIELD(happyAvailable);
  FOLLOWER_LOAD_INTERACTION_FIELD(selectedMotion);
#undef FOLLOWER_LOAD_INTERACTION_FIELD
  return result;
}

void UpdateDittoTransformDiagnostics(
  const DittoTransformDiagnosticSnapshot& snapshot,
  bool incrementAttempt,
  bool incrementSuccess
)
{
#define FOLLOWER_STORE_DITTO_FIELD(field) \
  __atomic_store_n( \
    &g_DittoTransformDiagnostics.field, \
    snapshot.field, \
    __ATOMIC_RELAXED \
    )
  FOLLOWER_STORE_DITTO_FIELD(state);
  FOLLOWER_STORE_DITTO_FIELD(result);
  FOLLOWER_STORE_DITTO_FIELD(modelId);
  FOLLOWER_STORE_DITTO_FIELD(parentHeapFree);
  FOLLOWER_STORE_DITTO_FIELD(cloneHeapFree);
  FOLLOWER_STORE_DITTO_FIELD(motionId);
  FOLLOWER_STORE_DITTO_FIELD(motionPhase);
  FOLLOWER_STORE_DITTO_FIELD(motionFrame);
  FOLLOWER_STORE_DITTO_FIELD(motionEndFrame);
#undef FOLLOWER_STORE_DITTO_FIELD
  if (incrementAttempt)
  {
    __atomic_fetch_add(
      &g_DittoTransformDiagnostics.attemptCount,
      1U,
      __ATOMIC_RELAXED
      );
  }
  if (incrementSuccess)
  {
    __atomic_fetch_add(
      &g_DittoTransformDiagnostics.successCount,
      1U,
      __ATOMIC_RELAXED
      );
  }
}

DittoTransformDiagnosticSnapshot GetDittoTransformDiagnostics()
{
  DittoTransformDiagnosticSnapshot result;
#define FOLLOWER_LOAD_DITTO_FIELD(field) \
  result.field = __atomic_load_n( \
    &g_DittoTransformDiagnostics.field, \
    __ATOMIC_RELAXED \
    )
  FOLLOWER_LOAD_DITTO_FIELD(attemptCount);
  FOLLOWER_LOAD_DITTO_FIELD(successCount);
  FOLLOWER_LOAD_DITTO_FIELD(state);
  FOLLOWER_LOAD_DITTO_FIELD(result);
  FOLLOWER_LOAD_DITTO_FIELD(modelId);
  FOLLOWER_LOAD_DITTO_FIELD(parentHeapFree);
  FOLLOWER_LOAD_DITTO_FIELD(cloneHeapFree);
  FOLLOWER_LOAD_DITTO_FIELD(motionId);
  FOLLOWER_LOAD_DITTO_FIELD(motionPhase);
  FOLLOWER_LOAD_DITTO_FIELD(motionFrame);
  FOLLOWER_LOAD_DITTO_FIELD(motionEndFrame);
#undef FOLLOWER_LOAD_DITTO_FIELD
  return result;
}

void UpdateCastformWeatherDiagnostics(
  const CastformWeatherDiagnosticSnapshot& snapshot,
  bool incrementAttempt,
  bool incrementSuccess
)
{
#define FOLLOWER_STORE_CASTFORM_FIELD(field) \
  __atomic_store_n( \
    &g_CastformWeatherDiagnostics.field, \
    snapshot.field, \
    __ATOMIC_RELAXED \
    )
  FOLLOWER_STORE_CASTFORM_FIELD(result);
  FOLLOWER_STORE_CASTFORM_FIELD(requestedWeather);
  FOLLOWER_STORE_CASTFORM_FIELD(previousWeather);
  FOLLOWER_STORE_CASTFORM_FIELD(previousForceWeather);
#undef FOLLOWER_STORE_CASTFORM_FIELD
  if (incrementAttempt)
  {
    __atomic_fetch_add(
      &g_CastformWeatherDiagnostics.attemptCount,
      1U,
      __ATOMIC_RELAXED
      );
  }
  if (incrementSuccess)
  {
    __atomic_fetch_add(
      &g_CastformWeatherDiagnostics.successCount,
      1U,
      __ATOMIC_RELAXED
      );
  }
}

CastformWeatherDiagnosticSnapshot GetCastformWeatherDiagnostics()
{
  CastformWeatherDiagnosticSnapshot result;
#define FOLLOWER_LOAD_CASTFORM_FIELD(field) \
  result.field = __atomic_load_n( \
    &g_CastformWeatherDiagnostics.field, \
    __ATOMIC_RELAXED \
    )
  FOLLOWER_LOAD_CASTFORM_FIELD(attemptCount);
  FOLLOWER_LOAD_CASTFORM_FIELD(successCount);
  FOLLOWER_LOAD_CASTFORM_FIELD(result);
  FOLLOWER_LOAD_CASTFORM_FIELD(requestedWeather);
  FOLLOWER_LOAD_CASTFORM_FIELD(previousWeather);
  FOLLOWER_LOAD_CASTFORM_FIELD(previousForceWeather);
#undef FOLLOWER_LOAD_CASTFORM_FIELD
  return result;
}

void ShutdownPerformanceDiagnostics()
{
  if (!g_Initialized)
  {
    return;
  }
  __atomic_store_n(&g_PerformanceOptions, 0U, __ATOMIC_RELAXED);
  __atomic_store_n(
    &g_TransientPerformanceOptions,
    0U,
    __ATOMIC_RELAXED
    );
  g_Initialized = false;
  CTRPluginFramework::OSD::Stop(PerformanceOverlayCallback);
}

} // namespace Gen7Follower3gx

#endif
