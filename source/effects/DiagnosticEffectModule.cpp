#include "compat/GameAbi.hpp"

#include "CroModule.hpp"
#include "DiagnosticEffectModule.hpp"

#if FOLLOWER_3GX_DIAGNOSTIC

namespace Gen7Follower3gx
{

bool CroModuleNameEquals(void* module, const char* expectedName);
bool IsRetailApiBound();

namespace
{

const unsigned int DLL_HEAP_MINIMUM_FREE = 0x20000;
const char FIELD_EFFECT_UNIQUE_NAME[] = "FieldEffectUnique";
const char FIELD_EFFECT_UNIQUE_FILE[] = "FieldEffectUnique.cro";

// Creating these effects doesn't start them. They need a separate activation call.
const unsigned int UNIQUE_PARTICLE_CREATE_OFFSET = 0x00000c78;
const unsigned int UNIQUE_MODEL_CREATE_OFFSET = 0x00002de8;
const int UNIQUE_PARTICLE_Z_VARIANT = 3;
const int UNIQUE_MODEL_Z_VARIANT = 103;

const unsigned int UNIQUE_PARTICLE_CREATE_SIGNATURE[] =
{
  0xe92d4070,
  0xe1a04000,
  0xe5900018,
  0xe24dd010,
  0xe1a05001,
  0xe3500000,
  0xe3a06000,
};

const unsigned int UNIQUE_MODEL_CREATE_SIGNATURE[] =
{
  0xe92d4070,
  0xe1a04000,
  0xe5900024,
  0xe24dd008,
  0xe1a05001,
  0xe3500000,
};

DiagnosticEffectModuleSnapshot g_Snapshot;
gfl2::ro::RoManager* g_RoManager = NULL;
nn::ro::Module* g_UniqueModule = NULL;
nn::ro::Module* g_LoadCandidate = NULL;
bool g_Initialized = false;
bool g_OwnsUniqueModule = false;
bool g_LoadInProgress = false;

void StoreResult(DiagnosticEffectModuleResult result)
{
  __atomic_store_n(
    &g_Snapshot.result,
    static_cast<unsigned int>(result),
    __ATOMIC_RELAXED
    );
}

void StoreRequirement(DiagnosticEffectModuleRequirement requirement)
{
  __atomic_store_n(
    &g_Snapshot.requirement,
    static_cast<unsigned int>(requirement),
    __ATOMIC_RELAXED
    );
}

void StoreOwnership()
{
  __atomic_store_n(
    &g_Snapshot.ownsModule,
    g_OwnsUniqueModule ? 1U : 0U,
    __ATOMIC_RELAXED
    );
}

unsigned int ReadDllHeapFree()
{
  if (!IsRetailApiBound())
  {
    return 0;
  }
  gfl2::heap::HeapBase* heap =
    gfl2::heap::Manager::GetHeapByHeapId(HEAPID_DLL_LOAD);
  return heap ? heap->GetTotalAllocatableSize() : 0;
}

DiagnosticEffectModuleResult Fail(DiagnosticEffectModuleResult result)
{
  StoreResult(result);
  return result;
}

DiagnosticEffectModuleResult Ready(DiagnosticEffectModuleResult result)
{
  StoreResult(result);
  __atomic_fetch_add(&g_Snapshot.successCount, 1U, __ATOMIC_RELAXED);
  return result;
}

bool MatchesWords(
  unsigned int address,
  const unsigned int* expected,
  unsigned int count
)
{
  const volatile unsigned int* words =
    reinterpret_cast<const volatile unsigned int*>(address);
  for (unsigned int index = 0; index < count; ++index)
  {
    if (words[index] != expected[index])
    {
      return false;
    }
  }
  return true;
}

bool InvokeUniqueController(
  void* effect,
  unsigned int textOffset,
  const unsigned int* signature,
  unsigned int signatureCount,
  int variant
)
{
  if (!effect || !g_UniqueModule)
  {
    return false;
  }

  CroModuleView unique;
  if (!unique.Initialize(g_UniqueModule, FIELD_EFFECT_UNIQUE_NAME) ||
      textOffset + signatureCount * sizeof(unsigned int) > unique.TextSize())
  {
    return false;
  }

  const unsigned int address = unique.TextBase() + textOffset;
  if (!MatchesWords(address, signature, signatureCount))
  {
    return false;
  }

  typedef void (*CreateEffectVariantFunction)(void*, int);
  reinterpret_cast<CreateEffectVariantFunction>(address)(effect, variant);
  return true;
}

} // namespace

void InitializeDiagnosticEffectModule()
{
  g_RoManager = NULL;
  g_UniqueModule = NULL;
  g_LoadCandidate = NULL;
  g_OwnsUniqueModule = false;
  g_LoadInProgress = false;
  g_Initialized = true;
  __atomic_store_n(&g_Snapshot.attemptCount, 0U, __ATOMIC_RELAXED);
  __atomic_store_n(&g_Snapshot.successCount, 0U, __ATOMIC_RELAXED);
  StoreRequirement(DIAGNOSTIC_EFFECT_MODULE_NONE);
  StoreResult(DIAGNOSTIC_EFFECT_MODULE_RESULT_NOT_ATTEMPTED);
  __atomic_store_n(&g_Snapshot.dllHeapFreeBefore, 0U, __ATOMIC_RELAXED);
  __atomic_store_n(&g_Snapshot.dllHeapFreeAfter, 0U, __ATOMIC_RELAXED);
  StoreOwnership();
}

void ShutdownDiagnosticEffectModule()
{
  ReleaseOwnedDiagnosticEffectModule();
  g_RoManager = NULL;
  g_UniqueModule = NULL;
  g_LoadCandidate = NULL;
  g_OwnsUniqueModule = false;
  g_LoadInProgress = false;
  g_Initialized = false;
  StoreOwnership();
}

DiagnosticEffectModuleResult EnsureDiagnosticEffectModule(
  unsigned int effectType,
  void* fieldmap
)
{
  const DiagnosticEffectModuleRequirement requirement =
    GetDiagnosticEffectModuleRequirement(effectType);
  StoreRequirement(requirement);
  if (requirement == DIAGNOSTIC_EFFECT_MODULE_NONE)
  {
    return Fail(DIAGNOSTIC_EFFECT_MODULE_RESULT_NOT_REQUIRED);
  }

  __atomic_fetch_add(&g_Snapshot.attemptCount, 1U, __ATOMIC_RELAXED);
  if (requirement != DIAGNOSTIC_EFFECT_MODULE_FIELD_EFFECT_UNIQUE)
  {
    return Fail(DIAGNOSTIC_EFFECT_MODULE_RESULT_UNSUPPORTED);
  }
  if (!g_Initialized || !fieldmap || !IsRetailApiBound())
  {
    return Fail(DIAGNOSTIC_EFFECT_MODULE_RESULT_NO_FIELDMAP);
  }

  const unsigned int freeBefore = ReadDllHeapFree();
  __atomic_store_n(
    &g_Snapshot.dllHeapFreeBefore,
    freeBefore,
    __ATOMIC_RELAXED
    );
  if (g_UniqueModule)
  {
    __atomic_store_n(
      &g_Snapshot.dllHeapFreeAfter,
      freeBefore,
      __ATOMIC_RELAXED
      );
    return Ready(
      g_OwnsUniqueModule
        ? DIAGNOSTIC_EFFECT_MODULE_RESULT_READY_PLUGIN
        : DIAGNOSTIC_EFFECT_MODULE_RESULT_READY_RETAIL
      );
  }

  Field::Fieldmap* retailFieldmap =
    reinterpret_cast<Field::Fieldmap*>(fieldmap);
  GameSys::GameManager* gameManager = retailFieldmap->GetGameManager();
  if (!gameManager)
  {
    return Fail(DIAGNOSTIC_EFFECT_MODULE_RESULT_NO_GAME_MANAGER);
  }
  gfl2::fs::AsyncFileManager* fileManager =
    gameManager->GetAsyncFileManager();
  if (!fileManager)
  {
    return Fail(DIAGNOSTIC_EFFECT_MODULE_RESULT_NO_FILE_MANAGER);
  }

  g_RoManager = GFL_SINGLETON_INSTANCE(gfl2::ro::RoManager);
  if (!g_RoManager)
  {
    return Fail(DIAGNOSTIC_EFFECT_MODULE_RESULT_NO_RO_MANAGER);
  }
  gfl2::heap::HeapBase* dllHeap =
    gfl2::heap::Manager::GetHeapByHeapId(HEAPID_DLL_LOAD);
  if (!dllHeap)
  {
    return Fail(DIAGNOSTIC_EFFECT_MODULE_RESULT_NO_DLL_HEAP);
  }
  if (freeBefore < DLL_HEAP_MINIMUM_FREE)
  {
    return Fail(DIAGNOSTIC_EFFECT_MODULE_RESULT_DLL_HEAP_LOW);
  }

  nn::ro::Module* module = g_RoManager->LoadModule(
    fileManager,
    FIELD_EFFECT_UNIQUE_FILE,
    NULL,
    gfl2::ro::FIX_LEVEL_1
    );
  if (!module)
  {
    return Fail(DIAGNOSTIC_EFFECT_MODULE_RESULT_LOAD_FAILED);
  }

  g_UniqueModule = module;
  g_LoadCandidate = module;
  g_OwnsUniqueModule = true;
  g_LoadInProgress = true;
  StoreOwnership();
  g_RoManager->StartModule(module, false);
  g_LoadInProgress = false;
  g_LoadCandidate = NULL;

  const unsigned int freeAfter = ReadDllHeapFree();
  __atomic_store_n(
    &g_Snapshot.dllHeapFreeAfter,
    freeAfter,
    __ATOMIC_RELAXED
    );
  return Ready(DIAGNOSTIC_EFFECT_MODULE_RESULT_LOADED);
}

bool IsDiagnosticEffectModuleReady(DiagnosticEffectModuleResult result)
{
  return result == DIAGNOSTIC_EFFECT_MODULE_RESULT_NOT_REQUIRED ||
    result == DIAGNOSTIC_EFFECT_MODULE_RESULT_READY_RETAIL ||
    result == DIAGNOSTIC_EFFECT_MODULE_RESULT_READY_PLUGIN ||
    result == DIAGNOSTIC_EFFECT_MODULE_RESULT_LOADED;
}

bool InitializeDiagnosticEffectInstance(
  unsigned int effectType,
  void* effect
)
{
  switch (effectType)
  {
  case Field::Effect::EFFECT_TYPE_DEMO_NEW_TRIAL5_01:
    return InvokeUniqueController(
      effect,
      UNIQUE_PARTICLE_CREATE_OFFSET,
      UNIQUE_PARTICLE_CREATE_SIGNATURE,
      sizeof(UNIQUE_PARTICLE_CREATE_SIGNATURE) /
        sizeof(UNIQUE_PARTICLE_CREATE_SIGNATURE[0]),
      UNIQUE_PARTICLE_Z_VARIANT
      );
  case Field::Effect::EFFECT_TYPE_DEMO_NEW_TRIAL5_02:
    return InvokeUniqueController(
      effect,
      UNIQUE_MODEL_CREATE_OFFSET,
      UNIQUE_MODEL_CREATE_SIGNATURE,
      sizeof(UNIQUE_MODEL_CREATE_SIGNATURE) /
        sizeof(UNIQUE_MODEL_CREATE_SIGNATURE[0]),
      UNIQUE_MODEL_Z_VARIANT
      );
  default:
    return true;
  }
}

void ReleaseOwnedDiagnosticEffectModule()
{
  if (!g_Initialized || !g_UniqueModule || !g_OwnsUniqueModule ||
      !IsRetailApiBound())
  {
    return;
  }

  if (!g_RoManager)
  {
    g_RoManager = GFL_SINGLETON_INSTANCE(gfl2::ro::RoManager);
  }
  if (!g_RoManager)
  {
    return;
  }

  nn::ro::Module* module = g_UniqueModule;
  g_RoManager->DisposeModule(module);
  if (g_UniqueModule == module)
  {
    g_UniqueModule = NULL;
    g_OwnsUniqueModule = false;
  }
  StoreOwnership();
  __atomic_store_n(
    &g_Snapshot.dllHeapFreeAfter,
    ReadDllHeapFree(),
    __ATOMIC_RELAXED
    );
  StoreResult(DIAGNOSTIC_EFFECT_MODULE_RESULT_RELEASED);
}

void ObserveStartedDiagnosticEffectModule(void* module)
{
  if (!g_Initialized ||
      !CroModuleNameEquals(module, FIELD_EFFECT_UNIQUE_NAME))
  {
    return;
  }

  nn::ro::Module* retailModule = reinterpret_cast<nn::ro::Module*>(module);
  const bool owned = g_LoadInProgress && retailModule == g_LoadCandidate;
  if (g_UniqueModule && g_UniqueModule != retailModule)
  {
    return;
  }
  g_UniqueModule = retailModule;
  g_OwnsUniqueModule = owned;
  StoreRequirement(DIAGNOSTIC_EFFECT_MODULE_FIELD_EFFECT_UNIQUE);
  StoreOwnership();
  StoreResult(
    owned
      ? DIAGNOSTIC_EFFECT_MODULE_RESULT_READY_PLUGIN
      : DIAGNOSTIC_EFFECT_MODULE_RESULT_READY_RETAIL
    );
}

void ObserveDisposingDiagnosticEffectModule(void* module)
{
  if (!g_Initialized || module != g_UniqueModule)
  {
    return;
  }
  g_UniqueModule = NULL;
  g_OwnsUniqueModule = false;
  StoreOwnership();
  StoreResult(DIAGNOSTIC_EFFECT_MODULE_RESULT_RELEASED);
}

DiagnosticEffectModuleSnapshot GetDiagnosticEffectModuleSnapshot()
{
  DiagnosticEffectModuleSnapshot result;
  result.attemptCount = __atomic_load_n(
    &g_Snapshot.attemptCount,
    __ATOMIC_RELAXED
    );
  result.successCount = __atomic_load_n(
    &g_Snapshot.successCount,
    __ATOMIC_RELAXED
    );
  result.requirement = __atomic_load_n(
    &g_Snapshot.requirement,
    __ATOMIC_RELAXED
    );
  result.result = __atomic_load_n(
    &g_Snapshot.result,
    __ATOMIC_RELAXED
    );
  result.dllHeapFreeBefore = __atomic_load_n(
    &g_Snapshot.dllHeapFreeBefore,
    __ATOMIC_RELAXED
    );
  result.dllHeapFreeAfter = __atomic_load_n(
    &g_Snapshot.dllHeapFreeAfter,
    __ATOMIC_RELAXED
    );
  result.ownsModule = __atomic_load_n(
    &g_Snapshot.ownsModule,
    __ATOMIC_RELAXED
    );
  return result;
}

} // namespace Gen7Follower3gx

#else

namespace Gen7Follower3gx
{

void InitializeDiagnosticEffectModule() {}
void ShutdownDiagnosticEffectModule() {}
DiagnosticEffectModuleResult EnsureDiagnosticEffectModule(
  unsigned int,
  void*
)
{
  return DIAGNOSTIC_EFFECT_MODULE_RESULT_UNSUPPORTED;
}
bool IsDiagnosticEffectModuleReady(DiagnosticEffectModuleResult)
{
  return false;
}
bool InitializeDiagnosticEffectInstance(unsigned int, void*)
{
  return false;
}
void ReleaseOwnedDiagnosticEffectModule() {}
void ObserveStartedDiagnosticEffectModule(void*) {}
void ObserveDisposingDiagnosticEffectModule(void*) {}
DiagnosticEffectModuleSnapshot GetDiagnosticEffectModuleSnapshot()
{
  DiagnosticEffectModuleSnapshot result = {};
  return result;
}

} // namespace Gen7Follower3gx

#endif
