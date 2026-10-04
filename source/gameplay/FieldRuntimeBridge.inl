#if FOLLOWER_CARRIER_THREEGX
#include "FreeCamera.hpp"
#endif
#ifndef FOLLOWER_CARRIER_PIPELINE_EDGE_NORMAL_OFFSET
#define FOLLOWER_CARRIER_PIPELINE_EDGE_NORMAL_OFFSET 0x0d10
#endif

#ifndef FOLLOWER_CARRIER_PIPELINE_BLOOM_ENABLE_OFFSET
#define FOLLOWER_CARRIER_PIPELINE_BLOOM_ENABLE_OFFSET 0x03e0
#endif

#ifndef FOLLOWER_CARRIER_PIPELINE_DOF_ENABLE_OFFSET
#define FOLLOWER_CARRIER_PIPELINE_DOF_ENABLE_OFFSET 0x0d2c
#endif

#ifndef FOLLOWER_CARRIER_PIPELINE_SKYBOX_DRAW_MANAGER_OFFSET
#define FOLLOWER_CARRIER_PIPELINE_SKYBOX_DRAW_MANAGER_OFFSET 0x0154
#endif

#ifndef FOLLOWER_CARRIER_GAME_MANAGER_EVENT_MANAGER_OFFSET
#define FOLLOWER_CARRIER_GAME_MANAGER_EVENT_MANAGER_OFFSET 0x20
#endif

#ifndef FOLLOWER_CARRIER_GAME_MANAGER_GAME_DATA_OFFSET
#define FOLLOWER_CARRIER_GAME_MANAGER_GAME_DATA_OFFSET 0x24
#endif

#ifndef FOLLOWER_CARRIER_GAME_MANAGER_FIELD_SCRIPT_SYSTEM_OFFSET
#define FOLLOWER_CARRIER_GAME_MANAGER_FIELD_SCRIPT_SYSTEM_OFFSET 0x34
#endif

#ifndef FOLLOWER_CARRIER_SAVEDATA_EVENT_WORK_OFFSET
#define FOLLOWER_CARRIER_SAVEDATA_EVENT_WORK_OFFSET 0x15d8
#endif

#ifndef FOLLOWER_CARRIER_THREEGX
#define FOLLOWER_CARRIER_THREEGX 0
#endif

#ifndef FOLLOWER_CARRIER_ENABLE_CUTSCENE_FAST_FORWARD
#define FOLLOWER_CARRIER_ENABLE_CUTSCENE_FAST_FORWARD 1
#endif

#if FOLLOWER_CARRIER_THREEGX
#include <new>
#include "FollowerSettings.hpp"
#include "PerformanceDiagnostics.hpp"
#endif

#define FOLLOWER_POKEMON_USE_AREA_RESOURCE_HEAP 1
#define FOLLOWER_POKEMON_GET_GAME_EVENT_MANAGER(pGameManager) \
  (*reinterpret_cast<GameSys::GameEventManager**>( \
    reinterpret_cast<unsigned char*>(pGameManager) + \
    FOLLOWER_CARRIER_GAME_MANAGER_EVENT_MANAGER_OFFSET))
#define FOLLOWER_POKEMON_GET_GAME_DATA(pGameManager) \
  (*reinterpret_cast<GameSys::GameData**>( \
    reinterpret_cast<unsigned char*>(pGameManager) + \
    FOLLOWER_CARRIER_GAME_MANAGER_GAME_DATA_OFFSET))
#define FOLLOWER_POKEMON_GET_EVENT_WORK(pGameData) \
  (reinterpret_cast<Field::EventWork*>( \
    *reinterpret_cast<unsigned char**>( \
      reinterpret_cast<unsigned char*>(pGameData) + 0x04) + \
    FOLLOWER_CARRIER_SAVEDATA_EVENT_WORK_OFFSET))
#define FOLLOWER_POKEMON_GET_AREA_RESOURCE_HEAP(pArea) \
  (*reinterpret_cast<gfl2::heap::HeapBase**>(reinterpret_cast<unsigned char*>(pArea) + 0x08))
#define FOLLOWER_POKEMON_GET_EDGE_NORMAL_MAP_ENABLE(pPipeLine) \
  (*reinterpret_cast<const int*>(reinterpret_cast<const unsigned char*>(pPipeLine) + FOLLOWER_CARRIER_PIPELINE_EDGE_NORMAL_OFFSET) != 0)
#define FOLLOWER_POKEMON_SET_EDGE_NORMAL_MAP_ENABLE(pPipeLine, flag) \
  (*reinterpret_cast<int*>(reinterpret_cast<unsigned char*>(pPipeLine) + FOLLOWER_CARRIER_PIPELINE_EDGE_NORMAL_OFFSET) = ((flag) ? 1 : 0))
#include "FollowerRuntime.hpp"

#ifndef FOLLOWER_CARRIER_ENABLE_LOGGING
#define FOLLOWER_CARRIER_ENABLE_LOGGING 1
#endif

#if FOLLOWER_CARRIER_ENABLE_LOGGING
#if FOLLOWER_CARRIER_THREEGX
extern "C" int svcOutputDebugString(const char* text, int length);
extern "C" unsigned int FollowerCarrier_OutputDebugString(
  const void* text,
  int length
)
{
  return static_cast<unsigned int>(
    svcOutputDebugString(static_cast<const char*>(text), length)
    );
}
#else
extern "C" __asm unsigned int FollowerCarrier_OutputDebugString(
  const void* text,
  int length
)
{
  SVC 0x3D
  BX lr
}
#endif
#endif

namespace
{
enum
{
  FIELD_LIGHT_SET_NO = 0,
  FIELD_STENCIL_NEUTRAL_ID = 0xff,
  // Use the boundary between character and background IDs for a subtle follower outline.
  FIELD_STENCIL_SOFT_ID = 150,
  FIELD_STENCIL_MEDIUM_ID = 149,
  FIELD_PIPELINE_EDGE_PATH_OFFSET = 0x0084,
  FIELD_PIPELINE_OUTLINE_PATH_OFFSET = 0x0088,
  FIELD_PIPELINE_MAIN_DRAW_MANAGER_OFFSET = 0x0090,
  OUTLINE_PATH_NORMAL_MODEL_OFFSET = 0x0014,
  OUTLINE_TEXTURE_SAMPLE_COUNT = 3,
  FOLLOWER_OUTLINE_COLOR_LIFT = 24,
  EDGE_PATH_DRAWABLE_CONTAINER_OFFSET = 0x0038,
  EDGE_CONTAINER_BUFFER_OFFSET = 0x0004,
  EDGE_CONTAINER_LINK_LIST_OFFSET = 0x0008,
  EDGE_CONTAINER_INDEXER_OFFSET = 0x000c,
  EDGE_CONTAINER_BUFFER_SIZE_OFFSET = 0x0010,
  EDGE_CONTAINER_EMPTY_TOP_OFFSET = 0x0014,
  EDGE_CONTAINER_DATA_TOP_OFFSET = 0x0018,
  EDGE_CONTAINER_USED_SIZE_OFFSET = 0x001c,
  FIELDMAP_MOVE_MODEL_SHADOW_MANAGER_OFFSET = 0x00e4,
  MOVE_MODEL_SHADOW_FIRST_EFFECT_OFFSET = 0x000c,
  MOVE_MODEL_SHADOW_EFFECT_STRIDE = 0x0004,
  EFFECT_SHADOW_MODEL_NODE_OFFSET = 0x0024,
  PERFORMANCE_CHARACTER_SHADOW_CAPACITY = 4,
  EDGE_MATERIAL_STATE_CAPACITY = 512,
  FOLLOWER_MATERIAL_CAPACITY = 128,
  WATER_RIDE_REQUEST_GRACE_FRAMES = 180,
  FIELD_SCRIPT_NOW_OBJECT_OFFSET = 0x0050,
  PAWN_BASE_HALT_FLAG_OFFSET = 0x008a,
  REGULAR_OBJECT_WAIT_FUNC_OFFSET = 0x03f0,
  REGULAR_OBJECT_WAIT_LABEL_OFFSET = 0x03f4,
  REGULAR_OBJECT_WAIT_WORK_OFFSET = 0x03f8,
  FIELD_TALK_WINDOW_INNER_OFFSET = 0x0024,
  MESSAGE_WINDOW_FINISH_TYPE_OFFSET = 0x0018,
  MESSAGE_WINDOW_SEQUENCE_OFFSET = 0x002d,
  STRWIN_FINISH_NONE = 0,
  STRWIN_SEQUENCE_DONE = 0,
  FIELDRO_UPDATE_COMMAND_PAUSE = 1,
  FIELDRO_UPDATE_COMMAND_RUN_POST = 2,
  CUTSCENE_FAST_FORWARD_EXTRA_UPDATES = 31,
  CUTSCENE_FAST_FORWARD_COMMAND_SHIFT = 8,
};

const float FOLLOWER_OUTLINE_SAMPLE_SCALE = 0.75f;

class FollowerSceneIntegration
{
public:
  FollowerSceneIntegration();

  bool Apply(
    gfl2::renderingengine::scenegraph::instance::ModelInstanceNode* followerNode
  );
#if FOLLOWER_CARRIER_THREEGX
  void SetOutlineMode(
    Gen7Follower3gx::FollowerOutlineMode outlineMode
  );
#endif
  void Reset();

private:
  bool ConfigureStencil(
    gfl2::renderingengine::scenegraph::instance::ModelInstanceNode* followerNode
  );
  void ApplyFieldLighting(
    gfl2::renderingengine::scenegraph::instance::ModelInstanceNode* followerNode
  );

  gfl2::renderingengine::scenegraph::instance::ModelInstanceNode*
    m_pConfiguredNode;
#if FOLLOWER_CARRIER_THREEGX
  struct StencilReferenceState
  {
    gfl2::gfx::DepthStencilStateObject* stateObject;
    unsigned char originalReference;
  };

  StencilReferenceState m_StencilReferenceStates[FOLLOWER_MATERIAL_CAPACITY];
  unsigned int m_StencilReferenceStateCount;
  Gen7Follower3gx::FollowerOutlineMode m_OutlineMode;
#endif
};

FollowerSceneIntegration::FollowerSceneIntegration()
: m_pConfiguredNode(NULL)
#if FOLLOWER_CARRIER_THREEGX
, m_StencilReferenceStateCount(0)
, m_OutlineMode(Gen7Follower3gx::FOLLOWER_OUTLINE_OFF)
#endif
{
}

bool FollowerSceneIntegration::ConfigureStencil(
  gfl2::renderingengine::scenegraph::instance::ModelInstanceNode* followerNode
)
{
  typedef gfl2::renderingengine::scenegraph::instance::MaterialInstanceNode
    MaterialInstanceNode;

#if FOLLOWER_CARRIER_THREEGX
  m_StencilReferenceStateCount = 0;
  m_OutlineMode = Gen7Follower3gx::FOLLOWER_OUTLINE_OFF;
#endif
  bool configured = false;
  const unsigned int materialCount = followerNode->GetMaterialNum();
  for (unsigned int materialIndex = 0;
       materialIndex < materialCount;
       ++materialIndex)
  {
    MaterialInstanceNode* material =
      followerNode->GetMaterialInstanceNode(materialIndex);
    if (!material || !material->GetDepthStencilStateObject())
    {
      continue;
    }

    gfl2::gfx::DepthStencilStateObject* depthStencil =
      material->GetDepthStencilStateObject();
#if FOLLOWER_CARRIER_THREEGX
    if (m_StencilReferenceStateCount < FOLLOWER_MATERIAL_CAPACITY)
    {
      StencilReferenceState& state =
        m_StencilReferenceStates[m_StencilReferenceStateCount++];
      state.stateObject = depthStencil;
      state.originalReference = depthStencil->GetStencilReference();
    }
#endif
    depthStencil->SetStencilTestEnable(true);
    depthStencil->SetStencilFunc(
      gfl2::gfx::PolygonFace::CW,
      gfl2::gfx::CompareFunc::Always,
      FIELD_STENCIL_NEUTRAL_ID,
      0xff
      );
    depthStencil->SetStencilFunc(
      gfl2::gfx::PolygonFace::CCW,
      gfl2::gfx::CompareFunc::Always,
      FIELD_STENCIL_NEUTRAL_ID,
      0xff
      );
    depthStencil->SetStencilWriteMask(0xff);
    depthStencil->SetStencilOp(
      gfl2::gfx::PolygonFace::CW,
      gfl2::gfx::StencilOp::Keep,
      gfl2::gfx::StencilOp::Keep,
      gfl2::gfx::StencilOp::Replace
      );
    depthStencil->SetStencilOp(
      gfl2::gfx::PolygonFace::CCW,
      gfl2::gfx::StencilOp::Keep,
      gfl2::gfx::StencilOp::Keep,
      gfl2::gfx::StencilOp::Replace
      );
    depthStencil->UpdateState();
    configured = true;
  }
  return configured;
}

void FollowerSceneIntegration::ApplyFieldLighting(
  gfl2::renderingengine::scenegraph::instance::ModelInstanceNode* followerNode
)
{
  typedef gfl2::renderingengine::scenegraph::instance::MaterialInstanceNode
    MaterialInstanceNode;
  typedef gfl2::renderingengine::scenegraph::resource::MaterialResourceNode
    MaterialResourceNode;

  const unsigned int materialCount = followerNode->GetMaterialNum();
  for (unsigned int materialIndex = 0;
       materialIndex < materialCount;
       ++materialIndex)
  {
    MaterialInstanceNode* material =
      followerNode->GetMaterialInstanceNode(materialIndex);
    if (!material)
    {
      continue;
    }

    MaterialResourceNode::AttributeParam* attributes =
      material->GetAttributeParam();
    attributes->m_LightSetNo = FIELD_LIGHT_SET_NO;

    // The field light already adds this tint, so don't apply it twice.
    attributes->m_ConstantColor[5].r = 0xff;
    attributes->m_ConstantColor[5].g = 0xff;
    attributes->m_ConstantColor[5].b = 0xff;
    attributes->m_ConstantColor[5].a = 0xff;
  }
}

bool FollowerSceneIntegration::Apply(
  gfl2::renderingengine::scenegraph::instance::ModelInstanceNode* followerNode
)
{
  if (!followerNode)
  {
    Reset();
    return false;
  }

  ApplyFieldLighting(followerNode);
  if (m_pConfiguredNode == followerNode)
  {
    return false;
  }

  if (!ConfigureStencil(followerNode))
  {
    return false;
  }

  m_pConfiguredNode = followerNode;
  return true;
}

#if FOLLOWER_CARRIER_THREEGX
void FollowerSceneIntegration::SetOutlineMode(
  Gen7Follower3gx::FollowerOutlineMode outlineMode
)
{
  if (m_OutlineMode == outlineMode)
  {
    return;
  }

  for (unsigned int i = 0; i < m_StencilReferenceStateCount; ++i)
  {
    StencilReferenceState& state = m_StencilReferenceStates[i];
    if (!state.stateObject)
    {
      continue;
    }

    unsigned char reference = FIELD_STENCIL_NEUTRAL_ID;
    switch (outlineMode)
    {
    case Gen7Follower3gx::FOLLOWER_OUTLINE_ID_ORIGINAL:
      reference = state.originalReference;
      break;
    case Gen7Follower3gx::FOLLOWER_OUTLINE_ID_SOFT:
      reference = FIELD_STENCIL_SOFT_ID;
      break;
    case Gen7Follower3gx::FOLLOWER_OUTLINE_ID_MEDIUM:
      reference = FIELD_STENCIL_MEDIUM_ID;
      break;
    default:
      break;
    }
    state.stateObject->SetStencilFunc(
      gfl2::gfx::PolygonFace::CW,
      gfl2::gfx::CompareFunc::Always,
      reference,
      0xff
      );
    state.stateObject->SetStencilFunc(
      gfl2::gfx::PolygonFace::CCW,
      gfl2::gfx::CompareFunc::Always,
      reference,
      0xff
      );
    state.stateObject->UpdateState();
  }
  m_OutlineMode = outlineMode;
}
#endif

void FollowerSceneIntegration::Reset()
{
  m_pConfiguredNode = NULL;
#if FOLLOWER_CARRIER_THREEGX
  m_StencilReferenceStateCount = 0;
  m_OutlineMode = Gen7Follower3gx::FOLLOWER_OUTLINE_OFF;
#endif
}

template <typename T>
T& RawField(void* object, unsigned int offset)
{
  return *reinterpret_cast<T*>(
    reinterpret_cast<unsigned char*>(object) + offset
    );
}

struct EdgeContainerState
{
  void* buffer;
  void* linkList;
  void* indexer;
  unsigned int bufferSize;
  void* emptyTop;
  void* dataTop;
  unsigned int usedSize;
};

struct EdgeLinkData
{
  void** data;
  EdgeLinkData* previous;
  EdgeLinkData* next;
  int index;
};

typedef gfl2::renderingengine::scenegraph::resource::MaterialResourceNode::UserData
  EdgeMaterialUserData;
typedef gfl2::renderingengine::scenegraph::resource::MaterialResourceNode::AttributeParam
  EdgePostAttributeParam;
typedef gfl2::renderingengine::scenegraph::resource::MaterialResourceNode::TextureInfo
  EdgePostTextureInfo;

struct EdgeMaterialState
{
  EdgeMaterialUserData* userData;
  int edgeType;
};

#if FOLLOWER_3GX_PERFORMANCE_FEATURES
struct PerformanceMaterialState
{
  EdgePostAttributeParam* attributes;
  EdgeMaterialUserData* userData;
  bool psLightingEnable;
  bool fogEnable;
  int bumpMapNo;
  float rimScale;
  float phongScale;
};
#endif

class FollowerEdgeIsolation
{
public:
  FollowerEdgeIsolation();

  bool Apply(
    Field::MyRenderingPipeLine* renderingPipeline,
    gfl2::renderingengine::scenegraph::instance::ModelInstanceNode* followerNode
#if FOLLOWER_3GX_PERFORMANCE_FEATURES
    , bool suppressOccluderDraws
    , bool suppressFollowerDraws
#endif
  );
#if FOLLOWER_CARRIER_THREEGX
  bool SuppressFollowerOutline(
    Field::MyRenderingPipeLine* renderingPipeline,
    gfl2::renderingengine::scenegraph::instance::ModelInstanceNode* followerNode
  );
#endif
#if FOLLOWER_3GX_PERFORMANCE_FEATURES
  bool ApplyPerformanceLite(
    gfl2::renderingengine::scenegraph::instance::ModelInstanceNode* followerNode
  );
  bool SuppressBloom(Field::MyRenderingPipeLine* renderingPipeline);
  void ApplyWorldIsolation(
    Field::MyRenderingPipeLine* renderingPipeline,
    Field::Fieldmap* fieldmap,
    Gen7Follower3gx::PerformanceOptionMask options
  );
#endif
  void Restore();

private:
  static EdgeContainerState ReadState(void* container);
  bool BuildFollowerMaterialList(
    gfl2::renderingengine::scenegraph::instance::ModelInstanceNode* followerNode
  );
  bool IsFollowerMaterial(const EdgeMaterialUserData* userData) const;
  bool ApplyPostProcessStyle(Field::MyRenderingPipeLine* renderingPipeline);
  void RestorePostProcessStyle();
  void RestoreMaterialStates();
#if FOLLOWER_CARRIER_THREEGX
  void RestoreFollowerOutline();
#endif
#if FOLLOWER_3GX_PERFORMANCE_FEATURES
  bool SuppressDrawableContainer(
    void* container,
    unsigned int** usedSizeSlot,
    unsigned int* savedUsedSize
  );
  void RestorePerformanceLite();
  void RestoreBloom();
  void RestoreWorldIsolation();
#endif

  Field::MyRenderingPipeLine* m_pSwappedPipeline;
  EdgeMaterialState m_MaterialStates[EDGE_MATERIAL_STATE_CAPACITY];
  EdgeMaterialUserData* m_pFollowerMaterials[FOLLOWER_MATERIAL_CAPACITY];
  unsigned int m_MaterialStateCount;
  unsigned int m_FollowerMaterialCount;
  bool m_IsApplied;
  bool m_SavedNormalEdgeEnable;
  EdgePostAttributeParam* m_pStyledAttributes;
  EdgePostTextureInfo* m_pStyledTextures[OUTLINE_TEXTURE_SAMPLE_COUNT];
  gfl2::gfx::ColorU8 m_SavedOutlineColor;
  float m_SavedOutlineTranslateU[OUTLINE_TEXTURE_SAMPLE_COUNT];
  float m_SavedOutlineTranslateV[OUTLINE_TEXTURE_SAMPLE_COUNT];
  bool m_IsPostStyleApplied;
#if FOLLOWER_CARRIER_THREEGX
  EdgeMaterialState m_FollowerOutlineStates[FOLLOWER_MATERIAL_CAPACITY];
  unsigned int m_FollowerOutlineStateCount;
#endif
#if FOLLOWER_3GX_PERFORMANCE_FEATURES
  PerformanceMaterialState m_PerformanceMaterialStates[FOLLOWER_MATERIAL_CAPACITY];
  unsigned int m_PerformanceMaterialStateCount;
  Field::MyRenderingPipeLine* m_pBloomPipeline;
  bool m_SavedBloomEnable;
  bool m_IsBloomSuppressed;
  unsigned int* m_pSkyboxUsedSize;
  unsigned int m_SavedSkyboxUsedSize;
  unsigned int* m_pWeatherUsedSize;
  unsigned int m_SavedWeatherUsedSize;
  void** m_pEffectGroupListSlot;
  void* m_pSavedEffectGroupList;
  gfl2::renderingengine::renderer::DrawManager*
    m_pReducedPrecisionDrawManagers[3];
  unsigned int m_ReducedPrecisionDrawManagerCount;
  Field::MyRenderingPipeLine* m_pDofPipeline;
  b32 m_SavedDofEnable;
  bool m_IsDofSuppressed;
  gfl2::renderingengine::scenegraph::instance::DrawableNode*
    m_pCharacterShadowNodes[PERFORMANCE_CHARACTER_SHADOW_CAPACITY];
  b32 m_SavedCharacterShadowVisibility[PERFORMANCE_CHARACTER_SHADOW_CAPACITY];
  unsigned int m_CharacterShadowNodeCount;
#endif
};

FollowerEdgeIsolation::FollowerEdgeIsolation()
: m_pSwappedPipeline(NULL)
, m_MaterialStateCount(0)
, m_FollowerMaterialCount(0)
, m_IsApplied(false)
, m_SavedNormalEdgeEnable(false)
, m_pStyledAttributes(NULL)
, m_IsPostStyleApplied(false)
#if FOLLOWER_CARRIER_THREEGX
, m_FollowerOutlineStateCount(0)
#endif
#if FOLLOWER_3GX_PERFORMANCE_FEATURES
, m_PerformanceMaterialStateCount(0)
, m_pBloomPipeline(NULL)
, m_SavedBloomEnable(false)
, m_IsBloomSuppressed(false)
, m_pSkyboxUsedSize(NULL)
, m_SavedSkyboxUsedSize(0)
, m_pWeatherUsedSize(NULL)
, m_SavedWeatherUsedSize(0)
, m_pEffectGroupListSlot(NULL)
, m_pSavedEffectGroupList(NULL)
, m_ReducedPrecisionDrawManagerCount(0)
, m_pDofPipeline(NULL)
, m_SavedDofEnable(false)
, m_IsDofSuppressed(false)
, m_CharacterShadowNodeCount(0)
#endif
{
  for (unsigned int i = 0; i < OUTLINE_TEXTURE_SAMPLE_COUNT; ++i)
  {
    m_pStyledTextures[i] = NULL;
    m_SavedOutlineTranslateU[i] = 0.0f;
    m_SavedOutlineTranslateV[i] = 0.0f;
  }
#if FOLLOWER_3GX_PERFORMANCE_FEATURES
  for (unsigned int i = 0; i < PERFORMANCE_CHARACTER_SHADOW_CAPACITY; ++i)
  {
    m_pCharacterShadowNodes[i] = NULL;
    m_SavedCharacterShadowVisibility[i] = false;
  }
  for (unsigned int i = 0; i < 3; ++i)
  {
    m_pReducedPrecisionDrawManagers[i] = NULL;
  }
#endif
}

#if FOLLOWER_3GX_PERFORMANCE_FEATURES
bool FollowerEdgeIsolation::ApplyPerformanceLite(
  gfl2::renderingengine::scenegraph::instance::ModelInstanceNode* followerNode
)
{
  typedef gfl2::renderingengine::renderer::MeshDrawTag MeshDrawTag;

  RestorePerformanceLite();
  if (!followerNode)
  {
    return false;
  }

  const unsigned int drawTagCount = followerNode->GetDrawTagNum();
  for (unsigned int drawTagIndex = 0; drawTagIndex < drawTagCount; ++drawTagIndex)
  {
    MeshDrawTag* drawTag = static_cast<MeshDrawTag*>(
      followerNode->GetDrawTag(drawTagIndex)
      );
    if (!drawTag || !drawTag->GetMaterialInstanceNode())
    {
      continue;
    }

    gfl2::renderingengine::scenegraph::instance::MaterialInstanceNode* material =
      drawTag->GetMaterialInstanceNode();
    EdgePostAttributeParam* attributes = material->GetAttributeParam();
    if (!attributes)
    {
      continue;
    }

    bool alreadyAdded = false;
    for (unsigned int i = 0; i < m_PerformanceMaterialStateCount; ++i)
    {
      if (m_PerformanceMaterialStates[i].attributes == attributes)
      {
        alreadyAdded = true;
        break;
      }
    }
    if (alreadyAdded)
    {
      continue;
    }
    if (m_PerformanceMaterialStateCount >= FOLLOWER_MATERIAL_CAPACITY)
    {
      RestorePerformanceLite();
      return false;
    }

    PerformanceMaterialState& state =
      m_PerformanceMaterialStates[m_PerformanceMaterialStateCount++];
    state.attributes = attributes;
    state.userData = const_cast<EdgeMaterialUserData*>(&material->GetUserData());
    state.psLightingEnable = attributes->m_PsLightingEnable;
    state.fogEnable = attributes->m_FogEnable;
    state.bumpMapNo = attributes->m_BumpMapNo;
    state.rimScale = state.userData->m_RimScale;
    state.phongScale = state.userData->m_PhongScale;

    attributes->m_PsLightingEnable = false;
    attributes->m_FogEnable = false;
    attributes->m_BumpMapNo = -1;
    state.userData->m_RimScale = 0.0f;
    state.userData->m_PhongScale = 0.0f;
  }
  return m_PerformanceMaterialStateCount != 0;
}

void FollowerEdgeIsolation::RestorePerformanceLite()
{
  for (unsigned int i = 0; i < m_PerformanceMaterialStateCount; ++i)
  {
    PerformanceMaterialState& state = m_PerformanceMaterialStates[i];
    if (state.attributes)
    {
      state.attributes->m_PsLightingEnable = state.psLightingEnable;
      state.attributes->m_FogEnable = state.fogEnable;
      state.attributes->m_BumpMapNo = state.bumpMapNo;
    }
    if (state.userData)
    {
      state.userData->m_RimScale = state.rimScale;
      state.userData->m_PhongScale = state.phongScale;
    }
    state.attributes = NULL;
    state.userData = NULL;
  }
  m_PerformanceMaterialStateCount = 0;
}

bool FollowerEdgeIsolation::SuppressBloom(
  Field::MyRenderingPipeLine* renderingPipeline
)
{
  RestoreBloom();
  if (!renderingPipeline)
  {
    return false;
  }

  // Bloom is in render-path slot 12. The byte at +0x10 turns it on or off.
  unsigned char& bloomEnable = RawField<unsigned char>(
    renderingPipeline,
    FOLLOWER_CARRIER_PIPELINE_BLOOM_ENABLE_OFFSET
    );
  m_pBloomPipeline = renderingPipeline;
  m_SavedBloomEnable = bloomEnable != 0;
  bloomEnable = 0;
  m_IsBloomSuppressed = true;
  return true;
}

void FollowerEdgeIsolation::RestoreBloom()
{
  if (!m_IsBloomSuppressed)
  {
    return;
  }
  if (m_pBloomPipeline)
  {
    RawField<unsigned char>(
      m_pBloomPipeline,
      FOLLOWER_CARRIER_PIPELINE_BLOOM_ENABLE_OFFSET
      ) = m_SavedBloomEnable ? 1 : 0;
  }
  m_pBloomPipeline = NULL;
  m_SavedBloomEnable = false;
  m_IsBloomSuppressed = false;
}

bool FollowerEdgeIsolation::SuppressDrawableContainer(
  void* container,
  unsigned int** usedSizeSlot,
  unsigned int* savedUsedSize
)
{
  if (!container || !usedSizeSlot || !savedUsedSize)
  {
    return false;
  }

  const EdgeContainerState state = ReadState(container);
  if (state.usedSize == 0 || state.bufferSize == 0 ||
      state.usedSize > state.bufferSize || !state.buffer ||
      !state.linkList || !state.indexer)
  {
    return false;
  }

  *usedSizeSlot = &RawField<unsigned int>(
    container,
    EDGE_CONTAINER_USED_SIZE_OFFSET
    );
  *savedUsedSize = state.usedSize;
  **usedSizeSlot = 0;
  return true;
}

void FollowerEdgeIsolation::ApplyWorldIsolation(
  Field::MyRenderingPipeLine* renderingPipeline,
  Field::Fieldmap* fieldmap,
  Gen7Follower3gx::PerformanceOptionMask options
)
{
  typedef gfl2::renderingengine::scenegraph::instance::DrawableNode DrawableNode;

  RestoreWorldIsolation();
  if (Gen7Follower3gx::HasPerformanceOption(
        options,
        Gen7Follower3gx::PERFORMANCE_OPTION_REDUCED_TRANSFORM_PRECISION
        ) &&
      renderingPipeline)
  {
    typedef gfl2::renderingengine::renderer::DrawManager DrawManager;
    DrawManager* candidates[3];
    void* edgePath = RawField<void*>(
      renderingPipeline,
      FIELD_PIPELINE_EDGE_PATH_OFFSET
      );
    candidates[0] = edgePath ? RawField<DrawManager*>(edgePath, 0x04) : NULL;
    candidates[1] = RawField<DrawManager*>(
      renderingPipeline,
      FIELD_PIPELINE_MAIN_DRAW_MANAGER_OFFSET
      );
    candidates[2] = RawField<DrawManager*>(
      renderingPipeline,
      FOLLOWER_CARRIER_PIPELINE_SKYBOX_DRAW_MANAGER_OFFSET
      );
    for (unsigned int candidateIndex = 0; candidateIndex < 3; ++candidateIndex)
    {
      DrawManager* candidate = candidates[candidateIndex];
      if (!candidate)
      {
        continue;
      }
      bool duplicate = false;
      for (unsigned int savedIndex = 0;
           savedIndex < m_ReducedPrecisionDrawManagerCount;
           ++savedIndex)
      {
        if (m_pReducedPrecisionDrawManagers[savedIndex] == candidate)
        {
          duplicate = true;
          break;
        }
      }
      if (duplicate)
      {
        continue;
      }
      m_pReducedPrecisionDrawManagers[m_ReducedPrecisionDrawManagerCount++] =
        candidate;
      candidate->ViewSpaceRenderEnable(false);
    }
  }

  if (Gen7Follower3gx::HasPerformanceOption(
        options,
        Gen7Follower3gx::PERFORMANCE_OPTION_DISABLE_DEPTH_OF_FIELD
        ) &&
      renderingPipeline)
  {
    b32& dofEnable = RawField<b32>(
      renderingPipeline,
      FOLLOWER_CARRIER_PIPELINE_DOF_ENABLE_OFFSET
      );
    m_pDofPipeline = renderingPipeline;
    m_SavedDofEnable = dofEnable;
    m_IsDofSuppressed = true;
    dofEnable = false;
  }

  if (Gen7Follower3gx::HasPerformanceOption(
        options,
        Gen7Follower3gx::PERFORMANCE_OPTION_DISABLE_WEATHER
        ) &&
      renderingPipeline)
  {
    void* weatherContainer = reinterpret_cast<unsigned char*>(renderingPipeline) +
      FOLLOWER_CARRIER_PIPELINE_WEATHER_CONTAINER_OFFSET;
    SuppressDrawableContainer(
      weatherContainer,
      &m_pWeatherUsedSize,
      &m_SavedWeatherUsedSize
      );
  }

  if (Gen7Follower3gx::HasPerformanceOption(
        options,
        Gen7Follower3gx::PERFORMANCE_OPTION_DISABLE_EFFECTS
        ) &&
      renderingPipeline)
  {
    m_pEffectGroupListSlot = reinterpret_cast<void**>(
      reinterpret_cast<unsigned char*>(renderingPipeline) +
      FOLLOWER_CARRIER_PIPELINE_EFFECT_GROUP_LIST_OFFSET
      );
    m_pSavedEffectGroupList = *m_pEffectGroupListSlot;
    *m_pEffectGroupListSlot = NULL;
  }

  if (Gen7Follower3gx::HasPerformanceOption(
        options,
        Gen7Follower3gx::PERFORMANCE_OPTION_DISABLE_SKYBOX
        ) &&
      renderingPipeline)
  {
    void* skyboxContainer = reinterpret_cast<unsigned char*>(renderingPipeline) +
      FOLLOWER_CARRIER_PIPELINE_SKYBOX_CONTAINER_OFFSET;
    SuppressDrawableContainer(
      skyboxContainer,
      &m_pSkyboxUsedSize,
      &m_SavedSkyboxUsedSize
      );
  }

  if (!Gen7Follower3gx::HasPerformanceOption(
        options,
        Gen7Follower3gx::PERFORMANCE_OPTION_DISABLE_CHARACTER_SHADOWS
        ) ||
      !fieldmap)
  {
    return;
  }

  void* shadowManager = RawField<void*>(
    fieldmap,
    FIELDMAP_MOVE_MODEL_SHADOW_MANAGER_OFFSET
    );
  if (!shadowManager)
  {
    return;
  }

  for (unsigned int slot = 0;
       slot < PERFORMANCE_CHARACTER_SHADOW_CAPACITY;
       ++slot)
  {
    void* shadow = RawField<void*>(
      shadowManager,
      MOVE_MODEL_SHADOW_FIRST_EFFECT_OFFSET +
        slot * MOVE_MODEL_SHADOW_EFFECT_STRIDE
      );
    if (!shadow)
    {
      continue;
    }

    DrawableNode* node = RawField<DrawableNode*>(
      shadow,
      EFFECT_SHADOW_MODEL_NODE_OFFSET
      );
    if (!node)
    {
      continue;
    }

    bool alreadySaved = false;
    for (unsigned int i = 0; i < m_CharacterShadowNodeCount; ++i)
    {
      if (m_pCharacterShadowNodes[i] == node)
      {
        alreadySaved = true;
        break;
      }
    }
    if (alreadySaved ||
        m_CharacterShadowNodeCount >= PERFORMANCE_CHARACTER_SHADOW_CAPACITY)
    {
      continue;
    }

    const unsigned int index = m_CharacterShadowNodeCount++;
    m_pCharacterShadowNodes[index] = node;
    m_SavedCharacterShadowVisibility[index] = node->IsVisible();
    node->SetVisible(false);
  }
}

void FollowerEdgeIsolation::RestoreWorldIsolation()
{
  for (unsigned int i = 0; i < m_ReducedPrecisionDrawManagerCount; ++i)
  {
    if (m_pReducedPrecisionDrawManagers[i])
    {
      m_pReducedPrecisionDrawManagers[i]->ViewSpaceRenderEnable(true);
    }
    m_pReducedPrecisionDrawManagers[i] = NULL;
  }
  m_ReducedPrecisionDrawManagerCount = 0;

  if (m_IsDofSuppressed && m_pDofPipeline)
  {
    RawField<b32>(
      m_pDofPipeline,
      FOLLOWER_CARRIER_PIPELINE_DOF_ENABLE_OFFSET
      ) = m_SavedDofEnable;
  }
  m_pDofPipeline = NULL;
  m_SavedDofEnable = false;
  m_IsDofSuppressed = false;

  if (m_pSkyboxUsedSize)
  {
    *m_pSkyboxUsedSize = m_SavedSkyboxUsedSize;
  }
  m_pSkyboxUsedSize = NULL;
  m_SavedSkyboxUsedSize = 0;

  if (m_pWeatherUsedSize)
  {
    *m_pWeatherUsedSize = m_SavedWeatherUsedSize;
  }
  m_pWeatherUsedSize = NULL;
  m_SavedWeatherUsedSize = 0;

  if (m_pEffectGroupListSlot)
  {
    *m_pEffectGroupListSlot = m_pSavedEffectGroupList;
  }
  m_pEffectGroupListSlot = NULL;
  m_pSavedEffectGroupList = NULL;

  for (unsigned int i = 0; i < m_CharacterShadowNodeCount; ++i)
  {
    if (m_pCharacterShadowNodes[i])
    {
      m_pCharacterShadowNodes[i]->SetVisible(
        m_SavedCharacterShadowVisibility[i]
        );
    }
    m_pCharacterShadowNodes[i] = NULL;
    m_SavedCharacterShadowVisibility[i] = false;
  }
  m_CharacterShadowNodeCount = 0;
}
#endif

EdgeContainerState FollowerEdgeIsolation::ReadState(void* container)
{
  EdgeContainerState state;
  state.buffer = RawField<void*>(container, EDGE_CONTAINER_BUFFER_OFFSET);
  state.linkList = RawField<void*>(container, EDGE_CONTAINER_LINK_LIST_OFFSET);
  state.indexer = RawField<void*>(container, EDGE_CONTAINER_INDEXER_OFFSET);
  state.bufferSize = RawField<unsigned int>(container, EDGE_CONTAINER_BUFFER_SIZE_OFFSET);
  state.emptyTop = RawField<void*>(container, EDGE_CONTAINER_EMPTY_TOP_OFFSET);
  state.dataTop = RawField<void*>(container, EDGE_CONTAINER_DATA_TOP_OFFSET);
  state.usedSize = RawField<unsigned int>(container, EDGE_CONTAINER_USED_SIZE_OFFSET);
  return state;
}

bool FollowerEdgeIsolation::BuildFollowerMaterialList(
  gfl2::renderingengine::scenegraph::instance::ModelInstanceNode* followerNode
)
{
  typedef gfl2::renderingengine::renderer::MeshDrawTag MeshDrawTag;

  m_FollowerMaterialCount = 0;
  const unsigned int drawTagCount = followerNode->GetDrawTagNum();
  for (unsigned int drawTagIndex = 0; drawTagIndex < drawTagCount; ++drawTagIndex)
  {
    MeshDrawTag* drawTag = static_cast<MeshDrawTag*>(
      followerNode->GetDrawTag(drawTagIndex)
      );
    if (!drawTag || !drawTag->GetMaterialInstanceNode())
    {
      continue;
    }

    EdgeMaterialUserData* userData = const_cast<EdgeMaterialUserData*>(
      &drawTag->GetMaterialInstanceNode()->GetUserData()
      );
    bool alreadyAdded = false;
    for (unsigned int i = 0; i < m_FollowerMaterialCount; ++i)
    {
      if (m_pFollowerMaterials[i] == userData)
      {
        alreadyAdded = true;
        break;
      }
    }
    if (alreadyAdded)
    {
      continue;
    }
    if (m_FollowerMaterialCount >= FOLLOWER_MATERIAL_CAPACITY)
    {
      m_FollowerMaterialCount = 0;
      return false;
    }
    m_pFollowerMaterials[m_FollowerMaterialCount++] = userData;
  }
  return m_FollowerMaterialCount != 0;
}

bool FollowerEdgeIsolation::IsFollowerMaterial(
  const EdgeMaterialUserData* userData
) const
{
  for (unsigned int i = 0; i < m_FollowerMaterialCount; ++i)
  {
    if (m_pFollowerMaterials[i] == userData)
    {
      return true;
    }
  }
  return false;
}

#if FOLLOWER_CARRIER_THREEGX
bool FollowerEdgeIsolation::SuppressFollowerOutline(
  Field::MyRenderingPipeLine* renderingPipeline,
  gfl2::renderingengine::scenegraph::instance::ModelInstanceNode* followerNode
)
{
  Restore();
  FOLLOWER_PERF_SCOPE(
    outlineSuppressPerformance,
    Gen7Follower3gx::PERFORMANCE_ZONE_OUTLINE
    );
  if (!renderingPipeline)
  {
    return false;
  }

  // Reset the existing normal-edge state. Leave ride trails and encounter shadows alone.
  m_pSwappedPipeline = renderingPipeline;
  m_SavedNormalEdgeEnable =
    FOLLOWER_POKEMON_GET_EDGE_NORMAL_MAP_ENABLE(renderingPipeline);
  FOLLOWER_POKEMON_SET_EDGE_NORMAL_MAP_ENABLE(renderingPipeline, false);
  m_IsApplied = true;

  if (!followerNode || !BuildFollowerMaterialList(followerNode))
  {
    return true;
  }

  m_FollowerOutlineStateCount = m_FollowerMaterialCount;
  for (unsigned int i = 0; i < m_FollowerOutlineStateCount; ++i)
  {
    EdgeMaterialState& state = m_FollowerOutlineStates[i];
    state.userData = m_pFollowerMaterials[i];
    state.edgeType = state.userData->m_EdgeType;
    state.userData->m_EdgeType =
      gfl2::renderingengine::scenegraph::resource::EdgeType::None;
  }
  return m_FollowerOutlineStateCount != 0;
}

void FollowerEdgeIsolation::RestoreFollowerOutline()
{
  for (unsigned int i = 0; i < m_FollowerOutlineStateCount; ++i)
  {
    EdgeMaterialState& state = m_FollowerOutlineStates[i];
    if (state.userData)
    {
      state.userData->m_EdgeType = state.edgeType;
      state.userData = NULL;
    }
  }
  m_FollowerOutlineStateCount = 0;
  m_FollowerMaterialCount = 0;
}
#endif

bool FollowerEdgeIsolation::ApplyPostProcessStyle(
  Field::MyRenderingPipeLine* renderingPipeline
)
{
  typedef gfl2::renderingengine::scenegraph::instance::MaterialInstanceNode
    MaterialInstanceNode;
  typedef gfl2::renderingengine::scenegraph::instance::ModelInstanceNode
    ModelInstanceNode;

  void* outlinePath = RawField<void*>(
    renderingPipeline,
    FIELD_PIPELINE_OUTLINE_PATH_OFFSET
    );
  if (!outlinePath)
  {
    return false;
  }

  ModelInstanceNode* normalOutlineModel = RawField<ModelInstanceNode*>(
    outlinePath,
    OUTLINE_PATH_NORMAL_MODEL_OFFSET
    );
  if (!normalOutlineModel || normalOutlineModel->GetMaterialNum() == 0)
  {
    return false;
  }

  MaterialInstanceNode* outlineMaterial =
    normalOutlineModel->GetMaterialInstanceNode(0);
  if (!outlineMaterial)
  {
    return false;
  }

  EdgePostAttributeParam* attributes = outlineMaterial->GetAttributeParam();
  if (!attributes)
  {
    return false;
  }

  for (unsigned int i = 0; i < OUTLINE_TEXTURE_SAMPLE_COUNT; ++i)
  {
    m_pStyledTextures[i] = outlineMaterial->GetTextureInfo(i);
    if (!m_pStyledTextures[i])
    {
      for (unsigned int resetIndex = 0;
           resetIndex < OUTLINE_TEXTURE_SAMPLE_COUNT;
           ++resetIndex)
      {
        m_pStyledTextures[resetIndex] = NULL;
      }
      return false;
    }
  }

  m_pStyledAttributes = attributes;
  m_SavedOutlineColor = attributes->m_ConstantColor[1];
  attributes->m_ConstantColor[1].r = static_cast<unsigned char>(
    (m_SavedOutlineColor.r + FOLLOWER_OUTLINE_COLOR_LIFT > 0xff)
      ? 0xff
      : m_SavedOutlineColor.r + FOLLOWER_OUTLINE_COLOR_LIFT
    );
  attributes->m_ConstantColor[1].g = static_cast<unsigned char>(
    (m_SavedOutlineColor.g + FOLLOWER_OUTLINE_COLOR_LIFT > 0xff)
      ? 0xff
      : m_SavedOutlineColor.g + FOLLOWER_OUTLINE_COLOR_LIFT
    );
  attributes->m_ConstantColor[1].b = static_cast<unsigned char>(
    (m_SavedOutlineColor.b + FOLLOWER_OUTLINE_COLOR_LIFT > 0xff)
      ? 0xff
      : m_SavedOutlineColor.b + FOLLOWER_OUTLINE_COLOR_LIFT
    );

  for (unsigned int i = 0; i < OUTLINE_TEXTURE_SAMPLE_COUNT; ++i)
  {
    EdgePostTextureInfo::Attribute& textureAttributes =
      m_pStyledTextures[i]->m_Attribute;
    m_SavedOutlineTranslateU[i] = textureAttributes.m_TranslateU;
    m_SavedOutlineTranslateV[i] = textureAttributes.m_TranslateV;
    textureAttributes.m_TranslateU *= FOLLOWER_OUTLINE_SAMPLE_SCALE;
    textureAttributes.m_TranslateV *= FOLLOWER_OUTLINE_SAMPLE_SCALE;
  }

  m_IsPostStyleApplied = true;
  return true;
}

void FollowerEdgeIsolation::RestorePostProcessStyle()
{
  if (!m_IsPostStyleApplied)
  {
    return;
  }

  if (m_pStyledAttributes)
  {
    m_pStyledAttributes->m_ConstantColor[1] = m_SavedOutlineColor;
  }
  for (unsigned int i = 0; i < OUTLINE_TEXTURE_SAMPLE_COUNT; ++i)
  {
    if (m_pStyledTextures[i])
    {
      m_pStyledTextures[i]->m_Attribute.m_TranslateU =
        m_SavedOutlineTranslateU[i];
      m_pStyledTextures[i]->m_Attribute.m_TranslateV =
        m_SavedOutlineTranslateV[i];
    }
    m_pStyledTextures[i] = NULL;
  }

  m_pStyledAttributes = NULL;
  m_IsPostStyleApplied = false;
}

void FollowerEdgeIsolation::RestoreMaterialStates()
{
  for (unsigned int i = 0; i < m_MaterialStateCount; ++i)
  {
    if (m_MaterialStates[i].userData)
    {
      m_MaterialStates[i].userData->m_EdgeType = m_MaterialStates[i].edgeType;
    }
  }
  m_MaterialStateCount = 0;
  m_FollowerMaterialCount = 0;
}

bool FollowerEdgeIsolation::Apply(
  Field::MyRenderingPipeLine* renderingPipeline,
  gfl2::renderingengine::scenegraph::instance::ModelInstanceNode* followerNode
#if FOLLOWER_3GX_PERFORMANCE_FEATURES
  , bool suppressOccluderDraws
  , bool suppressFollowerDraws
#endif
)
{
  Restore();
  FOLLOWER_PERF_SCOPE(
    outlineApplyPerformance,
    Gen7Follower3gx::PERFORMANCE_ZONE_OUTLINE
    );
  if (!renderingPipeline || !followerNode)
  {
    return false;
  }

#if FOLLOWER_3GX_PERFORMANCE_FEATURES
  const bool normalEdgeWasEnabled =
    FOLLOWER_POKEMON_GET_EDGE_NORMAL_MAP_ENABLE(renderingPipeline);
  if (normalEdgeWasEnabled && !suppressOccluderDraws)
#else
  if (FOLLOWER_POKEMON_GET_EDGE_NORMAL_MAP_ENABLE(renderingPipeline))
#endif
  {
    return false;
  }

  void* edgePath = RawField<void*>(
    renderingPipeline,
    FIELD_PIPELINE_EDGE_PATH_OFFSET
    );
  if (!edgePath)
  {
    return false;
  }

  void* container = reinterpret_cast<unsigned char*>(edgePath) +
    EDGE_PATH_DRAWABLE_CONTAINER_OFFSET;
  const EdgeContainerState stockState = ReadState(container);
  if (!stockState.buffer || !stockState.linkList || !stockState.indexer ||
      stockState.bufferSize == 0 || stockState.usedSize > stockState.bufferSize)
  {
    return false;
  }

  if (!BuildFollowerMaterialList(followerNode))
  {
    return false;
  }

#if FOLLOWER_3GX_PERFORMANCE_FEATURES
  if (suppressFollowerDraws)
  {
    m_FollowerOutlineStateCount = m_FollowerMaterialCount;
    for (unsigned int i = 0; i < m_FollowerOutlineStateCount; ++i)
    {
      EdgeMaterialState& state = m_FollowerOutlineStates[i];
      state.userData = m_pFollowerMaterials[i];
      state.edgeType = state.userData->m_EdgeType;
      state.userData->m_EdgeType =
        gfl2::renderingengine::scenegraph::resource::EdgeType::None;
    }
  }
#endif

  typedef gfl2::renderingengine::scenegraph::instance::DrawableNode DrawableNode;
  typedef gfl2::renderingengine::renderer::MeshDrawTag MeshDrawTag;
  EdgeLinkData** indexer = reinterpret_cast<EdgeLinkData**>(stockState.indexer);
  m_MaterialStateCount = 0;

  for (unsigned int nodeIndex = 0; nodeIndex < stockState.usedSize; ++nodeIndex)
  {
    EdgeLinkData* linkData = indexer[nodeIndex];
    DrawableNode* drawableNode = NULL;
    if (linkData && linkData->data)
    {
      drawableNode = reinterpret_cast<DrawableNode*>(*linkData->data);
    }
    if (!drawableNode || drawableNode == followerNode)
    {
      continue;
    }

    const unsigned int drawTagCount = drawableNode->GetDrawTagNum();
    for (unsigned int drawTagIndex = 0; drawTagIndex < drawTagCount; ++drawTagIndex)
    {
      MeshDrawTag* drawTag = static_cast<MeshDrawTag*>(
        drawableNode->GetDrawTag(drawTagIndex)
        );
      if (!drawTag || !drawTag->GetMaterialInstanceNode())
      {
        continue;
      }

      EdgeMaterialUserData* userData = const_cast<EdgeMaterialUserData*>(
        &drawTag->GetMaterialInstanceNode()->GetUserData()
        );
      if (IsFollowerMaterial(userData) ||
          userData->m_EdgeType ==
            gfl2::renderingengine::scenegraph::resource::EdgeType::None
#if FOLLOWER_3GX_PERFORMANCE_FEATURES
          || (!suppressOccluderDraws &&
              userData->m_EdgeType ==
                gfl2::renderingengine::scenegraph::resource::EdgeType::Erase)
#else
          || userData->m_EdgeType ==
            gfl2::renderingengine::scenegraph::resource::EdgeType::Erase
#endif
          )
      {
        continue;
      }
      if (m_MaterialStateCount >= EDGE_MATERIAL_STATE_CAPACITY)
      {
        RestoreMaterialStates();
        return false;
      }

      EdgeMaterialState& materialState =
        m_MaterialStates[m_MaterialStateCount++];
      materialState.userData = userData;
      materialState.edgeType = userData->m_EdgeType;
#if FOLLOWER_3GX_PERFORMANCE_FEATURES
      userData->m_EdgeType = suppressOccluderDraws
        ? gfl2::renderingengine::scenegraph::resource::EdgeType::None
        : gfl2::renderingengine::scenegraph::resource::EdgeType::Erase;
#else
      userData->m_EdgeType =
        gfl2::renderingengine::scenegraph::resource::EdgeType::Erase;
#endif
    }
  }

#if FOLLOWER_3GX_PERFORMANCE_FEATURES
  if (!normalEdgeWasEnabled)
  {
    ApplyPostProcessStyle(renderingPipeline);
  }
  m_SavedNormalEdgeEnable = normalEdgeWasEnabled;
#else
  ApplyPostProcessStyle(renderingPipeline);
  m_SavedNormalEdgeEnable = false;
#endif
  m_pSwappedPipeline = renderingPipeline;
  FOLLOWER_POKEMON_SET_EDGE_NORMAL_MAP_ENABLE(renderingPipeline, true);
  m_IsApplied = true;
  return true;
}

void FollowerEdgeIsolation::Restore()
{
  FOLLOWER_PERF_SCOPE(
    outlineRestorePerformance,
    Gen7Follower3gx::PERFORMANCE_ZONE_OUTLINE
  );
#if FOLLOWER_3GX_PERFORMANCE_FEATURES
  RestoreWorldIsolation();
  RestoreBloom();
  RestorePerformanceLite();
#endif
  RestorePostProcessStyle();
#if FOLLOWER_CARRIER_THREEGX
  RestoreFollowerOutline();
#endif
  if (!m_IsApplied)
  {
    return;
  }

  RestoreMaterialStates();
  if (m_pSwappedPipeline)
  {
    FOLLOWER_POKEMON_SET_EDGE_NORMAL_MAP_ENABLE(
      m_pSwappedPipeline,
      m_SavedNormalEdgeEnable
      );
  }

  m_pSwappedPipeline = NULL;
  m_IsApplied = false;
}

class SharedHeapBootGateEvent : public GameSys::GameEvent
{
public:
  SharedHeapBootGateEvent()
  : GameSys::GameEvent(NULL)
  {
  }

  virtual ~SharedHeapBootGateEvent()
  {
  }

  virtual bool BootChk(GameSys::GameManager*)
  {
    return false;
  }

  virtual void InitFunc(GameSys::GameManager*)
  {
  }

  virtual GameSys::GMEVENT_RESULT MainFunc(GameSys::GameManager*)
  {
    return GameSys::GMEVENT_RES_CONTINUE;
  }

  virtual void EndFunc(GameSys::GameManager*)
  {
  }
};

class CutsceneBlackoutFade : public gfl2::Fade::FadeSuper
{
public:
  CutsceneBlackoutFade()
  : m_IsActive(false)
  {
  }

  void SetActive(bool active)
  {
    m_IsActive = active;
  }

  virtual void UpdateFunc()
  {
  }

  virtual void DrawFunc(gfl2::gfx::CtrDisplayNo)
  {
    gfl2::gfx::GFGL::ClearRenderTargetDepthStencil(
      gfl2::gfx::Color(0.0f, 0.0f, 0.0f, 1.0f),
      1.0f,
      0xff
      );
  }

  virtual void RequestOut(
    gfl2::Fade::FADE_TYPE,
    const gfl2::math::Vector4*,
    const gfl2::math::Vector4*,
    u32,
    bool
  )
  {
    m_IsActive = true;
  }

  virtual void RequestIn(gfl2::Fade::FADE_TYPE, u32)
  {
    m_IsActive = false;
  }

  virtual void ForceOut(const gfl2::math::Vector4*)
  {
    m_IsActive = true;
  }

  virtual bool IsEnd()
  {
    return !m_IsActive;
  }

  virtual bool IsEndStatus()
  {
    return !m_IsActive;
  }

  virtual void SetPause(bool)
  {
  }

  virtual bool IsPause() const
  {
    return false;
  }

  virtual void Reset()
  {
    m_IsActive = false;
  }

private:
  bool m_IsActive;
};

#if FOLLOWER_CARRIER_THREEGX
unsigned char g_FollowerManagerStorage[
  sizeof(Field::FollowerRuntime::Manager)
] __attribute__((aligned(__alignof__(Field::FollowerRuntime::Manager))));
unsigned char g_FollowerSceneIntegrationStorage[
  sizeof(FollowerSceneIntegration)
] __attribute__((aligned(__alignof__(FollowerSceneIntegration))));
unsigned char g_RemoteFollowerSceneIntegrationStorage
  [Field::FollowerRuntime::FOLLOWER_REMOTE_REPLICA_MAX]
  [sizeof(FollowerSceneIntegration)]
  __attribute__((aligned(__alignof__(FollowerSceneIntegration))));
unsigned char g_FollowerEdgeIsolationStorage[
  sizeof(FollowerEdgeIsolation)
] __attribute__((aligned(__alignof__(FollowerEdgeIsolation))));
unsigned char g_SharedHeapBootGateEventStorage[
  sizeof(SharedHeapBootGateEvent)
] __attribute__((aligned(__alignof__(SharedHeapBootGateEvent))));
unsigned char g_CutsceneBlackoutFadeStorage[
  sizeof(CutsceneBlackoutFade)
] __attribute__((aligned(__alignof__(CutsceneBlackoutFade))));

Field::FollowerRuntime::Manager* g_pFollowerManager = NULL;
FollowerSceneIntegration* g_pFollowerSceneIntegration = NULL;
FollowerSceneIntegration* g_pRemoteFollowerSceneIntegrations
  [Field::FollowerRuntime::FOLLOWER_REMOTE_REPLICA_MAX] = {};
FollowerEdgeIsolation* g_pFollowerEdgeIsolation = NULL;
SharedHeapBootGateEvent* g_pSharedHeapBootGateEvent = NULL;
CutsceneBlackoutFade* g_pCutsceneBlackoutFade = NULL;
bool g_FollowerCarrierRuntimeInitialized = false;

#define g_FollowerManager (*g_pFollowerManager)
#define g_FollowerSceneIntegration (*g_pFollowerSceneIntegration)
#define g_FollowerEdgeIsolation (*g_pFollowerEdgeIsolation)
#define g_SharedHeapBootGateEvent (*g_pSharedHeapBootGateEvent)
#define g_CutsceneBlackoutFade (*g_pCutsceneBlackoutFade)
#else
Field::FollowerRuntime::Manager g_FollowerManager;
FollowerSceneIntegration g_FollowerSceneIntegration;
FollowerEdgeIsolation g_FollowerEdgeIsolation;
SharedHeapBootGateEvent g_SharedHeapBootGateEvent;
CutsceneBlackoutFade g_CutsceneBlackoutFade;
#endif
gfl2::Fade::FadeSuper* g_pPreviousCustomUpperFade = NULL;
gfl2::Fade::FadeSuper* g_pPreviousCustomLowerFade = NULL;
bool g_CutsceneBlackoutInstalled = false;
bool g_WaterRideSuspendRequested = false;
bool g_WaterRideObserved = false;
unsigned int g_WaterRideGraceFrames = 0;
bool g_CutsceneFastForwardActive = false;
GameSys::GameEventManager* g_pPendingSharedHeapEventManager = NULL;
GameSys::GameEvent* g_pPendingSharedHeapEvent = NULL;
void* g_pPendingSharedHeapEventVtable = NULL;

#if FOLLOWER_CARRIER_THREEGX
void ResetRemoteFollowerSceneIntegrations()
{
  for (unsigned int index = 0;
       index < Field::FollowerRuntime::FOLLOWER_REMOTE_REPLICA_MAX;
       ++index)
  {
    if (g_pRemoteFollowerSceneIntegrations[index])
    {
      g_pRemoteFollowerSceneIntegrations[index]->Reset();
    }
  }
}
#endif

#if FOLLOWER_CARRIER_THREEGX && FOLLOWER_3GX_DIAGNOSTIC
enum { FOLLOWER_DIAGNOSTIC_MAX_JOINTS = 512 };
gfl2::renderingengine::scenegraph::instance::ModelInstanceNode*
  g_pDiagnosticTopologyNode = NULL;

void ResetFollowerSceneGraphDiagnostics()
{
  g_pDiagnosticTopologyNode = NULL;
  Gen7Follower3gx::SetFollowerSceneGraphDiagnostics(0, 0, 0);
}

void RefreshFollowerSceneGraphDiagnostics(
  gfl2::renderingengine::scenegraph::instance::ModelInstanceNode* node
)
{
  using gfl2::renderingengine::scenegraph::DagNode;
  using gfl2::renderingengine::scenegraph::instance::JointInstanceNode;

  if (node == g_pDiagnosticTopologyNode)
  {
    return;
  }
  g_pDiagnosticTopologyNode = node;
  if (!node)
  {
    Gen7Follower3gx::SetFollowerSceneGraphDiagnostics(0, 0, 0);
    return;
  }

  const unsigned int totalJoints = node->GetJointNum();
  const unsigned int maximumInspectedJoints =
    static_cast<unsigned int>(FOLLOWER_DIAGNOSTIC_MAX_JOINTS);
  const unsigned int inspectedJoints =
    totalJoints < maximumInspectedJoints
      ? totalJoints
      : maximumInspectedJoints;
  JointInstanceNode* joints[FOLLOWER_DIAGNOSTIC_MAX_JOINTS];
  bool required[FOLLOWER_DIAGNOSTIC_MAX_JOINTS] = {};
  unsigned int renderJoints = 0;

  for (unsigned int i = 0; i < inspectedJoints; ++i)
  {
    joints[i] = node->GetJointInstanceNode(i);
    if (joints[i] && joints[i]->IsNeedRendering())
    {
      ++renderJoints;
    }
  }

  const DagNode* const modelNode = reinterpret_cast<const DagNode*>(node);
  for (unsigned int i = 0; i < inspectedJoints; ++i)
  {
    if (!joints[i] || !joints[i]->IsNeedRendering())
    {
      continue;
    }

    const DagNode* ancestor = reinterpret_cast<const DagNode*>(joints[i]);
    for (unsigned int depth = 0;
         ancestor && ancestor != modelNode && depth <= inspectedJoints;
         ++depth)
    {
      for (unsigned int candidate = 0;
           candidate < inspectedJoints;
           ++candidate)
      {
        if (ancestor == reinterpret_cast<const DagNode*>(joints[candidate]))
        {
          required[candidate] = true;
          break;
        }
      }
      ancestor = ancestor->GetParent();
    }
  }

  unsigned int requiredJoints = 0;
  for (unsigned int i = 0; i < inspectedJoints; ++i)
  {
    if (required[i])
    {
      ++requiredJoints;
    }
  }
  if (totalJoints > inspectedJoints)
  {
    renderJoints = totalJoints;
    requiredJoints = totalJoints;
  }

  Gen7Follower3gx::SetFollowerSceneGraphDiagnostics(
    totalJoints,
    renderJoints,
    requiredJoints
    );
}
#endif

#if FOLLOWER_CARRIER_THREEGX && FOLLOWER_3GX_PERFORMANCE_FEATURES
void TraverseFollowerForPerformance(
  gfl2::renderingengine::scenegraph::instance::ModelInstanceNode* node
)
{
#if FOLLOWER_3GX_DIAGNOSTIC
  RefreshFollowerSceneGraphDiagnostics(node);
#endif
  if (!node)
  {
    return;
  }

  FOLLOWER_PERF_SCOPE(
    followerTraversalPerformance,
    Gen7Follower3gx::PERFORMANCE_ZONE_TRAVERSAL
    );
  gfl2::renderingengine::scenegraph::SceneGraphManager::TraverseModelFast(
    reinterpret_cast<gfl2::renderingengine::scenegraph::DagNode*>(node)
    );
}
#endif

bool IsRetailFieldScriptRunning(
  Field::FieldScript::FieldScriptSystem* scriptSystem
)
{
  return scriptSystem &&
    RawField<void*>(scriptSystem, FIELD_SCRIPT_NOW_OBJECT_OFFSET) != NULL;
}

gfl2::Fade::FadeSuper*& GetCustomFadeSlot(
  gfl2::Fade::FadeManager* fadeManager,
  gfl2::Fade::DISP display
)
{
  return reinterpret_cast<gfl2::Fade::FadeSuper**>(fadeManager)[display];
}

void EnableCutsceneBlackout()
{
  gfl2::Fade::FadeManager* fadeManager =
    GFL_SINGLETON_INSTANCE(gfl2::Fade::FadeManager);
  if (!fadeManager)
  {
    return;
  }

  gfl2::Fade::FadeSuper*& upper = GetCustomFadeSlot(
    fadeManager,
    gfl2::Fade::DISP_CUSTOM_UPPER
    );
  gfl2::Fade::FadeSuper*& lower = GetCustomFadeSlot(
    fadeManager,
    gfl2::Fade::DISP_CUSTOM_LOWER
    );
  if (!g_CutsceneBlackoutInstalled || upper != &g_CutsceneBlackoutFade)
  {
    g_pPreviousCustomUpperFade = upper;
  }
  if (!g_CutsceneBlackoutInstalled || lower != &g_CutsceneBlackoutFade)
  {
    g_pPreviousCustomLowerFade = lower;
  }

  g_CutsceneBlackoutFade.SetActive(true);
  upper = &g_CutsceneBlackoutFade;
  lower = &g_CutsceneBlackoutFade;
  g_CutsceneBlackoutInstalled = true;
}

void DisableCutsceneBlackout()
{
  if (!g_CutsceneBlackoutInstalled)
  {
    return;
  }

  gfl2::Fade::FadeManager* fadeManager =
    GFL_SINGLETON_INSTANCE(gfl2::Fade::FadeManager);
  if (fadeManager)
  {
    gfl2::Fade::FadeSuper*& upper = GetCustomFadeSlot(
      fadeManager,
      gfl2::Fade::DISP_CUSTOM_UPPER
      );
    gfl2::Fade::FadeSuper*& lower = GetCustomFadeSlot(
      fadeManager,
      gfl2::Fade::DISP_CUSTOM_LOWER
      );
    if (upper == &g_CutsceneBlackoutFade)
    {
      upper = g_pPreviousCustomUpperFade;
    }
    if (lower == &g_CutsceneBlackoutFade)
    {
      lower = g_pPreviousCustomLowerFade;
    }
  }

  g_CutsceneBlackoutFade.SetActive(false);
  g_pPreviousCustomUpperFade = NULL;
  g_pPreviousCustomLowerFade = NULL;
  g_CutsceneBlackoutInstalled = false;
}

bool IsRetailKeyWaitFunction(const void* waitFunction)
{
  if (!waitFunction)
  {
    return false;
  }

  const unsigned int* code =
    reinterpret_cast<const unsigned int*>(waitFunction);
  return code[0] == 0xe92d41f0 &&
    code[1] == 0xe1a04000 &&
    code[2] == 0xe1a05001 &&
    code[3] == 0xe590601c &&
    code[4] == 0xe1a00006 &&
    code[6] == 0xe3a01000 &&
    code[8] == 0xe1a08000 &&
    code[9] == 0xe1a00006 &&
    code[11] == 0xe3a01000 &&
    code[13] == 0xe1a07000 &&
    code[14] == 0xe1a00006 &&
    code[16] == 0xe3a01000 &&
    code[18] == 0xe3550000 &&
    code[19] == 0xe1a06000 &&
    code[20] == 0xe3a02000;
}

void CompleteRetailKeyWait(
  Field::FieldScript::FieldScriptSystem* scriptSystem
)
{
  void* regularObject = scriptSystem
    ? RawField<void*>(scriptSystem, FIELD_SCRIPT_NOW_OBJECT_OFFSET)
    : NULL;
  if (!regularObject)
  {
    return;
  }

  void* waitFunction = RawField<void*>(
    regularObject,
    REGULAR_OBJECT_WAIT_FUNC_OFFSET
    );
  if (!IsRetailKeyWaitFunction(waitFunction))
  {
    return;
  }

  // Clear the key wait without passing A to the next prompt. This may differ from the game's full transition.
  RawField<unsigned short>(regularObject, PAWN_BASE_HALT_FLAG_OFFSET) = 0;
  RawField<void*>(regularObject, REGULAR_OBJECT_WAIT_FUNC_OFFSET) = NULL;
  RawField<unsigned int>(regularObject, REGULAR_OBJECT_WAIT_LABEL_OFFSET) = 0;
  RawField<void*>(regularObject, REGULAR_OBJECT_WAIT_WORK_OFFSET) = NULL;
}

void CompleteRetailFieldTalkWindow(
  Field::FieldWindow::FieldTalkWindow* fieldTalkWindow
)
{
  if (!fieldTalkWindow)
  {
    return;
  }

  void* talkWindow = RawField<void*>(
    fieldTalkWindow,
    FIELD_TALK_WINDOW_INNER_OFFSET
    );
  if (!talkWindow)
  {
    return;
  }

  App::Tool::TalkWindow* retailTalkWindow =
    reinterpret_cast<App::Tool::TalkWindow*>(talkWindow);
  app::util::G2DUtil* g2dUtil = retailTalkWindow->GetG2DUtil();
  print::MessageWindow* messageWindow = g2dUtil
    ? g2dUtil->GetMsgWin()
    : NULL;
  if (messageWindow)
  {
    // Finish the dialogue through its normal update so the script can continue.
    RawField<unsigned int>(
      messageWindow,
      MESSAGE_WINDOW_FINISH_TYPE_OFFSET
      ) = STRWIN_FINISH_NONE;
    RawField<unsigned char>(
      messageWindow,
      MESSAGE_WINDOW_SEQUENCE_OFFSET
      ) = STRWIN_SEQUENCE_DONE;
  }
}

void CompleteAllRetailFieldTalkWindows(
  Field::FieldScript::SystemSingletones* singletons
)
{
  if (!singletons)
  {
    return;
  }

  for (unsigned int i = 0; i < FIELDTALKWINDOW_MAX; ++i)
  {
    CompleteRetailFieldTalkWindow(singletons->m_pFieldTalkWindow[i]);
  }
}

void StopCutsceneFastForward(
  Field::FieldScript::SystemSingletones*
)
{
  DisableCutsceneBlackout();
  g_CutsceneFastForwardActive = false;
}

void StopCutsceneFastForward(Field::Fieldmap* fieldmap)
{
  GameSys::GameManager* gameManager =
    fieldmap ? fieldmap->GetGameManager() : NULL;
  Field::FieldScript::FieldScriptSystem* scriptSystem = gameManager
    ? *reinterpret_cast<Field::FieldScript::FieldScriptSystem**>(
        reinterpret_cast<unsigned char*>(gameManager) +
        FOLLOWER_CARRIER_GAME_MANAGER_FIELD_SCRIPT_SYSTEM_OFFSET
        )
    : NULL;
  Field::FieldScript::SystemWork* systemWork = scriptSystem
    ? scriptSystem->GetSystemWork()
    : NULL;
  StopCutsceneFastForward(
    systemWork ? systemWork->GetSingletones() : NULL
    );
}

unsigned int UpdateCutsceneFastForward(Field::Fieldmap* fieldmap)
{
  GameSys::GameManager* gameManager =
    fieldmap ? fieldmap->GetGameManager() : NULL;
  Field::FieldScript::FieldScriptSystem* scriptSystem = gameManager
    ? *reinterpret_cast<Field::FieldScript::FieldScriptSystem**>(
        reinterpret_cast<unsigned char*>(gameManager) +
        FOLLOWER_CARRIER_GAME_MANAGER_FIELD_SCRIPT_SYSTEM_OFFSET
        )
    : NULL;
  Field::FieldScript::SystemWork* systemWork = scriptSystem
    ? scriptSystem->GetSystemWork()
    : NULL;
  Field::FieldScript::SystemSingletones* singletons = systemWork
    ? systemWork->GetSingletones()
    : NULL;
  if (!IsRetailFieldScriptRunning(scriptSystem))
  {
    StopCutsceneFastForward(singletons);
    return 0;
  }

  gfl2::ui::DeviceManager* deviceManager =
    gameManager->GetUiDeviceManager();
  gfl2::ui::Button* button = deviceManager
    ? deviceManager->GetButton(gfl2::ui::DeviceManager::BUTTON_STANDARD)
    : NULL;
  if (!button)
  {
    StopCutsceneFastForward(singletons);
    return 0;
  }

  if (button->IsTrigger(gfl2::ui::BUTTON_START))
  {
    g_CutsceneFastForwardActive = true;
  }
  if (!g_CutsceneFastForwardActive)
  {
    return 0;
  }

  if (!singletons)
  {
    StopCutsceneFastForward(singletons);
    return 0;
  }

  // Let the player answer choices, then resume the blackout.
  if (singletons->m_pYesNoWin ||
      singletons->m_pListMenu ||
      singletons->m_pProc)
  {
    DisableCutsceneBlackout();
    return 0;
  }
  const unsigned int decisionButtons =
    gfl2::ui::BUTTON_A |
    gfl2::ui::BUTTON_B |
    gfl2::ui::BUTTON_X |
    gfl2::ui::BUTTON_Y |
    gfl2::ui::BUTTON_L |
    gfl2::ui::BUTTON_R |
    gfl2::ui::BUTTON_ZL |
    gfl2::ui::BUTTON_ZR |
    gfl2::ui::BUTTON_SELECT |
    gfl2::ui::BUTTON_CROSS;
  if (button->IsTrigger(decisionButtons))
  {
    DisableCutsceneBlackout();
    return 0;
  }

  CompleteRetailKeyWait(scriptSystem);
  CompleteAllRetailFieldTalkWindows(singletons);
  EnableCutsceneBlackout();

  return CUTSCENE_FAST_FORWARD_EXTRA_UPDATES
    << CUTSCENE_FAST_FORWARD_COMMAND_SHIFT;
}

enum SharedHeapEventGateResult
{
  SHARED_HEAP_EVENT_GATE_NONE,
  SHARED_HEAP_EVENT_GATE_CAPTURED,
  SHARED_HEAP_EVENT_GATE_WAITING,
  SHARED_HEAP_EVENT_GATE_RESTORED,
  SHARED_HEAP_EVENT_GATE_CONFLICT,
};

void* GetGameEventVtable(GameSys::GameEvent* event)
{
  return event ? *reinterpret_cast<void**>(event) : NULL;
}

void SetGameEventVtable(GameSys::GameEvent* event, void* vtable)
{
  if (event)
  {
    *reinterpret_cast<void**>(event) = vtable;
  }
}

void RestorePendingSharedHeapEventVtable()
{
  if (g_pPendingSharedHeapEvent && g_pPendingSharedHeapEventVtable)
  {
    SetGameEventVtable(
      g_pPendingSharedHeapEvent,
      g_pPendingSharedHeapEventVtable
      );
  }
  g_pPendingSharedHeapEventManager = NULL;
  g_pPendingSharedHeapEvent = NULL;
  g_pPendingSharedHeapEventVtable = NULL;
}

SharedHeapEventGateResult UpdateSharedHeapEventGate(
  Field::Fieldmap* fieldmap
)
{
  GameSys::GameManager* gameManager =
    fieldmap ? fieldmap->GetGameManager() : NULL;
  GameSys::GameEventManager* eventManager =
    gameManager
      ? FOLLOWER_POKEMON_GET_GAME_EVENT_MANAGER(gameManager)
      : NULL;

  if (g_pPendingSharedHeapEvent)
  {
    if (!eventManager ||
        eventManager != g_pPendingSharedHeapEventManager ||
        eventManager->GetGameEvent() != g_pPendingSharedHeapEvent)
    {
      RestorePendingSharedHeapEventVtable();
      return SHARED_HEAP_EVENT_GATE_CONFLICT;
    }

    g_FollowerEdgeIsolation.Restore();
    g_FollowerSceneIntegration.Reset();
#if FOLLOWER_CARRIER_THREEGX
    ResetRemoteFollowerSceneIntegrations();
#endif
#if FOLLOWER_POKEMON_ENABLE_BALL_TRANSITION
    g_FollowerManager.AdvanceBallTransitionWhileFieldPaused();
#endif
    if (!g_FollowerManager.Terminate())
    {
      return SHARED_HEAP_EVENT_GATE_WAITING;
    }

    RestorePendingSharedHeapEventVtable();
    return SHARED_HEAP_EVENT_GATE_RESTORED;
  }

  if (!eventManager ||
      !g_FollowerManager.NeedsFieldEventResourceRelease())
  {
    return SHARED_HEAP_EVENT_GATE_NONE;
  }

  GameSys::GameEvent* event = eventManager->GetGameEvent();
  if (!event)
  {
    return SHARED_HEAP_EVENT_GATE_NONE;
  }

#if FOLLOWER_3GX_INTERACTION_FEATURES
  if (Gen7Follower3gx::GetMountedInteractionMode()==Gen7Follower3gx::MOUNTED_INTERACTION_KEEP ||
      Gen7Follower3gx::KeepFollowerVisibleDuringEvents())
  {
    Follower3gx_NotifyEventCleanup(
      g_FollowerManager.GetFieldEventResourceReleaseReasons(),
      g_FollowerManager.GetDiagnosticState(),
      g_FollowerManager.GetDiagnosticParentHeapSource(),
      GetGameEventVtable(event));
    g_FollowerManager.ReportFieldEventMemory(fieldmap);
  }
#endif

  // Keep the event registered, but don't start it until follower cleanup is done.
  g_pPendingSharedHeapEventManager = eventManager;
  g_pPendingSharedHeapEvent = event;
  g_pPendingSharedHeapEventVtable = GetGameEventVtable(event);
  SetGameEventVtable(event, GetGameEventVtable(&g_SharedHeapBootGateEvent));

  g_FollowerEdgeIsolation.Restore();
  g_FollowerSceneIntegration.Reset();
#if FOLLOWER_CARRIER_THREEGX
  ResetRemoteFollowerSceneIntegrations();
#endif
#if FOLLOWER_POKEMON_ENABLE_BALL_TRANSITION
  g_FollowerManager.AdvanceBallTransitionWhileFieldPaused();
#endif
  if (g_FollowerManager.Terminate())
  {
    RestorePendingSharedHeapEventVtable();
    return SHARED_HEAP_EVENT_GATE_RESTORED;
  }

  return SHARED_HEAP_EVENT_GATE_CAPTURED;
}

void ResetSharedHeapEventGate()
{
  RestorePendingSharedHeapEventVtable();
}

bool IsRideActive(Field::Fieldmap* fieldmap)
{
  GameSys::GameManager* gameManager =
    fieldmap ? fieldmap->GetGameManager() : NULL;
  if (!gameManager)
  {
    return false;
  }

  const Field::RIDE_POKEMON_ID ride =
    Field::EventPokemonRideTool::GetPokemonRideOnID(gameManager);
  const int rideId = static_cast<int>(ride);
  return rideId >= static_cast<int>(Field::RIDE_POKEMON_ID_ABI) &&
    rideId < static_cast<int>(Field::RIDE_POKEMON_ID_COUNT);
}

bool UpdateWaterRideSuspension(Field::Fieldmap* fieldmap)
{
  if (IsRideActive(fieldmap))
  {
    // Land rides don't use the surf hook, so block follower reloads here too.
    g_WaterRideSuspendRequested = true;
    g_WaterRideObserved = true;
    g_WaterRideGraceFrames = 0;
    return true;
  }

  if (!g_WaterRideSuspendRequested)
  {
    return false;
  }

  if (g_WaterRideObserved)
  {
    g_WaterRideSuspendRequested = false;
    g_WaterRideObserved = false;
    g_WaterRideGraceFrames = 0;
    return false;
  }

  if (g_WaterRideGraceFrames > 0)
  {
    --g_WaterRideGraceFrames;
    return true;
  }

  g_WaterRideSuspendRequested = false;
  return false;
}

#if FOLLOWER_CARRIER_ENABLE_LOGGING
unsigned int g_UpdateCount = 0;
unsigned int g_LastState = 0xffffffffU;
unsigned int g_LastIdleReason = 0xffffffffU;
unsigned int g_LastSharedHeapWaitBlocker = 0xffffffffU;
bool g_HasLoggedEdgeIsolation = false;
bool g_HasLoggedSceneIntegration = false;
bool g_HasLoggedSharedHeapConflict = false;
unsigned int g_LastRepelStepCount = 0xffffffffU;
unsigned int g_LastEventChainSignature[6] = {
  0xffffffffU,
  0xffffffffU,
  0xffffffffU,
  0xffffffffU,
  0xffffffffU,
  0xffffffffU,
};

static const char REGISTERED[] = "[FollowerRuntime] carrier registered\n";
static const char CONTEXT_OK[] = "[FollowerRuntime] retail Fieldmap context valid\n";
static const char CONTEXT_BAD[] = "[FollowerRuntime] retail Fieldmap context missing\n";
static const char STATE_IDLE[] = "[FollowerRuntime] state idle\n";
static const char STATE_SUPPRESSED[] = "[FollowerRuntime] state load-suppressed\n";
static const char STATE_INIT_WAIT[] = "[FollowerRuntime] state init-wait\n";
static const char STATE_CREATE_WAIT[] = "[FollowerRuntime] state system-create-wait\n";
static const char STATE_MODEL_WAIT[] = "[FollowerRuntime] state model-load-wait\n";
static const char STATE_ACTIVE[] = "[FollowerRuntime] state active\n";
#if FOLLOWER_3GX_INTERACTION_FEATURES
static const char STATE_FORM_DELETE_WAIT[] = "[FollowerRuntime] state form-delete-wait\n";
#endif
static const char STATE_DELETE_WAIT[] = "[FollowerRuntime] state delete-wait\n";
static const char STATE_TERM_WAIT[] = "[FollowerRuntime] state term-wait\n";
static const char IDLE_READY[] = "[FollowerRuntime] idle input ready\n";
static const char IDLE_EVENT_ACTIVE[] = "[FollowerRuntime] idle blocked by active field event\n";
static const char IDLE_FIELDMAP_MISSING[] = "[FollowerRuntime] idle blocked by missing Fieldmap\n";
static const char IDLE_GAME_MANAGER_MISSING[] = "[FollowerRuntime] idle blocked by missing GameManager\n";
static const char IDLE_GAME_DATA_MISSING[] = "[FollowerRuntime] idle blocked by missing GameData\n";
static const char IDLE_PARTY_MISSING[] = "[FollowerRuntime] idle blocked by missing player party\n";
static const char IDLE_PARTY_EMPTY[] = "[FollowerRuntime] idle blocked by empty player party\n";
static const char IDLE_PARTY_INVALID[] = "[FollowerRuntime] idle blocked by egg/null-only player party\n";
static const char LOAD_FAIL_UNKNOWN[] = "[FollowerRuntime] load failure unknown\n";
static const char LOAD_FAIL_PARENT_MISSING[] = "[FollowerRuntime] load failure parent heap missing\n";
static const char LOAD_FAIL_PARENT_LOW[] = "[FollowerRuntime] load failure independent parent heap below 0x00368000\n";
static const char LOAD_FAIL_HEAP_CREATE[] = "[FollowerRuntime] load failure follower heap create\n";
static const char LOAD_FAIL_FACTORY_CREATE[] = "[FollowerRuntime] load failure factory create\n";
static const char LOAD_FAIL_MODEL_CREATE[] = "[FollowerRuntime] load failure model create\n";
static const char LOAD_FAIL_EVENT_BUSY[] = "[FollowerRuntime] load deferred while a field event owns event-device heap\n";
static const char LOAD_FAIL_EVENT_LOW[] = "[FollowerRuntime] load failure event-device heap below 0x00340000\n";
static const char LOAD_FAIL_REPEL_PENDING[] = "[FollowerRuntime] load deferred during final Repel steps\n";
static const char HEAP_LAYOUT_GATED_EVENT[] = "[FollowerRuntime] heap layout free-field-gated event-device\n";
static const char SHARED_EVENT_CAPTURED[] = "[FollowerRuntime] shared-heap event boot held in manager\n";
static const char SHARED_EVENT_RESTORED[] = "[FollowerRuntime] shared-heap event boot released after follower release\n";
static const char SHARED_EVENT_CONFLICT[] = "[FollowerRuntime] shared-heap event gate conflict\n";
static const char SHARED_WAIT_READY[] = "[FollowerRuntime] shared-heap wait ready\n";
static const char SHARED_WAIT_BALL[] = "[FollowerRuntime] shared-heap wait ball transition\n";
static const char SHARED_WAIT_MODEL_DELETE[] = "[FollowerRuntime] shared-heap wait model CanDelete\n";
static const char SHARED_WAIT_MODEL_REFERENCE[] = "[FollowerRuntime] shared-heap wait model reference\n";
static const char SHARED_WAIT_FACTORY_DELETE[] = "[FollowerRuntime] shared-heap wait factory delete\n";
static const char SHARED_WAIT_MODEL_LOAD[] = "[FollowerRuntime] shared-heap wait model load\n";
static const char SHARED_WAIT_UNKNOWN[] = "[FollowerRuntime] shared-heap wait unknown\n";
static const char PARENT_HEAP_NONE[] = "[FollowerRuntime] parent heap none\n";
static const char PARENT_HEAP_FIELD_EXT[] = "[FollowerRuntime] parent heap field-ext\n";
static const char PARENT_HEAP_AREA[] = "[FollowerRuntime] parent heap area-resource\n";
static const char PARENT_HEAP_FIELD[] = "[FollowerRuntime] parent heap field\n";
static const char PARENT_HEAP_APP_DEVICE[] = "[FollowerRuntime] parent heap app-device fallback\n";
static const char EDGE_ISOLATED[] = "[FollowerRuntime] follower-only material edge filter active\n";
static const char SCENE_INTEGRATED[] = "[FollowerRuntime] field lighting and stencil occlusion active\n";

template <unsigned int N>
void LogMarker(const char (&text)[N])
{
  FollowerCarrier_OutputDebugString(text, N - 1);
}

void LogState(unsigned int state)
{
  switch (state)
  {
  case 0: LogMarker(STATE_IDLE); break;
  case 1: LogMarker(STATE_SUPPRESSED); break;
  case 2: LogMarker(STATE_INIT_WAIT); break;
  case 3: LogMarker(STATE_CREATE_WAIT); break;
  case 4: LogMarker(STATE_MODEL_WAIT); break;
  case 5: LogMarker(STATE_ACTIVE); break;
#if FOLLOWER_3GX_INTERACTION_FEATURES
  case 6: LogMarker(STATE_FORM_DELETE_WAIT); break;
  case 7: LogMarker(STATE_DELETE_WAIT); break;
  case 8: LogMarker(STATE_TERM_WAIT); break;
#else
  case 6: LogMarker(STATE_DELETE_WAIT); break;
  case 7: LogMarker(STATE_TERM_WAIT); break;
#endif
  }
}

void LogIdleReason(unsigned int reason)
{
  switch (reason)
  {
  case 0: LogMarker(IDLE_READY); break;
  case 1: LogMarker(IDLE_EVENT_ACTIVE); break;
  case 2: LogMarker(IDLE_FIELDMAP_MISSING); break;
  case 3: LogMarker(IDLE_GAME_MANAGER_MISSING); break;
  case 4: LogMarker(IDLE_GAME_DATA_MISSING); break;
  case 5: LogMarker(IDLE_PARTY_MISSING); break;
  case 6: LogMarker(IDLE_PARTY_EMPTY); break;
  case 7: LogMarker(IDLE_PARTY_INVALID); break;
  }
}

void LogLoadFailure(unsigned int failure)
{
  switch (failure)
  {
  case 1: LogMarker(LOAD_FAIL_PARENT_MISSING); break;
  case 2: LogMarker(LOAD_FAIL_PARENT_LOW); break;
  case 3: LogMarker(LOAD_FAIL_HEAP_CREATE); break;
  case 4: LogMarker(LOAD_FAIL_FACTORY_CREATE); break;
  case 5: LogMarker(LOAD_FAIL_MODEL_CREATE); break;
  case 6: LogMarker(LOAD_FAIL_EVENT_BUSY); break;
  case 7: LogMarker(LOAD_FAIL_EVENT_LOW); break;
  case 8: LogMarker(LOAD_FAIL_REPEL_PENDING); break;
  default: LogMarker(LOAD_FAIL_UNKNOWN); break;
  }
}

void LogSharedHeapWaitBlocker(unsigned int blocker)
{
  switch (blocker)
  {
  case 0: LogMarker(SHARED_WAIT_READY); break;
  case 1: LogMarker(SHARED_WAIT_BALL); break;
  case 2: LogMarker(SHARED_WAIT_MODEL_DELETE); break;
  case 3: LogMarker(SHARED_WAIT_MODEL_REFERENCE); break;
  case 4: LogMarker(SHARED_WAIT_FACTORY_DELETE); break;
  case 5: LogMarker(SHARED_WAIT_MODEL_LOAD); break;
  default: LogMarker(SHARED_WAIT_UNKNOWN); break;
  }
}

void LogSharedHeapEventGateResult(SharedHeapEventGateResult result)
{
  switch (result)
  {
  case SHARED_HEAP_EVENT_GATE_CAPTURED:
    g_HasLoggedSharedHeapConflict = false;
    g_LastSharedHeapWaitBlocker = 0xffffffffU;
    LogMarker(SHARED_EVENT_CAPTURED);
    break;
  case SHARED_HEAP_EVENT_GATE_WAITING:
    {
      const unsigned int state = g_FollowerManager.GetDiagnosticState();
      if (state != g_LastState)
      {
        g_LastState = state;
        LogState(state);
      }
      const unsigned int blocker =
        g_FollowerManager.GetDiagnosticTerminateBlocker();
      if (blocker != g_LastSharedHeapWaitBlocker)
      {
        g_LastSharedHeapWaitBlocker = blocker;
        LogSharedHeapWaitBlocker(blocker);
      }
    }
    break;
  case SHARED_HEAP_EVENT_GATE_RESTORED:
    g_HasLoggedSharedHeapConflict = false;
    g_LastSharedHeapWaitBlocker = 0xffffffffU;
    LogMarker(SHARED_EVENT_RESTORED);
    break;
  case SHARED_HEAP_EVENT_GATE_CONFLICT:
    if (!g_HasLoggedSharedHeapConflict)
    {
      g_HasLoggedSharedHeapConflict = true;
      LogMarker(SHARED_EVENT_CONFLICT);
    }
    break;
  default:
    break;
  }
}

void LogParentHeap(unsigned int source)
{
  switch (source)
  {
  case 1: LogMarker(PARENT_HEAP_FIELD_EXT); break;
  case 2: LogMarker(PARENT_HEAP_AREA); break;
  case 3: LogMarker(PARENT_HEAP_FIELD); break;
  case 4: LogMarker(PARENT_HEAP_APP_DEVICE); break;
  default: LogMarker(PARENT_HEAP_NONE); break;
  }
}

void LogParentHeapAllocatableSize(unsigned int value)
{
  static const char HEX[] = "0123456789abcdef";
  char text[] = "[FollowerRuntime] parent heap max allocatable 0x00000000\n";
  unsigned int digitOffset = 0;
  for (unsigned int i = 0; i + 1 < sizeof(text); ++i)
  {
    if (text[i] == '0' && text[i + 1] == 'x')
    {
      digitOffset = i + 2;
      break;
    }
  }
  for (unsigned int i = 0; i < 8; ++i)
  {
    const unsigned int shift = (7 - i) * 4;
    text[digitOffset + i] = HEX[(value >> shift) & 0xf];
  }
  FollowerCarrier_OutputDebugString(text, sizeof(text) - 1);
}

void LogRepelStepCount(Field::Fieldmap* fieldmap)
{
  const unsigned int value =
    Field::FollowerRuntime::GetRepelStepCount(fieldmap);
  if (value == g_LastRepelStepCount)
  {
    return;
  }
  g_LastRepelStepCount = value;

  static const char HEX[] = "0123456789abcdef";
  char text[] = "[FollowerRuntime] repel steps 0x00000000\n";
  unsigned int digitOffset = 0;
  for (unsigned int i = 0; i + 1 < sizeof(text); ++i)
  {
    if (text[i] == '0' && text[i + 1] == 'x')
    {
      digitOffset = i + 2;
      break;
    }
  }
  for (unsigned int i = 0; i < 8; ++i)
  {
    const unsigned int shift = (7 - i) * 4;
    text[digitOffset + i] = HEX[(value >> shift) & 0xf];
  }
  FollowerCarrier_OutputDebugString(text, sizeof(text) - 1);
}

unsigned int GetObjectVtable(const void* object)
{
  return object ? *reinterpret_cast<const unsigned int*>(object) : 0;
}

void LogEventChain(Field::Fieldmap* fieldmap)
{
  GameSys::GameManager* gameManager =
    fieldmap ? fieldmap->GetGameManager() : NULL;
  GameSys::GameEventManager* eventManager =
    gameManager
      ? FOLLOWER_POKEMON_GET_GAME_EVENT_MANAGER(gameManager)
      : NULL;
  GameSys::GameEvent* event0 =
    eventManager ? eventManager->GetGameEvent() : NULL;
  GameSys::GameEvent* event1 =
    event0 ? event0->GetParentEventPointer() : NULL;
  GameSys::GameEvent* event2 =
    event1 ? event1->GetParentEventPointer() : NULL;
  const unsigned int signature[6] = {
    reinterpret_cast<unsigned int>(event0),
    GetObjectVtable(event0),
    reinterpret_cast<unsigned int>(event1),
    GetObjectVtable(event1),
    reinterpret_cast<unsigned int>(event2),
    GetObjectVtable(event2),
  };

  bool changed = false;
  for (unsigned int i = 0; i < 6; ++i)
  {
    if (signature[i] != g_LastEventChainSignature[i])
    {
      changed = true;
      g_LastEventChainSignature[i] = signature[i];
    }
  }
  if (!changed)
  {
    return;
  }

  static const char HEX[] = "0123456789abcdef";
  char text[] =
    "[FollowerRuntime] event chain e0=0x00000000/v=0x00000000 e1=0x00000000/v=0x00000000 e2=0x00000000/v=0x00000000\n";
  unsigned int valueIndex = 0;
  for (unsigned int i = 0;
       i + 1 < sizeof(text) && valueIndex < 6;
       ++i)
  {
    if (text[i] != '0' || text[i + 1] != 'x')
    {
      continue;
    }
    const unsigned int value = signature[valueIndex++];
    for (unsigned int digit = 0; digit < 8; ++digit)
    {
      const unsigned int shift = (7 - digit) * 4;
      text[i + 2 + digit] = HEX[(value >> shift) & 0xf];
    }
  }
  FollowerCarrier_OutputDebugString(text, sizeof(text) - 1);
}
#endif

}

#if !FOLLOWER_CARRIER_THREEGX
struct FollowerCarrierHostStateData
{
  void* module;
  unsigned int (*update)(
    Field::Fieldmap*,
    Field::MyRenderingPipeLine*,
    BaseCollisionScene*,
    BaseCollisionScene*,
    unsigned int
  );
  unsigned int (*terminate)(Field::Fieldmap*);
  unsigned int abiVersion;
  void (*suspendForWaterRide)();
};

#if FOLLOWER_CARRIER_GCC
extern "C" FollowerCarrierHostStateData g_FollowerCarrierHostState
  __asm__("offset_import_FieldRo_2_734");
#else
extern "C" FollowerCarrierHostStateData g_FollowerCarrierHostState;
#endif
#endif

extern "C" bool FollowerCarrier_InitializeRuntime()
{
#if FOLLOWER_CARRIER_THREEGX
  if (g_FollowerCarrierRuntimeInitialized)
  {
    return true;
  }

  g_pFollowerManager = new (g_FollowerManagerStorage)
    Field::FollowerRuntime::Manager();
  g_pFollowerSceneIntegration = new (g_FollowerSceneIntegrationStorage)
    FollowerSceneIntegration();
  for (unsigned int index = 0;
       index < Field::FollowerRuntime::FOLLOWER_REMOTE_REPLICA_MAX;
       ++index)
  {
    g_pRemoteFollowerSceneIntegrations[index] = new (
      g_RemoteFollowerSceneIntegrationStorage[index]
      ) FollowerSceneIntegration();
  }
  g_pFollowerEdgeIsolation = new (g_FollowerEdgeIsolationStorage)
    FollowerEdgeIsolation();
  g_pSharedHeapBootGateEvent = new (g_SharedHeapBootGateEventStorage)
    SharedHeapBootGateEvent();
  g_pCutsceneBlackoutFade = new (g_CutsceneBlackoutFadeStorage)
    CutsceneBlackoutFade();

  g_pPreviousCustomUpperFade = NULL;
  g_pPreviousCustomLowerFade = NULL;
  g_CutsceneBlackoutInstalled = false;
  g_WaterRideSuspendRequested = false;
  g_WaterRideObserved = false;
  g_WaterRideGraceFrames = 0;
  g_CutsceneFastForwardActive = false;
  g_pPendingSharedHeapEventManager = NULL;
  g_pPendingSharedHeapEvent = NULL;
  g_pPendingSharedHeapEventVtable = NULL;
#if FOLLOWER_3GX_DIAGNOSTIC
  ResetFollowerSceneGraphDiagnostics();
#endif
  g_FollowerCarrierRuntimeInitialized = true;
#endif
  return true;
}

extern "C" void FollowerCarrier_AfterEventCheck(Field::Fieldmap* fieldmap)
{
#if FOLLOWER_CARRIER_THREEGX
  if (!g_FollowerCarrierRuntimeInitialized)
  {
    return;
  }
#endif

  // Catch new events before they start using memory that's still held by the follower.
  const SharedHeapEventGateResult result =
    UpdateSharedHeapEventGate(fieldmap);
#if FOLLOWER_CARRIER_ENABLE_LOGGING
  LogSharedHeapEventGateResult(result);
#else
  (void)result;
#endif
}

extern "C" unsigned int FollowerCarrier_Update(
  Field::Fieldmap* fieldmap,
  Field::MyRenderingPipeLine* renderingPipeline,
  // Wall and physics checks use the scene at +0x9c. The height scene is separate.
  BaseCollisionScene* terrainWallScene,
  BaseCollisionScene* staticScene,
  unsigned int phase
)
{
#if FOLLOWER_CARRIER_THREEGX
  if (!g_FollowerCarrierRuntimeInitialized)
  {
    return 0U;
  }
#endif

#if FOLLOWER_3GX_INTERACTION_FEATURES
  // Restore before the game starts an event or destroys the player model.
  if (phase == 0)
  {
    g_FollowerManager.RestoreRidePlayerVisibility();
  }
#endif

  const SharedHeapEventGateResult sharedHeapGateResult =
    UpdateSharedHeapEventGate(fieldmap);

#if FOLLOWER_CARRIER_THREEGX && FOLLOWER_3GX_DIAGNOSTIC
  Gen7Follower3gx::UpdateFollowerLifecycleDiagnostics(
    g_FollowerManager.GetDiagnosticState(),
    g_FollowerManager.GetDiagnosticTerminateBlocker(),
    g_FollowerManager.GetDiagnosticLoadFailure()
    );
#endif

#if FOLLOWER_CARRIER_ENABLE_LOGGING
  LogSharedHeapEventGateResult(sharedHeapGateResult);
#endif

  if (sharedHeapGateResult != SHARED_HEAP_EVENT_GATE_NONE)
  {
#if FOLLOWER_CARRIER_ENABLE_CUTSCENE_FAST_FORWARD
    StopCutsceneFastForward(fieldmap);
#endif
    g_FollowerEdgeIsolation.Restore();
    g_FollowerSceneIntegration.Reset();
#if FOLLOWER_CARRIER_THREEGX
    ResetRemoteFollowerSceneIntegrations();
#endif
    return phase == 0
      ? static_cast<unsigned int>(FIELDRO_UPDATE_COMMAND_PAUSE)
      : 0U;
  }

  if (phase == 0)
  {
    g_FollowerEdgeIsolation.Restore();
#if FOLLOWER_3GX_INTERACTION_FEATURES
    if (
#if FOLLOWER_CARRIER_THREEGX
        !Gen7Follower3gx::IsFreeCameraActive() &&
#endif
        g_FollowerManager.TryStartInteraction(fieldmap))
    {
      return static_cast<unsigned int>(
        FIELDRO_UPDATE_COMMAND_PAUSE |
        FIELDRO_UPDATE_COMMAND_RUN_POST
        );
    }
#endif
#if FOLLOWER_CARRIER_ENABLE_CUTSCENE_FAST_FORWARD
    return UpdateCutsceneFastForward(fieldmap);
#else
    return 0U;
#endif
  }

  if (UpdateWaterRideSuspension(fieldmap))
  {
#if FOLLOWER_CARRIER_ENABLE_CUTSCENE_FAST_FORWARD
    StopCutsceneFastForward(fieldmap);
#endif
    g_FollowerEdgeIsolation.Restore();
    g_FollowerSceneIntegration.Reset();
#if FOLLOWER_CARRIER_THREEGX
    ResetRemoteFollowerSceneIntegrations();
#endif
    g_FollowerManager.Terminate();
    return 0U;
  }

#if FOLLOWER_CARRIER_ENABLE_CUTSCENE_FAST_FORWARD
  if (g_CutsceneFastForwardActive)
  {
    g_FollowerEdgeIsolation.Restore();
    g_FollowerSceneIntegration.Reset();
#if FOLLOWER_CARRIER_THREEGX
    ResetRemoteFollowerSceneIntegrations();
#endif
    return 0U;
  }
#endif

#if FOLLOWER_CARRIER_ENABLE_LOGGING
  if (g_UpdateCount++ == 0)
  {
    if (fieldmap && renderingPipeline && terrainWallScene && staticScene)
    {
      LogMarker(CONTEXT_OK);
    }
    else
    {
      LogMarker(CONTEXT_BAD);
    }
  }
#endif

  g_FollowerManager.Update(
    fieldmap,
    renderingPipeline,
    terrainWallScene,
    staticScene
  );

#if FOLLOWER_CARRIER_THREEGX && FOLLOWER_3GX_DIAGNOSTIC
  Gen7Follower3gx::UpdateFollowerLifecycleDiagnostics(
    g_FollowerManager.GetDiagnosticState(),
    g_FollowerManager.GetDiagnosticTerminateBlocker(),
    g_FollowerManager.GetDiagnosticLoadFailure()
    );
#endif

  gfl2::renderingengine::scenegraph::instance::ModelInstanceNode*
    followerNode = g_FollowerManager.GetFollowerModelInstanceNode();
#if FOLLOWER_CARRIER_THREEGX
  gfl2::renderingengine::scenegraph::instance::ModelInstanceNode*
    remoteFollowerNodes[
      Field::FollowerRuntime::FOLLOWER_REMOTE_REPLICA_MAX
      ] = {};
  for (unsigned int index = 0;
       index < Field::FollowerRuntime::FOLLOWER_REMOTE_REPLICA_MAX;
       ++index)
  {
    remoteFollowerNodes[index] =
      g_FollowerManager.GetRemoteReplicaModelInstanceNode(index);
  }
#endif

#if FOLLOWER_CARRIER_THREEGX && FOLLOWER_3GX_PERFORMANCE_FEATURES
  if (g_FollowerManager.ShouldTraversePerformanceAnimation())
  {
    TraverseFollowerForPerformance(followerNode);
    for (unsigned int index = 0;
         index < Field::FollowerRuntime::FOLLOWER_REMOTE_REPLICA_MAX;
         ++index)
    {
      TraverseFollowerForPerformance(remoteFollowerNodes[index]);
    }
  }
#endif

#if FOLLOWER_CARRIER_ENABLE_LOGGING
  if (g_FollowerSceneIntegration.Apply(followerNode) &&
      !g_HasLoggedSceneIntegration)
  {
    g_HasLoggedSceneIntegration = true;
    LogMarker(SCENE_INTEGRATED);
  }
#else
  g_FollowerSceneIntegration.Apply(followerNode);
#endif

#if FOLLOWER_CARRIER_THREEGX
  for (unsigned int index = 0;
       index < Field::FollowerRuntime::FOLLOWER_REMOTE_REPLICA_MAX;
       ++index)
  {
    if (g_pRemoteFollowerSceneIntegrations[index])
    {
      g_pRemoteFollowerSceneIntegrations[index]->Apply(
        remoteFollowerNodes[index]
        );
    }
  }
#endif

#if FOLLOWER_CARRIER_THREEGX
  g_FollowerSceneIntegration.SetOutlineMode(
    Gen7Follower3gx::GetFollowerOutlineMode()
    );
  for (unsigned int index = 0;
       index < Field::FollowerRuntime::FOLLOWER_REMOTE_REPLICA_MAX;
       ++index)
  {
    if (g_pRemoteFollowerSceneIntegrations[index])
    {
      g_pRemoteFollowerSceneIntegrations[index]->SetOutlineMode(
        Gen7Follower3gx::GetFollowerOutlineMode()
        );
    }
  }
#if FOLLOWER_3GX_PERFORMANCE_FEATURES
  const Gen7Follower3gx::PerformanceOptionMask performanceOptions =
    Gen7Follower3gx::GetPerformanceOptions();
  if (Gen7Follower3gx::HasPerformanceOption(
        performanceOptions,
        Gen7Follower3gx::PERFORMANCE_OPTION_DISABLE_FIELD_OUTLINES
        ))
  {
    g_FollowerEdgeIsolation.Apply(
      renderingPipeline,
      followerNode,
      true,
      true
      );
  }
  else
  {
    g_FollowerEdgeIsolation.SuppressFollowerOutline(
      renderingPipeline,
      followerNode
      );
  }
  if (Gen7Follower3gx::HasPerformanceOption(
        performanceOptions,
        Gen7Follower3gx::PERFORMANCE_OPTION_LITE_FOLLOWER_MATERIALS
        ))
  {
    g_FollowerEdgeIsolation.ApplyPerformanceLite(followerNode);
  }
  if (Gen7Follower3gx::HasPerformanceOption(
        performanceOptions,
        Gen7Follower3gx::PERFORMANCE_OPTION_DISABLE_BLOOM
        ))
  {
    g_FollowerEdgeIsolation.SuppressBloom(renderingPipeline);
  }
  g_FollowerEdgeIsolation.ApplyWorldIsolation(
    renderingPipeline,
    fieldmap,
    performanceOptions
    );
#else
  g_FollowerEdgeIsolation.SuppressFollowerOutline(
    renderingPipeline,
    followerNode
    );
#endif
#else
#if FOLLOWER_CARRIER_ENABLE_LOGGING
  if (g_FollowerEdgeIsolation.Apply(
        renderingPipeline,
        followerNode
        ) && !g_HasLoggedEdgeIsolation)
  {
    g_HasLoggedEdgeIsolation = true;
    LogMarker(EDGE_ISOLATED);
  }
#else
  g_FollowerEdgeIsolation.Apply(
    renderingPipeline,
    followerNode
    );
#endif
#endif

#if FOLLOWER_CARRIER_ENABLE_LOGGING
  const unsigned int state = g_FollowerManager.GetDiagnosticState();
  const unsigned int idleReason = g_FollowerManager.GetDiagnosticIdleReason();
  if (idleReason != g_LastIdleReason)
  {
    g_LastIdleReason = idleReason;
    LogIdleReason(idleReason);
  }
  if (state != g_LastState)
  {
    g_LastState = state;
    LogState(state);
    if (state == 1 || state == 2)
    {
      if (state == 2 &&
          g_FollowerManager.GetDiagnosticParentHeapAllocatableSize() < 0x368000)
      {
        LogMarker(HEAP_LAYOUT_GATED_EVENT);
      }
      LogParentHeap(g_FollowerManager.GetDiagnosticParentHeapSource());
      LogParentHeapAllocatableSize(
        g_FollowerManager.GetDiagnosticParentHeapAllocatableSize()
        );
    }
    if (state == 1)
    {
      LogLoadFailure(g_FollowerManager.GetDiagnosticLoadFailure());
    }
  }
#endif

  return 0U;
}

extern "C" unsigned int FollowerCarrier_Terminate()
{
#if FOLLOWER_CARRIER_THREEGX
  if (!g_FollowerCarrierRuntimeInitialized)
  {
    return 1U;
  }
#endif

#if FOLLOWER_CARRIER_ENABLE_CUTSCENE_FAST_FORWARD
  DisableCutsceneBlackout();
  g_CutsceneFastForwardActive = false;
#endif
  return g_FollowerManager.Terminate() ? 1U : 0U;
}

#if FOLLOWER_CARRIER_THREEGX
extern "C" bool FollowerCarrier_GetNetworkState(
  Gen7Follower3gx::FollowerNetworkState* state
)
{
  if (!state)
  {
    return false;
  }
  *state = Gen7Follower3gx::FollowerNetworkState();
  return g_FollowerCarrierRuntimeInitialized &&
    g_FollowerManager.GetNetworkState(state);
}

extern "C" void FollowerCarrier_ConfigureRemoteReplicas(
  void* externalHeap,
  unsigned int replicaCapacity
)
{
  if (!g_FollowerCarrierRuntimeInitialized)
  {
    return;
  }
  g_FollowerManager.ConfigureRemoteReplicas(
    reinterpret_cast<gfl2::heap::HeapBase*>(externalHeap),
    replicaCapacity
    );
}

extern "C" void FollowerCarrier_SetRemoteReplicaState(
  unsigned int replicaIndex,
  const Gen7Follower3gx::FollowerNetworkState* state
)
{
  if (!g_FollowerCarrierRuntimeInitialized || !state)
  {
    return;
  }
  g_FollowerManager.SetRemoteReplicaState(replicaIndex, *state);
}

extern "C" bool FollowerCarrier_GetRemoteReplicaDiagnostics(
  Gen7Follower3gx::FollowerReplicaDiagnostics* diagnostics
)
{
  if (!g_FollowerCarrierRuntimeInitialized || !diagnostics)
  {
    return false;
  }
  g_FollowerManager.GetRemoteReplicaDiagnostics(diagnostics);
  return true;
}
#endif

#if FOLLOWER_CARRIER_THREEGX
extern "C" bool Follower3gx_StartPcEvent(void*);
extern "C" bool FollowerCarrier_OpenPc(Field::Fieldmap* fieldmap)
{
  return fieldmap && Follower3gx_StartPcEvent(fieldmap->GetGameManager());
}
extern "C" bool FollowerCarrier_IsFreeField(Field::Fieldmap* fieldmap)
{
  if (!g_FollowerCarrierRuntimeInitialized || !fieldmap ||
      g_pPendingSharedHeapEvent || g_WaterRideSuspendRequested ||
      IsRideActive(fieldmap))
  {
    return false;
  }

  GameSys::GameManager* const gameManager = fieldmap->GetGameManager();
  GameSys::GameEventManager* const eventManager = gameManager
    ? FOLLOWER_POKEMON_GET_GAME_EVENT_MANAGER(gameManager)
    : NULL;
  return gameManager && (!eventManager || !eventManager->IsExists());
}
#endif

extern "C" unsigned int FollowerCarrier_HostTerminate(
  Field::Fieldmap* fieldmap
)
{
#if FOLLOWER_CARRIER_THREEGX
  if (!g_FollowerCarrierRuntimeInitialized)
  {
    return 1U;
  }
#endif

  g_WaterRideSuspendRequested = false;
  g_WaterRideObserved = false;
  g_WaterRideGraceFrames = 0;
#if FOLLOWER_CARRIER_ENABLE_CUTSCENE_FAST_FORWARD
  StopCutsceneFastForward(fieldmap);
#endif
  ResetSharedHeapEventGate();
  g_FollowerEdgeIsolation.Restore();
  g_FollowerSceneIntegration.Reset();
#if FOLLOWER_CARRIER_THREEGX
  ResetRemoteFollowerSceneIntegrations();
#endif
#if FOLLOWER_POKEMON_ENABLE_BALL_TRANSITION
  g_FollowerManager.AdvanceBallTransitionWhileFieldPaused();
#endif
  return FollowerCarrier_Terminate();
}

extern "C" bool FollowerCarrier_ShutdownRuntime()
{
#if FOLLOWER_CARRIER_THREEGX
  if (!g_FollowerCarrierRuntimeInitialized)
  {
    return true;
  }

  if (FollowerCarrier_HostTerminate(NULL) == 0U)
  {
    return false;
  }

  g_pCutsceneBlackoutFade->~CutsceneBlackoutFade();
  g_pSharedHeapBootGateEvent->~SharedHeapBootGateEvent();
  g_pFollowerEdgeIsolation->~FollowerEdgeIsolation();
  g_pFollowerSceneIntegration->~FollowerSceneIntegration();
  for (unsigned int index = 0;
       index < Field::FollowerRuntime::FOLLOWER_REMOTE_REPLICA_MAX;
       ++index)
  {
    if (g_pRemoteFollowerSceneIntegrations[index])
    {
      g_pRemoteFollowerSceneIntegrations[index]
        ->~FollowerSceneIntegration();
      g_pRemoteFollowerSceneIntegrations[index] = NULL;
    }
  }
  g_pFollowerManager->~Manager();
  g_pCutsceneBlackoutFade = NULL;
  g_pSharedHeapBootGateEvent = NULL;
  g_pFollowerEdgeIsolation = NULL;
  g_pFollowerSceneIntegration = NULL;
  g_pFollowerManager = NULL;
#if FOLLOWER_3GX_DIAGNOSTIC
  ResetFollowerSceneGraphDiagnostics();
#endif
  g_FollowerCarrierRuntimeInitialized = false;
#endif
  return true;
}

extern "C" void FollowerCarrier_SuspendForWaterRide()
{
#if FOLLOWER_CARRIER_THREEGX
  if (!g_FollowerCarrierRuntimeInitialized)
  {
    return;
  }
#endif

  g_WaterRideSuspendRequested = true;
  g_WaterRideGraceFrames = WATER_RIDE_REQUEST_GRACE_FRAMES;
}

namespace
{
#if !FOLLOWER_CARRIER_THREEGX
void RegisterFollowerCarrier()
{
  g_FollowerCarrierHostState.update = FollowerCarrier_Update;
  g_FollowerCarrierHostState.terminate = FollowerCarrier_HostTerminate;
  g_FollowerCarrierHostState.abiVersion = 5;
  g_FollowerCarrierHostState.suspendForWaterRide =
    FollowerCarrier_SuspendForWaterRide;
#if FOLLOWER_CARRIER_ENABLE_LOGGING
  LogMarker(REGISTERED);
#endif
}

#if !FOLLOWER_CARRIER_GCC
class FollowerCarrierRegistration
{
public:
  FollowerCarrierRegistration()
  {
    RegisterFollowerCarrier();
  }
};

FollowerCarrierRegistration g_FollowerCarrierRegistration;
#endif
#endif
}

#if FOLLOWER_CARRIER_GCC && !FOLLOWER_CARRIER_THREEGX
extern "C" void nnroProlog()
{
  RegisterFollowerCarrier();
}

extern "C" void nnroEpilog()
{
  g_FollowerCarrierHostState.update = NULL;
  g_FollowerCarrierHostState.terminate = NULL;
  g_FollowerCarrierHostState.abiVersion = 0;
  g_FollowerCarrierHostState.suspendForWaterRide = NULL;
}
#endif
