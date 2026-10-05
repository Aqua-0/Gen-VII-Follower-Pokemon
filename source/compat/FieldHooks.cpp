#include "NormalEdgeFilter.hpp"
#include "FollowerTalk.hpp"
#include "FieldConvenience.hpp"
#include "RidePresentation.hpp"
#include "MountedRideFeedback.hpp"
#include "FieldHooks.hpp"

#include <CTRPluginFramework/System/Hook.hpp>
#include <CTRPluginFramework/System/Process.hpp>
#include <CTRPluginFramework/System/System.hpp>

#include "BattleWeatherHandoff.hpp"
#include "Diagnostics.hpp"
#include "FollowerController.hpp"
#include "FollowerSettings.hpp"
#include "PerformanceDiagnostics.hpp"
#include "RetailFunctions.hpp"

extern "C" void Follower3gx_UpdateHookBridge();
extern "C" void Follower3gx_TerminateHookBridge();
extern "C" Result Follower3gx_SetProcessMemoryRwx(Handle process);
extern "C" Result Follower3gx_RefreshAndCheckUserWriteTlb(
  u32 updateTarget,
  u32 terminateTarget,
  u32 surfCallsite
  );
extern "C" Result svcCustomBackdoor(void* function, ...);
extern "C" void svcFlushEntireDataCache();
extern "C"
{
u32 g_Follower3gxUpdateExpectedReturn = 0;
u32 g_Follower3gxTerminateExpectedReturn = 0;
}

namespace Gen7Follower3gx
{
namespace
{

CTRPluginFramework::Hook g_UpdateHook;
CTRPluginFramework::Hook g_TerminateHook;
CTRPluginFramework::Hook g_SurfHook;
CTRPluginFramework::Hook g_EventCheckHook;
#if FOLLOWER_3GX_DIAGNOSTIC
CTRPluginFramework::Hook g_TerrainLodHook;
#endif
#if FOLLOWER_3GX_INTERACTION_FEATURES
CTRPluginFramework::Hook g_BattleSituationHook;
#endif
#if FOLLOWER_3GX_PERFORMANCE_FEATURES
CTRPluginFramework::Hook g_MotionBlurStartHook;
CTRPluginFramework::Hook g_ShadowUpdateHook;
CTRPluginFramework::Hook g_SimpleLightPickUpHook;
#endif
bool g_Installed = false;

u32 ReadU32(u32 address)
{
  return *reinterpret_cast<const volatile u32*>(address);
}

Result SetProcessMemoryRwx()
{
  Result result = Follower3gx_SetProcessMemoryRwx(
    CTRPluginFramework::Process::GetHandle()
    );
  if (R_SUCCEEDED(result))
  {
    return result;
  }

  // Some emulators need a real process handle for custom SVCs.
  u32 processId = 0;
  Handle processHandle = 0;
  if (R_FAILED(svcGetProcessId(&processId, CUR_PROCESS_HANDLE)) ||
      R_FAILED(svcOpenProcess(&processHandle, processId)))
  {
    return result;
  }

  result = Follower3gx_SetProcessMemoryRwx(processHandle);
  svcCloseHandle(processHandle);
  return result;
}

bool EnsureHookTargetsWritable(
  u32 updateTarget,
  u32 terminateTarget,
  u32 surfCallsite,
  u32 eventCheckTarget
  )
{
  if (CTRPluginFramework::System::IsCitra())
  {
    // Emulators already allow CRO writes. Changing the region metadata breaks later loads in Azahar.
    return true;
  }

  // New CRO pages are read/execute only. After changing permissions, clear the TLB before adding hooks.
  Result result = SetProcessMemoryRwx();
  FOLLOWER_3GX_TRACE_VALUE("FieldRo permission refresh", result);
  if (R_FAILED(result))
  {
    return false;
  }

  // Flush the page-table changes first, or the TLB can pick up the old read/execute permissions again.
  svcFlushEntireDataCache();
  result = svcCustomBackdoor(
    reinterpret_cast<void*>(Follower3gx_RefreshAndCheckUserWriteTlb),
    updateTarget,
    terminateTarget,
    surfCallsite
    );
  FOLLOWER_3GX_TRACE_VALUE("FieldRo user-write translation", result);
  if (result != 1)
  {
    return false;
  }

  // The backdoor only takes three arguments, so check the extra target separately.
  result = svcCustomBackdoor(
    reinterpret_cast<void*>(Follower3gx_RefreshAndCheckUserWriteTlb),
    eventCheckTarget,
    eventCheckTarget,
    eventCheckTarget
    );
  FOLLOWER_3GX_TRACE_VALUE("event-check user-write translation", result);
  return result == 1;
}

bool ValidateHook(const HookProfile& hook, const CroModuleView& fieldRo)
{
  if (hook.callsiteTextOffset + 4 > fieldRo.TextSize() ||
      hook.targetTextOffset + 8 > fieldRo.TextSize())
  {
    return false;
  }

  const u32 callsite = fieldRo.TextBase() + hook.callsiteTextOffset;
  const u32 target = fieldRo.TextBase() + hook.targetTextOffset;
  if (ReadU32(callsite) != hook.expectedCallInstruction ||
      ReadU32(target) != hook.expectedTargetInstructions[0])
  {
    return false;
  }

  const u32 targetWord1 = ReadU32(target + 4);
  const bool isRelocatedImportVeneer =
    hook.expectedTargetInstructions[0] == 0xe51ff004 &&
    hook.expectedTargetInstructions[1] == 0;
  if (isRelocatedImportVeneer)
  {
    const u32 resolvedTarget = targetWord1 & ~1U;
    return resolvedTarget != 0 &&
      CTRPluginFramework::Process::CheckAddress(
        resolvedTarget,
        MEMPERM_EXECUTE
        );
  }

  return targetWord1 == hook.expectedTargetInstructions[1];
}

#if FOLLOWER_3GX_PERFORMANCE_FEATURES
bool ValidateStaticHook(const StaticHookProfile& hook)
{
  return CTRPluginFramework::Process::CheckAddress(
      hook.targetAddress,
      MEMPERM_EXECUTE
      ) &&
    ReadU32(hook.targetAddress) == hook.expectedTargetInstructions[0] &&
    ReadU32(hook.targetAddress + 4) == hook.expectedTargetInstructions[1];
}
#endif

extern "C" u32 Follower3gx_SurfAfter(u32 result)
{
  if (result != 0)
  {
    FollowerSuspendForWaterRide();
  }
  return result;
}

extern "C" void Follower3gx_EventCheckCallback(void* fieldmap)
{
  CTRPluginFramework::HookContext::GetCurrent().OriginalFunction<void>(
    fieldmap
    );
  FollowerAfterEventCheck(fieldmap);
}

#if FOLLOWER_3GX_INTERACTION_FEATURES
enum
{
  // The weather field follows a 0x18-byte background block.
  BATTLE_FIELD_SITUATION_WEATHER_OFFSET = 0x18,
};

extern "C" void Follower3gx_SetUpFieldSituationCallback(
  void* situation,
  void* gameManager,
  const void** zoneLoader,
  u32 attribute,
  u32 farType,
  u32 nearType,
  s8 trainerLevelAdjust
  )
{
  CTRPluginFramework::HookContext::GetCurrent().OriginalFunction<void>(
    situation,
    gameManager,
    zoneLoader,
    attribute,
    farType,
    nearType,
    trainerLevelAdjust
    );

  u8 weather = BATTLE_WEATHER_NONE;
  if (situation && ConsumeBattleWeatherHandoff(gameManager, &weather))
  {
    *(reinterpret_cast<u8*>(situation) +
      BATTLE_FIELD_SITUATION_WEATHER_OFFSET) = weather;
  }
}
#endif

#if FOLLOWER_3GX_DIAGNOSTIC
extern "C" void Follower3gx_TerrainChangeLodCallback(
  void* terrainBlock,
  u32 lod
  )
{
  if (GetFollowerTerrainDetailMode() == FOLLOWER_TERRAIN_DETAIL_MEDIUM &&
      lod == 0)
  {
    lod = 1;
  }
  CTRPluginFramework::HookContext::GetCurrent().OriginalFunction<void>(
    terrainBlock,
    lod
    );
}
#endif

#if FOLLOWER_3GX_PERFORMANCE_FEATURES
extern "C" void Follower3gx_MotionBlurRenderStartCallback(
  void* motionBlur,
  u32 blend,
  u32 animation,
  const void* targetSurface,
  u32 loop
  )
{
  if (IsPerformanceOptionEnabled(PERFORMANCE_OPTION_DISABLE_MOTION_BLUR))
  {
    typedef void (*RenderOffFunction)(void*);
    reinterpret_cast<RenderOffFunction>(
      g_RetailFunctionPointers[RetailFunction_MotionBlurRenderOff]
      )(motionBlur);
    return;
  }
  CTRPluginFramework::HookContext::GetCurrent().OriginalFunction<void>(
    motionBlur,
    blend,
    animation,
    targetSurface,
    loop
  );
}

extern "C" void Follower3gx_ShadowUpdateCallback(void* shadowManager)
{
  if (IsPerformanceOptionEnabled(
        PERFORMANCE_OPTION_DISABLE_CHARACTER_SHADOWS
        ))
  {
    return;
  }
  BeginRideShadowPass();
  CTRPluginFramework::HookContext::GetCurrent().OriginalFunction<void>(
    shadowManager
    );
  EndRideShadowPass();
}

struct SimpleLightPickUp
{
  enum
  {
    LIGHT_CAPACITY = 32,
    LIGHT_TYPE_AMBIENT = 0,
    LIGHT_TYPE_DIRECTIONAL = 1,
  };
  const void* nodes[LIGHT_CAPACITY];
};

enum
{
  LIGHT_SET_INDEXER_OFFSET = 0x8c,
  LIGHT_SET_USED_SIZE_OFFSET = 0x9c,
  LIGHT_NODE_TYPE_OFFSET = 0xf8,
};

extern "C" void Follower3gx_SimpleLightPickUpCallback(
  void* drawManager,
  SimpleLightPickUp* lights,
  const void* bounds,
  void* drawEnvironment,
  s32 lightSetNumber
  )
{
  if (!IsPerformanceOptionEnabled(
        PERFORMANCE_OPTION_SIMPLE_WORLD_LIGHTING
        ))
  {
    CTRPluginFramework::HookContext::GetCurrent().OriginalFunction<void>(
      drawManager,
      lights,
      bounds,
      drawEnvironment,
      lightSetNumber
      );
    return;
  }

  if (!lights || !drawEnvironment || lightSetNumber < 0)
  {
    return;
  }

  typedef void* (*GetLightSetFunction)(void*, s32);
  void* lightSet = reinterpret_cast<GetLightSetFunction>(
    g_RetailFunctionPointers[RetailFunction_DrawEnvGetLightSet]
    )(drawEnvironment, lightSetNumber);
  if (!lightSet)
  {
    return;
  }

  const u8* lightSetBytes = reinterpret_cast<const u8*>(lightSet);
  const u32 lightCount = *reinterpret_cast<const u32*>(
    lightSetBytes + LIGHT_SET_USED_SIZE_OFFSET
    );
  void* const* indexer = *reinterpret_cast<void* const* const*>(
    lightSetBytes + LIGHT_SET_INDEXER_OFFSET
    );
  if (!indexer)
  {
    return;
  }
  u32 simpleLightCount = 0;
  for (u32 index = 0;
       index < lightCount &&
         simpleLightCount < SimpleLightPickUp::LIGHT_CAPACITY;
       ++index)
  {
    const void* link = indexer[index];
    const void* data = link
      ? *reinterpret_cast<void* const*>(link)
      : NULL;
    const void* light = data
      ? *reinterpret_cast<void* const*>(data)
      : NULL;
    const u8 lightType = light
      ? *(reinterpret_cast<const u8*>(light) + LIGHT_NODE_TYPE_OFFSET)
      : 0xff;
    if (lightType == SimpleLightPickUp::LIGHT_TYPE_AMBIENT ||
        lightType == SimpleLightPickUp::LIGHT_TYPE_DIRECTIONAL)
    {
      lights->nodes[simpleLightCount++] = light;
    }
  }
}
#endif

} // namespace

bool InstallFieldHooks(const GameProfile& profile, const CroModuleView& fieldRo)
{
  if (g_Installed)
  {
    FOLLOWER_3GX_TRACE("field hooks already installed");
    return false;
  }
  const u32 updateTarget =
    fieldRo.TextBase() + profile.updateHook.targetTextOffset;
  const u32 terminateTarget =
    fieldRo.TextBase() + profile.terminateHook.targetTextOffset;
  const u32 surfCallsite =
    fieldRo.TextBase() + profile.surfHook.callsiteTextOffset;
  const u32 eventCheckTarget =
    fieldRo.TextBase() + profile.eventCheckHook.targetTextOffset;
#if FOLLOWER_3GX_DIAGNOSTIC
  const u32 terrainLodTarget = fieldRo.TextBase() +
    profile.retailAddresses[RetailFunction_TerrainStoreDetailRequest];
#endif
#if FOLLOWER_3GX_INTERACTION_FEATURES
  const u32 battleSituationTarget =
    g_RetailFunctionPointers[RetailFunction_FieldSetUpFieldSituation];
#endif
#if FOLLOWER_3GX_PERFORMANCE_FEATURES
  const u32 motionBlurStartTarget =
    profile.retailAddresses[RetailFunction_MotionBlurRenderStart];
#endif

  // Change permissions before validation reads cache the old read/execute mapping.
  if (!EnsureHookTargetsWritable(
        updateTarget,
        terminateTarget,
        surfCallsite,
        eventCheckTarget
        ))
  {
    FOLLOWER_3GX_TRACE("FieldRo text is not writable");
    return false;
  }
  if (!ValidateHook(profile.updateHook, fieldRo))
  {
    FOLLOWER_3GX_TRACE("update hook validation failed");
    return false;
  }
  if (!ValidateHook(profile.terminateHook, fieldRo))
  {
    FOLLOWER_3GX_TRACE("terminate hook validation failed");
    return false;
  }
  if (!ValidateHook(profile.surfHook, fieldRo))
  {
    FOLLOWER_3GX_TRACE("surf hook validation failed");
    return false;
  }
  if (!ValidateHook(profile.eventCheckHook, fieldRo))
  {
    FOLLOWER_3GX_TRACE("event-check hook validation failed");
    return false;
  }
#if FOLLOWER_3GX_PERFORMANCE_FEATURES
  if (!ValidateStaticHook(profile.simpleLightPickUpHook))
  {
    FOLLOWER_3GX_TRACE("simple light hook validation failed");
    return false;
  }
#endif

  g_Follower3gxUpdateExpectedReturn =
    fieldRo.TextBase() + profile.updateHook.callsiteTextOffset + 4;
  g_Follower3gxTerminateExpectedReturn =
    fieldRo.TextBase() + profile.terminateHook.callsiteTextOffset + 4;

  using CTRPluginFramework::HookResult;
  if (g_EventCheckHook.InitializeForMitm(
        eventCheckTarget,
        reinterpret_cast<u32>(Follower3gx_EventCheckCallback)
        ).Enable() != HookResult::Success)
  {
    RemoveFieldHooks();
    return false;
  }
  if (g_UpdateHook.InitializeForMitm(
        updateTarget,
        reinterpret_cast<u32>(Follower3gx_UpdateHookBridge)
        ).Enable() != HookResult::Success)
  {
    RemoveFieldHooks();
    return false;
  }
  if (g_TerminateHook.InitializeForMitm(
        terminateTarget,
        reinterpret_cast<u32>(Follower3gx_TerminateHookBridge)
        ).Enable() != HookResult::Success)
  {
    RemoveFieldHooks();
    return false;
  }
  if (g_SurfHook.InitializeForSubWrap(
        surfCallsite,
        0,
        reinterpret_cast<u32>(Follower3gx_SurfAfter)
        ).Enable() != HookResult::Success)
  {
    RemoveFieldHooks();
    return false;
  }
#if FOLLOWER_3GX_INTERACTION_FEATURES
  if (g_BattleSituationHook.InitializeForMitm(
        battleSituationTarget,
        reinterpret_cast<u32>(Follower3gx_SetUpFieldSituationCallback)
        ).Enable() != HookResult::Success)
  {
    RemoveFieldHooks();
    return false;
  }
#endif
#if FOLLOWER_3GX_DIAGNOSTIC
  if (g_TerrainLodHook.InitializeForMitm(
        terrainLodTarget,
        reinterpret_cast<u32>(Follower3gx_TerrainChangeLodCallback)
        ).Enable() != HookResult::Success)
  {
    RemoveFieldHooks();
    return false;
  }
#endif
#if FOLLOWER_3GX_PERFORMANCE_FEATURES
  if (g_MotionBlurStartHook.InitializeForMitm(
        motionBlurStartTarget,
        reinterpret_cast<u32>(Follower3gx_MotionBlurRenderStartCallback)
        ).Enable() != HookResult::Success)
  {
    RemoveFieldHooks();
    return false;
  }
  if (g_ShadowUpdateHook.InitializeForMitm(
        g_RetailFunctionPointers[RetailFunction_FieldMoveModelShadowUpdate],
        reinterpret_cast<u32>(Follower3gx_ShadowUpdateCallback)
        ).Enable() != HookResult::Success)
  {
    RemoveFieldHooks();
    return false;
  }
  if (g_SimpleLightPickUpHook.InitializeForMitm(
        profile.simpleLightPickUpHook.targetAddress,
        reinterpret_cast<u32>(Follower3gx_SimpleLightPickUpCallback)
        ).Enable() != HookResult::Success)
  {
    RemoveFieldHooks();
    return false;
  }
#endif

  InstallNormalEdgeFilter(profile);
  BindPcEvent(profile);
  BindFollowerTalk(profile, fieldRo.TextBase());
  if (!InstallRidePresentationHooks(profile)) {
    Follower3gx_NotifyRide("Follower unavailable: ride movement/effect hook validation failed");
    RemoveFieldHooks(); return false;
  }
  g_Installed = true;
  return true;
}

void RemoveFieldHooks()
{
  RemoveNormalEdgeFilter();
  UnbindFollowerTalk();
  UnbindPcEvent();
  RemoveRidePresentationHooks();
#if FOLLOWER_3GX_DIAGNOSTIC
  g_TerrainLodHook.Disable();
#endif
#if FOLLOWER_3GX_INTERACTION_FEATURES
  g_BattleSituationHook.Disable();
  ClearBattleWeatherHandoff();
#endif
#if FOLLOWER_3GX_PERFORMANCE_FEATURES
  g_SimpleLightPickUpHook.Disable();
  g_ShadowUpdateHook.Disable();
  g_MotionBlurStartHook.Disable();
#endif
  g_SurfHook.Disable();
  g_TerminateHook.Disable();
  g_UpdateHook.Disable();
  g_EventCheckHook.Disable();
  g_Follower3gxUpdateExpectedReturn = 0;
  g_Follower3gxTerminateExpectedReturn = 0;
  g_Installed = false;
}

bool AreFieldHooksInstalled()
{
  return g_Installed;
}

} // namespace Gen7Follower3gx
