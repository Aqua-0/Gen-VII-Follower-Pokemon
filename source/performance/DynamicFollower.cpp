#include "DynamicFollower.hpp"

#include <3ds.h>
#include <CTRPluginFramework/Graphics/OSD.hpp>
#include <CTRPluginFramework/Menu/PluginMenu.hpp>

#include "FollowerSettings.hpp"

namespace Gen7Follower3gx
{
namespace
{

const u64 SYSTEM_TICKS_PER_SECOND = 268111856ULL;
const u64 MAX_SAMPLE_TICKS = SYSTEM_TICKS_PER_SECOND / 4;
const u32 SAMPLE_FRAMES = 30;
const u32 DISABLE_FPS_TENTHS = 290;
const u32 ENABLE_FPS_TENTHS = 295;
const u32 DISABLE_WINDOWS = 2;
const u32 INITIAL_RECOVERY_WINDOWS = 8;
const u32 REENABLE_GRACE_WINDOWS = 3;
const u32 STABLE_FOLLOWER_WINDOWS = 20;
const u32 MAX_RETRY_LEVEL = 3;

u32 g_Initialized = 0;
u32 g_FieldReady = 0;
u32 g_ResetRequested = 0;
u32 g_AutoSuppressed = 0;
u32 g_FpsTenths = 0;
u64 g_PreviousTick = 0;
u64 g_WindowTicks = 0;
u32 g_WindowFrames = 0;
u32 g_SlowWindows = 0;
u32 g_RecoveryWindows = 0;
u32 g_GraceWindows = 0;
u32 g_StableFollowerWindows = 0;
u32 g_RetryLevel = 0;
bool g_PolicyWasActive = false;
bool g_ReenableProbe = false;

void ResetSampleWindow()
{
  g_PreviousTick = 0;
  g_WindowTicks = 0;
  g_WindowFrames = 0;
}

void ResetPolicyState(bool clearSuppression)
{
  ResetSampleWindow();
  g_SlowWindows = 0;
  g_RecoveryWindows = 0;
  g_GraceWindows = 0;
  g_StableFollowerWindows = 0;
  g_RetryLevel = 0;
  g_ReenableProbe = false;
  __atomic_store_n(&g_FpsTenths, 0U, __ATOMIC_RELAXED);
  if (clearSuppression)
  {
    __atomic_store_n(&g_AutoSuppressed, 0U, __ATOMIC_RELAXED);
  }
}

void ProcessPerformanceWindow(u32 fpsTenths)
{
  __atomic_store_n(&g_FpsTenths, fpsTenths, __ATOMIC_RELAXED);
  if (__atomic_load_n(&g_AutoSuppressed, __ATOMIC_RELAXED))
  {
    if (fpsTenths >= ENABLE_FPS_TENTHS)
    {
      ++g_RecoveryWindows;
    }
    else
    {
      g_RecoveryWindows = 0;
    }

    const u32 requiredWindows = INITIAL_RECOVERY_WINDOWS << g_RetryLevel;
    if (g_RecoveryWindows >= requiredWindows)
    {
      __atomic_store_n(&g_AutoSuppressed, 0U, __ATOMIC_RELAXED);
      g_RecoveryWindows = 0;
      g_SlowWindows = 0;
      g_GraceWindows = REENABLE_GRACE_WINDOWS;
      g_StableFollowerWindows = 0;
      g_ReenableProbe = true;
    }
    return;
  }

  if (g_GraceWindows > 0)
  {
    --g_GraceWindows;
    return;
  }

  if (fpsTenths < DISABLE_FPS_TENTHS)
  {
    ++g_SlowWindows;
    g_StableFollowerWindows = 0;
  }
  else
  {
    g_SlowWindows = 0;
    if (fpsTenths >= ENABLE_FPS_TENTHS)
    {
      ++g_StableFollowerWindows;
    }
    else
    {
      g_StableFollowerWindows = 0;
    }
  }

  if (g_StableFollowerWindows >= STABLE_FOLLOWER_WINDOWS)
  {
    g_RetryLevel = 0;
    g_ReenableProbe = false;
    g_StableFollowerWindows = STABLE_FOLLOWER_WINDOWS;
  }

  if (g_SlowWindows < DISABLE_WINDOWS)
  {
    return;
  }

  if (g_ReenableProbe && g_RetryLevel < MAX_RETRY_LEVEL)
  {
    ++g_RetryLevel;
  }
  __atomic_store_n(&g_AutoSuppressed, 1U, __ATOMIC_RELAXED);
  g_SlowWindows = 0;
  g_RecoveryWindows = 0;
  g_StableFollowerWindows = 0;
  g_ReenableProbe = false;
}

bool DynamicFollowerFrameCallback(const CTRPluginFramework::Screen& screen)
{
  if (!screen.IsTop ||
      !__atomic_load_n(&g_Initialized, __ATOMIC_RELAXED))
  {
    return false;
  }

  const bool policyActive =
    IsFollowerEnabled() && IsDynamicFollowerEnabled();
  if (!policyActive)
  {
    if (g_PolicyWasActive)
    {
      ResetPolicyState(true);
    }
    g_PolicyWasActive = false;
    return false;
  }

  if (!g_PolicyWasActive ||
      __atomic_exchange_n(&g_ResetRequested, 0U, __ATOMIC_RELAXED))
  {
    ResetPolicyState(true);
    g_PolicyWasActive = true;
  }

  CTRPluginFramework::PluginMenu* const menu =
    CTRPluginFramework::PluginMenu::GetRunningInstance();
  if (!__atomic_load_n(&g_FieldReady, __ATOMIC_RELAXED) ||
      (menu && menu->IsOpen()))
  {
    ResetSampleWindow();
    g_SlowWindows = 0;
    g_RecoveryWindows = 0;
    return false;
  }

  const u64 currentTick = svcGetSystemTick();
  if (!g_PreviousTick)
  {
    g_PreviousTick = currentTick;
    return false;
  }

  const u64 frameTicks = currentTick - g_PreviousTick;
  g_PreviousTick = currentTick;
  if (!frameTicks || frameTicks > MAX_SAMPLE_TICKS)
  {
    ResetSampleWindow();
    g_PreviousTick = currentTick;
    g_SlowWindows = 0;
    g_RecoveryWindows = 0;
    return false;
  }

  g_WindowTicks += frameTicks;
  ++g_WindowFrames;
  if (g_WindowFrames < SAMPLE_FRAMES)
  {
    return false;
  }

  const u32 fpsTenths = static_cast<u32>(
    (static_cast<u64>(g_WindowFrames) * SYSTEM_TICKS_PER_SECOND * 10ULL +
      g_WindowTicks / 2) / g_WindowTicks
    );
  g_WindowTicks = 0;
  g_WindowFrames = 0;
  ProcessPerformanceWindow(fpsTenths);
  return false;
}

} // namespace

void InitializeDynamicFollower()
{
  if (__atomic_exchange_n(&g_Initialized, 1U, __ATOMIC_RELAXED))
  {
    return;
  }
  __atomic_store_n(&g_FieldReady, 0U, __ATOMIC_RELAXED);
  __atomic_store_n(&g_ResetRequested, 0U, __ATOMIC_RELAXED);
  g_PolicyWasActive = false;
  ResetPolicyState(true);
  CTRPluginFramework::OSD::Run(DynamicFollowerFrameCallback);
}

void ShutdownDynamicFollower()
{
  if (!__atomic_exchange_n(&g_Initialized, 0U, __ATOMIC_RELAXED))
  {
    return;
  }
  CTRPluginFramework::OSD::Stop(DynamicFollowerFrameCallback);
  __atomic_store_n(&g_FieldReady, 0U, __ATOMIC_RELAXED);
  g_PolicyWasActive = false;
  ResetPolicyState(true);
}

void ResetDynamicFollowerForField()
{
  __atomic_store_n(&g_FieldReady, 0U, __ATOMIC_RELAXED);
  __atomic_store_n(&g_AutoSuppressed, 0U, __ATOMIC_RELAXED);
  __atomic_store_n(&g_ResetRequested, 1U, __ATOMIC_RELAXED);
}

void SetDynamicFollowerFieldReady(bool ready)
{
  __atomic_store_n(
    &g_FieldReady,
    ready ? 1U : 0U,
    __ATOMIC_RELAXED
    );
}

bool ShouldFollowerRun()
{
  return IsFollowerEnabled() &&
    (!IsDynamicFollowerEnabled() || !IsDynamicFollowerSuppressed());
}

bool IsDynamicFollowerSuppressed()
{
  return __atomic_load_n(&g_AutoSuppressed, __ATOMIC_RELAXED) != 0;
}

unsigned int GetDynamicFollowerFpsTenths()
{
  return __atomic_load_n(&g_FpsTenths, __ATOMIC_RELAXED);
}

} // namespace Gen7Follower3gx
