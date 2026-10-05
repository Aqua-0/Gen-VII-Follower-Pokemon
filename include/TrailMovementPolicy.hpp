#ifndef GEN7_FIELD_FOLLOWER_TRAIL_MOVEMENT_POLICY_H
#define GEN7_FIELD_FOLLOWER_TRAIL_MOVEMENT_POLICY_H

namespace Field
{
namespace FollowerRuntime
{
namespace TrailMovementPolicy
{

enum State
{
  STATE_WAIT,
  STATE_WALK,
  STATE_RUN,
};

static const float PLAYER_MOVE_EPSILON = 0.25f;
static const float PLAYER_RUN_STEP = 8.0f;
static const unsigned int START_MOVE_FRAMES = 3;
static const unsigned int TRAIL_DELAY_FRAMES = 3;
static const float FAR_START_MULTIPLIER = 1.5f;
// The target already accounts for the model's radius, so don't add it again.
static const float WAIT_TO_WALK_DISTANCE = 30.0f;
static const float WALK_TO_WAIT_DISTANCE = 20.0f;
static const float WALK_TO_RUN_DISTANCE = 160.0f;
static const float RUN_TO_WALK_DISTANCE = 160.0f * (10.0f / 13.0f);
static const float DIRECT_RUN_MARGIN = 40.0f;
static const float NEAR_MATCH_MARGIN = 25.0f;
static const float NEAR_CATCH_UP_STEP = 1.5f;
static const float FACING_RESPONSE = 0.5f;
static const float STATIONARY_ARRIVAL_PLAYER_EPSILON = 0.75f;
static const float STATIONARY_ARRIVAL_OVERLAP_TOLERANCE = 8.0f;
static const float STATIONARY_ARRIVAL_MARGIN = 12.0f;

struct Thresholds
{
  Thresholds()
  : waitToWalk( WAIT_TO_WALK_DISTANCE )
  , walkToWait( WALK_TO_WAIT_DISTANCE )
  , walkToRun( WALK_TO_RUN_DISTANCE )
  , runToWalk( RUN_TO_WALK_DISTANCE )
  {
  }

  float waitToWalk;
  float walkToWait;
  float walkToRun;
  float runToWalk;
};

inline bool IsPlayerMoving( float playerStep )
{
  return playerStep > PLAYER_MOVE_EPSILON;
}

inline bool IsPlayerRunning( float playerStep )
{
  return playerStep > PLAYER_RUN_STEP;
}

inline bool ShouldHoldStationaryArrival(
  float playerStep,
  float playerDistanceSq,
  float playerSeparation,
  float idleGap
)
{
  if( playerStep > STATIONARY_ARRIVAL_PLAYER_EPSILON )
  {
    return false;
  }

  float minimumDistance =
    playerSeparation - STATIONARY_ARRIVAL_OVERLAP_TOLERANCE;
  if( minimumDistance < 0.0f )
  {
    minimumDistance = 0.0f;
  }
  const float maximumDistance =
    playerSeparation + idleGap + STATIONARY_ARRIVAL_MARGIN;
  return playerDistanceSq >= minimumDistance * minimumDistance &&
         playerDistanceSq <= maximumDistance * maximumDistance;
}

inline bool ShouldHoldForApproachingPlayer(
  float playerMoveX,
  float playerMoveZ,
  float followerOffsetX,
  float followerOffsetZ
)
{
  const float playerMoveSq =
    playerMoveX * playerMoveX + playerMoveZ * playerMoveZ;
  if( playerMoveSq <= PLAYER_MOVE_EPSILON * PLAYER_MOVE_EPSILON )
  {
    return false;
  }

  const float approachDot =
    playerMoveX * followerOffsetX + playerMoveZ * followerOffsetZ;
  return approachDot > 0.0f;
}

inline State SelectState(
  State currentState,
  const Thresholds& thresholds,
  float targetDistance,
  float playerStep,
  unsigned int playerMovingFrames,
  bool isInsidePlayer
)
{
  const bool playerMoving = IsPlayerMoving( playerStep );
  const bool playerRunning = IsPlayerRunning( playerStep );

  switch( currentState )
  {
  case STATE_WAIT:
    if( isInsidePlayer )
    {
      return STATE_WALK;
    }
    if( playerRunning &&
        targetDistance > thresholds.runToWalk + DIRECT_RUN_MARGIN )
    {
      return STATE_RUN;
    }
    if( targetDistance > thresholds.waitToWalk &&
        ( playerMovingFrames >= START_MOVE_FRAMES ||
          targetDistance > thresholds.waitToWalk * FAR_START_MULTIPLIER ) )
    {
      return STATE_WALK;
    }
    return STATE_WAIT;

  case STATE_WALK:
    if( (playerRunning && targetDistance > thresholds.waitToWalk + NEAR_MATCH_MARGIN) ||
        targetDistance > thresholds.walkToRun * 1.8f )
    {
      return STATE_RUN;
    }
    if( !playerMoving && targetDistance <= thresholds.walkToWait )
    {
      return STATE_WAIT;
    }
    return STATE_WALK;

  case STATE_RUN:
    if( !playerRunning )
    {
      if( !playerMoving && targetDistance <= thresholds.walkToWait )
      {
        return STATE_WAIT;
      }
      if( targetDistance <= thresholds.walkToRun )
      {
        return STATE_WALK;
      }
    }
    return STATE_RUN;
  }

  return STATE_WAIT;
}

inline float GetStopDistance( State state, const Thresholds& thresholds )
{
  (void)state;
  return thresholds.walkToWait;
}

inline float SelectDesiredSpeed(
  State state,
  const Thresholds& thresholds,
  float targetDistance,
  float playerStep,
  bool isInsidePlayer,
  float walkSpeed,
  float runSpeed,
  float nearCatchUpStep = NEAR_CATCH_UP_STEP
)
{
  if( state == STATE_WAIT )
  {
    return 0.0f;
  }

  const float speedLimit = state == STATE_RUN ? runSpeed : walkSpeed;
  const float stopDistance = GetStopDistance( state, thresholds );
  float gap = targetDistance - stopDistance;
  if (gap < 0.0f) gap = 0.0f;
  float desiredSpeed;
  if (IsPlayerMoving(playerStep))
  {
    float catchUpScale = gap / NEAR_MATCH_MARGIN;
    if (catchUpScale > 4.0f) catchUpScale = 4.0f;
    desiredSpeed = playerStep + nearCatchUpStep * catchUpScale;
  }
  else
  {
    desiredSpeed = gap * 0.25f;
    if (desiredSpeed > walkSpeed) desiredSpeed = walkSpeed;
  }
  if (desiredSpeed > speedLimit) desiredSpeed = speedLimit;

  if( isInsidePlayer && desiredSpeed < walkSpeed )
  {
    desiredSpeed = walkSpeed;
  }

  return desiredSpeed;
}

inline State SelectStandardState(
  State currentState,
  const Thresholds& thresholds,
  float targetDistance,
  float playerStep,
  unsigned int playerMovingFrames,
  bool isInsidePlayer
)
{
  const bool playerMoving = IsPlayerMoving( playerStep );
  const bool playerRunning = IsPlayerRunning( playerStep );

  switch( currentState )
  {
  case STATE_WAIT:
    if( isInsidePlayer )
    {
      return STATE_WALK;
    }
    if( playerRunning &&
        targetDistance > thresholds.runToWalk + DIRECT_RUN_MARGIN )
    {
      return STATE_RUN;
    }
    if( targetDistance > thresholds.waitToWalk &&
        ( playerMovingFrames >= 7U ||
          targetDistance > thresholds.waitToWalk * FAR_START_MULTIPLIER ) )
    {
      return STATE_WALK;
    }
    return STATE_WAIT;

  case STATE_WALK:
    if( targetDistance > thresholds.walkToRun &&
        ( playerRunning ||
          targetDistance > thresholds.walkToRun * 1.8f ) )
    {
      return STATE_RUN;
    }
    if( !playerMoving && targetDistance <= thresholds.walkToWait )
    {
      return STATE_WAIT;
    }
    return STATE_WALK;

  case STATE_RUN:
    if( !playerRunning )
    {
      if( !playerMoving && targetDistance <= thresholds.walkToWait )
      {
        return STATE_WAIT;
      }
      if( targetDistance <= thresholds.walkToRun )
      {
        return STATE_WALK;
      }
    }
    return STATE_RUN;
  }

  return STATE_WAIT;
}

inline float SelectStandardDesiredSpeed(
  State state,
  const Thresholds& thresholds,
  float targetDistance,
  float playerStep,
  bool isInsidePlayer,
  float walkSpeed,
  float runSpeed,
  float nearCatchUpStep = NEAR_CATCH_UP_STEP
)
{
  if( state == STATE_WAIT )
  {
    return 0.0f;
  }

  float desiredSpeed = state == STATE_RUN ? runSpeed : walkSpeed;
  const float stopDistance = GetStopDistance( state, thresholds );
  if( targetDistance <= stopDistance + NEAR_MATCH_MARGIN )
  {
    if( !IsPlayerMoving( playerStep ) && targetDistance > stopDistance )
    {
      desiredSpeed = walkSpeed;
    }
    else
    {
      float catchUpStep = targetDistance - stopDistance;
      if( catchUpStep < 0.0f )
      {
        catchUpStep = 0.0f;
      }
      else if( catchUpStep > nearCatchUpStep )
      {
        catchUpStep = nearCatchUpStep;
      }

      const float matchedSpeed = playerStep + catchUpStep;
      if( matchedSpeed < desiredSpeed )
      {
        desiredSpeed = matchedSpeed;
      }
    }
  }

  if( isInsidePlayer && desiredSpeed < walkSpeed )
  {
    desiredSpeed = walkSpeed;
  }

  return desiredSpeed;
}

inline float SmoothFollowSpeed(float previous, float desired)
{
  return previous + (desired - previous) * 0.35f;
}

inline bool NeedsRunForStride(float walkStride, float playbackLimit,
                             float desiredSpeed, bool wasRunning)
{
  if (walkStride <= 0.001f) return false;
  const float threshold=walkStride * playbackLimit * (wasRunning ? 0.85f : 1.1f);
  return desiredSpeed > threshold;
}

}
}
}

#endif
