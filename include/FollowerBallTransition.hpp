#pragma once

#include "FollowerSpecialEffectController.hpp"
#include "PerformanceDiagnostics.hpp"

namespace Field
{
namespace FollowerRuntime
{

class FollowerBallTransition
{
public:
  enum Kind
  {
    KIND_SPAWN,
    KIND_DESPAWN,
  };

  typedef FollowerSpecialEffectController::WorkSlotAvailability
    WorkSlotAvailability;

  static WorkSlotAvailability InspectWorkSlots(
    const Effect::EffectManager* pEffectManager
  )
  {
    return FollowerSpecialEffectController::InspectWorkSlots(
      pEffectManager
      );
  }

  static Effect::EffectManager::WorkType SelectWorkType(
    const WorkSlotAvailability& availability
  )
  {
    return FollowerSpecialEffectController::SelectWorkType( availability );
  }

  static u32 GetMinimumResourceHeapFree( void )
  {
    return MIN_RESOURCE_HEAP_FREE;
  }

  FollowerBallTransition()
  : m_pController( NULL )
  {
  }

  Gen7Follower3gx::BallTransitionStartResult Start(
    FollowerSpecialEffectController* pController,
    Effect::EffectManager* pEffectManager,
    gfl2::heap::HeapBase* pResourceHeap,
    const gfl2::math::Vector3& position,
    Kind kind,
    u32 minimumResourceHeapFree,
    Effect::EffectManager::WorkType workType
  )
  {
    if( !pController )
    {
      return Gen7Follower3gx::BALL_TRANSITION_START_NO_EFFECT_MANAGER;
    }

    FollowerSpecialEffectController::DynamicEffectRequest request(
      Effect::EFFECT_TYPE_B_DEMO,
      position
      );
    request.workType = workType;
    request.minimumResourceHeapFree = minimumResourceHeapFree;
    request.lifetimeFrames = EFFECT_LIFETIME_FRAMES;
    request.cueFrame = kind == KIND_SPAWN
      ? static_cast<u32>( SPAWN_REVEAL_FRAME )
      : static_cast<u32>( FollowerSpecialEffectController::NO_CUE_FRAME );
    request.releaseSettleFrames = RESOURCE_RELEASE_SETTLE_FRAMES;

    const FollowerSpecialEffectController::DynamicStartResult result =
      pController->StartDynamicEffect(
        FollowerSpecialEffectController::OWNER_BALL_TRANSITION,
        pEffectManager,
        pResourceHeap,
        request
        );
    const Gen7Follower3gx::BallTransitionStartResult mappedResult =
      MapStartResult( result );
    if( mappedResult == Gen7Follower3gx::BALL_TRANSITION_START_STARTED )
    {
      m_pController = pController;
    }
    return mappedResult;
  }

  bool ConsumeRevealRequest( void )
  {
    return m_pController && m_pController->ConsumeDynamicCue(
      FollowerSpecialEffectController::OWNER_BALL_TRANSITION
      );
  }

  bool IsBusy( void ) const
  {
    return m_pController && m_pController->IsDynamicOwnerActive(
      FollowerSpecialEffectController::OWNER_BALL_TRANSITION
      );
  }

  void Reset( void )
  {
    m_pController = NULL;
  }

private:
  static Gen7Follower3gx::BallTransitionStartResult MapStartResult(
    FollowerSpecialEffectController::DynamicStartResult result
  )
  {
    switch( result )
    {
    case FollowerSpecialEffectController::DYNAMIC_START_STARTED:
      return Gen7Follower3gx::BALL_TRANSITION_START_STARTED;
    case FollowerSpecialEffectController::DYNAMIC_START_NO_EFFECT_MANAGER:
      return Gen7Follower3gx::BALL_TRANSITION_START_NO_EFFECT_MANAGER;
    case FollowerSpecialEffectController::DYNAMIC_START_NO_RESOURCE_HEAP:
      return Gen7Follower3gx::BALL_TRANSITION_START_NO_RESOURCE_HEAP;
    case FollowerSpecialEffectController::DYNAMIC_START_BUSY:
      return Gen7Follower3gx::BALL_TRANSITION_START_BUSY;
    case FollowerSpecialEffectController::DYNAMIC_START_RESOURCE_ALREADY_LOADED:
      return Gen7Follower3gx::BALL_TRANSITION_START_RESOURCE_ALREADY_LOADED;
    case FollowerSpecialEffectController::DYNAMIC_START_RESOURCE_HEAP_LOW:
      return Gen7Follower3gx::BALL_TRANSITION_START_RESOURCE_HEAP_LOW;
    case FollowerSpecialEffectController::DYNAMIC_START_LOAD_FAILED:
      return Gen7Follower3gx::BALL_TRANSITION_START_LOAD_FAILED;
    case FollowerSpecialEffectController::DYNAMIC_START_NO_EFFECT_SLOT:
      return Gen7Follower3gx::BALL_TRANSITION_START_NO_EFFECT_SLOT;
    case FollowerSpecialEffectController::DYNAMIC_START_CREATE_FAILED:
      return Gen7Follower3gx::BALL_TRANSITION_START_CREATE_FAILED;
    default:
      return Gen7Follower3gx::BALL_TRANSITION_START_BUSY;
    }
  }

  enum
  {
    // The transition pack needs 54 KiB, plus some room for loading it.
    MIN_RESOURCE_HEAP_FREE = 0x20000,
    SPAWN_REVEAL_FRAME = 4,
    EFFECT_LIFETIME_FRAMES = 36,
    RESOURCE_RELEASE_SETTLE_FRAMES = 3,
  };

  FollowerSpecialEffectController* m_pController;
};

} // namespace FollowerRuntime
} // namespace Field
