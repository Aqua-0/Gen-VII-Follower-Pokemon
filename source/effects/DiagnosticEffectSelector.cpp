#include "DiagnosticEffectSelector.hpp"

#if FOLLOWER_3GX_DIAGNOSTIC

#include "PluginRuntime.hpp"

namespace Gen7Follower3gx
{
namespace
{

const unsigned int DEFAULT_EFFECT_LIFETIME_FRAMES = 120;

// Only list effects we can load on demand. Already-resident effects need different handling.
#define EFFECT_ENTRY(selection, name, placement) \
  { selection, name, { \
    static_cast<unsigned int>(selection), \
    DEFAULT_EFFECT_LIFETIME_FRAMES, placement, \
    DIAGNOSTIC_EFFECT_MODULE_NONE, false } }
#define ULTRA_EFFECT_ENTRY(selection, name, placement) \
  { selection, name, { \
    static_cast<unsigned int>(selection), \
    DEFAULT_EFFECT_LIFETIME_FRAMES, placement, \
    DIAGNOSTIC_EFFECT_MODULE_NONE, true } }
#define UNIQUE_EFFECT_ENTRY(selection, name, placement) \
  { selection, name, { \
    static_cast<unsigned int>(selection), \
    DEFAULT_EFFECT_LIFETIME_FRAMES, placement, \
    DIAGNOSTIC_EFFECT_MODULE_FIELD_EFFECT_UNIQUE, false } }
#define ULTRA_UNIQUE_EFFECT_ENTRY(selection, name, placement) \
  { selection, name, { \
    static_cast<unsigned int>(selection), \
    DEFAULT_EFFECT_LIFETIME_FRAMES, placement, \
    DIAGNOSTIC_EFFECT_MODULE_FIELD_EFFECT_UNIQUE, true } }
#define JOIN_FESTA_EFFECT_ENTRY(selection, name, placement) \
  { selection, name, { \
    static_cast<unsigned int>(selection), \
    DEFAULT_EFFECT_LIFETIME_FRAMES, placement, \
    DIAGNOSTIC_EFFECT_MODULE_FIELD_EFFECT_JOIN_FESTA, false } }

const DiagnosticEffectCatalogEntry DIAGNOSTIC_EFFECT_CATALOG[] =
{
  {
    DIAGNOSTIC_EFFECT_SPECIES_DEFAULT,
    "Species default",
    {
      0,
      0,
      DIAGNOSTIC_EFFECT_PLACEMENT_MID_BODY,
      DIAGNOSTIC_EFFECT_MODULE_NONE,
      false,
    },
  },
  JOIN_FESTA_EFFECT_ENTRY(
    DIAGNOSTIC_EFFECT_FESTIVAL_LEVEL_UP,
    "Festival level-up",
    DIAGNOSTIC_EFFECT_PLACEMENT_MID_BODY
    ),
  JOIN_FESTA_EFFECT_ENTRY(
    DIAGNOSTIC_EFFECT_FESTIVAL_SHOP_OPEN,
    "Festival shop opening",
    DIAGNOSTIC_EFFECT_PLACEMENT_MID_BODY
    ),
  JOIN_FESTA_EFFECT_ENTRY(
    DIAGNOSTIC_EFFECT_FESTIVAL_START_SPLASH,
    "Festival start splash",
    DIAGNOSTIC_EFFECT_PLACEMENT_GROUND
    ),
  JOIN_FESTA_EFFECT_ENTRY(
    DIAGNOSTIC_EFFECT_FESTIVAL_WARP,
    "Festival warp",
    DIAGNOSTIC_EFFECT_PLACEMENT_GROUND
    ),
  EFFECT_ENTRY(
    DIAGNOSTIC_EFFECT_FISHING_BUOY,
    "Fishing buoy",
    DIAGNOSTIC_EFFECT_PLACEMENT_GROUND
    ),
  EFFECT_ENTRY(
    DIAGNOSTIC_EFFECT_RIDE_APPEAR_LAND,
    "Ride appearance (land)",
    DIAGNOSTIC_EFFECT_PLACEMENT_GROUND
    ),
  EFFECT_ENTRY(
    DIAGNOSTIC_EFFECT_RIDE_APPEAR_SEA,
    "Ride appearance (sea)",
    DIAGNOSTIC_EFFECT_PLACEMENT_GROUND
    ),
  {
    DIAGNOSTIC_EFFECT_BALL_FLASH,
    "Ball flash",
    {
      DIAGNOSTIC_EFFECT_BALL_FLASH,
      36,
      DIAGNOSTIC_EFFECT_PLACEMENT_MID_BODY,
      DIAGNOSTIC_EFFECT_MODULE_NONE,
      false,
    },
  },
  EFFECT_ENTRY(
    DIAGNOSTIC_EFFECT_FESTIVAL_FIRE,
    "Festival fire",
    DIAGNOSTIC_EFFECT_PLACEMENT_GROUND
    ),
  EFFECT_ENTRY(
    DIAGNOSTIC_EFFECT_ROCK_SMOKE,
    "Rock smoke",
    DIAGNOSTIC_EFFECT_PLACEMENT_GROUND
    ),
  EFFECT_ENTRY(
    DIAGNOSTIC_EFFECT_ROCK_IMPACT,
    "Rock impact",
    DIAGNOSTIC_EFFECT_PLACEMENT_GROUND
    ),
  EFFECT_ENTRY(
    DIAGNOSTIC_EFFECT_FLY_FLASH,
    "Flying flash",
    DIAGNOSTIC_EFFECT_PLACEMENT_MID_BODY
    ),
  EFFECT_ENTRY(
    DIAGNOSTIC_EFFECT_WATER_SPLASH,
    "Water splash",
    DIAGNOSTIC_EFFECT_PLACEMENT_GROUND
    ),
  EFFECT_ENTRY(
    DIAGNOSTIC_EFFECT_TRIAL_SMOKE,
    "Trial smoke 1",
    DIAGNOSTIC_EFFECT_PLACEMENT_GROUND
    ),
  EFFECT_ENTRY(
    DIAGNOSTIC_EFFECT_TRIAL_SMOKE_2,
    "Trial smoke 2",
    DIAGNOSTIC_EFFECT_PLACEMENT_GROUND
    ),
  EFFECT_ENTRY(
    DIAGNOSTIC_EFFECT_BAG_BURST,
    "Bag burst",
    DIAGNOSTIC_EFFECT_PLACEMENT_MID_BODY
    ),
  UNIQUE_EFFECT_ENTRY(
    DIAGNOSTIC_EFFECT_TRIAL_3,
    "Trial 3 effect",
    DIAGNOSTIC_EFFECT_PLACEMENT_GROUND
    ),
  UNIQUE_EFFECT_ENTRY(
    DIAGNOSTIC_EFFECT_TRIAL_5,
    "Trial 5 effect",
    DIAGNOSTIC_EFFECT_PLACEMENT_GROUND
    ),
  EFFECT_ENTRY(
    DIAGNOSTIC_EFFECT_CONCENTRATE,
    "Concentration",
    DIAGNOSTIC_EFFECT_PLACEMENT_MID_BODY
    ),
  EFFECT_ENTRY(
    DIAGNOSTIC_EFFECT_FOG,
    "Fog",
    DIAGNOSTIC_EFFECT_PLACEMENT_GROUND
    ),
  EFFECT_ENTRY(
    DIAGNOSTIC_EFFECT_PETALS_YELLOW,
    "Yellow petals",
    DIAGNOSTIC_EFFECT_PLACEMENT_MID_BODY
    ),
  EFFECT_ENTRY(
    DIAGNOSTIC_EFFECT_PETALS_PINK,
    "Pink petals",
    DIAGNOSTIC_EFFECT_PLACEMENT_MID_BODY
    ),
  EFFECT_ENTRY(
    DIAGNOSTIC_EFFECT_FIREWORK_YELLOW,
    "Yellow firework",
    DIAGNOSTIC_EFFECT_PLACEMENT_MID_BODY
    ),
  EFFECT_ENTRY(
    DIAGNOSTIC_EFFECT_FIREWORK_PINK,
    "Pink firework",
    DIAGNOSTIC_EFFECT_PLACEMENT_MID_BODY
    ),
  EFFECT_ENTRY(
    DIAGNOSTIC_EFFECT_FIREWORK_RED,
    "Red firework",
    DIAGNOSTIC_EFFECT_PLACEMENT_MID_BODY
    ),
  EFFECT_ENTRY(
    DIAGNOSTIC_EFFECT_FIREWORK_PURPLE,
    "Purple firework",
    DIAGNOSTIC_EFFECT_PLACEMENT_MID_BODY
    ),
  EFFECT_ENTRY(
    DIAGNOSTIC_EFFECT_FLARE_SUN,
    "Sun flare",
    DIAGNOSTIC_EFFECT_PLACEMENT_MID_BODY
    ),
  EFFECT_ENTRY(
    DIAGNOSTIC_EFFECT_FLARE_MOON,
    "Moon flare",
    DIAGNOSTIC_EFFECT_PLACEMENT_MID_BODY
    ),
  ULTRA_UNIQUE_EFFECT_ENTRY(
    DIAGNOSTIC_EFFECT_ULTRA_DEN_1,
    "Ultra: den energy 1",
    DIAGNOSTIC_EFFECT_PLACEMENT_GROUND
    ),
  ULTRA_UNIQUE_EFFECT_ENTRY(
    DIAGNOSTIC_EFFECT_ULTRA_TRIAL_5_1,
    "Ultra: Trial 5 particle Z",
    DIAGNOSTIC_EFFECT_PLACEMENT_GROUND
    ),
  ULTRA_UNIQUE_EFFECT_ENTRY(
    DIAGNOSTIC_EFFECT_ULTRA_TRIAL_5_2,
    "Ultra: Trial 5 model Z",
    DIAGNOSTIC_EFFECT_PLACEMENT_GROUND
    ),
  ULTRA_UNIQUE_EFFECT_ENTRY(
    DIAGNOSTIC_EFFECT_ULTRA_ROCK_SMOKE,
    "Ultra: large rock smoke",
    DIAGNOSTIC_EFFECT_PLACEMENT_GROUND
    ),
  ULTRA_UNIQUE_EFFECT_ENTRY(
    DIAGNOSTIC_EFFECT_ULTRA_ROCK_IMPACT,
    "Ultra: large rock impact",
    DIAGNOSTIC_EFFECT_PLACEMENT_GROUND
    ),
  ULTRA_UNIQUE_EFFECT_ENTRY(
    DIAGNOSTIC_EFFECT_ULTRA_DEN_2,
    "Ultra: den energy 2",
    DIAGNOSTIC_EFFECT_PLACEMENT_GROUND
    ),
  ULTRA_UNIQUE_EFFECT_ENTRY(
    DIAGNOSTIC_EFFECT_ULTRA_UB_SLASH,
    "Ultra: UB slash",
    DIAGNOSTIC_EFFECT_PLACEMENT_MID_BODY
    ),
  ULTRA_UNIQUE_EFFECT_ENTRY(
    DIAGNOSTIC_EFFECT_ULTRA_UB_BLACKOUT,
    "Ultra: UB blackout",
    DIAGNOSTIC_EFFECT_PLACEMENT_MID_BODY
    ),
  ULTRA_EFFECT_ENTRY(
    DIAGNOSTIC_EFFECT_ULTRA_ROTOM_POWER,
    "Ultra: Rotom power",
    DIAGNOSTIC_EFFECT_PLACEMENT_MID_BODY
    ),
  ULTRA_EFFECT_ENTRY(
    DIAGNOSTIC_EFFECT_ULTRA_TRIAL_2,
    "Ultra: Trial 2 variant",
    DIAGNOSTIC_EFFECT_PLACEMENT_GROUND
    ),
  ULTRA_EFFECT_ENTRY(
    DIAGNOSTIC_EFFECT_ULTRA_TEAM_ROCKET,
    "Ultra: Team Rocket",
    DIAGNOSTIC_EFFECT_PLACEMENT_MID_BODY
    ),
  ULTRA_EFFECT_ENTRY(
    DIAGNOSTIC_EFFECT_ULTRA_BATTLE_FESTIVAL_WARP,
    "Ultra: Battle Festival warp",
    DIAGNOSTIC_EFFECT_PLACEMENT_GROUND
    ),
};

#undef EFFECT_ENTRY
#undef ULTRA_EFFECT_ENTRY
#undef UNIQUE_EFFECT_ENTRY
#undef ULTRA_UNIQUE_EFFECT_ENTRY
#undef JOIN_FESTA_EFFECT_ENTRY

const unsigned int DIAGNOSTIC_EFFECT_CATALOG_COUNT =
  sizeof(DIAGNOSTIC_EFFECT_CATALOG) /
  sizeof(DIAGNOSTIC_EFFECT_CATALOG[0]);

volatile int g_DiagnosticEffectSelection =
  DIAGNOSTIC_EFFECT_SPECIES_DEFAULT;
volatile unsigned int g_DiagnosticEffectHeapGuard =
  DIAGNOSTIC_EFFECT_DEFAULT_HEAP_GUARD;

const DiagnosticEffectCatalogEntry* FindDiagnosticEffect(int selection)
{
  for (unsigned int i = 0; i < DIAGNOSTIC_EFFECT_CATALOG_COUNT; ++i)
  {
    if (DIAGNOSTIC_EFFECT_CATALOG[i].selection == selection)
    {
      return &DIAGNOSTIC_EFFECT_CATALOG[i];
    }
  }
  return 0;
}

} // namespace

void SetDiagnosticEffectSelection(int selection)
{
  if (!IsDiagnosticEffectSelectionAvailable(selection))
  {
    return;
  }
  __atomic_store_n(
    &g_DiagnosticEffectSelection,
    selection,
    __ATOMIC_RELEASE
    );
}

int GetDiagnosticEffectSelection()
{
  return __atomic_load_n(
    &g_DiagnosticEffectSelection,
    __ATOMIC_ACQUIRE
    );
}

const char* GetDiagnosticEffectSelectionName(int selection)
{
  const DiagnosticEffectCatalogEntry* entry =
    FindDiagnosticEffect(selection);
  return entry ? entry->name : "Species default";
}

const DiagnosticEffectCatalogEntry* GetDiagnosticEffectCatalog(
  unsigned int* count
)
{
  if (count)
  {
    *count = DIAGNOSTIC_EFFECT_CATALOG_COUNT;
  }
  return DIAGNOSTIC_EFFECT_CATALOG;
}

bool IsDiagnosticEffectSelectionAvailable(int selection)
{
  const DiagnosticEffectCatalogEntry* entry =
    FindDiagnosticEffect(selection);
  return entry && (!entry->definition.ultraOnly || !IsRegularGameRuntime());
}

bool GetDiagnosticEffectDefinition(
  int selection,
  DiagnosticEffectDefinition* definition
)
{
  const DiagnosticEffectCatalogEntry* entry =
    FindDiagnosticEffect(selection);
  if (!definition || !entry ||
      selection == DIAGNOSTIC_EFFECT_SPECIES_DEFAULT ||
      !IsDiagnosticEffectSelectionAvailable(selection))
  {
    return false;
  }

  *definition = entry->definition;
  return true;
}

DiagnosticEffectModuleRequirement GetDiagnosticEffectModuleRequirement(
  unsigned int effectType
)
{
  for (unsigned int i = 0; i < DIAGNOSTIC_EFFECT_CATALOG_COUNT; ++i)
  {
    if (DIAGNOSTIC_EFFECT_CATALOG[i].definition.effectType == effectType)
    {
      return static_cast<DiagnosticEffectModuleRequirement>(
        DIAGNOSTIC_EFFECT_CATALOG[i].definition.moduleRequirement
        );
    }
  }
  return DIAGNOSTIC_EFFECT_MODULE_NONE;
}

void ResetDiagnosticEffectSelection()
{
  __atomic_store_n(
    &g_DiagnosticEffectSelection,
    static_cast<int>(DIAGNOSTIC_EFFECT_SPECIES_DEFAULT),
    __ATOMIC_RELEASE
    );
}

void SetDiagnosticEffectHeapGuard(unsigned int minimumFreeBytes)
{
  __atomic_store_n(
    &g_DiagnosticEffectHeapGuard,
    minimumFreeBytes,
    __ATOMIC_RELEASE
    );
}

unsigned int GetDiagnosticEffectHeapGuard()
{
  return __atomic_load_n(
    &g_DiagnosticEffectHeapGuard,
    __ATOMIC_ACQUIRE
    );
}

void ResetDiagnosticEffectHeapGuard()
{
  SetDiagnosticEffectHeapGuard(DIAGNOSTIC_EFFECT_DEFAULT_HEAP_GUARD);
}

} // namespace Gen7Follower3gx

#endif
