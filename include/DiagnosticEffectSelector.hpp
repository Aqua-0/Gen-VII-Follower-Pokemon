#pragma once

namespace Gen7Follower3gx
{

enum DiagnosticEffectSelection
{
  DIAGNOSTIC_EFFECT_SPECIES_DEFAULT = -1,
  DIAGNOSTIC_EFFECT_FESTIVAL_LEVEL_UP = 30,
  DIAGNOSTIC_EFFECT_FESTIVAL_SHOP_OPEN = 31,
  DIAGNOSTIC_EFFECT_FESTIVAL_START_SPLASH = 32,
  DIAGNOSTIC_EFFECT_FESTIVAL_WARP = 33,
  DIAGNOSTIC_EFFECT_FISHING_BUOY = 35,
  DIAGNOSTIC_EFFECT_RIDE_APPEAR_LAND = 36,
  DIAGNOSTIC_EFFECT_RIDE_APPEAR_SEA = 37,
  DIAGNOSTIC_EFFECT_BALL_FLASH = 41,
  DIAGNOSTIC_EFFECT_FESTIVAL_FIRE = 50,
  DIAGNOSTIC_EFFECT_ROCK_SMOKE = 52,
  DIAGNOSTIC_EFFECT_ROCK_IMPACT = 53,
  DIAGNOSTIC_EFFECT_FLY_FLASH = 64,
  DIAGNOSTIC_EFFECT_WATER_SPLASH = 65,
  DIAGNOSTIC_EFFECT_TRIAL_SMOKE = 67,
  DIAGNOSTIC_EFFECT_TRIAL_SMOKE_2 = 68,
  DIAGNOSTIC_EFFECT_BAG_BURST = 69,
  DIAGNOSTIC_EFFECT_TRIAL_3 = 70,
  DIAGNOSTIC_EFFECT_TRIAL_5 = 71,
  DIAGNOSTIC_EFFECT_CONCENTRATE = 75,
  DIAGNOSTIC_EFFECT_FOG = 76,
  DIAGNOSTIC_EFFECT_PETALS_YELLOW = 77,
  DIAGNOSTIC_EFFECT_PETALS_PINK = 78,
  DIAGNOSTIC_EFFECT_FIREWORK_YELLOW = 79,
  DIAGNOSTIC_EFFECT_FIREWORK_PINK = 80,
  DIAGNOSTIC_EFFECT_FIREWORK_RED = 81,
  DIAGNOSTIC_EFFECT_FIREWORK_PURPLE = 82,
  DIAGNOSTIC_EFFECT_FLARE_SUN = 83,
  DIAGNOSTIC_EFFECT_FLARE_MOON = 84,
  DIAGNOSTIC_EFFECT_ULTRA_DEN_1 = 97,
  DIAGNOSTIC_EFFECT_ULTRA_TRIAL_5_1 = 98,
  DIAGNOSTIC_EFFECT_ULTRA_TRIAL_5_2 = 99,
  DIAGNOSTIC_EFFECT_ULTRA_ROCK_SMOKE = 100,
  DIAGNOSTIC_EFFECT_ULTRA_ROCK_IMPACT = 101,
  DIAGNOSTIC_EFFECT_ULTRA_DEN_2 = 102,
  DIAGNOSTIC_EFFECT_ULTRA_UB_SLASH = 103,
  DIAGNOSTIC_EFFECT_ULTRA_UB_BLACKOUT = 104,
  DIAGNOSTIC_EFFECT_ULTRA_ROTOM_POWER = 105,
  DIAGNOSTIC_EFFECT_ULTRA_TRIAL_2 = 107,
  DIAGNOSTIC_EFFECT_ULTRA_TEAM_ROCKET = 116,
  DIAGNOSTIC_EFFECT_ULTRA_BATTLE_FESTIVAL_WARP = 117,
};

enum DiagnosticEffectPlacement
{
  DIAGNOSTIC_EFFECT_PLACEMENT_GROUND,
  DIAGNOSTIC_EFFECT_PLACEMENT_MID_BODY,
};

enum DiagnosticEffectModuleRequirement
{
  DIAGNOSTIC_EFFECT_MODULE_NONE,
  DIAGNOSTIC_EFFECT_MODULE_FIELD_EFFECT_UNIQUE,
  DIAGNOSTIC_EFFECT_MODULE_FIELD_EFFECT_JOIN_FESTA,
};

struct DiagnosticEffectDefinition
{
  unsigned int effectType;
  unsigned int lifetimeFrames;
  unsigned int placement;
  unsigned int moduleRequirement;
  bool ultraOnly;
};

struct DiagnosticEffectCatalogEntry
{
  DiagnosticEffectSelection selection;
  const char* name;
  DiagnosticEffectDefinition definition;
};

enum
{
  DIAGNOSTIC_EFFECT_DEFAULT_HEAP_GUARD = 0x20000,
};

void SetDiagnosticEffectSelection(int selection);
int GetDiagnosticEffectSelection();
const char* GetDiagnosticEffectSelectionName(int selection);
const DiagnosticEffectCatalogEntry* GetDiagnosticEffectCatalog(
  unsigned int* count
  );
bool IsDiagnosticEffectSelectionAvailable(int selection);
bool GetDiagnosticEffectDefinition(
  int selection,
  DiagnosticEffectDefinition* definition
  );
DiagnosticEffectModuleRequirement GetDiagnosticEffectModuleRequirement(
  unsigned int effectType
  );
void ResetDiagnosticEffectSelection();
void SetDiagnosticEffectHeapGuard(unsigned int minimumFreeBytes);
unsigned int GetDiagnosticEffectHeapGuard();
void ResetDiagnosticEffectHeapGuard();

} // namespace Gen7Follower3gx
