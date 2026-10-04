#include "FieldConvenience.hpp"
#include "FollowerController.hpp"
#include "MountedRideFeedback.hpp"
#include "RideEventSettings.hpp"
#include <string>
#include <CTRPluginFramework/Graphics/OSD.hpp>
#include <3ds/svc.h>
#include <cstdio>

#include "BattleWeatherHandoff.hpp"
#include "FreeCamera.hpp"
#include "DiagnosticEffectModule.hpp"
#include "Diagnostics.hpp"
#include "DrawDistance.hpp"
#include "DynamicFollower.hpp"
#include "PerformanceDiagnostics.hpp"
#include "FollowerSettings.hpp"

#if FOLLOWER_3GX_DIAGNOSTIC
extern "C" void Follower3gx_TraceRide(const char* message)
{
  FOLLOWER_3GX_TRACE(message);
}
#endif

extern "C" void Follower3gx_NotifyRide(const char* message)
{
#if FOLLOWER_3GX_DIAGNOSTIC
  CTRPluginFramework::OSD::Notify(message);
  char line[160];
  const int length = std::snprintf(line, sizeof(line), "[MountedRide] %s\n", message);
  if (length > 0)
  {
    svcOutputDebugString(line, length < static_cast<int>(sizeof(line))
      ? length : static_cast<int>(sizeof(line) - 1));
  }
#else
  (void)message;
#endif
}

extern "C" void Follower3gx_NotifyEventCleanup(unsigned int reasons,
  unsigned int followerState, unsigned int heapSource, const void* eventVtable)
{
#if FOLLOWER_3GX_DIAGNOSTIC
  using namespace Gen7Follower3gx;
  const struct { unsigned int flag; const char* name; } labels[]={
    {EVENT_CLEANUP_RIDE_POLICY,"ride policy"},
    {EVENT_CLEANUP_RIDER_RETIRING,"rider retiring"},
    {EVENT_CLEANUP_INTERACTION,"follower interaction"},
    {EVENT_CLEANUP_MOTION_PACK,"interaction motion pack"},
    {EVENT_CLEANUP_FORM_CHANGE,"form/model change"},
    {EVENT_CLEANUP_EFFECT,"active effect"},
    {EVENT_CLEANUP_EVENT_HEAP,"shared event memory"},
    {EVENT_CLEANUP_REGULAR_POLICY,"Sun/Moon event policy"}
  };
  std::string summary="Event cleanup: ";
  bool first=true;
  for (const auto& label : labels) if (reasons&label.flag) {
    if (!first) summary+=", ";
    summary+=label.name;
    first=false;
  }
  CTRPluginFramework::OSD::Notify(summary);
  const std::string names="[RideEvent] "+summary+"\n";
  svcOutputDebugString(names.c_str(),names.size());
  char detail[192];
  const int length=std::snprintf(detail,sizeof(detail),
    "[RideEvent] mask=%02x followerState=%u heapSource=%u eventVtable=%p mountedMode=%u keepFollower=%u\n",
    reasons,followerState,heapSource,eventVtable,
    static_cast<unsigned int>(GetMountedInteractionMode()),
    KeepFollowerVisibleDuringEvents() ? 1U : 0U);
  if (length>0) svcOutputDebugString(detail,length<static_cast<int>(sizeof(detail))
    ? length : static_cast<int>(sizeof(detail)-1));
#else
  (void)reasons;
  (void)followerState;
  (void)heapSource;
  (void)eventVtable;
#endif
}

extern "C" void Follower3gx_LogEventMemory(const RideEventMemorySnapshot* snapshot)
{
  if (!snapshot) return;
  char line[384];
  const int length=std::snprintf(line,sizeof(line),
    "[RideEventMemory] species=%u mounted=%u modelBorrowsEventHeap=%u "
    "modelSize=%u modelFree=%u systemSize=%u systemFree=%u "
    "independentSource=%u independentLargest=%u fullFollowerRequired=%u eventLargest=%u\n",
    snapshot->species,snapshot->mounted,snapshot->modelBorrowsEventHeap,
    snapshot->modelSize,snapshot->modelFree,snapshot->systemSize,snapshot->systemFree,
    snapshot->independentSource,snapshot->independentLargest,
    snapshot->fullFollowerRequired,snapshot->eventLargest);
  if (length>0) svcOutputDebugString(line,length<static_cast<int>(sizeof(line))
    ? length : static_cast<int>(sizeof(line)-1));
}

namespace
{

typedef bool (*InitializeFunction)();
typedef bool (*ShutdownFunction)();
typedef unsigned int (*UpdateFunction)(
  void*,
  void*,
  void*,
  void*,
  unsigned int
);
typedef unsigned int (*TerminateFunction)(void*);
typedef void (*SuspendFunction)();
typedef bool (*IsFreeFieldFunction)(void*);
typedef void (*AfterEventCheckFunction)(void*);

extern "C" bool FollowerCarrier_InitializeRuntimeRegular();
extern "C" bool FollowerCarrier_ShutdownRuntimeRegular();
extern "C" unsigned int FollowerCarrier_UpdateRegular(
  void*, void*, void*, void*, unsigned int
);
extern "C" bool FollowerCarrier_OpenPcRegular(void*);
extern "C" bool FollowerCarrier_OpenPcUltra(void*);
static bool (*g_OpenPc)(void*)=nullptr;
extern "C" unsigned int FollowerCarrier_HostTerminateRegular(void*);
extern "C" void FollowerCarrier_SuspendForWaterRideRegular();
extern "C" bool FollowerCarrier_IsFreeFieldRegular(void*);
extern "C" void FollowerCarrier_AfterEventCheckRegular(void*);

extern "C" bool FollowerCarrier_InitializeRuntimeUltra();
extern "C" bool FollowerCarrier_ShutdownRuntimeUltra();
extern "C" unsigned int FollowerCarrier_UpdateUltra(
  void*, void*, void*, void*, unsigned int
);
extern "C" unsigned int FollowerCarrier_HostTerminateUltra(void*);
extern "C" void FollowerCarrier_SuspendForWaterRideUltra();
extern "C" bool FollowerCarrier_IsFreeFieldUltra(void*);
extern "C" void FollowerCarrier_AfterEventCheckUltra(void*);

InitializeFunction g_Initialize = NULL;
ShutdownFunction g_Shutdown = NULL;
UpdateFunction g_Update = NULL;
TerminateFunction g_Terminate = NULL;
SuspendFunction g_Suspend = NULL;
IsFreeFieldFunction g_IsFreeField = NULL;
AfterEventCheckFunction g_AfterEventCheck = NULL;
bool g_Initialized = false;
bool g_RegularPokeModelLayout = false;

enum FollowerUpdateCommand
{
  FOLLOWER_UPDATE_COMMAND_PAUSE = 1U,
  FOLLOWER_UPDATE_COMMAND_RUN_POST = 2U,
};

template <typename T>
T ReadField(void* object, u32 offset)
{
  return *reinterpret_cast<T*>(
    reinterpret_cast<u8*>(object) + offset
    );
}

void ClearFunctions()
{
  g_Initialize = NULL;
  g_Shutdown = NULL;
  g_Update = NULL;
  g_Terminate = NULL;
  g_Suspend = NULL;
  g_IsFreeField = NULL;
  g_AfterEventCheck = NULL;
  g_RegularPokeModelLayout = false;
}

} // namespace

extern "C" int FollowerCarrier_IsRegularPokeModelLayout()
{
  return g_RegularPokeModelLayout ? 1 : 0;
}

namespace Gen7Follower3gx
{

bool InitializeFollowerController(GameFamily family)
{
  if (g_Initialized)
  {
    return false;
  }

  g_RegularPokeModelLayout = family == GameFamily::Regular;
  if (family == GameFamily::Regular)
  {
    g_OpenPc=FollowerCarrier_OpenPcRegular;
    g_Initialize = FollowerCarrier_InitializeRuntimeRegular;
    g_Shutdown = FollowerCarrier_ShutdownRuntimeRegular;
    g_Update = FollowerCarrier_UpdateRegular;
    g_Terminate = FollowerCarrier_HostTerminateRegular;
    g_Suspend = FollowerCarrier_SuspendForWaterRideRegular;
    g_IsFreeField = FollowerCarrier_IsFreeFieldRegular;
    g_AfterEventCheck = FollowerCarrier_AfterEventCheckRegular;
  }
  else
  {
    g_OpenPc=FollowerCarrier_OpenPcUltra;
    g_Initialize = FollowerCarrier_InitializeRuntimeUltra;
    g_Shutdown = FollowerCarrier_ShutdownRuntimeUltra;
    g_Update = FollowerCarrier_UpdateUltra;
    g_Terminate = FollowerCarrier_HostTerminateUltra;
    g_Suspend = FollowerCarrier_SuspendForWaterRideUltra;
    g_IsFreeField = FollowerCarrier_IsFreeFieldUltra;
    g_AfterEventCheck = FollowerCarrier_AfterEventCheckUltra;
  }

  if (!g_Initialize())
  {
    FOLLOWER_3GX_TRACE("carrier runtime returned false");
    ClearFunctions();
    return false;
  }
  InitializeDrawDistance();
  ResetDynamicFollowerForField();
#if FOLLOWER_3GX_DIAGNOSTIC
  InitializeDiagnosticEffectModule();

#endif
  InitializeFreeCamera();
  g_Initialized = true;
  return true;
}

bool ShutdownFollowerController()
{
  if (!g_Initialized)
  {
    return true;
  }

  ShutdownDrawDistance();
  ResetDynamicFollowerForField();
#if FOLLOWER_3GX_DIAGNOSTIC
  ClearBattleWeatherHandoff();

#endif
  ShutdownFreeCamera();
  if (!g_Shutdown())
  {
    return false;
  }
#if FOLLOWER_3GX_DIAGNOSTIC
  ShutdownDiagnosticEffectModule();
#endif
  g_Initialized = false;
  ClearFunctions();
  return true;
}

static bool areaTerminating=false;
u32 FollowerPreUpdate(void* fieldmap)
{
  if (!g_Initialized || !g_Update || !fieldmap)
  {
    return 0;
  }
  areaTerminating=false;
  if (PcRequestState()) {
    PrepareFreeCamera(fieldmap,false);
    static unsigned int pcWaitFrames=0;
    if (PcRequestState()==1) pcWaitFrames=0;
    if (!g_IsFreeField || !g_IsFreeField(fieldmap) || ++pcWaitFrames>300) {
      SetPcRequestState(0);
      Follower3gx_NotifyRide("PC cancelled: return to normal field control and try again");
    } else {
      SetPcRequestState(2);
      ClearAreaRide();
      RestoreDrawDistance();
      if (!g_Terminate(fieldmap)) return 1U;
      SetPcRequestState(0);
      if (!g_OpenPc || !g_OpenPc(fieldmap)) Follower3gx_NotifyRide("Could not start the PC event");
      return 0;
    }
  }
  FOLLOWER_PERF_SCOPE(
    followerPreUpdatePerformance,
    Gen7Follower3gx::PERFORMANCE_ZONE_FOLLOWER
    );
  const bool freeCameraField =
    g_IsFreeField && g_IsFreeField(fieldmap);
  PrepareFreeCamera(fieldmap, freeCameraField);
  const bool monitorPerformance =
    IsFollowerEnabled() && IsDynamicFollowerEnabled();
  const bool freeField = monitorPerformance &&
    g_IsFreeField && g_IsFreeField(fieldmap);
  SetDynamicFollowerFieldReady(freeField);
  if (!ShouldFollowerRun())
  {
    ClearAreaRide();
#if FOLLOWER_3GX_DIAGNOSTIC
    ClearBattleWeatherHandoff();
#endif
    RestoreDrawDistance();
    const bool terminated = g_Terminate(fieldmap);
#if FOLLOWER_3GX_DIAGNOSTIC
    if (terminated)
    {
      ReleaseOwnedDiagnosticEffectModule();
    }
#endif
    return terminated ? 0U : 1U;
  }
  UpdateDrawDistance(fieldmap);
  const u32 command = g_Update(
    fieldmap,
    ReadField<void*>(fieldmap, 0x90),
    ReadField<void*>(fieldmap, 0x9c),
    ReadField<void*>(fieldmap, 0xa0),
    0
    );
  // Keep reapplying outline isolation while an interaction pauses field updates.
  if (command & FOLLOWER_UPDATE_COMMAND_RUN_POST)
  {
    g_Update(
      fieldmap,
      ReadField<void*>(fieldmap, 0x90),
      ReadField<void*>(fieldmap, 0x9c),
      ReadField<void*>(fieldmap, 0xa0),
      1
      );
  }
  return command & ~FOLLOWER_UPDATE_COMMAND_RUN_POST;
}

void FollowerPostUpdate(void* fieldmap)
{
  if (!g_Initialized || !g_Update || !fieldmap)
  {
    return;
  }
  if (PcRequestState()) return;
  FOLLOWER_PERF_SCOPE(
    followerPostUpdatePerformance,
    Gen7Follower3gx::PERFORMANCE_ZONE_FOLLOWER
    );
  UpdateFreeCamera(fieldmap);
  if (!ShouldFollowerRun())
  {
    ClearAreaRide();
#if FOLLOWER_3GX_DIAGNOSTIC
    ClearBattleWeatherHandoff();
#endif
    RestoreDrawDistance();
    const bool terminated = g_Terminate(fieldmap);
#if FOLLOWER_3GX_DIAGNOSTIC
    if (terminated)
    {
      ReleaseOwnedDiagnosticEffectModule();
    }
#else
    (void)terminated;
#endif
    return;
  }
  UpdateDrawDistance(fieldmap);
  g_Update(
    fieldmap,
    ReadField<void*>(fieldmap, 0x90),
    ReadField<void*>(fieldmap, 0x9c),
    ReadField<void*>(fieldmap, 0xa0),
    1
    );
}

void FollowerAfterEventCheck(void* fieldmap)
{
  if (!g_Initialized || !g_AfterEventCheck || !fieldmap)
  {
    return;
  }
  g_AfterEventCheck(fieldmap);
}

u32 FollowerHostTerminate(void* fieldmap)
{
  if (!g_Initialized || !g_Terminate)
  {
    return 1;
  }
  SetPcRequestState(0);
  if (!areaTerminating) { MarkRideAreaTransition(); areaTerminating=true; }
  RestoreDrawDistance();
  ResetDynamicFollowerForField();
  TerminateFreeCameraField(fieldmap);
  const u32 terminated = g_Terminate(fieldmap);
#if FOLLOWER_3GX_DIAGNOSTIC
  if (terminated)
  {
    ReleaseOwnedDiagnosticEffectModule();
  }
#endif
  return terminated;
}

void FollowerSuspendForWaterRide()
{
  ClearAreaRide();
  if (g_Initialized && g_Suspend)
  {
    g_Suspend();
  }
}

bool IsFollowerControllerInitialized()
{
  return g_Initialized;
}

} // namespace Gen7Follower3gx

extern "C" u32 Follower3gx_PreUpdate(void* fieldmap)
{
  return Gen7Follower3gx::FollowerPreUpdate(fieldmap);
}

extern "C" void Follower3gx_PostUpdate(void* fieldmap)
{
  Gen7Follower3gx::FollowerPostUpdate(fieldmap);
}

extern "C" u32 Follower3gx_HostTerminate(void* fieldmap)
{
  return Gen7Follower3gx::FollowerHostTerminate(fieldmap);
}
