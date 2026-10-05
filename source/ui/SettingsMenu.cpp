#include "FieldConvenience.hpp"
#include "SettingsMenu.hpp"
#include "MenuHotkeySettings.hpp"
#include "FollowMode.hpp"
#include "DecimalKeyboard.hpp"

#include <3ds.h>
#include <CTRPluginFramework/Menu/Keyboard.hpp>
#include <CTRPluginFramework/Menu/MenuEntry.hpp>
#include <CTRPluginFramework/Menu/MenuFolder.hpp>
#include <CTRPluginFramework/Menu/MessageBox.hpp>
#include <CTRPluginFramework/Menu/PluginMenu.hpp>
#include <CTRPluginFramework/System/Controller.hpp>
#include <CTRPluginFramework/System/System.hpp>
#include <CTRPluginFramework/Utils/Utils.hpp>

#include <string>
#include <vector>

#include "DynamicFollower.hpp"
#include "FreeCamera.hpp"
#include "FollowerSettings.hpp"
#include "RiderAnimationSettings.hpp"
#include "RideProfileMenu.hpp"
#include "RideEventSettings.hpp"
#include "PartyFollowerSettings.hpp"
#include "FollowerArena.hpp"
#include "MountedRideFeedback.hpp"
#include "LayeredFsConflict.hpp"
#include "PerformanceDiagnostics.hpp"
#if FOLLOWER_3GX_DIAGNOSTIC
#include "DiagnosticEffectSelector.hpp"
#include "DiagnosticWeatherSelector.hpp"
#endif

namespace Gen7Follower3gx
{
namespace
{

const int OUTLINE_OPTION_COUNT = 7;
const FollowerOutlineMode OUTLINE_OPTIONS[OUTLINE_OPTION_COUNT] =
{
  FOLLOWER_APPEARANCE_SOFT_FIELD,
  FOLLOWER_APPEARANCE_NATIVE_FIELD,
  FOLLOWER_APPEARANCE_NORMAL_EDGE,
  FOLLOWER_OUTLINE_ID_SOFT,
  FOLLOWER_OUTLINE_ID_MEDIUM,
  FOLLOWER_OUTLINE_ID_ORIGINAL,
  FOLLOWER_OUTLINE_OFF,
};

CTRPluginFramework::MenuEntry* g_pOutlineEntry = NULL;
CTRPluginFramework::MenuEntry* g_pFollowerEnabledEntry = NULL;
CTRPluginFramework::MenuEntry* g_pDynamicFollowerEntry = NULL;
CTRPluginFramework::MenuEntry* g_pFormChangeEntry = NULL;
CTRPluginFramework::MenuEntry* g_pSpecialEffectsEntry = NULL;
CTRPluginFramework::MenuEntry* g_pPokeBallSoundVolumeEntry = NULL;
CTRPluginFramework::MenuEntry*
  g_pBehaviorEntries[FOLLOWER_BEHAVIOR_SETTING_COUNT] = {};
CTRPluginFramework::MenuEntry* g_pAnimationCapEntry = NULL;
CTRPluginFramework::MenuEntry* g_pLayeredFsConflictEntry = NULL;
#if FOLLOWER_3GX_DIAGNOSTIC
const int DRAW_DISTANCE_OPTION_COUNT = 4;
const FollowerDrawDistanceMode
  DRAW_DISTANCE_OPTIONS[DRAW_DISTANCE_OPTION_COUNT] =
{
  FOLLOWER_DRAW_DISTANCE_FULL,
  FOLLOWER_DRAW_DISTANCE_HIGH,
  FOLLOWER_DRAW_DISTANCE_MEDIUM,
  FOLLOWER_DRAW_DISTANCE_LOW,
};
const int TERRAIN_DETAIL_OPTION_COUNT = 2;
const FollowerTerrainDetailMode
  TERRAIN_DETAIL_OPTIONS[TERRAIN_DETAIL_OPTION_COUNT] =
{
  FOLLOWER_TERRAIN_DETAIL_STOCK,
  FOLLOWER_TERRAIN_DETAIL_MEDIUM,
};
const int COLLISION_QUALITY_OPTION_COUNT = 3;
const FollowerCollisionQualityMode
  COLLISION_QUALITY_OPTIONS[COLLISION_QUALITY_OPTION_COUNT] =
{
  FOLLOWER_COLLISION_QUALITY_FULL,
  FOLLOWER_COLLISION_QUALITY_BALANCED,
  FOLLOWER_COLLISION_QUALITY_FAST,
};
const int ANIMATION_RATE_OPTION_COUNT = 6;
const FollowerAnimationRateMode
  ANIMATION_RATE_OPTIONS[ANIMATION_RATE_OPTION_COUNT] =
{
  FOLLOWER_ANIMATION_RATE_30_FPS,
  FOLLOWER_ANIMATION_RATE_15_FPS,
  FOLLOWER_ANIMATION_RATE_15_FPS_INTERPOLATED,
  FOLLOWER_ANIMATION_RATE_10_FPS_INTERPOLATED,
  FOLLOWER_ANIMATION_RATE_7_5_FPS_INTERPOLATED,
  FOLLOWER_ANIMATION_RATE_5_FPS_INTERPOLATED,
};

struct DiagnosticWeatherOption
{
  DiagnosticWeatherSelection selection;
  const char* name;
};

const DiagnosticWeatherOption DIAGNOSTIC_WEATHER_OPTIONS[] =
{
  { DIAGNOSTIC_WEATHER_MAP_DEFAULT, "Map default" },
  { DIAGNOSTIC_WEATHER_SUNNY, "Sunny" },
  { DIAGNOSTIC_WEATHER_CLOUDY, "Cloudy" },
  { DIAGNOSTIC_WEATHER_RAIN, "Rain" },
  { DIAGNOSTIC_WEATHER_THUNDERSTORM, "Thunderstorm" },
  { DIAGNOSTIC_WEATHER_SNOW, "Snow" },
  { DIAGNOSTIC_WEATHER_SNOWSTORM, "Snowstorm" },
  { DIAGNOSTIC_WEATHER_DRY, "Dry" },
  { DIAGNOSTIC_WEATHER_SANDSTORM, "Sandstorm" },
  { DIAGNOSTIC_WEATHER_MIST, "Mist" },
  { DIAGNOSTIC_WEATHER_SUNSHOWER, "Sunshower" },
  { DIAGNOSTIC_WEATHER_DIAMOND_DUST, "Diamond dust" },
};
const int DIAGNOSTIC_WEATHER_OPTION_COUNT =
  sizeof(DIAGNOSTIC_WEATHER_OPTIONS) /
  sizeof(DIAGNOSTIC_WEATHER_OPTIONS[0]);

struct DiagnosticEffectHeapGuardOption
{
  unsigned int minimumFreeBytes;
  const char* name;
};

const DiagnosticEffectHeapGuardOption DIAGNOSTIC_EFFECT_HEAP_GUARD_OPTIONS[] =
{
  { 0x20000, "128 KiB (default)" },
  { 0x1c000, "112 KiB" },
  { 0x18000, "96 KiB" },
  { 0x14000, "80 KiB" },
  { 0x10000, "64 KiB" },
  { 0x0c000, "48 KiB" },
  { 0x08000, "32 KiB" },
  { 0x04000, "16 KiB" },
  { 0x00000, "No guard (unsafe)" },
};
const int DIAGNOSTIC_EFFECT_HEAP_GUARD_OPTION_COUNT =
  sizeof(DIAGNOSTIC_EFFECT_HEAP_GUARD_OPTIONS) /
  sizeof(DIAGNOSTIC_EFFECT_HEAP_GUARD_OPTIONS[0]);
#endif

enum PerformancePreset
{
  PERFORMANCE_PRESET_RETAIL,
  PERFORMANCE_PRESET_MEDIUM,
  PERFORMANCE_PRESET_LOW,
  PERFORMANCE_PRESET_LITE,
  PERFORMANCE_PRESET_COUNT,
};

struct PerformancePresetDescriptor
{
  const char* name;
  PerformanceOptionMask options;
  FollowerCollisionQualityMode collision;
  FollowerAnimationRateMode animation;
};

const PerformanceOptionMask PERFORMANCE_MEDIUM_OPTIONS =
  (1U << PERFORMANCE_OPTION_HIDE_FOLLOWER_SHADOW) |
  (1U << PERFORMANCE_OPTION_DISABLE_FIELD_OUTLINES) |
  (1U << PERFORMANCE_OPTION_DISABLE_BLOOM) |
  (1U << PERFORMANCE_OPTION_REDUCED_TRANSFORM_PRECISION) |
  (1U << PERFORMANCE_OPTION_DISABLE_DEPTH_OF_FIELD) |
  (1U << PERFORMANCE_OPTION_DISABLE_MOTION_BLUR);
const PerformanceOptionMask PERFORMANCE_LOW_OPTIONS =
  PERFORMANCE_MEDIUM_OPTIONS |
  (1U << PERFORMANCE_OPTION_DISABLE_WEATHER) |
  (1U << PERFORMANCE_OPTION_DISABLE_CHARACTER_SHADOWS) |
  (1U << PERFORMANCE_OPTION_SIMPLE_WORLD_LIGHTING);
const PerformanceOptionMask PERFORMANCE_LITE_OPTIONS =
  ((1U << PERFORMANCE_OPTION_COUNT) - 1U) &
  ~(1U << PERFORMANCE_OPTION_LITE_FOLLOWER_MATERIALS);

const PerformancePresetDescriptor
  PERFORMANCE_PRESETS[PERFORMANCE_PRESET_COUNT] =
{
  {
    "Retail",
    0U,
    FOLLOWER_COLLISION_QUALITY_FULL,
    FOLLOWER_ANIMATION_RATE_30_FPS,
  },
  {
    "Medium",
    PERFORMANCE_MEDIUM_OPTIONS,
    FOLLOWER_COLLISION_QUALITY_BALANCED,
    FOLLOWER_ANIMATION_RATE_15_FPS_INTERPOLATED,
  },
  {
    "Low",
    PERFORMANCE_LOW_OPTIONS,
    FOLLOWER_COLLISION_QUALITY_FAST,
    FOLLOWER_ANIMATION_RATE_10_FPS_INTERPOLATED,
  },
  {
    "Lite",
    PERFORMANCE_LITE_OPTIONS,
    FOLLOWER_COLLISION_QUALITY_FAST,
    FOLLOWER_ANIMATION_RATE_5_FPS_INTERPOLATED,
  },
};

PerformanceOptionMask GetPerformancePresetOptions(
  const PerformancePresetDescriptor& preset
)
{
  return FilterSupportedPerformanceOptions(preset.options);
}

void ApplyPerformancePreset(const PerformancePresetDescriptor& preset)
{
  SetPerformanceOptions(GetPerformancePresetOptions(preset));
  SetFollowerCollisionQualityMode(preset.collision);
  SetFollowerAnimationRateMode(preset.animation);
}

CTRPluginFramework::MenuEntry* g_pPerformancePresetEntry = NULL;
CTRPluginFramework::MenuEntry* g_pFreeCameraEntry = NULL;
#if FOLLOWER_3GX_DIAGNOSTIC
CTRPluginFramework::MenuEntry* g_pDrawDistanceEntry = NULL;
CTRPluginFramework::MenuEntry* g_pTerrainDetailEntry = NULL;
CTRPluginFramework::MenuEntry* g_pCollisionQualityEntry = NULL;
CTRPluginFramework::MenuEntry* g_pAnimationRateEntry = NULL;
CTRPluginFramework::MenuEntry* g_pDiagnosticsEntry = NULL;
CTRPluginFramework::MenuEntry* g_pWeatherSelectorEntry = NULL;
CTRPluginFramework::MenuEntry* g_pEffectSelectorEntry = NULL;
CTRPluginFramework::MenuEntry* g_pEffectHeapGuardEntry = NULL;
CTRPluginFramework::MenuEntry*
  g_pPerformanceEntries[PERFORMANCE_OPTION_COUNT] = {};
#endif

struct BehaviorOption
{
  FollowerBehaviorSetting setting;
  const char* name;
  bool integer;
};

BehaviorOption BEHAVIOR_OPTIONS[FOLLOWER_BEHAVIOR_SETTING_COUNT] =
{
  { FOLLOWER_BEHAVIOR_TRAIL_DELAY, "Trail delay", true },
  { FOLLOWER_BEHAVIOR_STOP_DISTANCE, "Stop gap", false },
  { FOLLOWER_BEHAVIOR_START_DISTANCE, "Start moving", false },
  { FOLLOWER_BEHAVIOR_RUN_DISTANCE, "Start running", false },
  { FOLLOWER_BEHAVIOR_WALK_SPEED, "Walk speed", false },
  { FOLLOWER_BEHAVIOR_RUN_SPEED, "Run speed", false },
  { FOLLOWER_BEHAVIOR_CATCH_UP_STEP, "Catch-up boost", false },
  { FOLLOWER_BEHAVIOR_WARP_DISTANCE, "Teleport at", false },
};

BehaviorOption* g_pEditingBehavior = NULL;

#if FOLLOWER_3GX_DIAGNOSTIC
struct PerformanceOptionDescriptor
{
  PerformanceOption option;
  const char* name;
  const char* note;
};

PerformanceOptionDescriptor PERFORMANCE_OPTIONS[PERFORMANCE_OPTION_COUNT] =
{
  {
    PERFORMANCE_OPTION_LITE_FOLLOWER_MATERIALS,
    "Simplified follower materials",
    "Disables follower pixel lighting, fog, bump mapping, rim lighting, and "
    "Phong highlights. This can noticeably change the model's appearance."
  },
  {
    PERFORMANCE_OPTION_HIDE_FOLLOWER_SHADOW,
    "Hide follower shadow",
    "Stops drawing the follower's projected field shadow."
  },
  {
    PERFORMANCE_OPTION_DISABLE_FIELD_OUTLINES,
    "Disable field outlines",
    "Disables the field edge pass, including the follower outline."
  },
  {
    PERFORMANCE_OPTION_DISABLE_BLOOM,
    "Disable bloom",
    "Disables the field bloom post-processing pass."
  },
  {
    PERFORMANCE_OPTION_DISABLE_EFFECTS,
    "Disable particle effects",
    "Disables field particle-effect rendering. Some gameplay cues may become "
    "invisible."
  },
  {
    PERFORMANCE_OPTION_DISABLE_WEATHER,
    "Disable weather",
    "Disables the field weather-board render path."
  },
  {
    PERFORMANCE_OPTION_DISABLE_CHARACTER_SHADOWS,
    "Disable character shadows",
    "Hides dynamic player and NPC shadow models and skips their retail joint "
    "and ground-alignment updates. Baked map lighting remains."
  },
  {
    PERFORMANCE_OPTION_DISABLE_SKYBOX,
    "Disable skybox",
    "Disables sky drawable rendering. Outdoor backgrounds may appear empty."
  },
  {
    PERFORMANCE_OPTION_REDUCED_TRANSFORM_PRECISION,
    "Reduced transform precision",
    "Uses the cheaper world-space transform path for field, edge, and sky "
    "drawing. Models may jitter when the map is far from the origin."
  },
  {
    PERFORMANCE_OPTION_DISABLE_DEPTH_OF_FIELD,
    "Disable depth of field",
    "Skips the field depth-of-field passes when a map or event enables them."
  },
  {
    PERFORMANCE_OPTION_DISABLE_MOTION_BLUR,
    "Disable motion blur",
    "Prevents field events from starting the motion-blur render path."
  },
  {
    PERFORMANCE_OPTION_SIMPLE_WORLD_LIGHTING,
    "Simple world lighting",
    "Keeps scene ambient and directional lights but skips per-object point "
    "and spot lights. Locally illuminated areas can look flatter."
  },
};
#endif

// CTRPF 0.8 has no public hotkey setter, so use its preferences symbol.
extern u32 g_CtrpfMenuHotkeys
  asm("_ZN18CTRPluginFramework11Preferences11MenuHotkeysE");

void ApplyMenuHotkey()
{
  g_CtrpfMenuHotkeys = GetMenuHotkey();
}

std::string MenuHotkeyLabel()
{
  CTRPluginFramework::Hotkey hotkey(GetMenuHotkey(),"Open plugin menu");
  return "Menu shortcut: "+hotkey.ToString();
}

void EditMenuHotkey(CTRPluginFramework::MenuEntry* entry)
{
  CTRPluginFramework::Keyboard keyboard("Menu shortcut", {
    "L + D-pad Down + Select", "Choose buttons...", "Start + Select (original)"});
  const int selected=keyboard.Open();
  if (selected<0) return;
  unsigned int keys=selected==0 ? AlternateMenuHotkey : DefaultMenuHotkey;
  if (selected==1) {
    CTRPluginFramework::Hotkey hotkey(GetMenuHotkey(),"Open plugin menu (B: review selection)");
    hotkey.AskForKeys();
    keys=hotkey.GetKeys();
    CTRPluginFramework::Keyboard confirm("Menu shortcut",{
      "Use "+hotkey.ToString(), "Cancel"});
    if (confirm.Open()!=0) return;
  }
  if (!IsValidMenuHotkey(keys)) {
    CTRPluginFramework::MessageBox("Choose at least one button and no opposite D-pad directions.")();
    return;
  }
  if (!SetMenuHotkey(keys)) {
    CTRPluginFramework::MessageBox("Could not save the shortcut. The previous shortcut is still active.")();
    return;
  }
  ApplyMenuHotkey();
  entry->Name()=MenuHotkeyLabel();
}

void SelectFollowMode(CTRPluginFramework::MenuEntry* entry)
{
  CTRPluginFramework::Keyboard keyboard("Following mode", {"Standard (default)","Close"});
  keyboard.ChangeSelectedEntry(IsCloseFollowEnabled() ? 1 : 0);
  const int choice=keyboard.Open();
  if (choice<0) return;
  if (!SetCloseFollowEnabled(choice==1)) {
    CTRPluginFramework::MessageBox("Could not save the following mode.")();
    return;
  }
  entry->Name()=std::string("Following mode: ")+(IsCloseFollowEnabled() ? "Close" : "Standard");
}

void RefreshLayeredFsConflictEntry()
{
  if (!g_pLayeredFsConflictEntry)
  {
    return;
  }

  const bool hasConflict = HasLayeredFsConflict();
  if (hasConflict && !g_pLayeredFsConflictEntry->IsVisible())
  {
    g_pLayeredFsConflictEntry->Show();
  }
  else if (!hasConflict && g_pLayeredFsConflictEntry->IsVisible())
  {
    g_pLayeredFsConflictEntry->Hide();
  }
}

void ShowLayeredFsConflict(CTRPluginFramework::MenuEntry*)
{
  CTRPluginFramework::MessageBox(
    "LayeredFS conflict",
    BuildLayeredFsConflictMessage()
    )();
}

void PluginFrameCallback()
{
  ApplyMenuHotkey();
  RefreshLayeredFsConflictEntry();
  if (ConsumeLayeredFsConflictWarning())
  {
    ShowLayeredFsConflict(NULL);
  }
}

void ApplyFollowerEnabled(CTRPluginFramework::MenuEntry* entry)
{
  if (entry->WasJustActivated())
  {
    SetFollowerEnabled(true);
  }
  else if (!entry->IsActivated())
  {
    SetFollowerEnabled(false);
  }
}

void ApplyDynamicFollowerEnabled(CTRPluginFramework::MenuEntry* entry)
{
  if (entry->WasJustActivated())
  {
    SetDynamicFollowerEnabled(true);
  }
  else if (!entry->IsActivated())
  {
    SetDynamicFollowerEnabled(false);
  }
}

void ApplyFormChangeEnabled(CTRPluginFramework::MenuEntry* entry)
{
  if (entry->WasJustActivated())
  {
    SetFollowerFormChangeEnabled(true);
  }
  else if (!entry->IsActivated())
  {
    SetFollowerFormChangeEnabled(false);
  }
}

void ApplySpecialEffectsEnabled(CTRPluginFramework::MenuEntry* entry)
{
  if (entry->WasJustActivated())
  {
    SetFollowerSpecialEffectsEnabled(true);
  }
  else if (!entry->IsActivated())
  {
    SetFollowerSpecialEffectsEnabled(false);
  }
}

void RefreshToggleEntry(
  CTRPluginFramework::MenuEntry* entry,
  bool enabled
)
{
  if (!entry)
  {
    return;
  }
  if (enabled && !entry->IsActivated())
  {
    entry->Enable();
  }
  else if (!enabled && entry->IsActivated())
  {
    entry->Disable();
  }
}

void RefreshFollowerToggleEntries()
{
  RefreshToggleEntry(g_pFollowerEnabledEntry, IsFollowerEnabled());
  RefreshToggleEntry(g_pDynamicFollowerEntry, IsDynamicFollowerEnabled());
  RefreshToggleEntry(g_pFormChangeEntry, IsFollowerFormChangeEnabled());
  RefreshToggleEntry(
    g_pSpecialEffectsEntry,
    IsFollowerSpecialEffectsEnabled()
    );
  if (g_pDynamicFollowerEntry)
  {
    g_pDynamicFollowerEntry->Name() = IsDynamicFollowerSuppressed()
      ? "Auto-hide when slow (hidden)"
      : "Auto-hide when slow";
  }
}

void RefreshPokeBallSoundVolumeEntry()
{
  if (!g_pPokeBallSoundVolumeEntry)
  {
    return;
  }
  g_pPokeBallSoundVolumeEntry->Name() =
    CTRPluginFramework::Utils::Format(
      "Poke Ball sound volume: %u%%",
      GetFollowerPokeBallSoundVolumePercent()
      );
}

bool ValidatePokeBallSoundVolume(const void* pInput, std::string& error)
{
  const u32 volumePercent = *static_cast<const u32*>(pInput);
  if (volumePercent <= 100U)
  {
    return true;
  }
  error = "Enter a volume from 0 to 100 percent.";
  return false;
}

void EditPokeBallSoundVolume(CTRPluginFramework::MenuEntry*)
{
  u32 volumePercent = GetFollowerPokeBallSoundVolumePercent();
  CTRPluginFramework::Keyboard keyboard(
    "Poke Ball sound volume (0-100%)"
    );
  keyboard.SetCompareCallback(ValidatePokeBallSoundVolume);
  if (keyboard.Open(volumePercent, volumePercent) >= 0 &&
      SetFollowerPokeBallSoundVolumePercent(volumePercent))
  {
    RefreshPokeBallSoundVolumeEntry();
  }
}

int FindOutlineOption(FollowerOutlineMode mode)
{
  for (int i = 0; i < OUTLINE_OPTION_COUNT; ++i)
  {
    if (OUTLINE_OPTIONS[i] == mode)
    {
      return i;
    }
  }
  return 0;
}

void RefreshOutlineEntry()
{
  if (!g_pOutlineEntry)
  {
    return;
  }
  g_pOutlineEntry->Name() = std::string("Follower appearance: ") +
    GetFollowerOutlineModeName(GetFollowerOutlineMode());
}

void SelectOutlineMode(CTRPluginFramework::MenuEntry*)
{
  std::vector<std::string> options;
  options.reserve(OUTLINE_OPTION_COUNT);
  for (int i = 0; i < OUTLINE_OPTION_COUNT; ++i)
  {
    options.push_back(GetFollowerOutlineModeName(OUTLINE_OPTIONS[i]));
  }

  CTRPluginFramework::Keyboard keyboard("Follower appearance - Native field preserves lighting and outlines", options);
  keyboard.ChangeSelectedEntry(FindOutlineOption(GetFollowerOutlineMode()));
  const int selection = keyboard.Open();
  if (selection >= 0 && selection < OUTLINE_OPTION_COUNT)
  {
    SetFollowerOutlineMode(OUTLINE_OPTIONS[selection]);
    RefreshOutlineEntry();
  }
}

int FindPerformancePreset()
{
  const PerformanceOptionMask options = GetPerformanceOptions();
  const FollowerCollisionQualityMode collision =
    GetFollowerCollisionQualityMode();
  const FollowerAnimationRateMode animation = GetFollowerAnimationRateMode();
  for (int i = 0; i < PERFORMANCE_PRESET_COUNT; ++i)
  {
    const PerformancePresetDescriptor& preset = PERFORMANCE_PRESETS[i];
    if (GetPerformancePresetOptions(preset) == options &&
        preset.collision == collision &&
        preset.animation == animation)
    {
      return i;
    }
  }
  return -1;
}

void RefreshPerformancePresetEntry()
{
  if (!g_pPerformancePresetEntry)
  {
    return;
  }
  const int preset = FindPerformancePreset();
  g_pPerformancePresetEntry->Name() = std::string("Performance level: ") +
    (preset >= 0 ? PERFORMANCE_PRESETS[preset].name : "Custom");
}

#if FOLLOWER_3GX_DIAGNOSTIC
int FindDrawDistanceOption(FollowerDrawDistanceMode mode)
{
  for (int i = 0; i < DRAW_DISTANCE_OPTION_COUNT; ++i)
  {
    if (DRAW_DISTANCE_OPTIONS[i] == mode)
    {
      return i;
    }
  }
  return 0;
}

void RefreshDrawDistanceEntry()
{
  if (!g_pDrawDistanceEntry)
  {
    return;
  }
  g_pDrawDistanceEntry->Name() = std::string("Draw distance: ") +
    GetFollowerDrawDistanceModeName(GetFollowerDrawDistanceMode());
}

void SelectDrawDistance(CTRPluginFramework::MenuEntry*)
{
  std::vector<std::string> options;
  options.reserve(DRAW_DISTANCE_OPTION_COUNT);
  for (int i = 0; i < DRAW_DISTANCE_OPTION_COUNT; ++i)
  {
    options.push_back(
      GetFollowerDrawDistanceModeName(DRAW_DISTANCE_OPTIONS[i])
      );
  }

  CTRPluginFramework::Keyboard keyboard("Draw distance", options);
  keyboard.ChangeSelectedEntry(
    FindDrawDistanceOption(GetFollowerDrawDistanceMode())
    );
  const int selection = keyboard.Open();
  if (selection >= 0 && selection < DRAW_DISTANCE_OPTION_COUNT)
  {
    SetFollowerDrawDistanceMode(DRAW_DISTANCE_OPTIONS[selection]);
    RefreshDrawDistanceEntry();
  }
}

void RefreshTerrainDetailEntry()
{
  if (g_pTerrainDetailEntry)
  {
    g_pTerrainDetailEntry->Name() = std::string("Terrain detail: ") +
      GetFollowerTerrainDetailModeName(GetFollowerTerrainDetailMode());
  }
}

void SelectTerrainDetail(CTRPluginFramework::MenuEntry*)
{
  std::vector<std::string> options;
  int selected = 0;
  for (int i = 0; i < TERRAIN_DETAIL_OPTION_COUNT; ++i)
  {
    options.push_back(GetFollowerTerrainDetailModeName(TERRAIN_DETAIL_OPTIONS[i]));
    if (TERRAIN_DETAIL_OPTIONS[i] == GetFollowerTerrainDetailMode())
    {
      selected = i;
    }
  }
  CTRPluginFramework::Keyboard keyboard("Terrain detail", options);
  keyboard.ChangeSelectedEntry(selected);
  const int selection = keyboard.Open();
  if (selection >= 0 && selection < TERRAIN_DETAIL_OPTION_COUNT)
  {
    SetFollowerTerrainDetailMode(TERRAIN_DETAIL_OPTIONS[selection]);
    RefreshTerrainDetailEntry();
  }
}

void RefreshCollisionQualityEntry()
{
  if (g_pCollisionQualityEntry)
  {
    g_pCollisionQualityEntry->Name() = std::string("Follower collision: ") +
      GetFollowerCollisionQualityModeName(GetFollowerCollisionQualityMode());
  }
}

void SelectCollisionQuality(CTRPluginFramework::MenuEntry*)
{
  std::vector<std::string> options;
  int selected = 0;
  for (int i = 0; i < COLLISION_QUALITY_OPTION_COUNT; ++i)
  {
    options.push_back(
      GetFollowerCollisionQualityModeName(COLLISION_QUALITY_OPTIONS[i])
      );
    if (COLLISION_QUALITY_OPTIONS[i] == GetFollowerCollisionQualityMode())
    {
      selected = i;
    }
  }
  CTRPluginFramework::Keyboard keyboard("Follower collision", options);
  keyboard.ChangeSelectedEntry(selected);
  const int selection = keyboard.Open();
  if (selection >= 0 && selection < COLLISION_QUALITY_OPTION_COUNT)
  {
    SetFollowerCollisionQualityMode(COLLISION_QUALITY_OPTIONS[selection]);
    RefreshCollisionQualityEntry();
    RefreshPerformancePresetEntry();
  }
}

void RefreshAnimationRateEntry()
{
  if (g_pAnimationRateEntry)
  {
    g_pAnimationRateEntry->Name() = std::string("Follower animation: ") +
      GetFollowerAnimationRateModeName(GetFollowerAnimationRateMode());
  }
}

void SelectAnimationRate(CTRPluginFramework::MenuEntry*)
{
  std::vector<std::string> options;
  int selected = 0;
  for (int i = 0; i < ANIMATION_RATE_OPTION_COUNT; ++i)
  {
    options.push_back(GetFollowerAnimationRateModeName(ANIMATION_RATE_OPTIONS[i]));
    if (ANIMATION_RATE_OPTIONS[i] == GetFollowerAnimationRateMode())
    {
      selected = i;
    }
  }
  CTRPluginFramework::Keyboard keyboard("Follower animation", options);
  keyboard.ChangeSelectedEntry(selected);
  const int selection = keyboard.Open();
  if (selection >= 0 && selection < ANIMATION_RATE_OPTION_COUNT)
  {
    SetFollowerAnimationRateMode(ANIMATION_RATE_OPTIONS[selection]);
    RefreshAnimationRateEntry();
    RefreshPerformancePresetEntry();
  }
}
#endif

std::string FormatBehaviorEntry(const BehaviorOption& option)
{
  const float value = GetFollowerBehaviorValue(option.setting);
  if (option.integer)
  {
    return CTRPluginFramework::Utils::Format(
      "%s: %u frames",
      option.name,
      static_cast<unsigned int>(value)
      );
  }
  return CTRPluginFramework::Utils::Format(
    "%s: %.1f",
    option.name,
    static_cast<double>(value)
    );
}

void RefreshBehaviorEntries()
{
  for (int i = 0; i < FOLLOWER_BEHAVIOR_SETTING_COUNT; ++i)
  {
    if (g_pBehaviorEntries[i])
    {
      g_pBehaviorEntries[i]->Name() = FormatBehaviorEntry(BEHAVIOR_OPTIONS[i]);
    }
  }
}

void RefreshAnimationCapEntry()
{
  if (!g_pAnimationCapEntry)
  {
    return;
  }
  g_pAnimationCapEntry->Name() = IsFollowerAnimationPlaybackUncapped()
    ? "Animation playback cap: Off"
    : "Animation playback cap: On";
}

void GetEffectiveBehaviorRange(
  FollowerBehaviorSetting setting,
  float* pMinimum,
  float* pMaximum
  )
{
  float minimum = GetFollowerBehaviorMinimum(setting);
  float maximum = GetFollowerBehaviorMaximum(setting);
  switch (setting)
  {
  case FOLLOWER_BEHAVIOR_STOP_DISTANCE:
    if (maximum > GetFollowerStartDistance())
    {
      maximum = GetFollowerStartDistance();
    }
    break;
  case FOLLOWER_BEHAVIOR_START_DISTANCE:
    if (minimum < GetFollowerStopDistance())
    {
      minimum = GetFollowerStopDistance();
    }
    if (maximum > GetFollowerRunDistance())
    {
      maximum = GetFollowerRunDistance();
    }
    break;
  case FOLLOWER_BEHAVIOR_RUN_DISTANCE:
    if (minimum < GetFollowerStartDistance())
    {
      minimum = GetFollowerStartDistance();
    }
    if (maximum > GetFollowerWarpDistance() - 50.0f)
    {
      maximum = GetFollowerWarpDistance() - 50.0f;
    }
    break;
  case FOLLOWER_BEHAVIOR_WALK_SPEED:
    if (maximum > GetFollowerRunSpeed())
    {
      maximum = GetFollowerRunSpeed();
    }
    break;
  case FOLLOWER_BEHAVIOR_RUN_SPEED:
    if (minimum < GetFollowerWalkSpeed())
    {
      minimum = GetFollowerWalkSpeed();
    }
    break;
  case FOLLOWER_BEHAVIOR_WARP_DISTANCE:
    if (minimum < GetFollowerRunDistance() + 50.0f)
    {
      minimum = GetFollowerRunDistance() + 50.0f;
    }
    break;
  default:
    break;
  }

  *pMinimum = minimum;
  *pMaximum = maximum;
}

bool ValidateBehaviorValue(float value, std::string& error)
{
  if (g_pEditingBehavior &&
      IsFollowerBehaviorValueValid(g_pEditingBehavior->setting, value))
  {
    return true;
  }

  if (!g_pEditingBehavior)
  {
    error = "No behavior setting is active.";
    return false;
  }

  float minimum = 0.0f;
  float maximum = 0.0f;
  GetEffectiveBehaviorRange(
    g_pEditingBehavior->setting,
    &minimum,
    &maximum
    );
  if (g_pEditingBehavior->integer)
  {
    error = CTRPluginFramework::Utils::Format(
      "Enter a whole number from %u to %u.",
      static_cast<unsigned int>(minimum),
      static_cast<unsigned int>(maximum)
      );
  }
  else
  {
    error = CTRPluginFramework::Utils::Format(
      "Enter a value from %.1f to %.1f.",
      static_cast<double>(minimum),
      static_cast<double>(maximum)
      );
  }
  return false;
}

bool ValidateBehaviorU32(const void* pInput, std::string& error)
{
  return ValidateBehaviorValue(
    static_cast<float>(*static_cast<const u32*>(pInput)),
    error
    );
}

void EditBehaviorSetting(CTRPluginFramework::MenuEntry* entry)
{
  BehaviorOption* const option =
    static_cast<BehaviorOption*>(entry ? entry->GetArg() : NULL);
  if (!option)
  {
    return;
  }

  g_pEditingBehavior = option;
  bool changed = false;
  if (option->integer)
  {
    u32 value = static_cast<u32>(GetFollowerBehaviorValue(option->setting));
    CTRPluginFramework::Keyboard keyboard(option->name);
    keyboard.SetCompareCallback(ValidateBehaviorU32);
    if (keyboard.Open(value, value) >= 0)
    {
      changed = SetFollowerBehaviorValue(
        option->setting,
        static_cast<float>(value)
        );
    }
  }
  else
  {
    float value = GetFollowerBehaviorValue(option->setting);
    float minimum=0.0f, maximum=0.0f;
    GetEffectiveBehaviorRange(option->setting,&minimum,&maximum);
    if (EditDecimalValue(option->name,value,minimum,maximum))
    {
      changed = SetFollowerBehaviorValue(option->setting, value);
    }
  }
  g_pEditingBehavior = NULL;

  if (changed)
  {
    RefreshBehaviorEntries();
  }
}

void ToggleAnimationPlaybackCap(CTRPluginFramework::MenuEntry*)
{
  if (IsFollowerAnimationPlaybackUncapped())
  {
    SetFollowerAnimationPlaybackUncapped(false);
    RefreshAnimationCapEntry();
    return;
  }

  std::vector<std::string> options;
  options.push_back("Cancel");
  options.push_back("Remove playback cap");
  CTRPluginFramework::Keyboard keyboard(
    "Warning: uncapped playback can look unnatural and may worsen clipping "
    "or collision at extreme speeds.",
    options
    );
  keyboard.ChangeSelectedEntry(0);
  if (keyboard.Open() == 1)
  {
    SetFollowerAnimationPlaybackUncapped(true);
  }
  RefreshAnimationCapEntry();
}

void ResetBehaviorDefaults(CTRPluginFramework::MenuEntry*)
{
  ResetFollowerBehaviorDefaults();
  RefreshBehaviorEntries();
  RefreshAnimationCapEntry();
}

#if FOLLOWER_3GX_DIAGNOSTIC
void ApplyPerformanceOption(CTRPluginFramework::MenuEntry* entry)
{
  PerformanceOptionDescriptor* const descriptor =
    static_cast<PerformanceOptionDescriptor*>(
      entry ? entry->GetArg() : NULL
      );
  if (!descriptor)
  {
    return;
  }

  if (entry->WasJustActivated())
  {
    SetPerformanceOptionEnabled(descriptor->option, true);
  }
  else if (!entry->IsActivated())
  {
    SetPerformanceOptionEnabled(descriptor->option, false);
  }
  RefreshPerformancePresetEntry();
}

void RefreshPerformanceOptionEntries()
{
  for (int i = 0; i < PERFORMANCE_OPTION_COUNT; ++i)
  {
    CTRPluginFramework::MenuEntry* const entry = g_pPerformanceEntries[i];
    if (!entry)
    {
      continue;
    }

    const bool shouldBeEnabled =
      IsPerformanceOptionEnabled(PERFORMANCE_OPTIONS[i].option);
    if (shouldBeEnabled && !entry->IsActivated())
    {
      entry->Enable();
    }
    else if (!shouldBeEnabled && entry->IsActivated())
    {
      entry->Disable();
    }
  }
}
#endif

const char* MountedInteractionName(MountedInteractionMode mode)
{
  switch (mode) {
  case MOUNTED_INTERACTION_DISMOUNT: return "Dismount and interact";
  case MOUNTED_INTERACTION_KEEP: return "Keep riding (test)";
  default: return "Block";
  }
}
void EditPartyFollowers(CTRPluginFramework::MenuEntry*)
{
  auto settings=GetPartyFollowerSettings();
  int selected=0;
  for (;;) {
    std::vector<std::string> options;
    options.push_back("Follower count: "+std::to_string(settings.count));
    for (unsigned int i=0;i<PartyFollowerMaximum;++i)
      options.push_back(std::string(i ? "Extra follower " : "Main follower ")+std::to_string(i+1)+": "+
        (settings.slots[i] ? "Party slot "+std::to_string(settings.slots[i]) : "Automatic"));
    CTRPluginFramework::Keyboard keyboard("Party followers - capacity this session: "+
      std::to_string(GetFollowerArenaCapacity())+"\nChanges save immediately. B: back.",options);
    keyboard.ChangeSelectedEntry(selected);
    const int action=keyboard.Open();
    if (action<0) return;
    selected=action;
    auto next=settings;
    if (action==0) {
      CTRPluginFramework::Keyboard count("Number of followers",{"1","2 (experimental)","3 (experimental)"});
      count.ChangeSelectedEntry(settings.count-1);
      const int value=count.Open();
      if (value<0 || static_cast<unsigned int>(value+1)==settings.count) continue;
      next.count=value+1;
    } else if (action<=3) {
      CTRPluginFramework::Keyboard slot("Select party position (empty slots/eggs stay absent)",
        {"Automatic: next eligible unused slot","Party slot 1","Party slot 2","Party slot 3","Party slot 4","Party slot 5","Party slot 6"});
      slot.ChangeSelectedEntry(settings.slots[action-1]);
      const int value=slot.Open();
      if (value<0 || static_cast<unsigned int>(value)==settings.slots[action-1]) continue;
      next.slots[action-1]=value;
    } else continue;
    if (!SavePartyFollowerSettings(next)) {
      CTRPluginFramework::MessageBox("Could not save this change. The previous value has been kept.")();
      continue;
    }
    if (next.count!=settings.count && next.count>GetFollowerArenaCapacity())
      Follower3gx_NotifyRide("Follower count saved - restart game for additional capacity");
    settings=next;
  }
}

void SelectMountedInteraction(CTRPluginFramework::MenuEntry* entry)
{
  CTRPluginFramework::Keyboard keyboard("Mounted A interactions",{
    "Block (original behavior)","Dismount and interact","Keep riding (experimental)"});
  keyboard.ChangeSelectedEntry(GetMountedInteractionMode());
  const int choice=keyboard.Open();
  if (choice<0 || choice>=MOUNTED_INTERACTION_COUNT) return;
  if (!SetRideEventSettings(static_cast<MountedInteractionMode>(choice),KeepFollowerVisibleDuringEvents())) {
    Follower3gx_NotifyRide("Could not save interaction test settings"); return;
  }
  entry->Name()=std::string("Talk while riding: ")+MountedInteractionName(GetMountedInteractionMode());
}
void SelectEventFollowerVisibility(CTRPluginFramework::MenuEntry* entry)
{
  CTRPluginFramework::Keyboard keyboard("Unmounted follower during field events",{
    "Hide (original behavior)","Keep visible (experimental)"});
  keyboard.ChangeSelectedEntry(KeepFollowerVisibleDuringEvents() ? 1 : 0);
  const int choice=keyboard.Open();
  if (choice<0) return;
  if (!SetRideEventSettings(GetMountedInteractionMode(),choice==1)) {
    Follower3gx_NotifyRide("Could not save interaction test settings"); return;
  }
  entry->Name()=std::string("Follower during events: ")+(KeepFollowerVisibleDuringEvents() ? "Keep visible (test)" : "Hide");
}

void OpenPartyPc(CTRPluginFramework::MenuEntry*)
{
  Follower3gx_NotifyRide(RequestOpenPc()
    ? "PC requested - close the plugin menu to open storage"
    : "PC unavailable or already requested");
}
void TogglePreserveRide(CTRPluginFramework::MenuEntry* entry)
{
  if (!SetPreserveRideBetweenAreas(!PreserveRideBetweenAreas())) {
    Follower3gx_NotifyRide("Could not save preserve ride setting"); return;
  }
  entry->Name()=std::string("Preserve ride between areas: ")+(PreserveRideBetweenAreas() ? "On" : "Off");
}
void SelectRiderAnimation(CTRPluginFramework::MenuEntry* entry)
{
  std::vector<std::string> options;
  for (int i = 0; i < RIDER_STYLE_COUNT; ++i)
  {
    options.push_back(GetRiderAnimationStyleName(static_cast<RiderAnimationStyle>(i)));
  }
  CTRPluginFramework::Keyboard keyboard("Rider animation (applies next mount)", options);
  keyboard.ChangeSelectedEntry(GetRiderAnimationStyle());
  const int selection = keyboard.Open();
  if (selection < 0 || selection >= RIDER_STYLE_COUNT) { return; }
  if (!SetRiderAnimationStyle(static_cast<RiderAnimationStyle>(selection)))
  {
    Follower3gx_NotifyRide("Could not save rider animation setting");
    return;
  }
  entry->Name() = std::string("Default rider animation: ") + GetRiderAnimationStyleName(GetRiderAnimationStyle());
}

void SelectPerformancePreset(CTRPluginFramework::MenuEntry*)
{
  std::vector<std::string> options;
  options.reserve(PERFORMANCE_PRESET_COUNT);
  for (int i = 0; i < PERFORMANCE_PRESET_COUNT; ++i)
  {
    options.push_back(PERFORMANCE_PRESETS[i].name);
  }

  CTRPluginFramework::Keyboard keyboard("Performance level", options);
  const int current = FindPerformancePreset();
  keyboard.ChangeSelectedEntry(current >= 0 ? current : 0);
  const int selection = keyboard.Open();
  if (selection < 0 || selection >= PERFORMANCE_PRESET_COUNT)
  {
    return;
  }

  const PerformancePresetDescriptor& preset = PERFORMANCE_PRESETS[selection];
  ApplyPerformancePreset(preset);
#if FOLLOWER_3GX_DIAGNOSTIC
  RefreshPerformanceOptionEntries();
  RefreshCollisionQualityEntry();
  RefreshAnimationRateEntry();
#endif
  RefreshPerformancePresetEntry();
}

#if FOLLOWER_3GX_DIAGNOSTIC
int FindDiagnosticWeatherOption(int selection)
{
  for (int i = 0; i < DIAGNOSTIC_WEATHER_OPTION_COUNT; ++i)
  {
    if (DIAGNOSTIC_WEATHER_OPTIONS[i].selection == selection)
    {
      return i;
    }
  }
  return 0;
}

void RefreshDiagnosticWeatherEntry()
{
  if (!g_pWeatherSelectorEntry)
  {
    return;
  }

  g_pWeatherSelectorEntry->Name() = std::string("Field weather: ") +
    GetDiagnosticWeatherSelectionName(GetDiagnosticWeatherSelection());
}

void SelectDiagnosticWeather(CTRPluginFramework::MenuEntry*)
{
  std::vector<std::string> options;
  options.reserve(DIAGNOSTIC_WEATHER_OPTION_COUNT);
  for (int i = 0; i < DIAGNOSTIC_WEATHER_OPTION_COUNT; ++i)
  {
    options.push_back(DIAGNOSTIC_WEATHER_OPTIONS[i].name);
  }

  CTRPluginFramework::Keyboard keyboard("Field weather", options);
  keyboard.ChangeSelectedEntry(
    FindDiagnosticWeatherOption(GetDiagnosticWeatherSelection())
    );
  const int selection = keyboard.Open();
  if (selection < 0 || selection >= DIAGNOSTIC_WEATHER_OPTION_COUNT)
  {
    return;
  }

  QueueDiagnosticWeatherSelection(
    DIAGNOSTIC_WEATHER_OPTIONS[selection].selection
    );
  RefreshDiagnosticWeatherEntry();
}

void RefreshDiagnosticEffectEntry()
{
  if (!g_pEffectSelectorEntry)
  {
    return;
  }

  g_pEffectSelectorEntry->Name() = std::string("Effect lab: ") +
    GetDiagnosticEffectSelectionName(GetDiagnosticEffectSelection());
}

void SelectDiagnosticEffect(CTRPluginFramework::MenuEntry*)
{
  unsigned int catalogCount = 0;
  const DiagnosticEffectCatalogEntry* catalog =
    GetDiagnosticEffectCatalog(&catalogCount);
  std::vector<std::string> options;
  std::vector<int> selections;
  options.reserve(catalogCount);
  selections.reserve(catalogCount);
  int selectedEntry = 0;
  const int currentSelection = GetDiagnosticEffectSelection();
  for (unsigned int i = 0; i < catalogCount; ++i)
  {
    if (!IsDiagnosticEffectSelectionAvailable(catalog[i].selection))
    {
      continue;
    }
    if (catalog[i].selection == currentSelection)
    {
      selectedEntry = static_cast<int>(options.size());
    }
    options.push_back(catalog[i].name);
    selections.push_back(static_cast<int>(catalog[i].selection));
  }

  CTRPluginFramework::Keyboard keyboard("Follower effect lab", options);
  keyboard.ChangeSelectedEntry(selectedEntry);
  const int selection = keyboard.Open();
  if (selection < 0 ||
      selection >= static_cast<int>(selections.size()))
  {
    return;
  }

  SetDiagnosticEffectSelection(selections[selection]);
  RefreshDiagnosticEffectEntry();
}

int FindDiagnosticEffectHeapGuardOption(unsigned int minimumFreeBytes)
{
  for (int i = 0; i < DIAGNOSTIC_EFFECT_HEAP_GUARD_OPTION_COUNT; ++i)
  {
    if (DIAGNOSTIC_EFFECT_HEAP_GUARD_OPTIONS[i].minimumFreeBytes ==
        minimumFreeBytes)
    {
      return i;
    }
  }
  return 0;
}

void RefreshDiagnosticEffectHeapGuardEntry()
{
  if (!g_pEffectHeapGuardEntry)
  {
    return;
  }
  const int option = FindDiagnosticEffectHeapGuardOption(
    GetDiagnosticEffectHeapGuard()
    );
  g_pEffectHeapGuardEntry->Name() =
    std::string("Effect heap guard: ") +
    DIAGNOSTIC_EFFECT_HEAP_GUARD_OPTIONS[option].name;
}

void SelectDiagnosticEffectHeapGuard(CTRPluginFramework::MenuEntry*)
{
  std::vector<std::string> options;
  options.reserve(DIAGNOSTIC_EFFECT_HEAP_GUARD_OPTION_COUNT);
  for (int i = 0; i < DIAGNOSTIC_EFFECT_HEAP_GUARD_OPTION_COUNT; ++i)
  {
    options.push_back(DIAGNOSTIC_EFFECT_HEAP_GUARD_OPTIONS[i].name);
  }

  CTRPluginFramework::Keyboard keyboard(
    "Effect heap guard (lower is riskier)",
    options
    );
  keyboard.ChangeSelectedEntry(
    FindDiagnosticEffectHeapGuardOption(GetDiagnosticEffectHeapGuard())
    );
  const int selection = keyboard.Open();
  if (selection < 0 ||
      selection >= DIAGNOSTIC_EFFECT_HEAP_GUARD_OPTION_COUNT)
  {
    return;
  }

  SetDiagnosticEffectHeapGuard(
    DIAGNOSTIC_EFFECT_HEAP_GUARD_OPTIONS[selection].minimumFreeBytes
    );
  RefreshDiagnosticEffectHeapGuardEntry();
}

void ResetPerformanceOptionEntries(CTRPluginFramework::MenuEntry*)
{
  ResetPerformanceOptions();
  SetFollowerDrawDistanceMode(FOLLOWER_DRAW_DISTANCE_FULL);
  SetFollowerTerrainDetailMode(FOLLOWER_TERRAIN_DETAIL_STOCK);
  SetFollowerCollisionQualityMode(FOLLOWER_COLLISION_QUALITY_FULL);
  SetFollowerAnimationRateMode(FOLLOWER_ANIMATION_RATE_30_FPS);
  for (int i = 0; i < PERFORMANCE_OPTION_COUNT; ++i)
  {
    if (g_pPerformanceEntries[i] &&
        g_pPerformanceEntries[i]->IsActivated())
    {
      g_pPerformanceEntries[i]->Disable();
    }
  }
  RefreshDrawDistanceEntry();
  RefreshTerrainDetailEntry();
  RefreshCollisionQualityEntry();
  RefreshAnimationRateEntry();
  RefreshPerformancePresetEntry();
}

void RefreshDiagnosticsEntry()
{
  if (!g_pDiagnosticsEntry)
  {
    return;
  }
  g_pDiagnosticsEntry->Name() = IsPerformanceOverlayVisible()
    ? "Hide performance diagnostics"
    : "Show performance diagnostics";
}

void TogglePerformanceOverlay(CTRPluginFramework::MenuEntry*)
{
  SetPerformanceOverlayVisible(!IsPerformanceOverlayVisible());
  RefreshDiagnosticsEntry();
}

#endif
void RefreshFreeCameraEntry()
{
  if (g_pFreeCameraEntry) g_pFreeCameraEntry->Name()=std::string("Camera mode: ")+
    (IsFreeCameraEnabled() ? (IsFollowingGameCamera() ? "Follow game camera" : (IsFreeCameraParked() ? "Fixed view" : "Camera controls")) : "Off");
}
void SelectCameraMode(CTRPluginFramework::MenuEntry*)
{
  CTRPluginFramework::Keyboard keyboard("Camera mode", {
    "Game camera (default)", "Adjust camera (hold player)",
    "Fixed view (control player)", "Follow game camera (keep framing)"});
  const int current = !IsFreeCameraEnabled() ? 0 :
    (IsFollowingGameCamera() ? 3 : (IsFreeCameraParked() ? 2 : 1));
  keyboard.ChangeSelectedEntry(current);
  const int selected = keyboard.Open();
  if (selected < 0) return;
  if (selected == 0) SetFreeCameraEnabled(false);
  else if (selected == 3) FollowGameCamera();
  else {
    SetFreeCameraParked(selected == 2);
    SetFreeCameraEnabled(true);
  }
  RefreshFreeCameraEntry();
}

void ControlFreeCamera(CTRPluginFramework::MenuEntry*)
{
  SetFreeCameraParked(false); SetFreeCameraEnabled(true); RefreshFreeCameraEntry();
}
void ParkFreeCamera(CTRPluginFramework::MenuEntry*)
{
  SetFreeCameraParked(true); SetFreeCameraEnabled(true); RefreshFreeCameraEntry();
}
void FollowRetailCamera(CTRPluginFramework::MenuEntry*)
{
  FollowGameCamera(); RefreshFreeCameraEntry();
}
void ResetFreeCamera(CTRPluginFramework::MenuEntry*) { RequestFreeCameraReset(); }
void EditFreeCameraSpeed(CTRPluginFramework::MenuEntry*)
{
  CTRPluginFramework::Keyboard keyboard("Free camera speed",{"Fine", "Normal", "Fast", "Very fast"});
  keyboard.ChangeSelectedEntry(GetFreeCameraSpeed());
  const int choice=keyboard.Open();
  if (choice>=0) SetFreeCameraSpeed(choice);
}

bool OnMenuOpening()
{
  RefreshFreeCameraEntry();
  RefreshLayeredFsConflictEntry();
  RefreshFollowerToggleEntries();
  RefreshOutlineEntry();
  RefreshBehaviorEntries();
  RefreshAnimationCapEntry();
  RefreshPerformancePresetEntry();
#if FOLLOWER_3GX_DIAGNOSTIC
  RefreshPerformanceOptionEntries();
  RefreshDrawDistanceEntry();
  RefreshTerrainDetailEntry();
  RefreshCollisionQualityEntry();
  RefreshAnimationRateEntry();
  RefreshDiagnosticsEntry();
  RefreshDiagnosticWeatherEntry();
#endif
  return true;
}

} // namespace

void InitializePerformanceLevelDefaults(bool settingsLoaded)
{
  if (settingsLoaded)
  {
    return;
  }

  const PerformancePreset defaultPreset = CTRPluginFramework::System::IsNew3DS()
    ? PERFORMANCE_PRESET_RETAIL
    : PERFORMANCE_PRESET_LITE;
  ApplyPerformancePreset(PERFORMANCE_PRESETS[defaultPreset]);
}

int RunSettingsMenu()
{
  // CTRPF 0.8 frees its tools folder twice when destroying the menu. Keep the menu until exit.
  CTRPluginFramework::PluginMenu* const menu =
    new CTRPluginFramework::PluginMenu(
    "Gen7 Field Follower",
    1,
    0,
    0,
    ""
    );
  menu->ShowWelcomeMessage(false);
  menu->SetHexEditorState(false);
  menu->SynchronizeWithFrame(true);
  menu->OnOpening = OnMenuOpening;
  menu->Callback(PluginFrameCallback);

  g_pLayeredFsConflictEntry = new CTRPluginFramework::MenuEntry(
    "WARNING: LayeredFS conflict",
    "Legacy follower CRO files were detected. Select this entry to see the "
    "exact files that must be removed before using the 3GX plugin."
    );
  g_pLayeredFsConflictEntry->SetMenuFunc(ShowLayeredFsConflict);
  g_pLayeredFsConflictEntry->UseBottomSeparator(
    CTRPluginFramework::Separator::Stippled
    );
  menu->Append(g_pLayeredFsConflictEntry);
  auto* menuShortcut=new CTRPluginFramework::MenuEntry(MenuHotkeyLabel(),
    "Choose the buttons used to open the menu. Saves on confirmation. "
    "L + D-pad Down + Select avoids the soft-reset buttons. "
    "ZL/ZR require a New 3DS or mapped emulator controls.");
  menuShortcut->SetMenuFunc(EditMenuHotkey);
  menu->Append(menuShortcut);
  RefreshLayeredFsConflictEntry();

  auto* followersFolder = new CTRPluginFramework::MenuFolder("Followers");
  auto* ridingFolder = new CTRPluginFramework::MenuFolder("Riding and carrying");
  auto* eventsFolder = new CTRPluginFramework::MenuFolder("Dialogue and events");
  menu->Append(followersFolder);
  menu->Append(ridingFolder);
  menu->Append(eventsFolder);

  g_pFollowerEnabledEntry = new CTRPluginFramework::MenuEntry(
    "Enable followers",
    ApplyFollowerEnabled,
    "Turns the follower system on or off. Turning it off releases the loaded "
    "follower resources."
    );
  followersFolder->Append(g_pFollowerEnabledEntry);

  g_pDynamicFollowerEntry = new CTRPluginFramework::MenuEntry(
    "Auto-hide when slow",
    ApplyDynamicFollowerEnabled,
    "Temporarily hides the follower after sustained frame rate loss. It "
    "retries only after the field holds near 30 FPS, with longer cooldowns "
    "when a retry slows the game again."
    );
  followersFolder->Append(g_pDynamicFollowerEntry);

  g_pFormChangeEntry = new CTRPluginFramework::MenuEntry(
    "Form change (L+A)",
    ApplyFormChangeEnabled,
    "Allows L+A near an idle follower to cycle through its forms. When off, "
    "L+A performs the normal follower interaction."
    );
  followersFolder->Append(g_pFormChangeEntry);

  g_pSpecialEffectsEntry = new CTRPluginFramework::MenuEntry(
    "Special effects (R+A)",
    ApplySpecialEffectsEnabled,
    "Allows species-specific R+A actions and field effects. When off, R+A "
    "performs the normal follower interaction."
    );
  followersFolder->Append(g_pSpecialEffectsEntry);

  g_pPokeBallSoundVolumeEntry = new CTRPluginFramework::MenuEntry(
    "Poke Ball sound volume",
    "Sets the follower's send-out and recall sound from 0 to 100 percent. "
    "Zero mutes the sound while keeping the visual transition."
    );
  g_pPokeBallSoundVolumeEntry->SetMenuFunc(EditPokeBallSoundVolume);
  RefreshPokeBallSoundVolumeEntry();
  followersFolder->Append(g_pPokeBallSoundVolumeEntry);
  auto* riderAnimationEntry = new CTRPluginFramework::MenuEntry(
    std::string("Default rider animation: ") + GetRiderAnimationStyleName(GetRiderAnimationStyle()),
    "Default style for species without a saved ride profile. Dismount and remount to apply."
    );
  riderAnimationEntry->SetMenuFunc(SelectRiderAnimation);
  ridingFolder->Append(riderAnimationEntry);
  auto* openPc=new CTRPluginFramework::MenuEntry("Open Pokemon storage (PC)",
    "Open native Pokemon storage after closing this menu. Requires normal field control. "
    "Followers reload from your updated party after leaving the PC.");
  openPc->SetMenuFunc(OpenPartyPc);
  menu->Append(openPc);
  auto* partyFollowers=new CTRPluginFramework::MenuEntry("Follower count and party slots",
    "Select party slots without rearranging the team. Main follower supports riding/interactions; "
    "extras follow behind it. Automatic skips eggs and already selected slots. "
    "Changes save when confirmed. B goes back. "
    "More followers use more memory; restart after increasing count.");
  partyFollowers->SetMenuFunc(EditPartyFollowers);
  followersFolder->Append(partyFollowers);
  auto* followMode=new CTRPluginFramework::MenuEntry(
    std::string("Following mode: ")+(IsCloseFollowEnabled() ? "Close" : "Standard"),
    "Standard uses the original following behavior and your behavior settings. "
    "Close uses shorter preset gaps and delay, smoother catch-up and travel independent "
    "of animation stride. Speed settings apply to both. Saves on confirmation.");
  followMode->SetMenuFunc(SelectFollowMode);
  followersFolder->Append(followMode);
  auto* rideProfileEntry=new CTRPluginFramework::MenuEntry(
    "Pokemon size and ride setup",
    "L+R+A: mount/carry or dismount. Edit a species by Pokedex number or use the last mounted Pokemon. "
    "Mount once to list its bones. Size applies on confirm; remount for other changes. Profiles are shared by forms. "
    "Bone attachment follows position; offsets use mount-facing right/up/forward axes. "
    "Bone edits use parent-space axes. Changes save when confirmed. B goes back.");
  rideProfileEntry->SetMenuFunc(EditRideProfile);
  ridingFolder->Append(rideProfileEntry);
  auto* preserveRide=new CTRPluginFramework::MenuEntry(
    std::string("Preserve ride between areas: ")+(PreserveRideBetweenAreas() ? "On" : "Off"),
    "Restore ride/carry after changing areas when the same Pokemon is selected. "
    "Models reload normally; riding resumes when field control returns. Saves immediately.");
  preserveRide->SetMenuFunc(TogglePreserveRide);
  ridingFolder->Append(preserveRide);
  auto* mountedInteractions=new CTRPluginFramework::MenuEntry(
    std::string("Talk while riding: ")+MountedInteractionName(GetMountedInteractionMode()),
    "Choose whether A is blocked, dismounts then reaches the game, or reaches the game "
    "while retaining the ride during field events. L+R+A still dismounts. "
    "Keep riding is experimental and also affects scripts, not just dialogue. "
    "Hidden players, teleports and required resource cleanup still cancel riding. Saves immediately.");
  mountedInteractions->SetMenuFunc(SelectMountedInteraction);
  eventsFolder->Append(mountedInteractions);
  auto* eventVisibility=new CTRPluginFramework::MenuEntry(
    std::string("Follower during events: ")+(KeepFollowerVisibleDuringEvents() ? "Keep visible (test)" : "Hide"),
    "Keep an unmounted follower in place with its idle animation during field events. "
    "Requires a visible player and retained model resources. Keep riding already shows its mount. "
    "Required resource cleanup still takes priority. Saves immediately.");
  eventVisibility->SetMenuFunc(SelectEventFollowerVisibility);
  eventsFolder->Append(eventVisibility);


  RefreshFollowerToggleEntries();

  g_pOutlineEntry = new CTRPluginFramework::MenuEntry(
    "Follower appearance",
    "Native field uses shared field lighting without extra Pokemon tint, with original outline IDs. "
    "Field soft uses a separate follower outline ID. Both field styles use scene stencil outlines to respect occlusion. "
    "Normal edge adds scenery occlusion where supported; some objects may still show outlines through them. "
    "ID styles use the previous outline adjustments. "
    "Performance settings still apply; use Full for comparison. Saves immediately."
    );
  g_pOutlineEntry->SetMenuFunc(SelectOutlineMode);
  RefreshOutlineEntry();
  followersFolder->Append(g_pOutlineEntry);

  CTRPluginFramework::MenuFolder* const behaviorFolder =
    new CTRPluginFramework::MenuFolder("Follower behavior");
  for (int i = 0; i < FOLLOWER_BEHAVIOR_SETTING_COUNT; ++i)
  {
    g_pBehaviorEntries[i] = new CTRPluginFramework::MenuEntry(
      BEHAVIOR_OPTIONS[i].name
      );
    g_pBehaviorEntries[i]->SetArg(&BEHAVIOR_OPTIONS[i]);
    g_pBehaviorEntries[i]->SetMenuFunc(EditBehaviorSetting);
    behaviorFolder->Append(g_pBehaviorEntries[i]);
  }
  g_pAnimationCapEntry = new CTRPluginFramework::MenuEntry(
    "Animation playback cap",
    "Warning: removing this cap can make animations unnaturally fast and "
    "can worsen clipping or collision at extreme speed settings."
    );
  g_pAnimationCapEntry->SetMenuFunc(ToggleAnimationPlaybackCap);
  behaviorFolder->Append(g_pAnimationCapEntry);
  CTRPluginFramework::MenuEntry* const resetBehaviorEntry =
    new CTRPluginFramework::MenuEntry("Reset behavior defaults");
  resetBehaviorEntry->SetMenuFunc(ResetBehaviorDefaults);
  resetBehaviorEntry->UseTopSeparator(
    CTRPluginFramework::Separator::Stippled
    );
  behaviorFolder->Append(resetBehaviorEntry);
  RefreshBehaviorEntries();
  RefreshAnimationCapEntry();
  followersFolder->Append(behaviorFolder);

  CTRPluginFramework::MenuFolder* const performanceFolder =
    new CTRPluginFramework::MenuFolder("Performance options");
  g_pPerformancePresetEntry = new CTRPluginFramework::MenuEntry(
    "Performance level",
    "Applies cumulative renderer, collision, and follower-animation settings. "
    "Draw distance and terrain detail are not changed."
#if FOLLOWER_3GX_DIAGNOSTIC
    " Individual settings "
    "remain editable and show the level as Custom."
#endif
    );
  g_pPerformancePresetEntry->SetMenuFunc(SelectPerformancePreset);
  RefreshPerformancePresetEntry();
  performanceFolder->Append(g_pPerformancePresetEntry);
#if FOLLOWER_3GX_DIAGNOSTIC
  g_pDrawDistanceEntry = new CTRPluginFramework::MenuEntry(
    "Draw distance",
    "Reduces the field camera range used for terrain, object, and character "
    "culling. Lower settings can cause visible pop-in."
    );
  g_pDrawDistanceEntry->SetMenuFunc(SelectDrawDistance);
  RefreshDrawDistanceEntry();
  performanceFolder->Append(g_pDrawDistanceEntry);
  g_pTerrainDetailEntry = new CTRPluginFramework::MenuEntry(
    "Terrain detail",
    "Medium blocks retail high-detail terrain requests. Existing blocks "
    "change as the terrain controller streams them."
    );
  g_pTerrainDetailEntry->SetMenuFunc(SelectTerrainDetail);
  RefreshTerrainDetailEntry();
  performanceFolder->Append(g_pTerrainDetailEntry);
  g_pCollisionQualityEntry = new CTRPluginFramework::MenuEntry(
    "Follower collision",
    "Balanced caches ground triangles and trims redundant queries. Fast "
    "uses longer caching and center-only mesh wall tests, so it may clip "
    "complex slopes or collision-only props."
    );
  g_pCollisionQualityEntry->SetMenuFunc(SelectCollisionQuality);
  RefreshCollisionQualityEntry();
  performanceFolder->Append(g_pCollisionQualityEntry);
  g_pAnimationRateEntry = new CTRPluginFramework::MenuEntry(
    "Follower animation",
    "15 FPS updates the follower pose and model traversal every other frame "
    "while movement remains at 30 FPS. Interpolated modes evaluate animation "
    "at the selected rate but blend joint poses and traverse at 30 FPS."
    );
  g_pAnimationRateEntry->SetMenuFunc(SelectAnimationRate);
  RefreshAnimationRateEntry();
  performanceFolder->Append(g_pAnimationRateEntry);
  for (int i = 0; i < PERFORMANCE_OPTION_COUNT; ++i)
  {
    PerformanceOptionDescriptor& descriptor = PERFORMANCE_OPTIONS[i];
    g_pPerformanceEntries[i] = new CTRPluginFramework::MenuEntry(
      descriptor.name,
      ApplyPerformanceOption,
      descriptor.note
      );
    g_pPerformanceEntries[i]->SetArg(&descriptor);
    performanceFolder->Append(g_pPerformanceEntries[i]);
  }
  CTRPluginFramework::MenuEntry* const resetPerformanceEntry =
    new CTRPluginFramework::MenuEntry("Restore retail defaults");
  resetPerformanceEntry->SetMenuFunc(ResetPerformanceOptionEntries);
  resetPerformanceEntry->UseTopSeparator(
    CTRPluginFramework::Separator::Stippled
    );
  performanceFolder->Append(resetPerformanceEntry);
  RefreshPerformanceOptionEntries();
#endif
  menu->Append(performanceFolder);

#if FOLLOWER_3GX_DIAGNOSTIC
  g_pWeatherSelectorEntry = new CTRPluginFramework::MenuEntry(
    "Field weather: Map default",
    "Queues a session-only field weather override after the paused menu "
    "closes. Outdoor maps only. Map default releases the override and "
    "restores the map's retail weather."
    );
  g_pWeatherSelectorEntry->SetMenuFunc(SelectDiagnosticWeather);
  RefreshDiagnosticWeatherEntry();
  menu->Append(g_pWeatherSelectorEntry);

  g_pEffectSelectorEntry = new CTRPluginFramework::MenuEntry(
    "Effect lab: Species default",
    "Selects a session-only retail field effect. Hold R and press A near "
    "any idle follower to play it with the normal reaction. A lab selection "
    "temporarily replaces species-specific R+A behavior."
    );
  g_pEffectSelectorEntry->SetMenuFunc(SelectDiagnosticEffect);
  RefreshDiagnosticEffectEntry();
  menu->Append(g_pEffectSelectorEntry);

  g_pEffectHeapGuardEntry = new CTRPluginFramework::MenuEntry(
    "Effect heap guard: 128 KiB (default)",
    "Sets the minimum contiguous resource-heap space required before an "
    "effect-lab resource is loaded. The plugin prefers spare Pokemon-model "
    "heap space, with a fixed 64 KiB model reserve, then tries Event Device. "
    "This is session-only. Lower values are riskier; No guard is unsafe."
    );
  g_pEffectHeapGuardEntry->SetMenuFunc(SelectDiagnosticEffectHeapGuard);
  RefreshDiagnosticEffectHeapGuardEntry();
  menu->Append(g_pEffectHeapGuardEntry);

  g_pDiagnosticsEntry = new CTRPluginFramework::MenuEntry(
    "Show performance diagnostics"
    );
  g_pDiagnosticsEntry->SetMenuFunc(TogglePerformanceOverlay);
  RefreshDiagnosticsEntry();
  menu->Append(g_pDiagnosticsEntry);

#endif
  auto* cameraFolder=new CTRPluginFramework::MenuFolder("Free camera");
  g_pFreeCameraEntry=new CTRPluginFramework::MenuEntry("Camera mode",
    "Circle Pad: move. D-pad/C-stick: look. A + Circle Pad: alternate look. "
    "L/R (or ZL/ZR): down/up. X: boost. Y: reset. B: hold view and control player. "
    "Your menu shortcut opens the plugin menu. Player controls are held while active. "
    "Free overworld only; session-only setting. Camera passes through scenery.");
  g_pFreeCameraEntry->SetMenuFunc(SelectCameraMode);
  cameraFolder->Append(g_pFreeCameraEntry);
  auto* controlCamera=new CTRPluginFramework::MenuEntry("Adjust camera position");
  controlCamera->SetMenuFunc(ControlFreeCamera); cameraFolder->Append(controlCamera);
  auto* parkCamera=new CTRPluginFramework::MenuEntry("Keep view / control player",
    "Hold camera position and angle while moving and interacting normally. "
    "Use Adjust camera position to fly again, or turn free camera Off for the normal camera.");
  parkCamera->SetMenuFunc(ParkFreeCamera); cameraFolder->Append(parkCamera);
  auto* followCamera=new CTRPluginFramework::MenuEntry("Follow game camera",
    "Keep the current framing relative to the game's moving camera. Set your view with "
    "Adjust camera position, then select this. Normal player controls remain available. "
    "Offsets survive area changes for this session; events use the normal camera. Reset removes offsets.");
  followCamera->SetMenuFunc(FollowRetailCamera); cameraFolder->Append(followCamera);
  auto* cameraSpeed=new CTRPluginFramework::MenuEntry("Camera movement speed");
  cameraSpeed->SetMenuFunc(EditFreeCameraSpeed); cameraFolder->Append(cameraSpeed);
  auto* cameraReset=new CTRPluginFramework::MenuEntry("Reset camera position");
  cameraReset->SetMenuFunc(ResetFreeCamera); cameraFolder->Append(cameraReset);
  menu->Append(cameraFolder);
  RefreshFreeCameraEntry();

  ApplyMenuHotkey();
  const int result = menu->Run();
  g_pLayeredFsConflictEntry = NULL;
  g_pFollowerEnabledEntry = NULL;
  g_pDynamicFollowerEntry = NULL;
  g_pFormChangeEntry = NULL;
  g_pSpecialEffectsEntry = NULL;
  g_pPokeBallSoundVolumeEntry = NULL;
  g_pOutlineEntry = NULL;
  for (int i = 0; i < FOLLOWER_BEHAVIOR_SETTING_COUNT; ++i)
  {
    g_pBehaviorEntries[i] = NULL;
  }
  g_pAnimationCapEntry = NULL;
  g_pEditingBehavior = NULL;
  g_pPerformancePresetEntry = NULL;
  g_pFreeCameraEntry = NULL;
#if FOLLOWER_3GX_DIAGNOSTIC
  g_pDrawDistanceEntry = NULL;
  g_pTerrainDetailEntry = NULL;
  g_pCollisionQualityEntry = NULL;
  g_pAnimationRateEntry = NULL;
  for (int i = 0; i < PERFORMANCE_OPTION_COUNT; ++i)
  {
    g_pPerformanceEntries[i] = NULL;
  }
  g_pDiagnosticsEntry = NULL;
  g_pFreeCameraEntry = NULL;
  g_pWeatherSelectorEntry = NULL;
  g_pEffectSelectorEntry = NULL;
  g_pEffectHeapGuardEntry = NULL;
#endif
  return result;
}

} // namespace Gen7Follower3gx
