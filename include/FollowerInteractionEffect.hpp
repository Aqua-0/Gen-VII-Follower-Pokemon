#pragma once

#include "FollowerSpecialEffectController.hpp"
#include "PerformanceDiagnostics.hpp"

namespace Field
{
namespace FollowerRuntime
{

class FollowerInteractionEffect
{
public:
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

  static Gen7Follower3gx::InteractionEffectStartResult Start(
    FollowerSpecialEffectController* pController,
    Effect::EffectManager* pEffectManager,
    gfl2::heap::HeapBase* pResourceHeap,
    Effect::Type effectType,
    const gfl2::math::Vector3& position,
    u32 lifetimeFrames,
    u32 minimumResourceHeapFree,
    Effect::EffectManager::WorkType workType,
    f32 scale = 1.0f
  )
  {
    if( !pController )
    {
      return Gen7Follower3gx::INTERACTION_EFFECT_START_NO_EFFECT_MANAGER;
    }

    FollowerSpecialEffectController::DynamicEffectRequest request(
      effectType,
      position
      );
    request.workType = workType;
    request.minimumResourceHeapFree = minimumResourceHeapFree;
    request.lifetimeFrames = lifetimeFrames;
    request.releaseSettleFrames = RESOURCE_RELEASE_SETTLE_FRAMES;
    request.scale = scale;
    request.playSound = false;

    return MapStartResult( pController->StartDynamicEffect(
      FollowerSpecialEffectController::OWNER_SPECIES_INTERACTION,
      pEffectManager,
      pResourceHeap,
      request
      ) );
  }

private:
  static Gen7Follower3gx::InteractionEffectStartResult MapStartResult(
    FollowerSpecialEffectController::DynamicStartResult result
  )
  {
    switch( result )
    {
    case FollowerSpecialEffectController::DYNAMIC_START_STARTED:
      return Gen7Follower3gx::INTERACTION_EFFECT_START_STARTED;
    case FollowerSpecialEffectController::DYNAMIC_START_NO_EFFECT_MANAGER:
      return Gen7Follower3gx::INTERACTION_EFFECT_START_NO_EFFECT_MANAGER;
    case FollowerSpecialEffectController::DYNAMIC_START_NO_RESOURCE_HEAP:
      return Gen7Follower3gx::INTERACTION_EFFECT_START_NO_RESOURCE_HEAP;
    case FollowerSpecialEffectController::DYNAMIC_START_BUSY:
      return Gen7Follower3gx::INTERACTION_EFFECT_START_BUSY;
    case FollowerSpecialEffectController::DYNAMIC_START_RESOURCE_ALREADY_LOADED:
      return Gen7Follower3gx::INTERACTION_EFFECT_START_RESOURCE_ALREADY_LOADED;
    case FollowerSpecialEffectController::DYNAMIC_START_RESOURCE_HEAP_LOW:
      return Gen7Follower3gx::INTERACTION_EFFECT_START_RESOURCE_HEAP_LOW;
    case FollowerSpecialEffectController::DYNAMIC_START_LOAD_FAILED:
      return Gen7Follower3gx::INTERACTION_EFFECT_START_LOAD_FAILED;
    case FollowerSpecialEffectController::DYNAMIC_START_NO_EFFECT_SLOT:
      return Gen7Follower3gx::INTERACTION_EFFECT_START_NO_EFFECT_SLOT;
    case FollowerSpecialEffectController::DYNAMIC_START_CREATE_FAILED:
      return Gen7Follower3gx::INTERACTION_EFFECT_START_CREATE_FAILED;
    default:
      return Gen7Follower3gx::INTERACTION_EFFECT_START_BUSY;
    }
  }

  enum
  {
    RESOURCE_RELEASE_SETTLE_FRAMES = 3,
  };
};

} // namespace FollowerRuntime
} // namespace Field
