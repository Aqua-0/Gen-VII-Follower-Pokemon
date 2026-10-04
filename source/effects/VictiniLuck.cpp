#include "VictiniLuck.hpp"

#include <3ds.h>
#include <stddef.h>
#include <CTRPluginFramework/System/Hook.hpp>
#include <CTRPluginFramework/System/Process.hpp>
#include <CTRPluginFramework/Utils/Utils.hpp>

#include "Diagnostics.hpp"
#include "GameProfile.hpp"
#include "RetailFunctions.hpp"

namespace Gen7Follower3gx
{

#if FOLLOWER_3GX_INTERACTION_FEATURES
namespace
{

const u64 SYSTEM_TICKS_PER_SECOND = 268111856ULL;
const u64 LUCK_DURATION_TICKS = SYSTEM_TICKS_PER_SECOND * 10ULL;
const u32 LUCK_TARGET_ROLLS = 1179;
const u64 INIT_SPEC_RARE_RANDOM = 0x00000003ffffffffULL;
const u64 INIT_SPEC_RARE_TRUE = 0x00000002ffffffffULL;
const u32 SHINY_ROLL_MASK = 0x00000fffU;

struct __attribute__((aligned(8))) RetailInitialSpec
{
  u64 personalRandom;
  u64 rareRandom;
  u64 id;
  u16 species;
  u8 form;
  u8 formPadding;
  u16 level;
  u16 sex;
  u16 nature;
  u8 abilityIndex;
  u8 rareTryCount;
  u16 talentPower[6];
  u32 familiarity;
  u8 talentVCount;
  u8 trailingPadding[3];
};

static_assert(sizeof(RetailInitialSpec) == 56, "retail InitialSpec size");
static_assert(
  offsetof(RetailInitialSpec, rareTryCount) == 35,
  "retail InitialSpec rare roll offset"
  );

CTRPluginFramework::Hook g_CallWildHook;
CTRPluginFramework::Hook g_CallWildExHook;
CTRPluginFramework::Hook g_InitialSpecFixHook;
CTRPluginFramework::Hook g_IntruderHook;
bool g_HooksInitialized = false;
bool g_HooksEnabled = false;
u64 g_ActivationTick = 0;
u32 g_WildScopeDepth = 0;
bool g_WildScopeBoosted = false;
u32 g_IntruderScopeDepth = 0;
bool g_IntruderScopeBoosted = false;
u32 g_ActivationCount = 0;
u32 g_EncounterCount = 0;
u32 g_PokemonCount = 0;
u32 g_IntruderCount = 0;
u32 g_PreviousRolls = 0;
u32 g_BoostedRolls = 0;

bool IsLuckActiveAt(u64 now)
{
  return g_ActivationTick != 0 &&
    now - g_ActivationTick < LUCK_DURATION_TICKS;
}

bool RollVirtualShiny(u32 retailRolls)
{
  for (u32 roll = retailRolls; roll < LUCK_TARGET_ROLLS; ++roll)
  {
    if ((CTRPluginFramework::Utils::Random() & SHINY_ROLL_MASK) == 0)
    {
      return true;
    }
  }
  return false;
}

void RecordBoost(u32 previous, u32 boosted, bool intruder)
{
  __atomic_store_n(
    &g_PreviousRolls,
    static_cast<u32>(previous),
    __ATOMIC_RELAXED
    );
  __atomic_store_n(
    &g_BoostedRolls,
    static_cast<u32>(boosted),
    __ATOMIC_RELAXED
    );
  __atomic_add_fetch(&g_PokemonCount, 1U, __ATOMIC_RELAXED);
  if (intruder)
  {
    __atomic_add_fetch(&g_IntruderCount, 1U, __ATOMIC_RELAXED);
  }
}

void BeginWildScope()
{
  if (g_WildScopeDepth++ != 0)
  {
    return;
  }
  g_WildScopeBoosted = IsLuckActiveAt(svcGetSystemTick());
  if (g_WildScopeBoosted)
  {
    __atomic_add_fetch(&g_EncounterCount, 1U, __ATOMIC_RELAXED);
  }
}

void EndWildScope()
{
  if (g_WildScopeDepth == 0)
  {
    return;
  }
  if (--g_WildScopeDepth == 0)
  {
    g_WildScopeBoosted = false;
  }
}

void BeginIntruderScope()
{
  if (g_IntruderScopeDepth++ != 0)
  {
    return;
  }
  g_IntruderScopeBoosted = IsLuckActiveAt(svcGetSystemTick());
}

void EndIntruderScope()
{
  if (g_IntruderScopeDepth == 0)
  {
    return;
  }
  if (--g_IntruderScopeDepth == 0)
  {
    g_IntruderScopeBoosted = false;
  }
}

extern "C" void* Follower3gx_VictiniCallWildCallback(
  void* eventManager,
  void* gameManager,
  void* pokeSet,
  u32 callOption,
  const void* party
)
{
  CTRPluginFramework::HookContext& hookContext =
    CTRPluginFramework::HookContext::GetCurrent();
  BeginWildScope();
  void* result = hookContext.OriginalFunction<void*>(
    eventManager,
    gameManager,
    pokeSet,
    callOption,
    party
    );
  EndWildScope();
  return result;
}

extern "C" void* Follower3gx_VictiniCallWildExCallback(
  void* eventManager,
  void* gameManager,
  void* pokeSet,
  u32 callOption,
  u32 returnBgmId,
  const void* party
)
{
  CTRPluginFramework::HookContext& hookContext =
    CTRPluginFramework::HookContext::GetCurrent();
  BeginWildScope();
  void* result = hookContext.OriginalFunction<void*>(
    eventManager,
    gameManager,
    pokeSet,
    callOption,
    returnBgmId,
    party
    );
  EndWildScope();
  return result;
}

extern "C" void Follower3gx_VictiniFixInitialSpecCallback(
  RetailInitialSpec* fixedSpec,
  const RetailInitialSpec* spec
)
{
  CTRPluginFramework::HookContext& hookContext =
    CTRPluginFramework::HookContext::GetCurrent();
  const bool intruder = g_IntruderScopeBoosted;
  if ((!g_WildScopeBoosted && !intruder) || !fixedSpec || !spec ||
      spec->rareRandom != INIT_SPEC_RARE_RANDOM)
  {
    hookContext.OriginalFunction<void>(fixedSpec, spec);
    return;
  }

  RetailInitialSpec boostedSpec = *spec;
  const u32 retailRolls = static_cast<u32>(spec->rareTryCount);
  const u32 effectiveRolls = retailRolls < LUCK_TARGET_ROLLS ?
    LUCK_TARGET_ROLLS : retailRolls;
  if (RollVirtualShiny(retailRolls))
  {
    boostedSpec.rareRandom = INIT_SPEC_RARE_TRUE;
  }
  RecordBoost(retailRolls, effectiveRolls, intruder);
  hookContext.OriginalFunction<void>(fixedSpec, &boostedSpec);
}

extern "C" bool Follower3gx_VictiniIntruderCallback(
  void* pokeSet,
  void* pokemon,
  void* random,
  u32 intruderType,
  u8 rareTryCount
)
{
  CTRPluginFramework::HookContext& hookContext =
    CTRPluginFramework::HookContext::GetCurrent();
  BeginIntruderScope();
  const bool result = hookContext.OriginalFunction<bool>(
    pokeSet,
    pokemon,
    random,
    intruderType,
    rareTryCount
    );
  EndIntruderScope();
  return result;
}

bool ValidateInitialSpecFixHook(const GameProfile& profile)
{
  const StaticHookProfile& hook = profile.initialSpecFixHook;
  return CTRPluginFramework::Process::CheckAddress(
      hook.targetAddress,
      MEMPERM_READ | MEMPERM_EXECUTE
      ) &&
    CTRPluginFramework::Process::CheckAddress(
      hook.targetAddress + sizeof(u32),
      MEMPERM_READ | MEMPERM_EXECUTE
      ) &&
    *reinterpret_cast<const volatile u32*>(hook.targetAddress) ==
      hook.expectedTargetInstructions[0] &&
    *reinterpret_cast<const volatile u32*>(hook.targetAddress + sizeof(u32)) ==
      hook.expectedTargetInstructions[1];
}

void DisableHooks()
{
  g_IntruderHook.Disable();
  g_InitialSpecFixHook.Disable();
  g_CallWildExHook.Disable();
  g_CallWildHook.Disable();
  g_HooksEnabled = false;
  g_WildScopeDepth = 0;
  g_WildScopeBoosted = false;
  g_IntruderScopeDepth = 0;
  g_IntruderScopeBoosted = false;
}

bool HookTargetsMatchProfile(const GameProfile& profile)
{
  return g_CallWildHook.GetContext().targetAddress ==
      g_RetailFunctionPointers[RetailFunction_EventBattleCallCallWild] &&
    g_CallWildExHook.GetContext().targetAddress ==
      g_RetailFunctionPointers[RetailFunction_EventBattleCallCallWildEx] &&
    g_InitialSpecFixHook.GetContext().targetAddress ==
      profile.initialSpecFixHook.targetAddress &&
    g_IntruderHook.GetContext().targetAddress ==
      g_RetailFunctionPointers[RetailFunction_PokeSetSetupLotteryIntruder];
}

} // namespace

void InitializeVictiniLuck()
{
  g_ActivationTick = 0;
  g_WildScopeDepth = 0;
  g_WildScopeBoosted = false;
  g_IntruderScopeDepth = 0;
  g_IntruderScopeBoosted = false;
  __atomic_store_n(&g_ActivationCount, 0U, __ATOMIC_RELAXED);
  __atomic_store_n(&g_EncounterCount, 0U, __ATOMIC_RELAXED);
  __atomic_store_n(&g_PokemonCount, 0U, __ATOMIC_RELAXED);
  __atomic_store_n(&g_IntruderCount, 0U, __ATOMIC_RELAXED);
  __atomic_store_n(&g_PreviousRolls, 0U, __ATOMIC_RELAXED);
  __atomic_store_n(&g_BoostedRolls, 0U, __ATOMIC_RELAXED);
}

void ShutdownVictiniLuck()
{
  DisableHooks();
  g_ActivationTick = 0;
}

bool InstallVictiniLuckHooks(const GameProfile& profile)
{
  using CTRPluginFramework::HookResult;
  if (!ValidateInitialSpecFixHook(profile))
  {
    return false;
  }
  if (!g_HooksInitialized)
  {
    g_CallWildHook.InitializeForMitm(
      g_RetailFunctionPointers[RetailFunction_EventBattleCallCallWild],
      reinterpret_cast<u32>(Follower3gx_VictiniCallWildCallback)
      );
    g_CallWildExHook.InitializeForMitm(
      g_RetailFunctionPointers[RetailFunction_EventBattleCallCallWildEx],
      reinterpret_cast<u32>(Follower3gx_VictiniCallWildExCallback)
      );
    g_InitialSpecFixHook.InitializeForMitm(
      profile.initialSpecFixHook.targetAddress,
      reinterpret_cast<u32>(Follower3gx_VictiniFixInitialSpecCallback)
      );
    g_IntruderHook.InitializeForMitm(
      g_RetailFunctionPointers[RetailFunction_PokeSetSetupLotteryIntruder],
      reinterpret_cast<u32>(Follower3gx_VictiniIntruderCallback)
      );
    g_HooksInitialized = true;
  }
  else if (!HookTargetsMatchProfile(profile))
  {
    return false;
  }

  if (g_CallWildHook.Enable() != HookResult::Success ||
      g_CallWildExHook.Enable() != HookResult::Success ||
      g_InitialSpecFixHook.Enable() != HookResult::Success ||
      g_IntruderHook.Enable() != HookResult::Success)
  {
    DisableHooks();
    return false;
  }
  g_HooksEnabled = true;
  return true;
}

void SuspendVictiniLuckHooksForRetailBind()
{
  DisableHooks();
}

void ActivateVictiniLuck()
{
  g_ActivationTick = svcGetSystemTick();
  __atomic_add_fetch(&g_ActivationCount, 1U, __ATOMIC_RELAXED);
}

VictiniLuckDiagnosticSnapshot GetVictiniLuckDiagnostics()
{
  VictiniLuckDiagnosticSnapshot snapshot = {};
  const u64 now = svcGetSystemTick();
  const bool active = IsLuckActiveAt(now);
  snapshot.active = active ? 1U : 0U;
  snapshot.hooksEnabled = g_HooksEnabled ? 1U : 0U;
  snapshot.targetRolls = LUCK_TARGET_ROLLS;
  if (active)
  {
    const u64 remaining = LUCK_DURATION_TICKS - (now - g_ActivationTick);
    snapshot.remainingTenths = static_cast<u32>(
      (remaining * 10ULL + SYSTEM_TICKS_PER_SECOND - 1ULL) /
        SYSTEM_TICKS_PER_SECOND
      );
  }
  snapshot.activationCount =
    __atomic_load_n(&g_ActivationCount, __ATOMIC_RELAXED);
  snapshot.encounterCount =
    __atomic_load_n(&g_EncounterCount, __ATOMIC_RELAXED);
  snapshot.pokemonCount =
    __atomic_load_n(&g_PokemonCount, __ATOMIC_RELAXED);
  snapshot.intruderCount =
    __atomic_load_n(&g_IntruderCount, __ATOMIC_RELAXED);
  snapshot.previousRolls =
    __atomic_load_n(&g_PreviousRolls, __ATOMIC_RELAXED);
  snapshot.boostedRolls =
    __atomic_load_n(&g_BoostedRolls, __ATOMIC_RELAXED);
  return snapshot;
}

#else

void InitializeVictiniLuck() {}
void ShutdownVictiniLuck() {}
bool InstallVictiniLuckHooks(const GameProfile&) { return true; }
void SuspendVictiniLuckHooksForRetailBind() {}
void ActivateVictiniLuck() {}
VictiniLuckDiagnosticSnapshot GetVictiniLuckDiagnostics()
{
  VictiniLuckDiagnosticSnapshot snapshot = {};
  return snapshot;
}

#endif

} // namespace Gen7Follower3gx
