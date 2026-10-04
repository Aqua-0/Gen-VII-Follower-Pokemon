#include "PluginRuntime.hpp"
#include "FollowerArena.hpp"
#include <CTRPluginFramework/Graphics/OSD.hpp>

#include <CTRPluginFramework/System/Hook.hpp>
#include <CTRPluginFramework/System/Process.hpp>

#include "CroModule.hpp"
#include "DiagnosticEffectModule.hpp"
#include "Diagnostics.hpp"
#include "FieldHooks.hpp"
#include "FollowerController.hpp"
#include "GameProfile.hpp"
#include "LayeredFsConflict.hpp"
#include "RetailApi.hpp"
#include "VictiniLuck.hpp"

namespace Gen7Follower3gx
{
namespace
{

enum class RuntimeState
{
  Stopped,
  WaitingForFieldRo,
  Attached,
  Disabled,
};

CTRPluginFramework::Hook g_StartModuleHook;
CTRPluginFramework::Hook g_DisposeModuleHook;
const GameProfile* g_Profile = NULL;
CroModuleView g_FieldRo;
RuntimeState g_State = RuntimeState::Stopped;

bool IsReadable(u32 address, u32 size)
{
  if (size == 0 || address + size < address)
  {
    return false;
  }
  return CTRPluginFramework::Process::CheckAddress(address, MEMPERM_READ) &&
    CTRPluginFramework::Process::CheckAddress(
      address + size - 1,
      MEMPERM_READ
      );
}

u32 ReadU32(u32 address)
{
  return *reinterpret_cast<const volatile u32*>(address);
}

bool ValidateLifecycleFunction(u32 address, const u32 expected[2])
{
  return IsReadable(address, 8) &&
    ReadU32(address) == expected[0] &&
    ReadU32(address + 4) == expected[1];
}

void DetachFieldRo()
{
  if (g_State != RuntimeState::Attached)
  {
    return;
  }

  RemoveFieldHooks();
  const bool shutdownComplete = ShutdownFollowerController();
  UnbindRetailApi();
  g_FieldRo.Reset();
  g_State = shutdownComplete
    ? RuntimeState::WaitingForFieldRo
    : RuntimeState::Disabled;
}

void AttachFieldRo(void* module)
{
  if (g_State != RuntimeState::WaitingForFieldRo || !g_Profile)
  {
    return;
  }

  CroModuleView candidate;
  if (!candidate.Initialize(module))
  {
    return;
  }
  FOLLOWER_3GX_TRACE("FieldRo identified");
  InspectEmulatorFieldRoForConflict(*g_Profile, candidate);
#if FOLLOWER_3GX_INTERACTION_FEATURES
  SuspendVictiniLuckHooksForRetailBind();
#endif
  if (!BindRetailApi(*g_Profile, candidate))
  {
    FOLLOWER_3GX_TRACE("retail ABI bind failed");
    return;
  }
  FOLLOWER_3GX_TRACE("retail ABI bound");
  if (!EnsureFollowerArena())
  {
#if FOLLOWER_3GX_DIAGNOSTIC
    CTRPluginFramework::OSD::Notify(std::string("Follower memory unavailable: ") + GetFollowerArenaStatus());
#endif
  }
  if (!InitializeFollowerController(g_Profile->family))
  {
    FOLLOWER_3GX_TRACE("follower controller init failed");
    UnbindRetailApi();
    return;
  }
  FOLLOWER_3GX_TRACE("follower controller initialized");
  if (!InstallFieldHooks(*g_Profile, candidate))
  {
    FOLLOWER_3GX_TRACE("field hook install failed");
    ShutdownFollowerController();
    UnbindRetailApi();
    return;
  }
#if FOLLOWER_3GX_INTERACTION_FEATURES
  if (!InstallVictiniLuckHooks(*g_Profile))
  {
    FOLLOWER_3GX_TRACE("Victini luck hook install failed");
  }
#endif

  g_FieldRo = candidate;
  g_State = RuntimeState::Attached;
  FOLLOWER_3GX_TRACE("FieldRo attached");
}

extern "C" void Follower3gx_StartModuleCallback(
  void* manager,
  void* module,
  bool linkCheck
)
{
  FOLLOWER_3GX_TRACE_VALUE(
    "StartModule module",
    reinterpret_cast<u32>(module)
    );
  CTRPluginFramework::HookContext::GetCurrent().OriginalFunction<void>(
    manager,
    module,
    linkCheck
    );
#if FOLLOWER_3GX_DIAGNOSTIC
  ObserveStartedDiagnosticEffectModule(module);
#endif
  AttachFieldRo(module);
}

extern "C" void Follower3gx_DisposeModuleCallback(
  void* manager,
  void* module
)
{
  CTRPluginFramework::HookContext& hookContext =
    CTRPluginFramework::HookContext::GetCurrent();
#if FOLLOWER_3GX_DIAGNOSTIC
  ObserveDisposingDiagnosticEffectModule(module);
#endif
  if (g_State == RuntimeState::Attached && module == g_FieldRo.Module())
  {
    DetachFieldRo();
  }
  hookContext.OriginalFunction<void>(
    manager,
    module
    );
}

void RemoveLifecycleHooks()
{
  g_DisposeModuleHook.Disable();
  g_StartModuleHook.Disable();
}

} // namespace

bool InitializePluginRuntime()
{
  FOLLOWER_3GX_TRACE("runtime initialize");
  if (g_State != RuntimeState::Stopped)
  {
    return false;
  }

#if FOLLOWER_3GX_INTERACTION_FEATURES
  InitializeVictiniLuck();
#endif

  const u64 titleId = CTRPluginFramework::Process::GetTitleID();
  const u16 version = CTRPluginFramework::Process::GetVersion();
  FOLLOWER_3GX_TRACE_VALUE("title low", static_cast<u32>(titleId));
  FOLLOWER_3GX_TRACE_VALUE("title high", static_cast<u32>(titleId >> 32));
  FOLLOWER_3GX_TRACE_VALUE("version", version);
  g_Profile = SelectGameProfile(titleId, version);
  if (!g_Profile)
  {
    FOLLOWER_3GX_TRACE("profile selection failed");
    g_State = RuntimeState::Disabled;
    return false;
  }
  FOLLOWER_3GX_TRACE("profile selected");

  const LifecycleProfile& lifecycle = g_Profile->lifecycle;
  if (!ValidateLifecycleFunction(
        lifecycle.startModuleAddress,
        lifecycle.startModuleInstructions
        ) ||
      !ValidateLifecycleFunction(
        lifecycle.disposeModuleAddress,
        lifecycle.disposeModuleInstructions
        ))
  {
    FOLLOWER_3GX_TRACE("lifecycle signature failed");
    g_Profile = NULL;
    g_State = RuntimeState::Disabled;
    return false;
  }
  FOLLOWER_3GX_TRACE("lifecycle signatures valid");

  using CTRPluginFramework::HookResult;
  if (g_StartModuleHook.InitializeForMitm(
        lifecycle.startModuleAddress,
        reinterpret_cast<u32>(Follower3gx_StartModuleCallback)
        ).Enable() != HookResult::Success)
  {
    FOLLOWER_3GX_TRACE("StartModule hook failed");
    RemoveLifecycleHooks();
    g_Profile = NULL;
    g_State = RuntimeState::Disabled;
    return false;
  }
  FOLLOWER_3GX_TRACE("StartModule hook enabled");
  if (g_DisposeModuleHook.InitializeForMitm(
        lifecycle.disposeModuleAddress,
        reinterpret_cast<u32>(Follower3gx_DisposeModuleCallback)
        ).Enable() != HookResult::Success)
  {
    FOLLOWER_3GX_TRACE("DisposeModule hook failed");
    RemoveLifecycleHooks();
    g_Profile = NULL;
    g_State = RuntimeState::Disabled;
    return false;
  }

  g_State = RuntimeState::WaitingForFieldRo;
  FOLLOWER_3GX_TRACE("lifecycle hooks ready");
  return true;
}

bool IsRegularGameRuntime()
{
  return g_Profile && g_Profile->family == GameFamily::Regular;
}

void ShutdownPluginRuntime()
{
#if FOLLOWER_3GX_INTERACTION_FEATURES
  ShutdownVictiniLuck();
#endif
  RemoveLifecycleHooks();
  if (g_State == RuntimeState::Attached)
  {
    DetachFieldRo();
  }
  else
  {
    RemoveFieldHooks();
    if (IsFollowerControllerInitialized())
    {
      ShutdownFollowerController();
    }
    UnbindRetailApi();
    g_FieldRo.Reset();
  }
  g_Profile = NULL;
  g_State = RuntimeState::Stopped;
}

} // namespace Gen7Follower3gx
