#include <3ds.h>
#include "types.h"
#include <CTRPluginFramework/System/FwkSettings.hpp>

#include "Diagnostics.hpp"
#include "DynamicFollower.hpp"
#include "FollowerSettings.hpp"
#include "RiderAnimationSettings.hpp"
#include "RideEventSettings.hpp"
#include "PartyFollowerSettings.hpp"
#include "LayeredFsConflict.hpp"
#include "PerformanceDiagnostics.hpp"
#include "PluginRuntime.hpp"
#include "SettingsMenu.hpp"
#include "MenuHotkeySettings.hpp"
#include "FollowMode.hpp"

namespace CTRPluginFramework
{

void PatchProcess(FwkSettings& settings)
{
  FOLLOWER_3GX_TRACE("PatchProcess");
  settings.AllowActionReplay = false;
  settings.AllowSearchEngine = false;
  settings.TryLoadSDSounds = false;
  settings.CloseMenuWithB = true;
  settings.WaitTimeToBoot = Time::Zero;
  Gen7Follower3gx::InitializeFollowMode();
  const bool settingsLoaded =
    Gen7Follower3gx::InitializeFollowerSettings();
  Gen7Follower3gx::InitializeMenuHotkeySettings();
  Gen7Follower3gx::InitializeRiderAnimationSettings();
  Gen7Follower3gx::InitializeRideEventSettings();
  Gen7Follower3gx::InitializePartyFollowerSettings();
  Gen7Follower3gx::InitializeDynamicFollower();
  Gen7Follower3gx::InitializeLayeredFsConflictDetection();
  Gen7Follower3gx::InitializePluginRuntime();
  Gen7Follower3gx::InitializePerformanceLevelDefaults(settingsLoaded);
  Gen7Follower3gx::InitializePerformanceDiagnostics();
}

void OnProcessExit()
{
  Gen7Follower3gx::ShutdownPerformanceDiagnostics();
  Gen7Follower3gx::ShutdownDynamicFollower();
  Gen7Follower3gx::ShutdownPluginRuntime();
  Gen7Follower3gx::ShutdownFollowerSettings();
}

int main()
{
  return Gen7Follower3gx::RunSettingsMenu();
}

} // namespace CTRPluginFramework
