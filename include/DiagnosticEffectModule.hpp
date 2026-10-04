#pragma once

#include "DiagnosticEffectSelector.hpp"

namespace Gen7Follower3gx
{

enum DiagnosticEffectModuleResult
{
  DIAGNOSTIC_EFFECT_MODULE_RESULT_NOT_ATTEMPTED,
  DIAGNOSTIC_EFFECT_MODULE_RESULT_NOT_REQUIRED,
  DIAGNOSTIC_EFFECT_MODULE_RESULT_READY_RETAIL,
  DIAGNOSTIC_EFFECT_MODULE_RESULT_READY_PLUGIN,
  DIAGNOSTIC_EFFECT_MODULE_RESULT_LOADED,
  DIAGNOSTIC_EFFECT_MODULE_RESULT_UNSUPPORTED,
  DIAGNOSTIC_EFFECT_MODULE_RESULT_NO_FIELDMAP,
  DIAGNOSTIC_EFFECT_MODULE_RESULT_NO_GAME_MANAGER,
  DIAGNOSTIC_EFFECT_MODULE_RESULT_NO_FILE_MANAGER,
  DIAGNOSTIC_EFFECT_MODULE_RESULT_NO_RO_MANAGER,
  DIAGNOSTIC_EFFECT_MODULE_RESULT_NO_DLL_HEAP,
  DIAGNOSTIC_EFFECT_MODULE_RESULT_DLL_HEAP_LOW,
  DIAGNOSTIC_EFFECT_MODULE_RESULT_LOAD_FAILED,
  DIAGNOSTIC_EFFECT_MODULE_RESULT_RELEASED,
};

struct DiagnosticEffectModuleSnapshot
{
  unsigned int attemptCount;
  unsigned int successCount;
  unsigned int requirement;
  unsigned int result;
  unsigned int dllHeapFreeBefore;
  unsigned int dllHeapFreeAfter;
  unsigned int ownsModule;
};

void InitializeDiagnosticEffectModule();
void ShutdownDiagnosticEffectModule();
DiagnosticEffectModuleResult EnsureDiagnosticEffectModule(
  unsigned int effectType,
  void* fieldmap
  );
bool IsDiagnosticEffectModuleReady(DiagnosticEffectModuleResult result);
bool InitializeDiagnosticEffectInstance(
  unsigned int effectType,
  void* effect
  );
void ReleaseOwnedDiagnosticEffectModule();
void ObserveStartedDiagnosticEffectModule(void* module);
void ObserveDisposingDiagnosticEffectModule(void* module);
DiagnosticEffectModuleSnapshot GetDiagnosticEffectModuleSnapshot();

} // namespace Gen7Follower3gx
