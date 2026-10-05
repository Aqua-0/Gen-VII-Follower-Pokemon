#include "FollowerTalk.hpp"
#include "FollowMode.hpp"
#include "FollowerPlacement.hpp"
#include "FollowerClearance.hpp"
#pragma once
#ifndef FOLLOWER_CARRIER_DEDICATED_ARENA
#define FOLLOWER_CARRIER_DEDICATED_ARENA 0
#endif
#if FOLLOWER_CARRIER_DEDICATED_ARENA
#include "FollowerArena.hpp"
#include "PartyFollowerSettings.hpp"
#include "PartyFollowerMotion.hpp"
#endif
#include "RiderPoseStabilizer.hpp"
#include "RideableSpecies.hpp"
#include "RiderBoneControl.hpp"
#include "RideEventSettings.hpp"
#include "RidePresentation.hpp"

#include "compat/GameAbi.hpp"

#include "TrailMovementPolicy.hpp"
#if FOLLOWER_CARRIER_THREEGX
#include "FollowerNetworkState.hpp"
#include "MountedRideFeedback.hpp"
#include "RiderAnimationSettings.hpp"
#endif
#include "PerformanceDiagnostics.hpp"

#ifndef FOLLOWER_POKEMON_ENABLE_BALL_TRANSITION
#define FOLLOWER_POKEMON_ENABLE_BALL_TRANSITION FOLLOWER_CARRIER_THREEGX
#endif

#if FOLLOWER_POKEMON_ENABLE_BALL_TRANSITION
#include "FollowerBallTransition.hpp"
#if FOLLOWER_3GX_INTERACTION_FEATURES
#include "BattleWeatherHandoff.hpp"
#include "DiagnosticEffectSelector.hpp"
#include "FollowerInteractionEffect.hpp"
#include "FollowerWeatherAdapter.hpp"
#include "VictiniLuck.hpp"
#endif
#if FOLLOWER_3GX_DIAGNOSTIC
#include "DiagnosticEffectModule.hpp"
#include "DiagnosticWeatherSelector.hpp"
#endif
#endif

#ifndef FOLLOWER_POKEMON_USE_TRAIL_POLICY
#define FOLLOWER_POKEMON_USE_TRAIL_POLICY 0
#endif

#ifndef FOLLOWER_POKEMON_RELEASE_INDEPENDENT_HEAP_FOR_FIELD_EVENTS
#define FOLLOWER_POKEMON_RELEASE_INDEPENDENT_HEAP_FOR_FIELD_EVENTS 0
#endif

#ifndef FOLLOWER_POKEMON_USE_AREA_RESOURCE_HEAP
#define FOLLOWER_POKEMON_USE_AREA_RESOURCE_HEAP 1
#endif

#ifndef FOLLOWER_POKEMON_GET_AREA_RESOURCE_HEAP
#define FOLLOWER_POKEMON_GET_AREA_RESOURCE_HEAP(pArea) ((pArea)->GetResourceHeap())
#endif

#ifndef FOLLOWER_POKEMON_GET_EDGE_NORMAL_MAP_ENABLE
#define FOLLOWER_POKEMON_GET_EDGE_NORMAL_MAP_ENABLE(pPipeLine) ((pPipeLine)->IsEdgeNormalMapEnable())
#endif

#ifndef FOLLOWER_POKEMON_SET_EDGE_NORMAL_MAP_ENABLE
#define FOLLOWER_POKEMON_SET_EDGE_NORMAL_MAP_ENABLE(pPipeLine, flag) ((pPipeLine)->SetEdgeNormalMapEnable(flag))
#endif

#ifndef FOLLOWER_POKEMON_GET_GAME_DATA
#define FOLLOWER_POKEMON_GET_GAME_DATA(pGameManager) ((pGameManager)->GetGameData())
#endif

#ifndef FOLLOWER_POKEMON_GET_GAME_EVENT_MANAGER
#define FOLLOWER_POKEMON_GET_GAME_EVENT_MANAGER(pGameManager) ((pGameManager)->GetGameEventManager())
#endif

#ifndef FOLLOWER_POKEMON_GET_EVENT_WORK
#define FOLLOWER_POKEMON_GET_EVENT_WORK(pGameData) ((pGameData)->GetEventWork())
#endif

#ifndef FOLLOWER_POKEMON_PREEMPT_REPEL_EXPIRY
#define FOLLOWER_POKEMON_PREEMPT_REPEL_EXPIRY (!FOLLOWER_CARRIER_THREEGX)
#endif

namespace Field
{
namespace FollowerRuntime
{

static const u32 FOLLOWER_TRAIL_COUNT = 72;
static const u32 FOLLOWER_TRAIL_DELAY = 16;
static const u32 FOLLOWER_BALL_TRANSITION_SOUND_ID = 589884;
static const s32 FOLLOWER_BALL_TRANSITION_SOUND_CONTROL_ID = 0x46504253;
static const f32 FOLLOWER_IDLE_DISTANCE = 38.0f;
static const f32 FOLLOWER_SLOW_DISTANCE = 150.0f;
static const f32 FOLLOWER_RUN_DISTANCE = 260.0f;
static const f32 FOLLOWER_RUN_EXIT_DISTANCE = 200.0f;
static const f32 FOLLOWER_MIN_WALK_SPEED = 3.0f;
static const f32 FOLLOWER_WALK_SPEED = 14.0f;
static const f32 FOLLOWER_RUN_SPEED = 30.0f;
static const f32 FOLLOWER_WARP_DISTANCE = 900.0f;
static const f32 FOLLOWER_ANIMATION_MOVE_EPSILON = 0.05f;
static const f32 FOLLOWER_ROOT_MOTION_EPSILON = 0.01f;
static const f32 FOLLOWER_ANIMATION_STEP_MIN = 0.10f;
static const f32 FOLLOWER_WALK_ANIMATION_STEP_MAX = 1.15f;
static const f32 FOLLOWER_RUN_ANIMATION_STEP_MAX = 1.30f;
static const f32 FOLLOWER_ANIMATION_STEP_RESPONSE = 0.12f;
static const f32 FOLLOWER_COLLISION_PLAYBACK_RECOVERY = 0.25f;
static const f32 FOLLOWER_ROOT_MOTION_SAMPLE_FRAMES = 12.0f;
static const f32 FOLLOWER_ROOT_MOTION_SAMPLE_RESPONSE = 0.25f;
static const f32 FOLLOWER_ROOT_MOTION_STEP_LIMIT = 2.50f;
static const f32 FOLLOWER_COLLISION_RADIUS_MIN = 28.0f;
static const f32 FOLLOWER_COLLISION_RADIUS_MAX = 90.0f;
static const f32 FOLLOWER_COLLISION_HEIGHT_RATE = 0.30f;
static const f32 FOLLOWER_PLAYER_COLLISION_RADIUS = 34.0f;
static const f32 FOLLOWER_PLAYER_BODY_RADIUS_MIN = 12.0f;
static const f32 FOLLOWER_PLAYER_BODY_RADIUS_MAX = 160.0f;
static const f32 FOLLOWER_PLAYER_BODY_BOUNDS_SCALE = 0.65f;
static const f32 FOLLOWER_IDLE_GAP_RADIUS_REFERENCE = 90.0f;
static const f32 FOLLOWER_IDLE_GAP_SCALE_MIN = 0.25f;
static const f32 FOLLOWER_SHADOW_DEFAULT_SCALE = 0.70f;
static const f32 FOLLOWER_SHADOW_HEIGHT_RATE = 0.01f;
static const f32 FOLLOWER_SHADOW_SCALE_MIN = 0.35f;
static const f32 FOLLOWER_SHADOW_SCALE_MAX = 1.35f;
static const f32 FOLLOWER_WALL_RAY_HEIGHT = 35.0f;
static const f32 FOLLOWER_GROUND_RAY_EXTENT = 1000000.0f;
static const f32 FOLLOWER_COLLISION_EPSILON = 0.001f;
static const u32 FOLLOWER_RUN_TRANSITION_FRAMES = 6;
static const u32 FOLLOWER_NO_MOVE_WAIT_FRAMES = 3;
static const s32 FOLLOWER_MOTION_BLEND_FRAMES = 6;
#if FOLLOWER_3GX_PERFORMANCE_FEATURES
static const u32 FOLLOWER_INTERPOLATED_POSE_MAX_JOINTS = 512;
static const f32 FOLLOWER_INTERPOLATED_QUATERNION_EPSILON = 0.000001f;
#endif
// Expanded model packs have more archive entries, so leave room for the larger index.
static const u32 FOLLOWER_ARCHIVE_INDEX_HEADROOM = 0x10000;
static const u32 FOLLOWER_MODEL_HEAP_SIZE = 0x368000 + FOLLOWER_ARCHIVE_INDEX_HEADROOM;
static const u32 FOLLOWER_SYSTEM_HEAP_SIZE = 0x40000 + FOLLOWER_ARCHIVE_INDEX_HEADROOM;
static const u32 FOLLOWER_POKEMODEL_HEAP_SIZE = 0x340000;
#if FOLLOWER_CARRIER_THREEGX
static const u32 FOLLOWER_REMOTE_REPLICA_MAX = 3;
// Leave room for model wrappers, load requests and the shared pool data.
static const u32 FOLLOWER_REMOTE_SYSTEM_HEAP_SIZE = 0x100000;
static const u32 FOLLOWER_REMOTE_REPLICA_RELEASE_FRAMES = 90;
static const f32 FOLLOWER_REMOTE_REPLICA_INTERPOLATION = 0.40f;
static const f32 FOLLOWER_REMOTE_REPLICA_TELEPORT_DISTANCE = 500.0f;
static const f32 FOLLOWER_REMOTE_REPLICA_PI = 3.14159265358979323846f;
static const f32 FOLLOWER_REMOTE_REPLICA_TWO_PI = 6.28318530717958647692f;
#endif
// Leave room for temporary allocations during animation loading and cleanup.
static const u32 FOLLOWER_DYNAMIC_EFFECT_MODEL_HEAP_RESERVE = 0x10000;
static const u32 FOLLOWER_SUPPRESSED_RETRY_FRAMES = 30;
#if FOLLOWER_3GX_INTERACTION_FEATURES
static const u32 FOLLOWER_INTERACTION_ARCHIVE_ID = 94;
static const u32 FOLLOWER_INTERACTION_ARCHIVE_MEMBER_STRIDE = 9;
static const u32 FOLLOWER_INTERACTION_KW_MEMBER_OFFSET = 6;
static const u32 FOLLOWER_INTERACTION_KW_RESPOND = 1;
static const u32 FOLLOWER_INTERACTION_KW_HAPPY_A = 11;
static const u32 FOLLOWER_INTERACTION_KW_HAPPY_B = 12;
static const u32 FOLLOWER_INTERACTION_KW_HAPPY_C = 13;
static const u16 FOLLOWER_INTERACTION_BINLINKER_SIGNATURE = 0x4350;
static const u32 FOLLOWER_INTERACTION_MAX_BINLINKER_ENTRY_COUNT = 64;
static const u32 FOLLOWER_INTERACTION_ANIMATION_PACK_RESOURCE_COUNT =
  FOLLOWER_INTERACTION_KW_HAPPY_C + 1;
// Leave room for decompression and animation nodes. We don't need to reserve the largest pack.
static const u32 FOLLOWER_INTERACTION_RESOURCE_HEADROOM = 0x016000;
static const u32 FOLLOWER_INTERACTION_MAX_PLAY_FRAMES = 600;
static const u32 FOLLOWER_INTERACTION_POSE_BLEND_FRAMES = 8;
static const u32 FOLLOWER_INTERACTION_POSE_BLEND_LONG_FRAMES = 18;
static const u32 FOLLOWER_INTERACTION_RETURN_BLEND_FRAMES = 10;
static const f32 FOLLOWER_INTERACTION_DISTANCE = 180.0f;
static const f32 FOLLOWER_INTERACTION_VERTICAL_DISTANCE = 120.0f;
static const f32 FOLLOWER_INTERACTION_BODY_REACH_RATE = 0.50f;
static const f32 FOLLOWER_INTERACTION_POSE_VERTICAL_THRESHOLD = 8.0f;
static const f32 FOLLOWER_INTERACTION_END_FRAME_EPSILON = 1.0f;
static const f32 FOLLOWER_INTERACTION_PI = 3.14159265358979323846f;
static const f32 FOLLOWER_INTERACTION_TWO_PI = 6.28318530717958647692f;
static const f32 FOLLOWER_INTERACTION_TURN_STEP = 0.10f;
static const u32 FOLLOWER_INTERACTION_EFFECT_DELAY_FRAMES = 8;
static const u32 FOLLOWER_INTERACTION_EFFECT_LIFETIME_FRAMES = 120;
static const u32 FOLLOWER_DITTO_SPECIES = 132;
static const u32 FOLLOWER_DITTO_CLONE_HEAP_SIZE = 0x40000;
static const u32 FOLLOWER_DITTO_CLONE_HOLD_FRAMES = 90;
static const u32 FOLLOWER_DITTO_SPIN_POSE_MOTION = 101;
static const u32 FOLLOWER_DITTO_SPIN_RECOVER_MOTION = 102;
static const u32 FOLLOWER_DITTO_CLONE_MOTION_MAX_FRAMES = 180;
static const u32 FOLLOWER_CASTFORM_SPECIES = 351;
static const u32 FOLLOWER_KYOGRE_SPECIES = 382;
static const u32 FOLLOWER_GROUDON_SPECIES = 383;
static const u32 FOLLOWER_JIRACHI_SPECIES = 385;
static const u32 FOLLOWER_VICTINI_SPECIES = 494;
static const u32 FOLLOWER_CELEBI_SPECIES = 251;
static const u32 FOLLOWER_DARKRAI_SPECIES = 491;
static const u32 FOLLOWER_SHAYMIN_SPECIES = 492;
static const u32 FOLLOWER_ZORUA_SPECIES = 570;
static const u32 FOLLOWER_ZOROARK_SPECIES = 571;
static const u32 FOLLOWER_HOOPA_SPECIES = 720;
static const u32 FOLLOWER_NECROZMA_SPECIES = 800;
static const u32 FOLLOWER_MARSHADOW_SPECIES = 802;
static const u32 FOLLOWER_SPECIES_ACTION_EFFECT_FRAMES = 30;
static const u32 FOLLOWER_ILLUSION_HOLD_FRAMES = 90;
static const u32 FOLLOWER_TIME_REWIND_PATH_FRAMES = 24;
static const u32 FOLLOWER_SHAYMIN_FLOWER_TRAIL_FRAMES = 300;
static const u32 FOLLOWER_SHAYMIN_FLOWER_INTERVAL_FRAMES = 36;
static const u32 FOLLOWER_WEATHER_LIFETIME_FRAMES = 450;
static const u32 FOLLOWER_CASTFORM_WEATHER_COUNT = 4;
static const pml::FormNo FOLLOWER_CASTFORM_FORM_NORMAL = 0;
static const pml::FormNo FOLLOWER_CASTFORM_FORM_SUN = 1;
static const pml::FormNo FOLLOWER_CASTFORM_FORM_RAIN = 2;
static const pml::FormNo FOLLOWER_CASTFORM_FORM_SNOW = 3;
static const pml::FormNo FOLLOWER_PRIMAL_FORM = 1;
#endif
#ifndef FOLLOWER_POKEMON_EVENT_WORK_SPRAY_COUNT_OFFSET
#define FOLLOWER_POKEMON_EVENT_WORK_SPRAY_COUNT_OFFSET 0x0a54
#endif
static const u32 FOLLOWER_EVENT_WORK_SPRAY_COUNT_OFFSET =
  FOLLOWER_POKEMON_EVENT_WORK_SPRAY_COUNT_OFFSET;
static const u16 FOLLOWER_REPEL_PREEMPT_STEPS = 8;

enum FollowerParentHeapSource
{
  FOLLOWER_PARENT_HEAP_NONE,
  FOLLOWER_PARENT_HEAP_FIELD_EXT,
  FOLLOWER_PARENT_HEAP_AREA_RESOURCE,
  FOLLOWER_PARENT_HEAP_FIELD,
  FOLLOWER_PARENT_HEAP_APP_DEVICE,
  FOLLOWER_PARENT_HEAP_EVENT_DEVICE,
#if FOLLOWER_CARRIER_THREEGX
  FOLLOWER_PARENT_HEAP_EXTERNAL_APPLICATION,
#endif
};

enum FollowerIdleReason
{
  FOLLOWER_IDLE_REASON_NONE,
  FOLLOWER_IDLE_REASON_EVENT_ACTIVE,
  FOLLOWER_IDLE_REASON_FIELDMAP_MISSING,
  FOLLOWER_IDLE_REASON_GAME_MANAGER_MISSING,
  FOLLOWER_IDLE_REASON_GAME_DATA_MISSING,
  FOLLOWER_IDLE_REASON_PARTY_MISSING,
  FOLLOWER_IDLE_REASON_PARTY_EMPTY,
  FOLLOWER_IDLE_REASON_NO_VALID_PARTY_MEMBER,
};

#if FOLLOWER_POKEMON_ENABLE_BALL_TRANSITION
struct FollowerDynamicEffectHeapSelection
{
  FollowerDynamicEffectHeapSelection()
  : pHeap( NULL )
  , source( Gen7Follower3gx::DYNAMIC_EFFECT_HEAP_NONE )
  , allocatableSize( 0 )
  , minimumFree( 0 )
  , candidateFound( false )
  {
  }

  gfl2::heap::HeapBase* pHeap;
  u32 source;
  u32 allocatableSize;
  u32 minimumFree;
  bool candidateFound;
};
#endif

struct FollowerShadowData
{
  u32 monsNo;
  u32 form;
  u32 type;
  f32 scale;
  f32 offsetX;
  f32 offsetZ;
};

struct FollowerShadowDataTable
{
  u32 count;
  FollowerShadowData data[1];
};

static_assert( sizeof(FollowerShadowData) == 0x18, "Follower shadow record ABI" );
static_assert( sizeof(FollowerShadowDataTable) == 0x1c, "Follower shadow table ABI" );

inline u16 GetRepelStepCount( Fieldmap* pFieldmap )
{
  GameSys::GameManager* pGameManager =
    pFieldmap ? pFieldmap->GetGameManager() : NULL;
  GameSys::GameData* pGameData =
    pGameManager ? FOLLOWER_POKEMON_GET_GAME_DATA( pGameManager ) : NULL;
  EventWork* pEventWork =
    pGameData ? FOLLOWER_POKEMON_GET_EVENT_WORK( pGameData ) : NULL;
  const u8* pCoreData = pEventWork
    ? reinterpret_cast<const u8*>( pEventWork->GetData() )
    : NULL;

  // Assumed Repel counter location. The surrounding fields still need checking.
  return pCoreData
    ? *reinterpret_cast<const u16*>(
        pCoreData + FOLLOWER_EVENT_WORK_SPRAY_COUNT_OFFSET
        )
    : 0;
}

inline bool IsRepelExpiryPending( Fieldmap* pFieldmap )
{
#if FOLLOWER_POKEMON_PREEMPT_REPEL_EXPIRY
  // Start cleanup before Repel runs out so its continuation event has enough memory.
  const u16 stepCount = GetRepelStepCount( pFieldmap );
  return stepCount > 0 && stepCount <= FOLLOWER_REPEL_PREEMPT_STEPS;
#else
  // Let the event gate handle cleanup. A low Repel counter that isn't ticking shouldn't keep the follower hidden.
  (void)pFieldmap;
  return false;
#endif
}

inline bool IsEventDeviceBorrowAllowed( Fieldmap* pFieldmap )
{
  GameSys::GameManager* pGameManager =
    pFieldmap ? pFieldmap->GetGameManager() : NULL;
  GameSys::GameEventManager* pEventManager =
    pGameManager
      ? FOLLOWER_POKEMON_GET_GAME_EVENT_MANAGER( pGameManager )
      : NULL;
  GameSys::GameEvent* pEvent =
    pEventManager ? pEventManager->GetGameEvent() : NULL;

  // Only borrow this memory when no game event is running.
  return pEvent == NULL;
}

inline const pml::pokepara::PokemonParam* FindFirstValidPartyPokemon(
  Fieldmap* pFieldmap,
  u32* pIdleReason = NULL
)
{
  if( pIdleReason ){ *pIdleReason = FOLLOWER_IDLE_REASON_NONE; }
  if( !pFieldmap )
  {
    if( pIdleReason ){ *pIdleReason = FOLLOWER_IDLE_REASON_FIELDMAP_MISSING; }
    return NULL;
  }

  GameSys::GameManager* pGameManager = pFieldmap->GetGameManager();
  if( !pGameManager )
  {
    if( pIdleReason ){ *pIdleReason = FOLLOWER_IDLE_REASON_GAME_MANAGER_MISSING; }
    return NULL;
  }

  GameSys::GameData* pGameData =
    FOLLOWER_POKEMON_GET_GAME_DATA( pGameManager );
  if( !pGameData )
  {
    if( pIdleReason ){ *pIdleReason = FOLLOWER_IDLE_REASON_GAME_DATA_MISSING; }
    return NULL;
  }

  const pml::PokeParty* pParty = pGameData->GetPlayerPartyConst();
  if( !pParty )
  {
    if( pIdleReason ){ *pIdleReason = FOLLOWER_IDLE_REASON_PARTY_MISSING; }
    return NULL;
  }

  const u32 memberCount = pParty->GetMemberCount();
  if( memberCount == 0 )
  {
    if( pIdleReason ){ *pIdleReason = FOLLOWER_IDLE_REASON_PARTY_EMPTY; }
    return NULL;
  }
#if FOLLOWER_CARRIER_DEDICATED_ARENA
  const u32 requestedSlot=Gen7Follower3gx::GetPartyFollowerSettings().slots[0];
#endif
  for( u32 i = 0; i < memberCount; ++i )
  {
#if FOLLOWER_CARRIER_DEDICATED_ARENA
    if( requestedSlot && requestedSlot!=i+1 ) continue;
#endif
    const pml::pokepara::PokemonParam* pPokemon = pParty->GetMemberPointerConst( i );
    if( !pPokemon ){ continue; }
    if( pPokemon->IsNull() ){ continue; }
    if( pPokemon->IsEgg( pml::pokepara::CHECK_BOTH_EGG ) ){ continue; }
    return pPokemon;
  }

  if( pIdleReason ){ *pIdleReason = FOLLOWER_IDLE_REASON_NO_VALID_PARTY_MEMBER; }
  return NULL;
}

inline gfl2::heap::HeapBase* GetFollowerWorkHeap()
{
#if FOLLOWER_CARRIER_DEDICATED_ARENA
  return static_cast<gfl2::heap::HeapBase*>(Follower3gx_GetDedicatedArenaHeap());
#else
  return gfl2::heap::Manager::GetHeapByHeapId( HEAPID_FILEREAD );
#endif
}

inline gfl2::heap::HeapBase* GetFollowerParentHeap(
  Fieldmap* pFieldmap,
  u32 requiredSize,
  u32* pSource,
  u32* pAllocatableSize
)
{
  if( pSource ){ *pSource = FOLLOWER_PARENT_HEAP_NONE; }
  if( pAllocatableSize ){ *pAllocatableSize = 0; }
  if( !pFieldmap ){ return NULL; }

#if FOLLOWER_CARRIER_DEDICATED_ARENA
  (void)requiredSize;
  auto* heap = static_cast<gfl2::heap::HeapBase*>(Follower3gx_GetDedicatedArenaHeap());
  if( pSource && heap ){ *pSource = FOLLOWER_PARENT_HEAP_EXTERNAL_APPLICATION; }
  if( pAllocatableSize && heap ){ *pAllocatableSize = heap->GetTotalAllocatableSize(); }
  return heap;
#else
  gfl2::heap::HeapBase* pBestHeap = NULL;
  u32 bestAllocatableSize = 0;

  // Leave the area-mode heap alone. Only borrow event memory while the player has field control.

#if FOLLOWER_POKEMON_USE_AREA_RESOURCE_HEAP
  GameSys::GameManager* pGameManager = pFieldmap->GetGameManager();
  GameSys::GameData* pGameData = pGameManager
    ? FOLLOWER_POKEMON_GET_GAME_DATA( pGameManager )
    : NULL;
  Area* pArea = pGameData ? pGameData->GetFieldArea() : NULL;
  if( pArea )
  {
    gfl2::heap::HeapBase* pAreaHeap = FOLLOWER_POKEMON_GET_AREA_RESOURCE_HEAP( pArea );
    if( pAreaHeap )
    {
      const u32 allocatableSize = pAreaHeap->GetTotalAllocatableSize();
      if( allocatableSize >= requiredSize )
      {
        if( pSource ){ *pSource = FOLLOWER_PARENT_HEAP_AREA_RESOURCE; }
        if( pAllocatableSize ){ *pAllocatableSize = allocatableSize; }
        return pAreaHeap;
      }
      if( allocatableSize > bestAllocatableSize )
      {
        pBestHeap = pAreaHeap;
        bestAllocatableSize = allocatableSize;
        if( pSource ){ *pSource = FOLLOWER_PARENT_HEAP_AREA_RESOURCE; }
        if( pAllocatableSize ){ *pAllocatableSize = allocatableSize; }
      }
    }
  }
#else
  gfl2::heap::HeapBase* pFieldHeap = pFieldmap->GetHeap();
  if( pFieldHeap )
  {
    const u32 allocatableSize = pFieldHeap->GetTotalAllocatableSize();
    if( allocatableSize >= requiredSize )
    {
      if( pSource ){ *pSource = FOLLOWER_PARENT_HEAP_FIELD; }
      if( pAllocatableSize ){ *pAllocatableSize = allocatableSize; }
      return pFieldHeap;
    }
    if( allocatableSize > bestAllocatableSize )
    {
      pBestHeap = pFieldHeap;
      bestAllocatableSize = allocatableSize;
      if( pSource ){ *pSource = FOLLOWER_PARENT_HEAP_FIELD; }
      if( pAllocatableSize ){ *pAllocatableSize = allocatableSize; }
    }
  }
#endif

  gfl2::heap::HeapBase* pAppDeviceHeap =
    gfl2::heap::Manager::GetHeapByHeapId( HEAPID_APP_DEVICE );
  if( pAppDeviceHeap )
  {
    const u32 allocatableSize = pAppDeviceHeap->GetTotalAllocatableSize();
    if( allocatableSize >= requiredSize )
    {
      if( pSource ){ *pSource = FOLLOWER_PARENT_HEAP_APP_DEVICE; }
      if( pAllocatableSize ){ *pAllocatableSize = allocatableSize; }
      return pAppDeviceHeap;
    }
    if( allocatableSize > bestAllocatableSize )
    {
      pBestHeap = pAppDeviceHeap;
      bestAllocatableSize = allocatableSize;
      if( pSource ){ *pSource = FOLLOWER_PARENT_HEAP_APP_DEVICE; }
      if( pAllocatableSize ){ *pAllocatableSize = allocatableSize; }
    }
  }

  return pBestHeap;
#endif
}

#if FOLLOWER_3GX_DIAGNOSTIC
inline void MeasureFollowerDiagnosticHeap(
  gfl2::heap::HeapBase* pHeap,
  Gen7Follower3gx::FollowerHeapDiagnosticMeasurement* pMeasurement
)
{
  if( !pMeasurement ){ return; }
  pMeasurement->totalSize = pHeap ? pHeap->GetTotalSize() : 0;
  pMeasurement->totalFree = pHeap ? pHeap->GetTotalFreeSize() : 0;
  pMeasurement->maximumAllocatable =
    pHeap ? pHeap->GetTotalAllocatableSize() : 0;
}

inline void RecordFollowerDiagnosticHeaps(
  Fieldmap* pFieldmap,
  gfl2::heap::HeapBase* pPokemonModelHeap,
  u32 parentSource,
  bool followerUsesEventHeap,
  bool captureBeforeFollower
)
{
  if( !pFieldmap ){ return; }

  Gen7Follower3gx::FollowerHeapDiagnosticSample sample = {};
  sample.parentSource = parentSource;
  sample.followerUsesEventHeap = followerUsesEventHeap ? 1U : 0U;

  GameSys::GameManager* pGameManager = pFieldmap->GetGameManager();
  GameSys::GameData* pGameData = pGameManager
    ? FOLLOWER_POKEMON_GET_GAME_DATA( pGameManager )
    : NULL;
  Area* pArea = pGameData ? pGameData->GetFieldArea() : NULL;
  gfl2::heap::HeapBase* pAreaHeap = pArea
    ? FOLLOWER_POKEMON_GET_AREA_RESOURCE_HEAP( pArea )
    : NULL;

  MeasureFollowerDiagnosticHeap(
    pAreaHeap,
    &sample.heaps[Gen7Follower3gx::FOLLOWER_HEAP_DIAGNOSTIC_AREA]
    );
  MeasureFollowerDiagnosticHeap(
    gfl2::heap::Manager::GetHeapByHeapId( HEAPID_APP_DEVICE ),
    &sample.heaps[Gen7Follower3gx::FOLLOWER_HEAP_DIAGNOSTIC_APP_DEVICE]
    );
  MeasureFollowerDiagnosticHeap(
    gfl2::heap::Manager::GetHeapByHeapId( HEAPID_EVENT_DEVICE ),
    &sample.heaps[Gen7Follower3gx::FOLLOWER_HEAP_DIAGNOSTIC_EVENT_DEVICE]
    );
  MeasureFollowerDiagnosticHeap(
    gfl2::heap::Manager::GetHeapByHeapId( HEAPID_DLL_LOAD ),
    &sample.heaps[Gen7Follower3gx::FOLLOWER_HEAP_DIAGNOSTIC_DLL]
    );
  MeasureFollowerDiagnosticHeap(
    pPokemonModelHeap,
    &sample.heaps[
      Gen7Follower3gx::FOLLOWER_HEAP_DIAGNOSTIC_POKEMON_MODEL
    ]
    );
  Gen7Follower3gx::RecordFollowerHeapDiagnostics(
    sample,
    captureBeforeFollower
    );
}
#endif

class Manager
{
public:
  Manager();

  void Update( Fieldmap* pFieldmap, MyRenderingPipeLine* pRenderingPipeLine, BaseCollisionScene* pTerrainWallScene, BaseCollisionScene* pStaticScene );
  bool Terminate( void );
  void SuspendForFieldEvent( Fieldmap* pFieldmap );
  void ResumeAfterFieldEvent( Fieldmap* pFieldmap );
#if FOLLOWER_3GX_INTERACTION_FEATURES
  bool TryStartInteraction( Fieldmap* pFieldmap );
  void RestoreRidePlayerVisibility( void );
  void ReportFieldEventMemory( Fieldmap* pFieldmap ) const;
#endif
#if FOLLOWER_POKEMON_ENABLE_BALL_TRANSITION
  void AdvanceBallTransitionWhileFieldPaused( void );
#endif
  u32 GetDiagnosticState( void ) const { return static_cast<u32>( m_State ); }
  u32 GetDiagnosticLoadFailure( void ) const { return m_DiagnosticLoadFailure; }
  u32 GetDiagnosticParentHeapSource( void ) const { return m_DiagnosticParentHeapSource; }
  u32 GetDiagnosticParentHeapAllocatableSize( void ) const { return m_DiagnosticParentHeapAllocatableSize; }
  u32 GetDiagnosticIdleReason( void ) const { return m_DiagnosticIdleReason; }
  u32 GetDiagnosticTerminateBlocker( void ) const;
  bool IsUsingEventDeviceHeap( void ) const
  {
#if FOLLOWER_POKEMON_ENABLE_BALL_TRANSITION
    gfl2::heap::HeapBase* pEventDeviceHeap =
      gfl2::heap::Manager::GetHeapByHeapId( HEAPID_EVENT_DEVICE );
    return m_UsesEventDeviceHeap ||
      (pEventDeviceHeap &&
       m_SpecialEffects.GetDynamicResourceHeap() == pEventDeviceHeap);
#else
    return m_UsesEventDeviceHeap;
#endif
  }
  bool CanRetainInteractionForFieldEvent( void ) const;
  void PrepareRetainedEventIdle( void );
  bool NeedsFieldEventResourceRelease( void ) const
  {
    return GetFieldEventResourceReleaseReasons()!=0;
  }
  u32 GetFieldEventResourceReleaseReasons( void ) const
  {
    u32 reasons=0;
#if FOLLOWER_3GX_INTERACTION_FEATURES
    if( m_MountedRideActive &&
        Gen7Follower3gx::GetMountedInteractionMode()!=Gen7Follower3gx::MOUNTED_INTERACTION_KEEP )
      reasons|=Gen7Follower3gx::EVENT_CLEANUP_RIDE_POLICY;
    if( m_MountedRideCleanup )
      reasons|=Gen7Follower3gx::EVENT_CLEANUP_RIDER_RETIRING;
    const bool retainInteraction = CanRetainInteractionForFieldEvent();
    if( !retainInteraction && (m_InteractionState != INTERACTION_STATE_NONE ||
        m_SpeciesActionKind != SPECIES_ACTION_NONE || m_DittoTransformState != DITTO_TRANSFORM_NONE) )
      reasons|=Gen7Follower3gx::EVENT_CLEANUP_INTERACTION;
    if( !retainInteraction && (m_pInteractionMotionPack || m_InteractionPackLoaded) )
      reasons|=Gen7Follower3gx::EVENT_CLEANUP_MOTION_PACK;
    if( m_WeatherFormChangePending || m_IllusionModelSwapPending ||
        m_State == STATE_WEATHER_FORM_DELETE_WAIT )
      reasons|=Gen7Follower3gx::EVENT_CLEANUP_FORM_CHANGE;
#endif
#if FOLLOWER_POKEMON_ENABLE_BALL_TRANSITION
    if( m_SpecialEffects.IsDynamicEffectBusy() )
      reasons|=Gen7Follower3gx::EVENT_CLEANUP_EFFECT;
#endif
    if( IsUsingEventDeviceHeap() )
      reasons|=Gen7Follower3gx::EVENT_CLEANUP_EVENT_HEAP;
#if FOLLOWER_POKEMON_RELEASE_INDEPENDENT_HEAP_FOR_FIELD_EVENTS && !FOLLOWER_CARRIER_DEDICATED_ARENA
    if( m_pFollowerHeap != NULL || m_pFactory != NULL || m_pTrialModel != NULL )
      reasons|=Gen7Follower3gx::EVENT_CLEANUP_REGULAR_POLICY;
#endif
    return reasons;
  }
  void UpdatePresentationAfterTraversal();
  gfl2::renderingengine::scenegraph::instance::ModelInstanceNode* GetFollowerModelInstanceNode( void ) const;
#if FOLLOWER_CARRIER_THREEGX
  bool GetNetworkState(
    Gen7Follower3gx::FollowerNetworkState* pState
    ) const;
#if FOLLOWER_CARRIER_DEDICATED_ARENA
  void UpdatePartyFollowers(Fieldmap*,BaseCollisionScene*,BaseCollisionScene*,BaseCollisionScene*);
  void ShowPartyFollowersForEvent(bool visible);
#endif
  void ConfigureRemoteReplicas(
    gfl2::heap::HeapBase* pExternalHeap,
    u32 replicaCapacity
    );
  void SetRemoteReplicaState(
    u32 replicaIndex,
    const Gen7Follower3gx::FollowerNetworkState& state
    );
  gfl2::renderingengine::scenegraph::instance::ModelInstanceNode*
    GetRemoteReplicaModelInstanceNode( u32 replicaIndex ) const;
  void GetRemoteReplicaDiagnostics(
    Gen7Follower3gx::FollowerReplicaDiagnostics* pDiagnostics
    ) const;
#endif
#if FOLLOWER_3GX_PERFORMANCE_FEATURES
  void SetPerformanceShadowVisible( bool visible );
  bool ShouldTraversePerformanceAnimation( void ) const
  {
    return m_AnimationUpdateThisFrame || m_HasInterpolatedAnimationPose;
  }
#endif

private:
#if FOLLOWER_CARRIER_THREEGX
  struct RemoteReplica
  {
    RemoteReplica()
    : requested(),
      simpleParam(),
      pTrialModel( NULL ),
      currentPosition( 0.0f, 0.0f, 0.0f ),
      currentYaw( 0.0f ),
      currentMotion( PokeTool::MODEL_ANIME_ERROR ),
      missingFrames( 0 ),
      retryFrames( 0 ),
      requestPresent( false ),
      modelCreated( false ),
      hasPose( false )
    {
    }

    Gen7Follower3gx::FollowerNetworkState requested;
    PokeTool::SimpleParam simpleParam;
    TrialModel::FieldTrialModel* pTrialModel;
    gfl2::math::Vector3 currentPosition;
    f32 currentYaw;
    s32 currentMotion;
    u32 missingFrames;
    u32 retryFrames;
    bool requestPresent;
    bool modelCreated;
    bool hasPose;
#if FOLLOWER_CARRIER_DEDICATED_ARENA
    Gen7Follower3gx::PartyFollowerMotion partyMotion;
#endif
  };
#endif

#if FOLLOWER_3GX_PERFORMANCE_FEATURES
  typedef gfl2::renderingengine::scenegraph::instance::JointLocalSrt
    JointLocalSrt;
#endif

  enum State
  {
    STATE_IDLE,
    STATE_LOAD_SUPPRESSED,
    STATE_INIT_WAIT,
    STATE_SYSTEM_CREATE_WAIT,
    STATE_MODEL_LOAD_WAIT,
    STATE_ACTIVE,
#if FOLLOWER_3GX_INTERACTION_FEATURES
    STATE_WEATHER_FORM_DELETE_WAIT,
#endif
    STATE_DELETE_WAIT,
    STATE_TERM_WAIT,
  };

  enum DiagnosticLoadFailure
  {
    DIAGNOSTIC_LOAD_FAILURE_NONE,
    DIAGNOSTIC_LOAD_FAILURE_PARENT_HEAP_MISSING,
    DIAGNOSTIC_LOAD_FAILURE_PARENT_HEAP_LOW,
    DIAGNOSTIC_LOAD_FAILURE_FOLLOWER_HEAP_CREATE,
    DIAGNOSTIC_LOAD_FAILURE_FACTORY_CREATE,
    DIAGNOSTIC_LOAD_FAILURE_MODEL_CREATE,
    DIAGNOSTIC_LOAD_FAILURE_EVENT_DEVICE_BUSY,
    DIAGNOSTIC_LOAD_FAILURE_EVENT_DEVICE_LOW,
    DIAGNOSTIC_LOAD_FAILURE_REPEL_EXPIRY_PENDING,
  };

#if FOLLOWER_3GX_INTERACTION_FEATURES
  enum InteractionState
  {
    INTERACTION_STATE_NONE,
    INTERACTION_STATE_LOADING,
    INTERACTION_STATE_READY,
    INTERACTION_STATE_PLAYING,
  };

  enum DittoTransformState
  {
    DITTO_TRANSFORM_NONE,
    DITTO_TRANSFORM_FLASH_IN,
    DITTO_TRANSFORM_WAIT_FLASH_IN_RELEASE,
    DITTO_TRANSFORM_CLONE_HOLD,
    DITTO_TRANSFORM_FLASH_OUT,
    DITTO_TRANSFORM_CLEANUP,
  };

  enum SpeciesActionKind
  {
    SPECIES_ACTION_NONE,
    SPECIES_ACTION_ILLUSION,
    SPECIES_ACTION_SHADOW_STEP,
    SPECIES_ACTION_TIME_REWIND,
    SPECIES_ACTION_PORTAL_BLINK,
    SPECIES_ACTION_FLOWER_TRAIL,
    SPECIES_ACTION_LIGHT_DRAIN,
  };

  enum SpeciesActionPhase
  {
    SPECIES_ACTION_PHASE_NONE,
    SPECIES_ACTION_PHASE_START,
    SPECIES_ACTION_PHASE_WAIT_EFFECT,
    SPECIES_ACTION_PHASE_ACTIVE,
    SPECIES_ACTION_PHASE_RETURN_EFFECT,
    SPECIES_ACTION_PHASE_WAIT_MODEL,
  };

  void ResetInteractionMembers( void );
  void PublishInteractionDiagnostics(
    Gen7Follower3gx::FollowerInteractionDiagnosticResult result,
    Gen7Follower3gx::FollowerInteractionDiagnosticMotion motion,
    bool incrementAttempt,
    bool incrementSuccess
    ) const;
  gfl2::animation::AnimationPackList*
    GetInteractionAnimationPackList( void );
  u32 GetInteractionAnimationDataIndex( void ) const;
  bool BeginInteractionMotionLoad( Fieldmap* pFieldmap );
  bool CompleteInteractionMotionLoad( Fieldmap* pFieldmap );
  bool ValidateInteractionMotionPack( void );
  bool StartPettingInteractionMotion( bool incrementAttempt );
  void StartFieldInteractionFallback(
    Gen7Follower3gx::FollowerInteractionDiagnosticResult reason,
    bool incrementAttempt
    );
  bool UpdateInteractionFacing(
    PokeTool::PokeModel* pPokeModel,
    const gfl2::math::Vector3& playerPosition
    );
  bool UpdateInteraction(
    Fieldmap* pFieldmap,
    BaseCollisionScene* pTerrainGroundScene
    );
  bool PrepareInteractionForTerminate( bool preserveSpeciesAction = false );
  void ResetSpeciesActionMembers( void );
  void CancelSpeciesAction( void );
  bool TryStartSpeciesAction( Fieldmap* pFieldmap, u32 species );
  bool UpdateSpeciesAction(
    Fieldmap* pFieldmap,
    BaseCollisionScene* pTerrainGroundScene
    );
  void FinishSpeciesAction( Fieldmap* pFieldmap );
  bool TryStartSpeciesActionEffect(
    PokeTool::PokeModel* pPokeModel,
    Fieldmap* pFieldmap,
    Effect::Type effectType,
    u32 lifetimeFrames,
    u32 placement,
    f32 scale = 1.0f
    );
  void SetSpeciesActionModelVisible( bool visible );
  void UpdateShayminFlowerTrail( Fieldmap* pFieldmap );
  bool SelectIllusionTarget(
    Fieldmap* pFieldmap,
    PokeTool::SimpleParam* pTarget
    ) const;
  bool BeginIllusionModelSwap(
    const PokeTool::SimpleParam& target,
    bool returning
    );
  void CompleteIllusionModelSwap( Fieldmap* pFieldmap );
  void ResetDittoTransformMembers( void );
  void PublishDittoTransformDiagnostics(
    Gen7Follower3gx::DittoTransformDiagnosticResult result,
    bool incrementAttempt,
    bool incrementSuccess
    ) const;
  bool PreparePlayerCloneAllocation( Fieldmap* pFieldmap );
  bool TryStartDittoTransform( Fieldmap* pFieldmap );
  bool TryStartMountedRide( Fieldmap* pFieldmap );
  bool UpdateMountedRide( Fieldmap* pFieldmap );
  bool UpdateCarriedPokemon( Fieldmap* pFieldmap );
  void StopMountedRide( void );
  void ResetPendingInteractionEffect( void );
  void QueueInteractionEffect(
    Effect::Type effectType,
    u32 lifetimeFrames,
    u32 placement
    );
  gfl2::math::Vector3 GetInteractionEffectPosition(
    PokeTool::PokeModel* pPokeModel,
    Effect::Type effectType,
    u32 placement
    ) const;
  bool GetSpeciesInteractionEffectDefinition(
    u32 species,
    Gen7Follower3gx::DiagnosticEffectDefinition* pDefinition
    ) const;
  bool TryStartPendingInteractionEffect(
    PokeTool::PokeModel* pPokeModel,
    Fieldmap* pFieldmap
    );
  void UpdatePendingInteractionEffect(
    PokeTool::PokeModel* pPokeModel,
    bool reactionAdvancing,
    Fieldmap* pFieldmap
    );
  bool BeginRawFollowerFormChange( pml::FormNo targetForm );
  bool TryCycleRawFollowerForm( void );
  bool TryCycleCastformWeather( Fieldmap* pFieldmap );
  bool TryStartPrimalWeather( Fieldmap* pFieldmap, u32 species );
  bool TryStartWeatherForm(
    Fieldmap* pFieldmap,
    weather::WeatherKind weatherKind,
    pml::FormNo targetForm,
    u8 battleWeather,
    bool advanceCastformCycle
    );
#if FOLLOWER_3GX_DIAGNOSTIC
  bool ProcessDiagnosticWeatherSelection( Fieldmap* pFieldmap );
  void PreserveDiagnosticWeatherSelection( void );
#endif
  bool BeginWeatherFormChange(
    pml::FormNo targetForm,
    bool playReaction
    );
  void ContinueWeatherFormChange( void );
  bool RestoreWeatherBaseFormAfterWeather( Fieldmap* pFieldmap );
  void ApplyWeatherFormOverride( PokeTool::SimpleParam* pParam );
  void CompleteWeatherFormChange( Fieldmap* pFieldmap );
  bool CreateDittoPlayerClone( bool hideFollower = true );
  bool StartDittoPlayerSpinReaction(
    poke_3d::model::BaseModel* pClone
    );
  bool UpdateDittoTransform(
    Fieldmap* pFieldmap,
    BaseCollisionScene* pTerrainGroundScene
    );
  bool ReleaseDittoPlayerClone( void );
  void FinishDittoTransform(
    Fieldmap* pFieldmap,
    bool completed
    );
  poke_3d::model::BaseModel* GetDittoPlayerCloneModel( void ) const;
#endif

  void ResetPointers( void );
  bool BeginSystemLoad( Fieldmap* pFieldmap, MyRenderingPipeLine* pRenderingPipeLine, BaseCollisionScene* pTerrainGroundScene );
  bool CreateFactorySystem( void );
  void BeginModelLoad( void );
  void CreateLoadedModel( Fieldmap* pFieldmap );
  bool BeginTerminate( bool playBallTransition = false );
  bool FinishTerminate( void );
  bool TryResumeSuppressedLoad( Fieldmap* pFieldmap );
  void SuppressLoadAttempts( DiagnosticLoadFailure failure );
  void DeleteFollowerHeap( void );
  void HideFollowerModel( void );
  void RetireFollowerEdgeTarget( void );
  bool IsPokeModelGpuIdle( void ) const;
  void SeedTrail( const gfl2::math::Vector3& playerPosition );
  void PushTrail( const gfl2::math::Vector3& playerPosition );
  gfl2::math::Vector3 GetTrailTarget( void ) const;
#if FOLLOWER_POKEMON_USE_TRAIL_POLICY
  gfl2::math::Vector3 GetDelayedTrailTarget( void ) const;
#endif
  PokeTool::PokeModel* GetPokeModel( void ) const;
  bool IsLocomotionMotion( PokeTool::MODEL_ANIME motion ) const;
  f32 GetFollowerAnimationGroundOffset(
    PokeTool::PokeModel* pPokeModel,
    f32* pRawRootY = NULL
    ) const;
  f32 GetFollowerRootMotionStep( PokeTool::PokeModel* pPokeModel ) const;
  void UpdateAnimationNominalRootSpeed( f32 rootMotionStep );
  f32 GetFollowerCollisionRadius( PokeTool::PokeModel* pPokeModel ) const;
  f32 GetFollowerPlayerBodyRadius( PokeTool::PokeModel* pPokeModel ) const;
  f32 GetFollowerIdleGapScale( PokeTool::PokeModel* pPokeModel ) const;
  void ConfigureFollowerShadow( void );
  void UpdateFollowerShadowScale( PokeTool::PokeModel* pPokeModel );
  gfl2::math::Vector3 GetDisplayPosition( PokeTool::PokeModel* pPokeModel ) const;
#if FOLLOWER_POKEMON_ENABLE_BALL_TRANSITION
  gfl2::math::Vector3 GetBallTransitionPosition( PokeTool::PokeModel* pPokeModel ) const;
  gfl2::heap::HeapBase* GetFollowerModelResourceHeap( void ) const;
  FollowerDynamicEffectHeapSelection SelectDynamicEffectResourceHeap(
    u32 minimumFree
    ) const;
  bool TryStartBallTransition( FollowerBallTransition::Kind kind );
  void UpdateBallTransition( void );
#endif
  void ApplyFollowerAnimationGroundOffset( PokeTool::PokeModel* pPokeModel ) const;
  BaseCollisionScene* GetTerrainGroundScene( Fieldmap* pFieldmap ) const;
  void EnforcePlayerSeparation( const gfl2::math::Vector3& playerPosition, f32 separationDistance, gfl2::math::Vector3* pTarget ) const;
  bool UpdateRunMotionState( f32 distance );
#if FOLLOWER_POKEMON_USE_TRAIL_POLICY
  void ResetTrailMovementPolicy( const gfl2::math::Vector3& playerPosition );
#endif
  void UpdateFollower( Fieldmap* pFieldmap, BaseCollisionScene* pTerrainGroundScene, BaseCollisionScene* pTerrainWallScene, BaseCollisionScene* pStaticScene );
#if FOLLOWER_3GX_PERFORMANCE_FEATURES
  bool TryApplyCachedGround( BaseCollisionScene* pTerrainGroundScene, gfl2::math::Vector3* pPosition ) const;
  void ResetInterpolatedAnimationPose( void );
  bool CaptureAnimationPose(
    gfl2::renderingengine::scenegraph::instance::ModelInstanceNode* pNode,
    JointLocalSrt* pPose,
    u32* pJointCount
    ) const;
  void BeginInterpolatedAnimationSample(
    gfl2::renderingengine::scenegraph::instance::ModelInstanceNode* pNode
    );
  void FinishInterpolatedAnimationSample(
    gfl2::renderingengine::scenegraph::instance::ModelInstanceNode* pNode,
    f32 weight
    );
  void ApplyInterpolatedAnimationPose(
    gfl2::renderingengine::scenegraph::instance::ModelInstanceNode* pNode,
    f32 weight
    );
#endif
  bool TryPlaceNearPlayer(const gfl2::math::Vector3& player, BaseCollisionScene* ground,
    BaseCollisionScene* walls, BaseCollisionScene* objects, f32 radius, f32 separation);
  bool ApplyGround( BaseCollisionScene* pTerrainGroundScene, gfl2::math::Vector3* pPosition ) const;
  bool IsSceneWallBlocked( BaseCollisionScene* pScene, const gfl2::math::Vector3& from, const gfl2::math::Vector3& to, f32 radius, bool checkMesh, bool checkShapes, bool centerMeshOnly ) const;
  bool IsWallBlocked( BaseCollisionScene* pTerrainWallScene, BaseCollisionScene* pStaticScene, const gfl2::math::Vector3& from, const gfl2::math::Vector3& to, f32 radius ) const;
  bool TryMove( BaseCollisionScene* pTerrainGroundScene, BaseCollisionScene* pTerrainWallScene, BaseCollisionScene* pStaticScene, const gfl2::math::Vector3& desiredPosition, f32 radius, gfl2::math::Vector3* pResolvedPosition ) const;
  bool ResolveMovement( BaseCollisionScene* pTerrainGroundScene, BaseCollisionScene* pTerrainWallScene, BaseCollisionScene* pStaticScene, const gfl2::math::Vector3& desiredPosition, f32 radius, gfl2::math::Vector3* pResolvedPosition ) const;
  void SetMotion( PokeTool::MODEL_ANIME motion );
#if FOLLOWER_CARRIER_THREEGX
  bool IsRemoteReplicaIdentityCurrent(
    const RemoteReplica& replica
    ) const;
  bool BeginRemoteReplicaLoad( RemoteReplica& replica );
  bool ReleaseRemoteReplica( RemoteReplica& replica );
  bool ReleaseRemoteReplicas( void );
  void UpdateRemoteReplicas( void );
  void SetRemoteReplicaMotion(
    RemoteReplica& replica,
    PokeTool::MODEL_ANIME motion
    );
  void ApplyRemoteReplicaGroundOffsets( void );
  void HideRemoteReplicas( void );
#endif

  State m_State;
  TrialModel::FieldModelPool* m_pFactory;
  TrialModel::FieldTrialModel* m_pTrialModel;
  MyRenderingPipeLine* m_pRenderingPipeLine;
  gfl2::heap::HeapBase* m_pFollowerHeap;
  gfl2::heap::HeapBase* m_pFollowerModelHeapParent;
#if FOLLOWER_CARRIER_THREEGX
  gfl2::heap::HeapBase* m_pRemoteReplicaExternalHeap;
  u32 m_RemoteReplicaCapacity;
  RemoteReplica m_RemoteReplicas[FOLLOWER_REMOTE_REPLICA_MAX];
#endif
#if FOLLOWER_POKEMON_ENABLE_BALL_TRANSITION
  Effect::EffectManager* m_pEffectManager;
  FollowerSpecialEffectController m_SpecialEffects;
  FollowerBallTransition m_BallTransition;
#endif
  PokeTool::SimpleParam m_SimpleParam;
  FollowerShadowDataTable m_FollowerShadowData;
  gfl2::math::Vector3 m_Position;
#if FOLLOWER_3GX_PERFORMANCE_FEATURES
  mutable gfl2::math::Vector4 m_CachedGroundPositions[3];
  mutable gfl2::math::Vector4 m_CachedGroundNormal;
  mutable BaseCollisionScene* m_pCachedGroundScene;
  mutable u32 m_CachedGroundUses;
  mutable bool m_HasCachedGround;
  JointLocalSrt m_InterpolatedAnimationStart[FOLLOWER_INTERPOLATED_POSE_MAX_JOINTS];
  JointLocalSrt m_InterpolatedAnimationTarget[FOLLOWER_INTERPOLATED_POSE_MAX_JOINTS];
  gfl2::renderingengine::scenegraph::instance::ModelInstanceNode*
    m_pInterpolatedAnimationNode;
  u32 m_AnimationUpdatePhase;
  u32 m_AnimationRateMode;
  u32 m_InterpolatedAnimationJointCount;
  bool m_AnimationUpdateThisFrame;
  bool m_InterpolatedAnimationSampleStarted;
  bool m_HasInterpolatedAnimationPose;
#endif
  gfl2::math::Vector3 m_FieldEventStartPlayerPosition;
  gfl2::math::Vector3 m_Trail[FOLLOWER_TRAIL_COUNT];
  u32 m_TrailHead;
  u32 m_TrailCount;
  u32 m_RunTransitionFrames;
  u32 m_NoMoveFrames;
  u32 m_BlockedFollowFrames = 0;
  f32 m_FollowSpeed = 0.0f;
  bool m_PlacementPending = true;
  s32 m_CurrentMotion;
  f32 m_AnimationStepFrame;
  f32 m_NominalRootSpeed;
  f32 m_WalkNominalRootSpeed;
  f32 m_RunNominalRootSpeed;
  f32 m_RootMotionSampleDistance;
  f32 m_RootMotionSampleFrames;
  f32 m_CollisionPlaybackRatio;
  f32 m_ModelFacingYaw;
  u32 m_DiagnosticLoadFailure;
  u32 m_DiagnosticParentHeapSource;
  u32 m_DiagnosticParentHeapAllocatableSize;
  u32 m_DiagnosticIdleReason;
  u32 m_SuppressedRetryFrames;
  bool m_UsesEventDeviceHeap;
  bool m_RunMode;
#if FOLLOWER_POKEMON_USE_TRAIL_POLICY
  TrailMovementPolicy::State m_TrailMovementState;
  gfl2::math::Vector3 m_TrailPreviousPlayerPosition;
  u32 m_TrailPlayerMovingFrames;
  f32 m_TrailFacingX;
  f32 m_TrailFacingZ;
  bool m_TrailHasPreviousPlayerPosition;
  bool m_TrailHasFacingDirection;
#endif
  bool m_IsFactoryInitialized;
  bool m_IsFactorySystemCreated;
  bool m_IsTrialModelCreated;
  bool m_ShouldSuppressAfterTerminate;
  bool m_HasSimpleParam;
  bool m_IsFieldEventSuspended;
  bool m_HasFieldEventStartPlayerPosition;
#if FOLLOWER_3GX_DIAGNOSTIC
  u32 m_HeapDiagnosticFrames;
#endif
#if FOLLOWER_3GX_INTERACTION_FEATURES
  InteractionState m_InteractionState;
  gfl2::animation::AnimationPackList m_InteractionAnimationPackList;
  gfl2::fs::AsyncFileManager* m_pInteractionFileManager;
  gfl2::heap::HeapBase* m_pInteractionResourceHeap;
  void* m_pInteractionMotionPack;
  u32 m_InteractionBufferSize;
  u32 m_InteractionRealSize;
  u32 m_InteractionDataId;
  u32 m_InteractionHeapFree;
  u32 m_InteractionResourceCount;
  u32 m_TalkReaction;
  f32 m_TalkHopOffset;
  u32 m_InteractionPlayingFrames;
  u32 m_InteractionDiagnosticResult;
  u32 m_InteractionSelectedMotion;
  u32 m_InteractionRandomState;
  u32 m_InteractionPoseBlendFrame;
  u32 m_InteractionPoseBlendFrames;
  u32 m_InteractionReturnGroundBlendFrame;
  f32 m_InteractionPoseBlendWeight;
  f32 m_InteractionPoseBlendStartGroundOffset;
  f32 m_InteractionReturnGroundBlendStart;
  f32 m_InteractionFacingYaw;
  bool m_InteractionRespondAvailable;
  u32 m_InteractionHappyMask;
  bool m_InteractionPackLoaded;
  bool m_InteractionPlayingExternal;
  bool m_InteractionPreferHappy;
  bool m_InteractionCancelPending;
  bool m_InteractionFacingComplete;
  bool m_InteractionCryPending;
  bool m_InteractionPoseBlendCapturePending;
  bool m_InteractionPoseBlendActive;
  bool m_InteractionReturnGroundBlendActive;
  u32 m_InteractionEffectType;
  u32 m_InteractionEffectLifetimeFrames;
  u32 m_InteractionEffectDelayFrames;
  u32 m_InteractionEffectElapsedFrames;
  u32 m_InteractionEffectPlacement;
  f32 m_InteractionEffectScale;
  bool m_InteractionEffectPending;
  bool m_VictiniLuckActivationPending;
  SpeciesActionKind m_SpeciesActionKind;
  SpeciesActionPhase m_SpeciesActionPhase;
  gfl2::math::Vector3 m_SpeciesActionOrigin;
  u32 m_SpeciesActionFrame;
  u32 m_SpeciesActionTrailHead;
  u32 m_SpeciesActionTrailCount;
  u32 m_ShayminFlowerTrailFrames;
  u32 m_ShayminFlowerIntervalFrames;
  bool m_ShayminFlowerAlternate;
  bool m_SpeciesActionModelHidden;
  bool m_NecrozmaLightDrainActive;
  PokeTool::SimpleParam m_IllusionOriginalParam;
  PokeTool::SimpleParam m_IllusionTargetParam;
  bool m_IllusionOverrideActive;
  bool m_IllusionModelSwapPending;
  bool m_IllusionReturning;
  bool m_MountedRideActive;
  bool m_MountedRideCleanup;
  poke_3d::model::BaseModel* m_pRideHiddenPlayer;
  bool m_RidePlayerWasVisible;
  RiderPoseStabilizer<
    gfl2::renderingengine::scenegraph::instance::ModelInstanceNode,
    gfl2::renderingengine::scenegraph::instance::JointLocalSrt> m_RiderStabilizer;
  Gen7Follower3gx::RideProfile m_RideProfile;
  Gen7Follower3gx::RiderBoneControl<
    gfl2::renderingengine::scenegraph::instance::ModelInstanceNode,
    gfl2::renderingengine::scenegraph::instance::JointLocalSrt> m_RiderBoneControl;
  int m_RideAttachmentJoint, m_RidePelvisJoint;
  u32 m_RideMotion;
  Gen7Follower3gx::RiderAnimationStyle m_RideStyle;
  void* m_pRideMotionData;
  gfl2::animation::AnimationPackList m_RideMotionPack;
  bool m_RideMotionPackLoaded;
  gfl2::math::Vector3 m_RidePreviousPosition;
  DittoTransformState m_DittoTransformState;
  Field::MoveModel::FieldMoveModelManager* m_pDittoMoveModelManager;
  gfl2::heap::HeapBase* m_pDittoCloneHeap;
  void* m_pDittoDressUpParam;
  u32 m_DittoPlayerCharacterId;
  u32 m_DittoModelId;
  u32 m_DittoHoldFrames;
  u32 m_DittoParentHeapFree;
  u32 m_DittoCloneHeapFree;
  u32 m_DittoDiagnosticResult;
  u32 m_DittoCloneMotionId;
  Gen7Follower3gx::DittoCloneMotionPhase m_DittoCloneMotionPhase;
  bool m_DittoRevealPending;
  bool m_DittoWorkReserved;
  bool m_DittoResourceCreated;
  u32 m_ManualFormSpecies;
  pml::FormNo m_ManualForm;
  pml::FormNo m_ManualFormPending;
  bool m_ManualFormOverrideActive;
  bool m_ManualFormEffectPending;
  u32 m_CastformWeatherIndex;
  u32 m_WeatherFormSpecies;
  pml::FormNo m_WeatherBaseForm;
  pml::FormNo m_WeatherTargetForm;
  bool m_WeatherFormOverrideActive;
  bool m_WeatherFormActive;
  bool m_WeatherFormChangePending;
  bool m_WeatherReactionPending;
#endif
};

#include "RuntimeLifecycle.inl"

inline void Manager::SeedTrail( const gfl2::math::Vector3& playerPosition )
{
  for( u32 i = 0; i < FOLLOWER_TRAIL_COUNT; ++i )
  {
    m_Trail[i] = playerPosition;
  }
  m_TrailHead = 0;
  m_TrailCount = FOLLOWER_TRAIL_COUNT;
}

inline void Manager::PushTrail( const gfl2::math::Vector3& playerPosition )
{
  m_TrailHead = ( m_TrailHead + 1 ) % FOLLOWER_TRAIL_COUNT;
  m_Trail[m_TrailHead] = playerPosition;
  if( m_TrailCount < FOLLOWER_TRAIL_COUNT )
  {
    ++m_TrailCount;
  }
}

inline gfl2::math::Vector3 Manager::GetTrailTarget( void ) const
{
  if( m_TrailCount == 0 )
  {
    return m_Position;
  }

  u32 delay = FOLLOWER_TRAIL_DELAY;
  if( delay >= m_TrailCount )
  {
    delay = m_TrailCount - 1;
  }

  const u32 index = ( m_TrailHead + FOLLOWER_TRAIL_COUNT - delay ) % FOLLOWER_TRAIL_COUNT;
  return m_Trail[index];
}

#if FOLLOWER_POKEMON_USE_TRAIL_POLICY
inline gfl2::math::Vector3 Manager::GetDelayedTrailTarget( void ) const
{
  if( m_TrailCount == 0 )
  {
    return m_Position;
  }

  u32 delay = TrailMovementPolicy::TRAIL_DELAY_FRAMES;
#if FOLLOWER_CARRIER_THREEGX
  delay = Gen7Follower3gx::GetFollowerTrailDelayFrames();
#endif
  if( delay >= m_TrailCount )
  {
    delay = m_TrailCount - 1;
  }

  const u32 index =
    ( m_TrailHead + FOLLOWER_TRAIL_COUNT - delay ) % FOLLOWER_TRAIL_COUNT;
  return m_Trail[index];
}
#endif

inline PokeTool::PokeModel* Manager::GetPokeModel( void ) const
{
  if( !m_pTrialModel ){ return NULL; }
  return m_pTrialModel->GetPokeModel();
}

#if FOLLOWER_3GX_INTERACTION_FEATURES
inline gfl2::animation::AnimationPackList*
Manager::GetInteractionAnimationPackList( void )
{
  return &m_InteractionAnimationPackList;
}

#include "RuntimeSpeciesActions.inl"

#include "RuntimeInteractionMotion.inl"

#include "MountedRide.inl"

#include "RuntimePlayerClone.inl"

#include "RuntimeWeatherForms.inl"

inline bool Manager::GetSpeciesInteractionEffectDefinition(
  u32 species,
  Gen7Follower3gx::DiagnosticEffectDefinition* pDefinition
) const
{
  if( !pDefinition )
  {
    return false;
  }

  struct SpeciesEffectEntry
  {
    u32 species;
    Effect::Type effectType;
    u32 placement;
  };

  static const SpeciesEffectEntry SPECIES_EFFECTS[] =
  {
    // Kanto
    { 144, Effect::EFFECT_TYPE_DEMO_FOG,
      Gen7Follower3gx::DIAGNOSTIC_EFFECT_PLACEMENT_GROUND },       // Articuno
    { 145, Effect::EFFECT_TYPE_DEMO_FIREWORK_YELLOW,
      Gen7Follower3gx::DIAGNOSTIC_EFFECT_PLACEMENT_MID_BODY },     // Zapdos
    { 146, Effect::EFFECT_TYPE_FESTIVAL_FIRE,
      Gen7Follower3gx::DIAGNOSTIC_EFFECT_PLACEMENT_GROUND },       // Moltres
    { 150, Effect::EFFECT_TYPE_DEMO_FIREWORK_PURPLE,
      Gen7Follower3gx::DIAGNOSTIC_EFFECT_PLACEMENT_MID_BODY },     // Mewtwo
    { 151, Effect::EFFECT_TYPE_DEMO_FLOWER_PINK,
      Gen7Follower3gx::DIAGNOSTIC_EFFECT_PLACEMENT_MID_BODY },     // Mew

    // Johto
    { 243, Effect::EFFECT_TYPE_DEMO_FIREWORK_YELLOW,
      Gen7Follower3gx::DIAGNOSTIC_EFFECT_PLACEMENT_MID_BODY },     // Raikou
    { 244, Effect::EFFECT_TYPE_FESTIVAL_FIRE,
      Gen7Follower3gx::DIAGNOSTIC_EFFECT_PLACEMENT_GROUND },       // Entei
    { 245, Effect::EFFECT_TYPE_DEMO_TRIAL2,
      Gen7Follower3gx::DIAGNOSTIC_EFFECT_PLACEMENT_GROUND },       // Suicune
    { 249, Effect::EFFECT_TYPE_DEMO_RIDE,
      Gen7Follower3gx::DIAGNOSTIC_EFFECT_PLACEMENT_MID_BODY },     // Lugia
    { 250, Effect::EFFECT_TYPE_DEMO_FLOWER_YELLOW,
      Gen7Follower3gx::DIAGNOSTIC_EFFECT_PLACEMENT_MID_BODY },     // Ho-Oh
    { 251, Effect::EFFECT_TYPE_DEMO_FLOWER_YELLOW,
      Gen7Follower3gx::DIAGNOSTIC_EFFECT_PLACEMENT_MID_BODY },     // Celebi

    // Hoenn
    { 377, Effect::EFFECT_TYPE_KAIRIKY_ROCK_DOWN,
      Gen7Follower3gx::DIAGNOSTIC_EFFECT_PLACEMENT_GROUND },       // Regirock
    { 378, Effect::EFFECT_TYPE_DEMO_FOG,
      Gen7Follower3gx::DIAGNOSTIC_EFFECT_PLACEMENT_GROUND },       // Regice
    { 379, Effect::EFFECT_TYPE_BAG_EFFECT,
      Gen7Follower3gx::DIAGNOSTIC_EFFECT_PLACEMENT_MID_BODY },     // Registeel
    { 380, Effect::EFFECT_TYPE_DEMO_FIREWORK_PINK,
      Gen7Follower3gx::DIAGNOSTIC_EFFECT_PLACEMENT_MID_BODY },     // Latias
    { 381, Effect::EFFECT_TYPE_DEMO_RIDE,
      Gen7Follower3gx::DIAGNOSTIC_EFFECT_PLACEMENT_MID_BODY },     // Latios
    { 382, Effect::EFFECT_TYPE_DEMO_TRIAL2,
      Gen7Follower3gx::DIAGNOSTIC_EFFECT_PLACEMENT_GROUND },       // Kyogre fallback
    { 383, Effect::EFFECT_TYPE_FESTIVAL_FIRE,
      Gen7Follower3gx::DIAGNOSTIC_EFFECT_PLACEMENT_GROUND },       // Groudon fallback
    { 384, Effect::EFFECT_TYPE_DEMO_RIDE,
      Gen7Follower3gx::DIAGNOSTIC_EFFECT_PLACEMENT_MID_BODY },     // Rayquaza
    { FOLLOWER_JIRACHI_SPECIES, Effect::EFFECT_TYPE_DEMO_FIREWORK_YELLOW,
      Gen7Follower3gx::DIAGNOSTIC_EFFECT_PLACEMENT_MID_BODY },     // Jirachi
    { 386, Effect::EFFECT_TYPE_DEMO_FIREWORK_PURPLE,
      Gen7Follower3gx::DIAGNOSTIC_EFFECT_PLACEMENT_MID_BODY },     // Deoxys

    // Sinnoh
    { 480, Effect::EFFECT_TYPE_DEMO_FLOWER_YELLOW,
      Gen7Follower3gx::DIAGNOSTIC_EFFECT_PLACEMENT_MID_BODY },     // Uxie
    { 481, Effect::EFFECT_TYPE_DEMO_FLOWER_PINK,
      Gen7Follower3gx::DIAGNOSTIC_EFFECT_PLACEMENT_MID_BODY },     // Mesprit
    { 482, Effect::EFFECT_TYPE_DEMO_FIREWORK_RED,
      Gen7Follower3gx::DIAGNOSTIC_EFFECT_PLACEMENT_MID_BODY },     // Azelf
    { 483, Effect::EFFECT_TYPE_DEMO_FLARE_SUN,
      Gen7Follower3gx::DIAGNOSTIC_EFFECT_PLACEMENT_MID_BODY },     // Dialga
    { 484, Effect::EFFECT_TYPE_DEMO_FIREWORK_PURPLE,
      Gen7Follower3gx::DIAGNOSTIC_EFFECT_PLACEMENT_MID_BODY },     // Palkia
    { 485, Effect::EFFECT_TYPE_FESTIVAL_FIRE,
      Gen7Follower3gx::DIAGNOSTIC_EFFECT_PLACEMENT_GROUND },       // Heatran
    { 486, Effect::EFFECT_TYPE_KAIRIKY_ROCK_DOWN,
      Gen7Follower3gx::DIAGNOSTIC_EFFECT_PLACEMENT_GROUND },       // Regigigas
    { 487, Effect::EFFECT_TYPE_DEMO_FOG,
      Gen7Follower3gx::DIAGNOSTIC_EFFECT_PLACEMENT_GROUND },       // Giratina
    { 488, Effect::EFFECT_TYPE_DEMO_FLARE_MOON,
      Gen7Follower3gx::DIAGNOSTIC_EFFECT_PLACEMENT_MID_BODY },     // Cresselia
    { 489, Effect::EFFECT_TYPE_DEMO_TRIAL2,
      Gen7Follower3gx::DIAGNOSTIC_EFFECT_PLACEMENT_GROUND },       // Phione
    { 490, Effect::EFFECT_TYPE_DEMO_TRIAL2,
      Gen7Follower3gx::DIAGNOSTIC_EFFECT_PLACEMENT_GROUND },       // Manaphy
    { 491, Effect::EFFECT_TYPE_DEMO_FOG,
      Gen7Follower3gx::DIAGNOSTIC_EFFECT_PLACEMENT_GROUND },       // Darkrai
    { 492, Effect::EFFECT_TYPE_DEMO_FLOWER_PINK,
      Gen7Follower3gx::DIAGNOSTIC_EFFECT_PLACEMENT_MID_BODY },     // Shaymin
    { 493, Effect::EFFECT_TYPE_DEMO_FLARE_SUN,
      Gen7Follower3gx::DIAGNOSTIC_EFFECT_PLACEMENT_MID_BODY },     // Arceus

    // Unova
    { FOLLOWER_VICTINI_SPECIES, Effect::EFFECT_TYPE_DEMO_FIREWORK_YELLOW,
      Gen7Follower3gx::DIAGNOSTIC_EFFECT_PLACEMENT_MID_BODY },     // Victini
    { 570, Effect::EFFECT_TYPE_KAIRIKY_ROCK_SMOKE,
      Gen7Follower3gx::DIAGNOSTIC_EFFECT_PLACEMENT_GROUND },       // Zorua
    { 571, Effect::EFFECT_TYPE_KAIRIKY_ROCK_SMOKE,
      Gen7Follower3gx::DIAGNOSTIC_EFFECT_PLACEMENT_GROUND },       // Zoroark
    { 638, Effect::EFFECT_TYPE_DEMO_RIDE,
      Gen7Follower3gx::DIAGNOSTIC_EFFECT_PLACEMENT_MID_BODY },     // Cobalion
    { 639, Effect::EFFECT_TYPE_KAIRIKY_ROCK_DOWN,
      Gen7Follower3gx::DIAGNOSTIC_EFFECT_PLACEMENT_GROUND },       // Terrakion
    { 640, Effect::EFFECT_TYPE_DEMO_FLOWER_YELLOW,
      Gen7Follower3gx::DIAGNOSTIC_EFFECT_PLACEMENT_MID_BODY },     // Virizion
    { 641, Effect::EFFECT_TYPE_DEMO_RIDE,
      Gen7Follower3gx::DIAGNOSTIC_EFFECT_PLACEMENT_MID_BODY },     // Tornadus
    { 642, Effect::EFFECT_TYPE_DEMO_FIREWORK_YELLOW,
      Gen7Follower3gx::DIAGNOSTIC_EFFECT_PLACEMENT_MID_BODY },     // Thundurus
    { 643, Effect::EFFECT_TYPE_DEMO_FLARE_SUN,
      Gen7Follower3gx::DIAGNOSTIC_EFFECT_PLACEMENT_MID_BODY },     // Reshiram
    { 644, Effect::EFFECT_TYPE_DEMO_FIREWORK_PURPLE,
      Gen7Follower3gx::DIAGNOSTIC_EFFECT_PLACEMENT_MID_BODY },     // Zekrom
    { 645, Effect::EFFECT_TYPE_KAIRIKY_ROCK_DOWN,
      Gen7Follower3gx::DIAGNOSTIC_EFFECT_PLACEMENT_GROUND },       // Landorus
    { 646, Effect::EFFECT_TYPE_DEMO_FOG,
      Gen7Follower3gx::DIAGNOSTIC_EFFECT_PLACEMENT_GROUND },       // Kyurem
    { 647, Effect::EFFECT_TYPE_DEMO_TRIAL2,
      Gen7Follower3gx::DIAGNOSTIC_EFFECT_PLACEMENT_GROUND },       // Keldeo
    { 648, Effect::EFFECT_TYPE_DEMO_FIREWORK_PINK,
      Gen7Follower3gx::DIAGNOSTIC_EFFECT_PLACEMENT_MID_BODY },     // Meloetta
    { 649, Effect::EFFECT_TYPE_DEMO_FIREWORK_PURPLE,
      Gen7Follower3gx::DIAGNOSTIC_EFFECT_PLACEMENT_MID_BODY },     // Genesect

    // Kalos
    { 716, Effect::EFFECT_TYPE_DEMO_FLOWER_YELLOW,
      Gen7Follower3gx::DIAGNOSTIC_EFFECT_PLACEMENT_MID_BODY },     // Xerneas
    { 717, Effect::EFFECT_TYPE_DEMO_FIREWORK_RED,
      Gen7Follower3gx::DIAGNOSTIC_EFFECT_PLACEMENT_MID_BODY },     // Yveltal
    { 718, Effect::EFFECT_TYPE_KAIRIKY_ROCK_DOWN,
      Gen7Follower3gx::DIAGNOSTIC_EFFECT_PLACEMENT_GROUND },       // Zygarde
    { 719, Effect::EFFECT_TYPE_DEMO_FIREWORK_PINK,
      Gen7Follower3gx::DIAGNOSTIC_EFFECT_PLACEMENT_MID_BODY },     // Diancie
    { 720, Effect::EFFECT_TYPE_DEMO_FIREWORK_PURPLE,
      Gen7Follower3gx::DIAGNOSTIC_EFFECT_PLACEMENT_MID_BODY },     // Hoopa
    { 721, Effect::EFFECT_TYPE_FESTIVAL_FIRE,
      Gen7Follower3gx::DIAGNOSTIC_EFFECT_PLACEMENT_GROUND },       // Volcanion

    // Alola
    { 772, Effect::EFFECT_TYPE_BAG_EFFECT,
      Gen7Follower3gx::DIAGNOSTIC_EFFECT_PLACEMENT_MID_BODY },     // Type: Null
    { 773, Effect::EFFECT_TYPE_DEMO_RIDE,
      Gen7Follower3gx::DIAGNOSTIC_EFFECT_PLACEMENT_MID_BODY },     // Silvally
    { 785, Effect::EFFECT_TYPE_DEMO_FIREWORK_YELLOW,
      Gen7Follower3gx::DIAGNOSTIC_EFFECT_PLACEMENT_MID_BODY },     // Tapu Koko
    { 786, Effect::EFFECT_TYPE_DEMO_FLOWER_PINK,
      Gen7Follower3gx::DIAGNOSTIC_EFFECT_PLACEMENT_MID_BODY },     // Tapu Lele
    { 787, Effect::EFFECT_TYPE_DEMO_FIREWORK_RED,
      Gen7Follower3gx::DIAGNOSTIC_EFFECT_PLACEMENT_MID_BODY },     // Tapu Bulu
    { 788, Effect::EFFECT_TYPE_DEMO_TRIAL2,
      Gen7Follower3gx::DIAGNOSTIC_EFFECT_PLACEMENT_GROUND },       // Tapu Fini
    { 789, Effect::EFFECT_TYPE_DEMO_FIREWORK_PURPLE,
      Gen7Follower3gx::DIAGNOSTIC_EFFECT_PLACEMENT_MID_BODY },     // Cosmog
    { 790, Effect::EFFECT_TYPE_BAG_EFFECT,
      Gen7Follower3gx::DIAGNOSTIC_EFFECT_PLACEMENT_MID_BODY },     // Cosmoem
    { 791, Effect::EFFECT_TYPE_DEMO_FLARE_SUN,
      Gen7Follower3gx::DIAGNOSTIC_EFFECT_PLACEMENT_MID_BODY },     // Solgaleo
    { 792, Effect::EFFECT_TYPE_DEMO_FLARE_MOON,
      Gen7Follower3gx::DIAGNOSTIC_EFFECT_PLACEMENT_MID_BODY },     // Lunala
    { 793, Effect::EFFECT_TYPE_DEMO_FOG,
      Gen7Follower3gx::DIAGNOSTIC_EFFECT_PLACEMENT_GROUND },       // Nihilego
    { 794, Effect::EFFECT_TYPE_KAIRIKY_ROCK_DOWN,
      Gen7Follower3gx::DIAGNOSTIC_EFFECT_PLACEMENT_GROUND },       // Buzzwole
    { 795, Effect::EFFECT_TYPE_DEMO_RIDE,
      Gen7Follower3gx::DIAGNOSTIC_EFFECT_PLACEMENT_MID_BODY },     // Pheromosa
    { 796, Effect::EFFECT_TYPE_DEMO_FIREWORK_YELLOW,
      Gen7Follower3gx::DIAGNOSTIC_EFFECT_PLACEMENT_MID_BODY },     // Xurkitree
    { 797, Effect::EFFECT_TYPE_KAIRIKY_ROCK_SMOKE,
      Gen7Follower3gx::DIAGNOSTIC_EFFECT_PLACEMENT_GROUND },       // Celesteela
    { 798, Effect::EFFECT_TYPE_DEMO_RIDE,
      Gen7Follower3gx::DIAGNOSTIC_EFFECT_PLACEMENT_MID_BODY },     // Kartana
    { 799, Effect::EFFECT_TYPE_DEMO_FIREWORK_PURPLE,
      Gen7Follower3gx::DIAGNOSTIC_EFFECT_PLACEMENT_MID_BODY },     // Guzzlord
    { 800, Effect::EFFECT_TYPE_DEMO_FIREWORK_PURPLE,
      Gen7Follower3gx::DIAGNOSTIC_EFFECT_PLACEMENT_MID_BODY },     // Necrozma
    { 801, Effect::EFFECT_TYPE_BAG_EFFECT,
      Gen7Follower3gx::DIAGNOSTIC_EFFECT_PLACEMENT_MID_BODY },     // Magearna
    { 802, Effect::EFFECT_TYPE_DEMO_FOG,
      Gen7Follower3gx::DIAGNOSTIC_EFFECT_PLACEMENT_GROUND },       // Marshadow
    { 803, Effect::EFFECT_TYPE_DEMO_FIREWORK_PURPLE,
      Gen7Follower3gx::DIAGNOSTIC_EFFECT_PLACEMENT_MID_BODY },     // Poipole
    { 804, Effect::EFFECT_TYPE_DEMO_FIREWORK_PURPLE,
      Gen7Follower3gx::DIAGNOSTIC_EFFECT_PLACEMENT_MID_BODY },     // Naganadel
    { 805, Effect::EFFECT_TYPE_KAIRIKY_ROCK_DOWN,
      Gen7Follower3gx::DIAGNOSTIC_EFFECT_PLACEMENT_GROUND },       // Stakataka
    { 806, Effect::EFFECT_TYPE_DEMO_FIREWORK_RED,
      Gen7Follower3gx::DIAGNOSTIC_EFFECT_PLACEMENT_MID_BODY },     // Blacephalon
    { 807, Effect::EFFECT_TYPE_DEMO_FIREWORK_YELLOW,
      Gen7Follower3gx::DIAGNOSTIC_EFFECT_PLACEMENT_MID_BODY },     // Zeraora
  };

  const u32 effectCount = sizeof(SPECIES_EFFECTS) /
    sizeof(SPECIES_EFFECTS[0]);
  for( u32 i = 0; i < effectCount; ++i )
  {
    if( SPECIES_EFFECTS[i].species != species )
    {
      continue;
    }

    pDefinition->effectType = static_cast<u32>(
      SPECIES_EFFECTS[i].effectType
      );
    pDefinition->lifetimeFrames =
      FOLLOWER_INTERACTION_EFFECT_LIFETIME_FRAMES;
    pDefinition->placement = SPECIES_EFFECTS[i].placement;
    pDefinition->moduleRequirement =
      Gen7Follower3gx::DIAGNOSTIC_EFFECT_MODULE_NONE;
    pDefinition->ultraOnly = false;
    return true;
  }
  return false;
}

inline void Manager::ResetPendingInteractionEffect( void )
{
  m_InteractionEffectType = 0;
  m_InteractionEffectLifetimeFrames = 0;
  m_InteractionEffectDelayFrames = 0;
  m_InteractionEffectElapsedFrames = 0;
  m_InteractionEffectPlacement =
    Gen7Follower3gx::DIAGNOSTIC_EFFECT_PLACEMENT_MID_BODY;
  m_InteractionEffectScale = 1.0f;
  m_InteractionEffectPending = false;
  m_VictiniLuckActivationPending = false;
}

inline void Manager::QueueInteractionEffect(
  Effect::Type effectType,
  u32 lifetimeFrames,
  u32 placement
)
{
  m_InteractionEffectType = static_cast<u32>( effectType );
  m_InteractionEffectLifetimeFrames = lifetimeFrames;
  m_InteractionEffectDelayFrames =
    FOLLOWER_INTERACTION_EFFECT_DELAY_FRAMES;
  m_InteractionEffectElapsedFrames = 0;
  m_InteractionEffectPlacement = placement;
  m_InteractionEffectScale = 1.0f;
  m_InteractionEffectPending = true;
}

inline gfl2::math::Vector3 Manager::GetInteractionEffectPosition(
  PokeTool::PokeModel* pPokeModel,
  Effect::Type effectType,
  u32 placement
) const
{
  if( effectType == Effect::EFFECT_TYPE_DEMO_CONCENTRATE ||
      effectType == Effect::EFFECT_TYPE_DEMO_FOG )
  {
    // Weather effects use screen coordinates. World coordinates put them off-screen.
    return gfl2::math::Vector3( 0.0f, 0.0f, 0.0f );
  }
  if( placement ==
      Gen7Follower3gx::DIAGNOSTIC_EFFECT_PLACEMENT_GROUND )
  {
    return m_Position;
  }
  return GetBallTransitionPosition( pPokeModel );
}

inline bool Manager::TryStartPendingInteractionEffect(
  PokeTool::PokeModel* pPokeModel,
  Fieldmap* pFieldmap
)
{
#if !FOLLOWER_POKEMON_ENABLE_BALL_TRANSITION
  (void)pPokeModel;
  (void)pFieldmap;
  return false;
#else
  const Effect::Type effectType = static_cast<Effect::Type>(
    m_InteractionEffectType
    );
  const FollowerInteractionEffect::WorkSlotAvailability slotAvailability =
    FollowerInteractionEffect::InspectWorkSlots( m_pEffectManager );
  const Effect::EffectManager::WorkType selectedWorkType =
    FollowerInteractionEffect::SelectWorkType( slotAvailability );
#if FOLLOWER_3GX_DIAGNOSTIC
  const u32 requestedMinimumHeapFree =
    Gen7Follower3gx::GetDiagnosticEffectHeapGuard();
#else
  (void)pFieldmap;
  const u32 requestedMinimumHeapFree =
    Gen7Follower3gx::DIAGNOSTIC_EFFECT_DEFAULT_HEAP_GUARD;
#endif
  const FollowerDynamicEffectHeapSelection heapSelection =
    SelectDynamicEffectResourceHeap( requestedMinimumHeapFree );
  const u32 resourceHeapFree = heapSelection.allocatableSize;
  const bool resourceWasLoaded = m_pEffectManager &&
    GFL_BOOL_CAST( m_pEffectManager->IsDataAvailable(
      effectType
      ) );

  Gen7Follower3gx::InteractionEffectStartResult result =
    Gen7Follower3gx::INTERACTION_EFFECT_START_NOT_ATTEMPTED;
  if( Gen7Follower3gx::IsPerformanceOptionEnabled(
        Gen7Follower3gx::PERFORMANCE_OPTION_DISABLE_EFFECTS
        ) )
  {
    result =
      Gen7Follower3gx::INTERACTION_EFFECT_START_PERFORMANCE_DISABLED;
  }
  else if( !m_pEffectManager )
  {
    result =
      Gen7Follower3gx::INTERACTION_EFFECT_START_NO_EFFECT_MANAGER;
  }
  else if( !pPokeModel || !pPokeModel->GetModelInstanceNode() )
  {
    result = Gen7Follower3gx::INTERACTION_EFFECT_START_NO_MODEL;
  }
  else if( m_SpecialEffects.IsDynamicEffectBusy() )
  {
    result = Gen7Follower3gx::INTERACTION_EFFECT_START_BUSY;
  }
  else if( resourceWasLoaded )
  {
    result = Gen7Follower3gx::
      INTERACTION_EFFECT_START_RESOURCE_ALREADY_LOADED;
  }
  else if( !heapSelection.pHeap )
  {
    result = heapSelection.candidateFound
      ? Gen7Follower3gx::INTERACTION_EFFECT_START_RESOURCE_HEAP_LOW
      : Gen7Follower3gx::INTERACTION_EFFECT_START_NO_RESOURCE_HEAP;
  }
  else
  {
#if FOLLOWER_3GX_DIAGNOSTIC
    const Gen7Follower3gx::DiagnosticEffectModuleResult moduleResult =
      Gen7Follower3gx::EnsureDiagnosticEffectModule(
        static_cast<u32>( effectType ),
        pFieldmap
        );
    if( !Gen7Follower3gx::IsDiagnosticEffectModuleReady( moduleResult ) )
    {
      if( moduleResult ==
          Gen7Follower3gx::DIAGNOSTIC_EFFECT_MODULE_RESULT_UNSUPPORTED )
      {
        result = Gen7Follower3gx::
          INTERACTION_EFFECT_START_MODULE_UNSUPPORTED;
      }
      else if( moduleResult ==
               Gen7Follower3gx::
                 DIAGNOSTIC_EFFECT_MODULE_RESULT_DLL_HEAP_LOW )
      {
        result = Gen7Follower3gx::
          INTERACTION_EFFECT_START_MODULE_DLL_HEAP_LOW;
      }
      else
      {
        result = Gen7Follower3gx::
          INTERACTION_EFFECT_START_MODULE_LOAD_FAILED;
      }
    }
    else
    {
#endif
      result = FollowerInteractionEffect::Start(
        &m_SpecialEffects,
        m_pEffectManager,
        heapSelection.pHeap,
        effectType,
        GetInteractionEffectPosition(
          pPokeModel,
          effectType,
          m_InteractionEffectPlacement
          ),
        m_InteractionEffectLifetimeFrames,
        heapSelection.minimumFree,
        selectedWorkType,
        m_InteractionEffectScale
        );
#if FOLLOWER_3GX_DIAGNOSTIC
      if( result == Gen7Follower3gx::INTERACTION_EFFECT_START_STARTED &&
          !Gen7Follower3gx::InitializeDiagnosticEffectInstance(
            static_cast<u32>( effectType ),
            m_SpecialEffects.GetDynamicEffect(
              FollowerSpecialEffectController::OWNER_SPECIES_INTERACTION
              )
            ) )
      {
        m_SpecialEffects.RequestStopDynamicEffect(
          FollowerSpecialEffectController::OWNER_SPECIES_INTERACTION
          );
        result = Gen7Follower3gx::
          INTERACTION_EFFECT_START_CREATE_FAILED;
      }
#endif
#if FOLLOWER_3GX_DIAGNOSTIC
    }
#endif
  }

  const u32 resourceHeapFreeAfter = heapSelection.pHeap
    ? heapSelection.pHeap->GetTotalAllocatableSize()
    : resourceHeapFree;

  Gen7Follower3gx::RecordInteractionEffectStart(
    result,
    static_cast<u32>( effectType ),
    heapSelection.source,
    resourceHeapFree,
    resourceHeapFreeAfter,
    heapSelection.minimumFree,
    resourceWasLoaded,
    static_cast<u32>( selectedWorkType )
    );
  return result == Gen7Follower3gx::INTERACTION_EFFECT_START_STARTED;
#endif
}

inline void Manager::UpdatePendingInteractionEffect(
  PokeTool::PokeModel* pPokeModel,
  bool reactionAdvancing,
  Fieldmap* pFieldmap
)
{
  if( !m_InteractionEffectPending || !reactionAdvancing )
  {
    return;
  }
  if( m_InteractionEffectElapsedFrames <
      m_InteractionEffectDelayFrames )
  {
    ++m_InteractionEffectElapsedFrames;
    return;
  }

  if( m_VictiniLuckActivationPending )
  {
    Gen7Follower3gx::ActivateVictiniLuck();
    m_VictiniLuckActivationPending = false;
  }

  TryStartPendingInteractionEffect( pPokeModel, pFieldmap );
  m_InteractionEffectPending = false;
}

inline bool Manager::TryStartInteraction( Fieldmap* pFieldmap )
{
  if( m_State != STATE_ACTIVE || !pFieldmap || m_IsFieldEventSuspended )
  {
    return false;
  }

  GameSys::GameManager* pGameManager = pFieldmap->GetGameManager();
  gfl2::ui::DeviceManager* pDeviceManager = pGameManager
    ? pGameManager->GetUiDeviceManager()
    : NULL;
  gfl2::ui::Button* pButton = pDeviceManager
    ? pDeviceManager->GetButton(
        gfl2::ui::DeviceManager::BUTTON_STANDARD
        )
    : NULL;
  if( !pButton || !pButton->IsTrigger( gfl2::ui::BUTTON_A ) )
  {
    return false;
  }

  GameSys::GameEventManager* pEventManager = pGameManager
    ? FOLLOWER_POKEMON_GET_GAME_EVENT_MANAGER( pGameManager )
    : NULL;
  if( pEventManager && pEventManager->IsExists() )
  {
    m_InteractionDiagnosticResult =
      Gen7Follower3gx::FOLLOWER_INTERACTION_RESULT_EVENT_ACTIVE;
    PublishInteractionDiagnostics(
      Gen7Follower3gx::FOLLOWER_INTERACTION_RESULT_EVENT_ACTIVE,
      static_cast<Gen7Follower3gx::FollowerInteractionDiagnosticMotion>(
        m_InteractionSelectedMotion
        ),
      true,
      false
      );
    return false;
  }

  const bool rideChord =
    (pButton->IsHold( gfl2::ui::BUTTON_L, gfl2::ui::Button::INPUT_STATE_ORIGINAL ) ||
     pButton->IsTrigger( gfl2::ui::BUTTON_L, gfl2::ui::Button::INPUT_STATE_ORIGINAL )) &&
    (pButton->IsHold( gfl2::ui::BUTTON_R, gfl2::ui::Button::INPUT_STATE_ORIGINAL ) ||
     pButton->IsTrigger( gfl2::ui::BUTTON_R, gfl2::ui::Button::INPUT_STATE_ORIGINAL ));
  if( m_MountedRideActive || m_MountedRideCleanup )
  {
    if( rideChord )
    {
      Gen7Follower3gx::ClearAreaRide();
      StopMountedRide();
      Follower3gx_NotifyRide( "Follower dismounted" );
    }
    const auto mode=Gen7Follower3gx::GetMountedInteractionMode();
    if( Gen7Follower3gx::ConsumeMountedInteraction(m_MountedRideCleanup,rideChord,mode) )
      return true;
    if( mode==Gen7Follower3gx::MOUNTED_INTERACTION_DISMOUNT ) {
      Gen7Follower3gx::ClearAreaRide();
      StopMountedRide();
    }
    return false;
  }

  if( m_InteractionState == INTERACTION_STATE_LOADING ||
      m_InteractionState == INTERACTION_STATE_PLAYING
#if FOLLOWER_3GX_INTERACTION_FEATURES
      || m_SpeciesActionKind != SPECIES_ACTION_NONE
#endif
#if FOLLOWER_POKEMON_ENABLE_BALL_TRANSITION
      || m_BallTransition.IsBusy()
#endif
    )
  {
    m_InteractionDiagnosticResult =
      Gen7Follower3gx::FOLLOWER_INTERACTION_RESULT_BUSY;
    PublishInteractionDiagnostics(
      Gen7Follower3gx::FOLLOWER_INTERACTION_RESULT_BUSY,
      static_cast<Gen7Follower3gx::FollowerInteractionDiagnosticMotion>(
        m_InteractionSelectedMotion
        ),
      true,
      false
      );
    return false;
  }

  PokeTool::PokeModel* pPokeModel = GetPokeModel();
  if( !pPokeModel || !pPokeModel->GetModelInstanceNode() )
  {
    return false;
  }

  const gfl2::math::Vector3 playerPosition =
    pFieldmap->GetPlayerPosition();
  const f32 dx = playerPosition.x - m_Position.x;
  const f32 dy = playerPosition.y - m_Position.y;
  const f32 dz = playerPosition.z - m_Position.z;
  const f32 verticalDistance = dy < 0.0f ? -dy : dy;
  const f32 horizontalInteractionDistance =
    FOLLOWER_INTERACTION_DISTANCE +
    GetFollowerPlayerBodyRadius( pPokeModel ) *
      FOLLOWER_INTERACTION_BODY_REACH_RATE;
  if( dx * dx + dz * dz >
        horizontalInteractionDistance * horizontalInteractionDistance ||
      verticalDistance > FOLLOWER_INTERACTION_VERTICAL_DISTANCE )
  {
    if( rideChord ){ Follower3gx_NotifyRide( "Follower ride: move closer" ); }
    m_InteractionDiagnosticResult =
      Gen7Follower3gx::FOLLOWER_INTERACTION_RESULT_TOO_FAR;
    PublishInteractionDiagnostics(
      Gen7Follower3gx::FOLLOWER_INTERACTION_RESULT_TOO_FAR,
      static_cast<Gen7Follower3gx::FollowerInteractionDiagnosticMotion>(
        m_InteractionSelectedMotion
        ),
      true,
      false
      );
    return false;
  }

  bool followerIsMoving = IsLocomotionMotion(
    static_cast<PokeTool::MODEL_ANIME>( m_CurrentMotion )
    );
#if FOLLOWER_POKEMON_USE_TRAIL_POLICY
  followerIsMoving = followerIsMoving ||
    m_TrailMovementState != TrailMovementPolicy::STATE_WAIT;
#endif
  if( followerIsMoving )
  {
    if( rideChord ){ Follower3gx_NotifyRide( "Follower ride: wait for the follower to stop" ); }
    m_InteractionDiagnosticResult =
      Gen7Follower3gx::FOLLOWER_INTERACTION_RESULT_MOVING;
    PublishInteractionDiagnostics(
      Gen7Follower3gx::FOLLOWER_INTERACTION_RESULT_MOVING,
      static_cast<Gen7Follower3gx::FollowerInteractionDiagnosticMotion>(
        m_InteractionSelectedMotion
        ),
      true,
      false
      );
    return false;
  }

  // Read held and newly pressed buttons together so pressing L/R/A at once works.
  const bool formChangeChord =
    pButton->IsHold(
      gfl2::ui::BUTTON_L,
      gfl2::ui::Button::INPUT_STATE_ORIGINAL
      ) ||
    pButton->IsTrigger(
      gfl2::ui::BUTTON_L,
      gfl2::ui::Button::INPUT_STATE_ORIGINAL
      ) ||
    pButton->IsHold( gfl2::ui::BUTTON_L ) ||
    pButton->IsTrigger( gfl2::ui::BUTTON_L );
  const bool specialEffectsChord =
    pButton->IsHold(
      gfl2::ui::BUTTON_R,
      gfl2::ui::Button::INPUT_STATE_ORIGINAL
      ) ||
    pButton->IsTrigger(
      gfl2::ui::BUTTON_R,
      gfl2::ui::Button::INPUT_STATE_ORIGINAL
      ) ||
    pButton->IsHold( gfl2::ui::BUTTON_R ) ||
    pButton->IsTrigger( gfl2::ui::BUTTON_R );
  if( formChangeChord && specialEffectsChord )
  {
    if( Gen7Follower3gx::IsRideableFollowerSpecies(
          static_cast<u32>( m_SimpleParam.monsNo ) ) )
    {
      TryStartMountedRide( pFieldmap );
    }
    else
    {
      Follower3gx_NotifyRide( "Ride: follower species ID is outside the supported range" );
    }
    return true;
  }
  const bool manualFormTrigger = formChangeChord &&
    Gen7Follower3gx::IsFollowerFormChangeEnabled();
  const bool specialTrigger = specialEffectsChord &&
    Gen7Follower3gx::IsFollowerSpecialEffectsEnabled();
  if( manualFormTrigger )
  {
    return TryCycleRawFollowerForm();
  }

  m_InteractionFacingYaw = m_ModelFacingYaw;
  m_InteractionFacingComplete = false;
  UpdateInteractionFacing( pPokeModel, playerPosition );
  if( !formChangeChord && !specialEffectsChord )
  {
    Gen7Follower3gx::ClearFollowerTalk();
    m_InteractionRandomState = m_InteractionRandomState * 1664525U + 1013904223U;
    m_TalkReaction = 1U + (m_InteractionRandomState >> 16) % 3U;
    m_TalkHopOffset = 0.0f;
    m_InteractionPlayingFrames = 0;
    m_InteractionState = INTERACTION_STATE_PLAYING;
    m_InteractionCryPending = true;
    SetMotion( PokeTool::MODEL_ANIME_FI_WAIT_A );
    return true;
  }

  ResetPendingInteractionEffect();
  const u32 followerSpecies = static_cast<u32>( m_SimpleParam.monsNo );
  Gen7Follower3gx::DiagnosticEffectDefinition labEffect = {};
  bool labOverride = false;
#if FOLLOWER_3GX_DIAGNOSTIC
  labOverride = specialTrigger &&
    Gen7Follower3gx::GetDiagnosticEffectDefinition(
      Gen7Follower3gx::GetDiagnosticEffectSelection(),
      &labEffect
      );
#endif
  Gen7Follower3gx::DiagnosticEffectDefinition speciesEffect = {};
  if( !labOverride && specialTrigger &&
      (followerSpecies == FOLLOWER_CASTFORM_SPECIES ||
       followerSpecies == FOLLOWER_KYOGRE_SPECIES ||
       followerSpecies == FOLLOWER_GROUDON_SPECIES) )
  {
    if( followerSpecies == FOLLOWER_CASTFORM_SPECIES )
    {
      TryCycleCastformWeather( pFieldmap );
    }
    else
    {
      TryStartPrimalWeather( pFieldmap, followerSpecies );
    }
    if( m_State != STATE_ACTIVE )
    {
      return true;
    }
  }
  if( !labOverride && followerSpecies == FOLLOWER_DITTO_SPECIES &&
      specialTrigger &&
      TryStartDittoTransform( pFieldmap ) )
  {
    return true;
  }
  if( !labOverride && specialTrigger &&
      TryStartSpeciesAction( pFieldmap, followerSpecies ) )
  {
    return true;
  }
  if( labOverride )
  {
    QueueInteractionEffect(
      static_cast<Effect::Type>( labEffect.effectType ),
      labEffect.lifetimeFrames,
      labEffect.placement
      );
  }
  else if( specialTrigger && GetSpeciesInteractionEffectDefinition(
             followerSpecies,
             &speciesEffect
             ) )
  {
    QueueInteractionEffect(
      static_cast<Effect::Type>( speciesEffect.effectType ),
      speciesEffect.lifetimeFrames,
      speciesEffect.placement
      );
    if( followerSpecies == FOLLOWER_VICTINI_SPECIES )
    {
      m_VictiniLuckActivationPending = true;
    }
  }
  SetMotion( PokeTool::MODEL_ANIME_FI_WAIT_A );
  pPokeModel->SetAnimationStepFrame( 1.0f );

  if( m_InteractionState == INTERACTION_STATE_READY &&
      m_InteractionPackLoaded )
  {
    if( !StartPettingInteractionMotion( true ) )
    {
      StartFieldInteractionFallback(
        Gen7Follower3gx::FOLLOWER_INTERACTION_RESULT_NO_KW_MOTION,
        true
        );
    }
    return true;
  }

  BeginInteractionMotionLoad( pFieldmap );
  return true;
}

inline bool Manager::UpdateInteraction(
  Fieldmap* pFieldmap,
  BaseCollisionScene* pTerrainGroundScene
)
{
  if( m_TalkReaction )
  {
    PokeTool::PokeModel* model = GetPokeModel();
    if( !model ) return false;
    const gfl2::math::Vector3 player = pFieldmap->GetPlayerPosition();
    ApplyGround( pTerrainGroundScene, &m_Position );
    if( UpdateInteractionFacing( model, player ) )
    {
      if( m_InteractionCryPending )
      {
        m_InteractionCryPending = false;
        Sound::PlayVoice( 0, m_SimpleParam.monsNo, m_SimpleParam.formNo,
          Sound::VOICE_TYPE_DEFAULT, false, 0 );
      }
      m_TalkHopOffset = Gen7Follower3gx::FollowerTalkHop(
        m_TalkReaction, m_InteractionPlayingFrames );
      if( ++m_InteractionPlayingFrames >
          Gen7Follower3gx::FollowerTalkDuration( m_TalkReaction ) )
      {
        Gen7Follower3gx::ShowFollowerTalk( m_TalkReaction );
        m_TalkReaction = 0;
        m_TalkHopOffset = 0.0f;
        m_InteractionState = m_InteractionPackLoaded
          ? INTERACTION_STATE_READY : INTERACTION_STATE_NONE;
        SeedTrail( player );
#if FOLLOWER_POKEMON_USE_TRAIL_POLICY
        ResetTrailMovementPolicy( player );
#endif
      }
    }
    model->SetPosition( GetDisplayPosition( model ) );
    model->SetAnimationStepFrame( 1.0f );
    if( m_pFactory ) m_pFactory->TickEntries();
    return true;
  }
  if( m_SpeciesActionKind != SPECIES_ACTION_NONE )
  {
    return UpdateSpeciesAction( pFieldmap, pTerrainGroundScene );
  }
  if( m_DittoTransformState != DITTO_TRANSFORM_NONE )
  {
    return UpdateDittoTransform( pFieldmap, pTerrainGroundScene );
  }

  if( m_InteractionState == INTERACTION_STATE_NONE ||
      m_InteractionState == INTERACTION_STATE_READY )
  {
    return false;
  }

  if( m_InteractionState == INTERACTION_STATE_LOADING )
  {
    CompleteInteractionMotionLoad( pFieldmap );
    if( m_InteractionState == INTERACTION_STATE_NONE ||
        m_InteractionState == INTERACTION_STATE_READY )
    {
      return false;
    }
  }

  PokeTool::PokeModel* pPokeModel = GetPokeModel();
  if( !pPokeModel )
  {
    return false;
  }

#if FOLLOWER_3GX_PERFORMANCE_FEATURES
  if( !m_InteractionPoseBlendCapturePending &&
      !m_InteractionPoseBlendActive )
  {
    ResetInterpolatedAnimationPose();
  }
  m_AnimationUpdatePhase = 0;
  m_AnimationUpdateThisFrame = true;
#endif

  if( m_InteractionState == INTERACTION_STATE_LOADING )
  {
    SetMotion( PokeTool::MODEL_ANIME_FI_WAIT_A );
    pPokeModel->SetAnimationStepFrame( 1.0f );
  }

  ApplyGround( pTerrainGroundScene, &m_Position );
  const gfl2::math::Vector3 playerPosition =
    pFieldmap->GetPlayerPosition();
  const bool facingComplete = UpdateInteractionFacing(
    pPokeModel,
    playerPosition
    );
  bool poseBlendBlocksPlayback = false;
#if FOLLOWER_3GX_PERFORMANCE_FEATURES
  poseBlendBlocksPlayback = m_InteractionPoseBlendCapturePending ||
    m_InteractionPoseBlendActive;
#endif
  if( m_InteractionState == INTERACTION_STATE_PLAYING )
  {
    pPokeModel->SetAnimationStepFrame(
      facingComplete && !poseBlendBlocksPlayback ? 1.0f : 0.0f
      );
    if( facingComplete && !poseBlendBlocksPlayback &&
        m_InteractionCryPending )
    {
      m_InteractionCryPending = false;
      Sound::PlayVoice(
        0,
        m_SimpleParam.monsNo,
        m_SimpleParam.formNo,
        Sound::VOICE_TYPE_DEFAULT,
        false,
        0
        );
    }
  }
  pPokeModel->SetPosition( GetDisplayPosition( pPokeModel ) );
  pPokeModel->SetVisible( true );
  if( m_pTrialModel )
  {
    m_pTrialModel->ForwardVisibility( true );
  }

  if( m_pFactory )
  {
    FOLLOWER_PERF_SCOPE(
      interactionModelUpdatePerformance,
      Gen7Follower3gx::PERFORMANCE_ZONE_MODEL
      );
    m_pFactory->TickEntries();
  }

#if FOLLOWER_3GX_PERFORMANCE_FEATURES
  gfl2::renderingengine::scenegraph::instance::ModelInstanceNode*
    pInteractionNode = pPokeModel->GetModelInstanceNode();
  if( m_InteractionPoseBlendCapturePending )
  {
    m_InteractionPoseBlendCapturePending = false;
    FinishInterpolatedAnimationSample( pInteractionNode, 0.0f );
    if( m_HasInterpolatedAnimationPose &&
        m_InterpolatedAnimationJointCount != 0 )
    {
      f32 verticalDelta =
        m_InterpolatedAnimationTarget[0].translate[1] -
        m_InterpolatedAnimationStart[0].translate[1];
      if( verticalDelta < 0.0f )
      {
        verticalDelta = -verticalDelta;
      }
      const bool useLongBlend =
        verticalDelta >= FOLLOWER_INTERACTION_POSE_VERTICAL_THRESHOLD;
      m_InteractionPoseBlendFrames = useLongBlend
        ? FOLLOWER_INTERACTION_POSE_BLEND_LONG_FRAMES
        : FOLLOWER_INTERACTION_POSE_BLEND_FRAMES;
      m_InteractionPoseBlendFrame = 0;
      m_InteractionPoseBlendActive = true;
    }
    else
    {
      m_InteractionPoseBlendFrames = FOLLOWER_INTERACTION_POSE_BLEND_FRAMES;
      m_InteractionPoseBlendFrame = 0;
      m_InteractionPoseBlendWeight = 0.0f;
      m_InteractionPoseBlendActive = true;
    }
  }
  if( m_InteractionPoseBlendActive )
  {
    const f32 blendWeight = m_InteractionPoseBlendFrames != 0
      ? static_cast<f32>( m_InteractionPoseBlendFrame ) /
        static_cast<f32>( m_InteractionPoseBlendFrames )
      : 1.0f;
    m_InteractionPoseBlendWeight = blendWeight;
    ApplyInterpolatedAnimationPose( pInteractionNode, blendWeight );
    if( facingComplete )
    {
      if( m_InteractionPoseBlendFrame < m_InteractionPoseBlendFrames )
      {
        ++m_InteractionPoseBlendFrame;
      }
      else
      {
        m_InteractionPoseBlendWeight = 1.0f;
        m_InteractionPoseBlendActive = false;
        ResetInterpolatedAnimationPose();
      }
    }
  }
#endif

  UpdatePendingInteractionEffect(
    pPokeModel,
    m_InteractionState == INTERACTION_STATE_PLAYING &&
      facingComplete && !poseBlendBlocksPlayback,
    pFieldmap
    );

  if( m_InteractionState != INTERACTION_STATE_PLAYING )
  {
    return true;
  }

  if( !facingComplete )
  {
    return true;
  }

  ++m_InteractionPlayingFrames;
  const f32 endFrame = pPokeModel->GetAnimationEndFrame();
  const f32 frame = pPokeModel->GetAnimationFrame();
  const bool reachedEnd = endFrame > 0.0f &&
    frame >= endFrame - FOLLOWER_INTERACTION_END_FRAME_EPSILON;
  if( !reachedEnd &&
      m_InteractionPlayingFrames < FOLLOWER_INTERACTION_MAX_PLAY_FRAMES )
  {
    return true;
  }

  const f32 returnGroundOffset =
    GetFollowerAnimationGroundOffset( pPokeModel );
  pPokeModel->ChangeAnimationSmooth(
    PokeTool::MODEL_ANIME_FI_WAIT_A,
    FOLLOWER_INTERACTION_RETURN_BLEND_FRAMES,
    true
    );
  pPokeModel->SetAnimationIsLoop( true );
  pPokeModel->SetAnimationStepFrame( 1.0f );
  m_CurrentMotion = PokeTool::MODEL_ANIME_FI_WAIT_A;
  m_InteractionState = m_InteractionPackLoaded
    ? INTERACTION_STATE_READY
    : INTERACTION_STATE_NONE;
  m_InteractionPlayingFrames = 0;
  m_InteractionPlayingExternal = false;
  m_InteractionCryPending = false;
  m_InteractionPoseBlendFrame = 0;
  m_InteractionPoseBlendFrames = 0;
  m_InteractionPoseBlendWeight = 1.0f;
  m_InteractionPoseBlendStartGroundOffset = 0.0f;
  m_InteractionPoseBlendCapturePending = false;
  m_InteractionPoseBlendActive = false;
  ResetPendingInteractionEffect();
  m_InteractionReturnGroundBlendFrame = 0;
  m_InteractionReturnGroundBlendStart = returnGroundOffset;
  m_InteractionReturnGroundBlendActive = true;
#if FOLLOWER_3GX_PERFORMANCE_FEATURES
  ResetInterpolatedAnimationPose();
#endif
  SeedTrail( playerPosition );
#if FOLLOWER_POKEMON_USE_TRAIL_POLICY
  ResetTrailMovementPolicy( playerPosition );
#endif
  PublishInteractionDiagnostics(
    static_cast<Gen7Follower3gx::FollowerInteractionDiagnosticResult>(
      m_InteractionDiagnosticResult
      ),
    static_cast<Gen7Follower3gx::FollowerInteractionDiagnosticMotion>(
      m_InteractionSelectedMotion
      ),
    false,
    false
    );
  return true;
}

inline bool Manager::PrepareInteractionForTerminate( bool preserveSpeciesAction )
{
  m_TalkReaction = 0;
  m_TalkHopOffset = 0.0f;
  Gen7Follower3gx::ClearFollowerTalk();
  StopMountedRide();
  if( m_MountedRideCleanup )
  {
    if( !ReleaseDittoPlayerClone() ){ return false; }
    m_MountedRideCleanup = false;
  }
  if( !preserveSpeciesAction )
  {
    CancelSpeciesAction();
  }
  if( m_DittoTransformState != DITTO_TRANSFORM_NONE )
  {
    m_DittoDiagnosticResult =
      Gen7Follower3gx::DITTO_TRANSFORM_RESULT_CANCELED;
    m_DittoTransformState = DITTO_TRANSFORM_CLEANUP;
    m_DittoRevealPending = false;
    PublishDittoTransformDiagnostics(
      Gen7Follower3gx::DITTO_TRANSFORM_RESULT_CANCELED,
      false,
      false
      );
    if( !ReleaseDittoPlayerClone() )
    {
      return false;
    }
    m_DittoTransformState = DITTO_TRANSFORM_NONE;
  }

  if( m_InteractionState == INTERACTION_STATE_LOADING )
  {
    m_InteractionCancelPending = true;
    m_InteractionDiagnosticResult =
      Gen7Follower3gx::FOLLOWER_INTERACTION_RESULT_CANCELED;
    m_InteractionSelectedMotion =
      Gen7Follower3gx::FOLLOWER_INTERACTION_MOTION_NONE;
    PublishInteractionDiagnostics(
      Gen7Follower3gx::FOLLOWER_INTERACTION_RESULT_CANCELED,
      Gen7Follower3gx::FOLLOWER_INTERACTION_MOTION_NONE,
      false,
      false
      );
    if( m_pInteractionFileManager &&
        !m_pInteractionFileManager->IsArcFileLoadDataFinished(
          &m_pInteractionMotionPack
          ) )
    {
      return false;
    }
  }

  PokeTool::PokeModel* pPokeModel = GetPokeModel();
  if( pPokeModel &&
      ( m_InteractionState == INTERACTION_STATE_PLAYING ||
        m_InteractionPackLoaded ) )
  {
    pPokeModel->ChangeAnimation(
      PokeTool::MODEL_ANIME_FI_WAIT_A,
      true
      );
    pPokeModel->SetAnimationIsLoop( true );
    pPokeModel->SetAnimationStepFrame( 1.0f );
    m_CurrentMotion = PokeTool::MODEL_ANIME_FI_WAIT_A;
  }
  if( m_InteractionPackLoaded )
  {
    GetInteractionAnimationPackList()->Finalize();
    m_InteractionPackLoaded = false;
  }
  if( m_pInteractionMotionPack )
  {
    gfl2::heap::GflHeapFreeMemoryBlock( m_pInteractionMotionPack );
    m_pInteractionMotionPack = NULL;
  }
  ResetInteractionMembers();
  return true;
}
#endif

#include "RuntimePartyModels.inl"

#include "RuntimeAnimationAndPlacement.inl"

#include "RuntimeMovementAndCollision.inl"

inline void Manager::SetMotion( PokeTool::MODEL_ANIME motion )
{
  PokeTool::PokeModel* pPokeModel = GetPokeModel();
  if( !pPokeModel ){ return; }

  if( !pPokeModel->IsAvailableAnimationDirect( static_cast<int>( motion ) ) )
  {
    motion = PokeTool::MODEL_ANIME_FI_WAIT_A;
  }

  if( m_CurrentMotion != static_cast<s32>( motion ) )
  {
    f32 normalizedPhase = 0.0f;
    const bool preserveLocomotionPhase =
      m_CurrentMotion != static_cast<s32>( PokeTool::MODEL_ANIME_ERROR ) &&
      IsLocomotionMotion( static_cast<PokeTool::MODEL_ANIME>( m_CurrentMotion ) ) &&
      IsLocomotionMotion( motion );
    if( preserveLocomotionPhase )
    {
      const f32 oldEndFrame = pPokeModel->GetAnimationEndFrame();
      if( oldEndFrame > FOLLOWER_ROOT_MOTION_EPSILON )
      {
        normalizedPhase = pPokeModel->GetAnimationFrame() / oldEndFrame;
        if( normalizedPhase < 0.0f )
        {
          normalizedPhase = 0.0f;
        }
        else if( normalizedPhase > 1.0f )
        {
          normalizedPhase = 1.0f;
        }
      }
    }

    if( m_CurrentMotion == static_cast<s32>( PokeTool::MODEL_ANIME_ERROR ) )
    {
      pPokeModel->ChangeAnimation( motion );
    }
    else
    {
      pPokeModel->ChangeAnimationSmooth( motion, FOLLOWER_MOTION_BLEND_FRAMES );
    }
    pPokeModel->SetAnimationIsLoop( true );

    if( preserveLocomotionPhase )
    {
      const f32 newEndFrame = pPokeModel->GetAnimationEndFrame();
      if( newEndFrame > FOLLOWER_ROOT_MOTION_EPSILON )
      {
        pPokeModel->SetAnimationFrame( newEndFrame * normalizedPhase );
      }
    }

    m_CurrentMotion = static_cast<s32>( motion );
    m_NominalRootSpeed = 0.0f;
    if( motion == PokeTool::MODEL_ANIME_WALK01 )
    {
      m_NominalRootSpeed = m_WalkNominalRootSpeed;
    }
    else if( motion == PokeTool::MODEL_ANIME_RUN01 )
    {
      m_NominalRootSpeed = m_RunNominalRootSpeed;
    }
    m_RootMotionSampleDistance = 0.0f;
    m_RootMotionSampleFrames = 0.0f;
  }
}

}
}
