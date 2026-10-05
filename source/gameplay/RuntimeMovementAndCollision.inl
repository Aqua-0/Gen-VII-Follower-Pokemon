inline void Manager::UpdateFollower(
  Fieldmap* pFieldmap,
  BaseCollisionScene* pTerrainGroundScene,
  BaseCollisionScene* pTerrainWallScene,
  BaseCollisionScene* pStaticScene
)
{
#if FOLLOWER_3GX_PERFORMANCE_FEATURES
  const Gen7Follower3gx::FollowerAnimationRateMode animationRateMode =
    Gen7Follower3gx::GetFollowerAnimationRateMode();
  const u32 animationRateModeValue = static_cast<u32>( animationRateMode );
  if( animationRateModeValue != m_AnimationRateMode )
  {
    m_AnimationRateMode = animationRateModeValue;
    m_AnimationUpdatePhase = 0;
    ResetInterpolatedAnimationPose();
  }
  u32 animationSampleInterval = 1;
  switch( animationRateMode )
  {
  case Gen7Follower3gx::FOLLOWER_ANIMATION_RATE_15_FPS:
  case Gen7Follower3gx::FOLLOWER_ANIMATION_RATE_15_FPS_INTERPOLATED:
    animationSampleInterval = 2;
    break;
  case Gen7Follower3gx::FOLLOWER_ANIMATION_RATE_10_FPS_INTERPOLATED:
    animationSampleInterval = 3;
    break;
  case Gen7Follower3gx::FOLLOWER_ANIMATION_RATE_7_5_FPS_INTERPOLATED:
    animationSampleInterval = 4;
    break;
  case Gen7Follower3gx::FOLLOWER_ANIMATION_RATE_5_FPS_INTERPOLATED:
    animationSampleInterval = 6;
    break;
  default:
    break;
  }
  const bool useReducedAnimationRate = animationSampleInterval > 1;
  const bool useInterpolatedAnimation =
    useReducedAnimationRate &&
    animationRateMode != Gen7Follower3gx::FOLLOWER_ANIMATION_RATE_15_FPS;
  u32 animationInterpolationStep = 1;
  if( useReducedAnimationRate )
  {
    m_AnimationUpdateThisFrame = m_AnimationUpdatePhase == 0;
    animationInterpolationStep = m_AnimationUpdatePhase + 1;
    ++m_AnimationUpdatePhase;
    if( m_AnimationUpdatePhase >= animationSampleInterval )
    {
      m_AnimationUpdatePhase = 0;
    }
  }
  else
  {
    m_AnimationUpdatePhase = 0;
    m_AnimationUpdateThisFrame = true;
  }
#endif
  PokeTool::PokeModel* pPokeModel = GetPokeModel();
  if( !pPokeModel ){ return; }

#if FOLLOWER_3GX_INTERACTION_FEATURES
  if( UpdateMountedRide( pFieldmap ) ){ return; }
  if( UpdateInteraction( pFieldmap, pTerrainGroundScene ) )
  {
    return;
  }
#endif

  const bool closeFollow = Gen7Follower3gx::IsCloseFollowEnabled();
  f32 configuredWalkSpeed = FOLLOWER_WALK_SPEED;
  f32 configuredRunSpeed = FOLLOWER_RUN_SPEED;
  f32 configuredWarpDistance = FOLLOWER_WARP_DISTANCE;
  bool animationPlaybackUncapped = false;
#if FOLLOWER_CARRIER_THREEGX
  configuredWalkSpeed = Gen7Follower3gx::GetFollowerWalkSpeed();
  configuredRunSpeed = Gen7Follower3gx::GetFollowerRunSpeed();
  configuredWarpDistance = Gen7Follower3gx::GetFollowerWarpDistance();
  animationPlaybackUncapped =
    Gen7Follower3gx::IsFollowerAnimationPlaybackUncapped();
#endif

  const gfl2::math::Vector3 playerPosition = pFieldmap->GetPlayerPosition();
#if FOLLOWER_POKEMON_USE_TRAIL_POLICY
  f32 belugaPlayerStep = 0.0f;
  f32 belugaPlayerMoveX = 0.0f;
  f32 belugaPlayerMoveZ = 0.0f;
  if( m_TrailHasPreviousPlayerPosition )
  {
    belugaPlayerMoveX =
      playerPosition.x - m_TrailPreviousPlayerPosition.x;
    belugaPlayerMoveZ =
      playerPosition.z - m_TrailPreviousPlayerPosition.z;
    belugaPlayerStep = gfl2::math::FSqrt(
      belugaPlayerMoveX * belugaPlayerMoveX +
      belugaPlayerMoveZ * belugaPlayerMoveZ
      );
  }
  m_TrailPreviousPlayerPosition = playerPosition;
  m_TrailHasPreviousPlayerPosition = true;
  if( TrailMovementPolicy::IsPlayerMoving( belugaPlayerStep ) )
  {
    if( m_TrailPlayerMovingFrames < (closeFollow ? TrailMovementPolicy::START_MOVE_FRAMES : 7U) )
    {
      ++m_TrailPlayerMovingFrames;
    }
  }
  else
  {
    m_TrailPlayerMovingFrames = 0;
  }
#endif
  const f32 collisionRadius = GetFollowerCollisionRadius( pPokeModel );
  const f32 playerBodyRadius = GetFollowerPlayerBodyRadius( pPokeModel );
  f32 playerSeparation = playerBodyRadius + FOLLOWER_PLAYER_COLLISION_RADIUS;
#if FOLLOWER_POKEMON_USE_TRAIL_POLICY
  if (closeFollow) playerSeparation += belugaPlayerStep*2.0f < 24.0f ? belugaPlayerStep*2.0f : 24.0f;
#endif

  if (m_PlacementPending)
  {
    TryPlaceNearPlayer(playerPosition,pTerrainGroundScene,pTerrainWallScene,
      pStaticScene,collisionRadius,playerSeparation);
    m_PlacementPending=false;
  }

  // Check the ground even when standing still, since the terrain can move.
  gfl2::math::Vector3 groundedPosition = m_Position;
  if( ApplyGround( pTerrainGroundScene, &groundedPosition ) )
  {
    m_Position = groundedPosition;
  }

  PushTrail( playerPosition );

#if FOLLOWER_POKEMON_USE_TRAIL_POLICY
  gfl2::math::Vector3 target = GetDelayedTrailTarget();
#else
  gfl2::math::Vector3 target = GetTrailTarget();
#endif
  EnforcePlayerSeparation( playerPosition, playerSeparation, &target );
#if FOLLOWER_3GX_PERFORMANCE_FEATURES
  const bool useOptimizedCollision =
    Gen7Follower3gx::GetFollowerCollisionQualityMode() !=
      Gen7Follower3gx::FOLLOWER_COLLISION_QUALITY_FULL;
  if( !useOptimizedCollision )
  {
    ApplyGround( pTerrainGroundScene, &target );
  }
#else
  ApplyGround( pTerrainGroundScene, &target );
#endif

  const f32 dx = target.x - m_Position.x;
  const f32 dz = target.z - m_Position.z;
  const f32 distanceSq = dx * dx + dz * dz;
  const f32 distance = gfl2::math::FSqrt( distanceSq );
  const f32 playerDx = m_Position.x - playerPosition.x;
  const f32 playerDz = m_Position.z - playerPosition.z;
  const f32 playerDistanceSq =
    playerDx * playerDx + playerDz * playerDz;
  const bool isInsidePlayer =
    playerDistanceSq < playerSeparation * playerSeparation;

  const gfl2::math::Vector3 movementStart = m_Position;
#if FOLLOWER_POKEMON_USE_TRAIL_POLICY
  TrailMovementPolicy::Thresholds belugaThresholds;
#if FOLLOWER_CARRIER_THREEGX
  belugaThresholds.waitToWalk = Gen7Follower3gx::GetFollowerStartDistance();
  belugaThresholds.walkToWait = Gen7Follower3gx::GetFollowerStopDistance();
  belugaThresholds.walkToRun = Gen7Follower3gx::GetFollowerRunDistance();
  belugaThresholds.runToWalk = Gen7Follower3gx::GetFollowerRunExitDistance();
#endif
  const f32 configuredStopDistance = belugaThresholds.walkToWait;
  f32 startHysteresis =
    belugaThresholds.waitToWalk - configuredStopDistance;
  if( startHysteresis < 0.0f )
  {
    startHysteresis = 0.0f;
  }
  belugaThresholds.walkToWait =
    configuredStopDistance * GetFollowerIdleGapScale( pPokeModel );
  belugaThresholds.waitToWalk =
    belugaThresholds.walkToWait + startHysteresis;
  const bool holdForApproachingPlayer =
    TrailMovementPolicy::ShouldHoldForApproachingPlayer(
      belugaPlayerMoveX,
      belugaPlayerMoveZ,
      playerDx,
      playerDz
      );
  const bool holdStationaryArrival =
    holdForApproachingPlayer ||
      TrailMovementPolicy::ShouldHoldStationaryArrival(
        belugaPlayerStep,
        playerDistanceSq,
        playerSeparation,
        belugaThresholds.walkToWait
        );
  if( holdStationaryArrival )
  {
    m_TrailMovementState = TrailMovementPolicy::STATE_WAIT;
  }
  else
  {
    m_TrailMovementState = (closeFollow ? TrailMovementPolicy::SelectState : TrailMovementPolicy::SelectStandardState)(
      m_TrailMovementState,
      belugaThresholds,
      distance,
      belugaPlayerStep,
      m_TrailPlayerMovingFrames,
      isInsidePlayer
      );
  }
  bool useRunMotion =
    m_TrailMovementState == TrailMovementPolicy::STATE_RUN;
  m_RunMode = useRunMotion;
  m_RunTransitionFrames = 0;
#else
  bool useRunMotion = UpdateRunMotionState( distance );
#endif
  bool warpedThisFrame = false;
  bool wantsMove = false;
  bool canAdvance = false;
  f32 desiredSpeed = 0.0f;
  f32 maxStep = 0.0f;
  f32 forwardX = 0.0f;
  f32 forwardZ = 0.0f;

  if(
#if FOLLOWER_POKEMON_USE_TRAIL_POLICY
      !holdStationaryArrival &&
#endif
      distance > configuredWarpDistance )
  {
    const bool placed = TryPlaceNearPlayer(playerPosition,pTerrainGroundScene,
      pTerrainWallScene,pStaticScene,collisionRadius,playerSeparation);
    m_RunMode = true;
    m_RunTransitionFrames = 0;
    useRunMotion = true;
#if FOLLOWER_POKEMON_USE_TRAIL_POLICY
    m_TrailMovementState = TrailMovementPolicy::STATE_RUN;
    m_TrailHasFacingDirection = false;
#endif
    warpedThisFrame = placed;
  }
#if FOLLOWER_POKEMON_USE_TRAIL_POLICY
  else if( !holdStationaryArrival &&
           ( m_TrailMovementState != TrailMovementPolicy::STATE_WAIT ||
             isInsidePlayer ) )
  {
#else
  else if( distance > FOLLOWER_IDLE_DISTANCE || isInsidePlayer )
  {
#endif
#if FOLLOWER_POKEMON_USE_TRAIL_POLICY
    desiredSpeed = (closeFollow ? TrailMovementPolicy::SelectDesiredSpeed : TrailMovementPolicy::SelectStandardDesiredSpeed)(
      m_TrailMovementState,
      belugaThresholds,
      distance,
      belugaPlayerStep,
      isInsidePlayer,
      configuredWalkSpeed,
      configuredRunSpeed,
#if FOLLOWER_CARRIER_THREEGX
      Gen7Follower3gx::GetFollowerCatchUpStep()
#else
      TrailMovementPolicy::NEAR_CATCH_UP_STEP
#endif
      );
    const f32 stopDistance = TrailMovementPolicy::GetStopDistance(
      m_TrailMovementState,
      belugaThresholds
      );
    maxStep = isInsidePlayer ? distance : distance - stopDistance;
#else
    desiredSpeed = useRunMotion ? configuredRunSpeed : configuredWalkSpeed;
    if( !isInsidePlayer && distance < FOLLOWER_SLOW_DISTANCE )
    {
      const f32 slowRange = FOLLOWER_SLOW_DISTANCE - FOLLOWER_IDLE_DISTANCE;
      const f32 slowRate = ( distance - FOLLOWER_IDLE_DISTANCE ) / slowRange;
      desiredSpeed = FOLLOWER_MIN_WALK_SPEED + ( configuredWalkSpeed - FOLLOWER_MIN_WALK_SPEED ) * slowRate;
    }

    maxStep = isInsidePlayer
      ? distance
      : distance - FOLLOWER_IDLE_DISTANCE;
#endif
    if( distance > FOLLOWER_COLLISION_EPSILON && maxStep > 0.0f )
    {
      const f32 invDistance = 1.0f / distance;
      forwardX = dx * invDistance;
      forwardZ = dz * invDistance;
      wantsMove = true;
      canAdvance = true;

      // Check for a clear path before advancing a blocked animation, so it can resume when the way opens up.
      if( m_NoMoveFrames > 0 )
      {
        f32 probeStep = desiredSpeed;
        if( probeStep > maxStep )
        {
          probeStep = maxStep;
        }

        gfl2::math::Vector3 probePosition = m_Position;
        probePosition.x += forwardX * probeStep;
        probePosition.z += forwardZ * probeStep;
        gfl2::math::Vector3 probeResolvedPosition;
        canAdvance = ResolveMovement(
          pTerrainGroundScene,
          pTerrainWallScene,
          pStaticScene,
          probePosition,
          collisionRadius,
          &probeResolvedPosition
          );
      }
    }

#if FOLLOWER_POKEMON_USE_TRAIL_POLICY
    if( distance > FOLLOWER_COLLISION_EPSILON )
    {
      const f32 invDistance = 1.0f / distance;
      const f32 targetFacingX = dx * invDistance;
      const f32 targetFacingZ = dz * invDistance;
      if( !m_TrailHasFacingDirection )
      {
        m_TrailFacingX = targetFacingX;
        m_TrailFacingZ = targetFacingZ;
        m_TrailHasFacingDirection = true;
      }
      else
      {
        m_TrailFacingX +=
          ( targetFacingX - m_TrailFacingX ) *
          TrailMovementPolicy::FACING_RESPONSE;
        m_TrailFacingZ +=
          ( targetFacingZ - m_TrailFacingZ ) *
          TrailMovementPolicy::FACING_RESPONSE;
        const f32 facingLengthSq =
          m_TrailFacingX * m_TrailFacingX +
          m_TrailFacingZ * m_TrailFacingZ;
        if( facingLengthSq >
            FOLLOWER_COLLISION_EPSILON * FOLLOWER_COLLISION_EPSILON )
        {
          const f32 invFacingLength =
            1.0f / gfl2::math::FSqrt( facingLengthSq );
          m_TrailFacingX *= invFacingLength;
          m_TrailFacingZ *= invFacingLength;
        }
        else
        {
          m_TrailFacingX = targetFacingX;
          m_TrailFacingZ = targetFacingZ;
        }
      }

      const f32 yaw = static_cast<f32>(
        atan2( m_TrailFacingX, m_TrailFacingZ )
        );
      pPokeModel->SetRotation( 0.0f, yaw, 0.0f );
      m_ModelFacingYaw = yaw;
    }
#else
    const f32 yaw = static_cast<f32>( atan2( dx, dz ) );
    pPokeModel->SetRotation( 0.0f, yaw, 0.0f );
    m_ModelFacingYaw = yaw;
#endif
  }

#if FOLLOWER_POKEMON_USE_TRAIL_POLICY
  if (closeFollow && wantsMove && canAdvance && !warpedThisFrame)
  {
    m_FollowSpeed = TrailMovementPolicy::SmoothFollowSpeed(m_FollowSpeed,desiredSpeed);
    desiredSpeed = m_FollowSpeed;
  }
  else m_FollowSpeed = 0.0f;
#endif

  if( wantsMove )
  {
    if( canAdvance )
    {
      m_CollisionPlaybackRatio +=
        ( 1.0f - m_CollisionPlaybackRatio ) * FOLLOWER_COLLISION_PLAYBACK_RECOVERY;
    }
    else
    {
      m_CollisionPlaybackRatio = 0.0f;
    }
  }
  else
  {
    m_CollisionPlaybackRatio = 1.0f;
  }

#if FOLLOWER_POKEMON_USE_TRAIL_POLICY
  if (closeFollow && wantsMove && canAdvance && !animationPlaybackUncapped &&
      pPokeModel->IsAvailableAnimationDirect(PokeTool::MODEL_ANIME_RUN01) &&
      TrailMovementPolicy::NeedsRunForStride(m_WalkNominalRootSpeed,
        FOLLOWER_WALK_ANIMATION_STEP_MAX,desiredSpeed,
        m_CurrentMotion == static_cast<s32>(PokeTool::MODEL_ANIME_RUN01)))
    useRunMotion = true;
#endif
  PokeTool::MODEL_ANIME motion = PokeTool::MODEL_ANIME_FI_WAIT_A;
  if( warpedThisFrame )
  {
    motion = PokeTool::MODEL_ANIME_RUN01;
  }
  else if( wantsMove && ( canAdvance || m_NoMoveFrames < FOLLOWER_NO_MOVE_WAIT_FRAMES ) )
  {
    motion = useRunMotion ? PokeTool::MODEL_ANIME_RUN01 : PokeTool::MODEL_ANIME_WALK01;
  }
  SetMotion( motion );

  if( IsLocomotionMotion( motion ) )
  {
    if( warpedThisFrame )
    {
      m_AnimationStepFrame = 1.0f;
    }
    else if( canAdvance )
    {
      f32 referenceSpeed = m_NominalRootSpeed;
      if( referenceSpeed <= FOLLOWER_ROOT_MOTION_EPSILON )
      {
        referenceSpeed = useRunMotion ? configuredRunSpeed : configuredWalkSpeed;
      }

      f32 targetStepFrame = desiredSpeed / referenceSpeed;
      if( targetStepFrame < FOLLOWER_ANIMATION_STEP_MIN )
      {
        targetStepFrame = FOLLOWER_ANIMATION_STEP_MIN;
      }
      const f32 maximumStepFrame = useRunMotion
        ? FOLLOWER_RUN_ANIMATION_STEP_MAX
        : FOLLOWER_WALK_ANIMATION_STEP_MAX;
      if( !animationPlaybackUncapped && targetStepFrame > maximumStepFrame )
      {
        targetStepFrame = maximumStepFrame;
      }
      targetStepFrame *= m_CollisionPlaybackRatio;
      m_AnimationStepFrame +=
        ( targetStepFrame - m_AnimationStepFrame ) * FOLLOWER_ANIMATION_STEP_RESPONSE;
    }
    else
    {
      m_AnimationStepFrame = 0.0f;
    }
  }
  else
  {
    m_AnimationStepFrame = 1.0f;
  }
  f32 playbackStepFrame = m_AnimationStepFrame;
#if FOLLOWER_3GX_PERFORMANCE_FEATURES
  if( useReducedAnimationRate )
  {
    playbackStepFrame = m_AnimationUpdateThisFrame
      ? m_AnimationStepFrame * static_cast<f32>( animationSampleInterval )
      : 0.0f;
  }
#endif
  pPokeModel->SetAnimationStepFrame( playbackStepFrame );

#if FOLLOWER_3GX_PERFORMANCE_FEATURES
  if( useInterpolatedAnimation && m_AnimationUpdateThisFrame )
  {
    FOLLOWER_PERF_SCOPE(
      animationInterpolationCapturePerformance,
      Gen7Follower3gx::PERFORMANCE_ZONE_MODEL
      );
    BeginInterpolatedAnimationSample( pPokeModel->GetModelInstanceNode() );
  }
#endif

  // Advance the animation before reading its root movement.
  if( m_pFactory
#if FOLLOWER_3GX_PERFORMANCE_FEATURES
      && m_AnimationUpdateThisFrame
#endif
    )
  {
    FOLLOWER_PERF_SCOPE(
      modelUpdatePerformance,
      Gen7Follower3gx::PERFORMANCE_ZONE_MODEL
    );
    m_pFactory->TickEntries();
  }

#if FOLLOWER_3GX_PERFORMANCE_FEATURES
  if( useInterpolatedAnimation )
  {
    FOLLOWER_PERF_SCOPE(
      animationInterpolationBlendPerformance,
      Gen7Follower3gx::PERFORMANCE_ZONE_MODEL
      );
    if( m_AnimationUpdateThisFrame )
    {
      FinishInterpolatedAnimationSample(
        pPokeModel->GetModelInstanceNode(),
        static_cast<f32>( animationInterpolationStep ) /
          static_cast<f32>( animationSampleInterval )
        );
    }
    else
    {
      ApplyInterpolatedAnimationPose(
        pPokeModel->GetModelInstanceNode(),
        static_cast<f32>( animationInterpolationStep ) /
          static_cast<f32>( animationSampleInterval )
        );
    }
  }
#endif

  f32 proposedStep = 0.0f;
  if( wantsMove && canAdvance )
  {
    f32 rootMotionStep = GetFollowerRootMotionStep( pPokeModel );
#if FOLLOWER_3GX_PERFORMANCE_FEATURES
    if( useReducedAnimationRate )
    {
      rootMotionStep = desiredSpeed;
    }
    else
#endif
    {
      UpdateAnimationNominalRootSpeed( rootMotionStep );
    }
    // Tiny animation strides must not cap the follower's travel speed.
    proposedStep = closeFollow || m_NominalRootSpeed <= FOLLOWER_ROOT_MOTION_EPSILON
      ? desiredSpeed : rootMotionStep;

#if FOLLOWER_POKEMON_USE_TRAIL_POLICY
    // Limit the step size near the target so the follower matches the player.
    const f32 safetyLimit = desiredSpeed;
#else
    const f32 safetyLimit = desiredSpeed * FOLLOWER_ROOT_MOTION_STEP_LIMIT;
#endif
    if( proposedStep > safetyLimit )
    {
      proposedStep = safetyLimit;
    }
    if( proposedStep > maxStep )
    {
      proposedStep = maxStep;
    }

    if( proposedStep > FOLLOWER_ROOT_MOTION_EPSILON )
    {
      gfl2::math::Vector3 desiredPosition = m_Position;
      const f32 clearFraction=Gen7Follower3gx::ClearFollowStep(
        m_Position.x-playerPosition.x,m_Position.z-playerPosition.z,
        forwardX*proposedStep,forwardZ*proposedStep,playerSeparation);
      desiredPosition.x += forwardX * proposedStep * clearFraction;
      desiredPosition.z += forwardZ * proposedStep * clearFraction;

      gfl2::math::Vector3 resolvedPosition;
      if( ResolveMovement(
            pTerrainGroundScene,
            pTerrainWallScene,
            pStaticScene,
            desiredPosition,
            collisionRadius,
            &resolvedPosition
            ) )
      {
        const f32 clearFraction=Gen7Follower3gx::ClearFollowStep(
          m_Position.x-playerPosition.x,m_Position.z-playerPosition.z,
          resolvedPosition.x-m_Position.x,resolvedPosition.z-m_Position.z,playerSeparation);
        if (clearFraction>=0.9999f) m_Position = resolvedPosition;
      }
    }
  }

  const f32 movedDx = m_Position.x - movementStart.x;
  const f32 movedDz = m_Position.z - movementStart.z;
  const f32 movedDistance = gfl2::math::FSqrt( movedDx * movedDx + movedDz * movedDz );
  const bool movedThisFrame =
    movedDistance > FOLLOWER_ANIMATION_MOVE_EPSILON;

  if( proposedStep > FOLLOWER_ROOT_MOTION_EPSILON && !warpedThisFrame )
  {
    f32 movementRatio = movedDistance / proposedStep;
    if( movementRatio > 1.0f )
    {
      movementRatio = 1.0f;
    }
    if( movementRatio < m_CollisionPlaybackRatio )
    {
      m_CollisionPlaybackRatio = movementRatio;
    }
  }

  if( warpedThisFrame || ( wantsMove && canAdvance &&
      ( movedThisFrame || proposedStep <= FOLLOWER_ROOT_MOTION_EPSILON ) ) )
  {
    m_NoMoveFrames = 0;
  }
  else if( wantsMove && m_NoMoveFrames < FOLLOWER_NO_MOVE_WAIT_FRAMES )
  {
    ++m_NoMoveFrames;
  }
  else if( !wantsMove )
  {
    m_NoMoveFrames = FOLLOWER_NO_MOVE_WAIT_FRAMES;
  }

  if (wantsMove && !movedThisFrame && !warpedThisFrame)
  {
    if (++m_BlockedFollowFrames >= 60)
    {
      if (TryPlaceNearPlayer(playerPosition,pTerrainGroundScene,pTerrainWallScene,
          pStaticScene,collisionRadius,playerSeparation))
      {
        SeedTrail(playerPosition);
        m_NoMoveFrames=0;
        m_CollisionPlaybackRatio=1.0f;
      }
      m_BlockedFollowFrames=0;
    }
  }
  else m_BlockedFollowFrames=0;

  pPokeModel->SetPosition( GetDisplayPosition( pPokeModel ) );
  pPokeModel->SetVisible( true );
  if( m_pTrialModel )
  {
    m_pTrialModel->ForwardVisibility( true );
  }
}

#if FOLLOWER_3GX_PERFORMANCE_FEATURES
inline bool Manager::TryApplyCachedGround(
  BaseCollisionScene* pTerrainGroundScene,
  gfl2::math::Vector3* pPosition
) const
{
  const u32 maximumCacheUses =
    Gen7Follower3gx::GetFollowerCollisionQualityMode() ==
      Gen7Follower3gx::FOLLOWER_COLLISION_QUALITY_FAST
        ? 64U
        : 16U;
  if(
    !m_HasCachedGround ||
    m_pCachedGroundScene != pTerrainGroundScene ||
    m_CachedGroundUses >= maximumCacheUses ||
    !pPosition
    )
  {
    return false;
  }

  const f32 v0pX = pPosition->x - m_CachedGroundPositions[0].x;
  const f32 v0pZ = pPosition->z - m_CachedGroundPositions[0].z;
  const f32 v01X =
    m_CachedGroundPositions[1].x - m_CachedGroundPositions[0].x;
  const f32 v01Z =
    m_CachedGroundPositions[1].z - m_CachedGroundPositions[0].z;
  const f32 v1pX = pPosition->x - m_CachedGroundPositions[1].x;
  const f32 v1pZ = pPosition->z - m_CachedGroundPositions[1].z;
  const f32 v12X =
    m_CachedGroundPositions[2].x - m_CachedGroundPositions[1].x;
  const f32 v12Z =
    m_CachedGroundPositions[2].z - m_CachedGroundPositions[1].z;
  const f32 v2pX = pPosition->x - m_CachedGroundPositions[2].x;
  const f32 v2pZ = pPosition->z - m_CachedGroundPositions[2].z;
  const f32 v20X =
    m_CachedGroundPositions[0].x - m_CachedGroundPositions[2].x;
  const f32 v20Z =
    m_CachedGroundPositions[0].z - m_CachedGroundPositions[2].z;
  const f32 cross0 = v0pZ * v01X - v01Z * v0pX;
  const f32 cross1 = v1pZ * v12X - v12Z * v1pX;
  const f32 cross2 = v2pZ * v20X - v20Z * v2pX;
  const bool hasPositive =
    cross0 > FOLLOWER_COLLISION_EPSILON ||
    cross1 > FOLLOWER_COLLISION_EPSILON ||
    cross2 > FOLLOWER_COLLISION_EPSILON;
  const bool hasNegative =
    cross0 < -FOLLOWER_COLLISION_EPSILON ||
    cross1 < -FOLLOWER_COLLISION_EPSILON ||
    cross2 < -FOLLOWER_COLLISION_EPSILON;
  if( hasPositive && hasNegative )
  {
    return false;
  }

  if(
    m_CachedGroundNormal.y <= FOLLOWER_COLLISION_EPSILON &&
    m_CachedGroundNormal.y >= -FOLLOWER_COLLISION_EPSILON
    )
  {
    return false;
  }
  pPosition->y = m_CachedGroundPositions[0].y -
    (
      m_CachedGroundNormal.x * v0pX +
      m_CachedGroundNormal.z * v0pZ
    ) / m_CachedGroundNormal.y;
  ++m_CachedGroundUses;
  return true;
}
#endif

inline bool Manager::ApplyGround( BaseCollisionScene* pTerrainGroundScene, gfl2::math::Vector3* pPosition ) const
{
  if( !pTerrainGroundScene || !pPosition ){ return false; }

#if FOLLOWER_3GX_PERFORMANCE_FEATURES
  const bool useGroundCache =
    Gen7Follower3gx::GetFollowerCollisionQualityMode() !=
      Gen7Follower3gx::FOLLOWER_COLLISION_QUALITY_FULL;
  if( useGroundCache && TryApplyCachedGround( pTerrainGroundScene, pPosition ) )
  {
    return true;
  }
  if( !useGroundCache )
  {
    m_HasCachedGround = false;
  }
#endif

  RaycastCustomCallback::HIT_DATA hitData;
  gfl2::math::Vector4 startVec(
    pPosition->x,
    pPosition->y + FOLLOWER_GROUND_RAY_EXTENT,
    pPosition->z,
    0.0f
    );
  gfl2::math::Vector4 endVec(
    pPosition->x,
    pPosition->y - FOLLOWER_GROUND_RAY_EXTENT,
    pPosition->z,
    0.0f
    );
  FOLLOWER_PERF_QUERY_BEGIN(groundMeshPerformance);
  const bool hitGround = pTerrainGroundScene->FindNearestMeshHit(
    startVec,
    endVec,
    &hitData
    );
  FOLLOWER_PERF_QUERY_END(
    Gen7Follower3gx::PERFORMANCE_COUNTER_GROUND_MESH,
    groundMeshPerformance
    );
  if( hitGround )
  {
    pPosition->y = hitData.intersection.y;
#if FOLLOWER_3GX_PERFORMANCE_FEATURES
    if( useGroundCache && hitData.pTriangle )
    {
      struct CollisionTriangleView
      {
        gfl2::math::Vector4 positions[3];
        gfl2::math::Vector4 normal;
      };
      const CollisionTriangleView* triangle =
        static_cast<const CollisionTriangleView*>( hitData.pTriangle );
      for( u32 i = 0; i < 3; ++i )
      {
        m_CachedGroundPositions[i] = triangle->positions[i];
      }
      m_CachedGroundNormal = triangle->normal;
      m_pCachedGroundScene = pTerrainGroundScene;
      m_CachedGroundUses = 0;
      m_HasCachedGround = true;
    }
#endif
    return true;
  }

#if FOLLOWER_3GX_PERFORMANCE_FEATURES
  if( useGroundCache )
  {
    m_HasCachedGround = false;
  }
#endif

  return false;
}

inline bool Manager::IsSceneWallBlocked(
  BaseCollisionScene* pScene,
  const gfl2::math::Vector3& from,
  const gfl2::math::Vector3& to,
  f32 radius,
  bool checkMesh,
  bool checkShapes,
  bool centerMeshOnly
) const
{
  if( !pScene ){ return false; }

  const f32 dx = to.x - from.x;
  const f32 dz = to.z - from.z;
  const f32 distanceSq = dx * dx + dz * dz;
  if( distanceSq <= FOLLOWER_COLLISION_EPSILON * FOLLOWER_COLLISION_EPSILON )
  {
    return false;
  }

  const f32 invDistance = 1.0f / gfl2::math::FSqrt( distanceSq );
  const f32 forwardX = dx * invDistance * radius;
  const f32 forwardZ = dz * invDistance * radius;
  const f32 sideX = -dz * invDistance * radius;
  const f32 sideZ = dx * invDistance * radius;

  if( checkMesh )
  {
    const s32 firstSide = centerMeshOnly ? 0 : -1;
    const s32 lastSide = centerMeshOnly ? 0 : 1;
    for( s32 side = firstSide; side <= lastSide; ++side )
    {
      const f32 offsetX = sideX * static_cast<f32>( side );
      const f32 offsetZ = sideZ * static_cast<f32>( side );
      gfl2::math::Vector4 startVec(
        from.x + offsetX,
        from.y + FOLLOWER_WALL_RAY_HEIGHT,
        from.z + offsetZ,
        0.0f
        );
      gfl2::math::Vector4 endVec(
        to.x + offsetX + forwardX,
        to.y + FOLLOWER_WALL_RAY_HEIGHT,
        to.z + offsetZ + forwardZ,
        0.0f
        );
      RaycastCustomCallback::HIT_DATA hitData;
      FOLLOWER_PERF_QUERY_BEGIN(wallMeshPerformance);
      const bool hitWallMesh = pScene->FindNearestMeshHit(
        startVec,
        endVec,
        &hitData
        );
      FOLLOWER_PERF_QUERY_END(
        Gen7Follower3gx::PERFORMANCE_COUNTER_WALL_MESH,
        wallMeshPerformance
        );
      if( hitWallMesh )
      {
        return true;
      }
    }
  }

  if( !checkShapes ){ return false; }

  gfl2::math::Vector4 startVec(
    from.x,
    from.y + FOLLOWER_WALL_RAY_HEIGHT,
    from.z,
    0.0f
    );
  gfl2::math::Vector4 endVec(
    to.x + forwardX,
    to.y + FOLLOWER_WALL_RAY_HEIGHT,
    to.z + forwardZ,
    0.0f
    );

  FOLLOWER_PERF_QUERY_BEGIN(wallCylinderPerformance);
  const bool hitWallCylinder =
    pScene->TestCategory1Segment( startVec, endVec );
  FOLLOWER_PERF_QUERY_END(
    Gen7Follower3gx::PERFORMANCE_COUNTER_WALL_CYLINDER,
    wallCylinderPerformance
    );
  if( hitWallCylinder ){ return true; }

  FOLLOWER_PERF_QUERY_BEGIN(wallBoxPerformance);
  const bool hitWallBox =
    pScene->TestCategory3Segment( startVec, endVec );
  FOLLOWER_PERF_QUERY_END(
    Gen7Follower3gx::PERFORMANCE_COUNTER_WALL_BOX,
    wallBoxPerformance
    );
  if( hitWallBox ){ return true; }
  return false;
}

inline bool Manager::IsWallBlocked(
  BaseCollisionScene* pTerrainWallScene,
  BaseCollisionScene* pStaticScene,
  const gfl2::math::Vector3& from,
  const gfl2::math::Vector3& to,
  f32 radius
) const
{
#if FOLLOWER_3GX_PERFORMANCE_FEATURES
  const Gen7Follower3gx::FollowerCollisionQualityMode collisionQuality =
    Gen7Follower3gx::GetFollowerCollisionQualityMode();
  if( collisionQuality == Gen7Follower3gx::FOLLOWER_COLLISION_QUALITY_FAST )
  {
    if( IsSceneWallBlocked(
          pTerrainWallScene,
          from,
          to,
          radius,
          true,
          false,
          true
          ) )
    {
      return true;
    }
    return IsSceneWallBlocked(
      pStaticScene,
      from,
      to,
      radius,
      true,
      false,
      true
      );
  }
  if( collisionQuality ==
        Gen7Follower3gx::FOLLOWER_COLLISION_QUALITY_BALANCED )
  {
    if( IsSceneWallBlocked(
          pTerrainWallScene,
          from,
          to,
          radius,
          true,
          false,
          false
          ) )
    {
      return true;
    }
    return IsSceneWallBlocked(
      pStaticScene,
      from,
      to,
      radius,
      false,
      true,
      false
      );
  }
#endif
  if( IsSceneWallBlocked(
        pTerrainWallScene,
        from,
        to,
        radius,
        true,
        true,
        false
        ) )
  {
    return true;
  }
  return IsSceneWallBlocked(
    pStaticScene,
    from,
    to,
    radius,
    true,
    true,
    false
    );
}

inline bool Manager::TryMove(
  BaseCollisionScene* pTerrainGroundScene,
  BaseCollisionScene* pTerrainWallScene,
  BaseCollisionScene* pStaticScene,
  const gfl2::math::Vector3& desiredPosition,
  f32 radius,
  gfl2::math::Vector3* pResolvedPosition
) const
{
  if( !pResolvedPosition ){ return false; }

  gfl2::math::Vector3 candidate = desiredPosition;
  if( pTerrainGroundScene && !ApplyGround( pTerrainGroundScene, &candidate ) )
  {
    return false;
  }
  if( IsWallBlocked(
        pTerrainWallScene,
        pStaticScene,
        m_Position,
        candidate,
        radius
        ) )
  {
    return false;
  }

  *pResolvedPosition = candidate;
  return true;
}

inline bool Manager::ResolveMovement(
  BaseCollisionScene* pTerrainGroundScene,
  BaseCollisionScene* pTerrainWallScene,
  BaseCollisionScene* pStaticScene,
  const gfl2::math::Vector3& desiredPosition,
  f32 radius,
  gfl2::math::Vector3* pResolvedPosition
) const
{
  if( TryMove(
        pTerrainGroundScene,
        pTerrainWallScene,
        pStaticScene,
        desiredPosition,
        radius,
        pResolvedPosition
        ) )
  {
    return true;
  }

  const f32 dx = desiredPosition.x - m_Position.x;
  const f32 dz = desiredPosition.z - m_Position.z;
  const f32 absDx = ( dx < 0.0f ) ? -dx : dx;
  const f32 absDz = ( dz < 0.0f ) ? -dz : dz;

  gfl2::math::Vector3 xPosition = m_Position;
  xPosition.x = desiredPosition.x;
  gfl2::math::Vector3 zPosition = m_Position;
  zPosition.z = desiredPosition.z;

  gfl2::math::Vector3 xResolved;
  gfl2::math::Vector3 zResolved;
  const bool xValid = absDx > FOLLOWER_COLLISION_EPSILON && TryMove(
    pTerrainGroundScene,
    pTerrainWallScene,
    pStaticScene,
    xPosition,
    radius,
    &xResolved
    );
  const bool zValid = absDz > FOLLOWER_COLLISION_EPSILON && TryMove(
    pTerrainGroundScene,
    pTerrainWallScene,
    pStaticScene,
    zPosition,
    radius,
    &zResolved
    );

  if( xValid && ( !zValid || absDx >= absDz ) )
  {
    *pResolvedPosition = xResolved;
    return true;
  }
  if( zValid )
  {
    *pResolvedPosition = zResolved;
    return true;
  }
  return false;
}


inline bool Manager::TryPlaceNearPlayer(const gfl2::math::Vector3& player,
  BaseCollisionScene* ground, BaseCollisionScene* walls, BaseCollisionScene* objects,
  f32 radius, f32 separation)
{
  if (!ground) return false;
  gfl2::math::Vector3 candidate;
  const f32 distance=separation+20.0f > 100.0f ? separation+20.0f : 100.0f;
  if (!Gen7Follower3gx::FindFollowerPlacement(player,distance,candidate,
      [&](gfl2::math::Vector3& position) {
        return ApplyGround(ground,&position) &&
          position.y-player.y <= 50.0f && player.y-position.y <= 50.0f &&
          !IsWallBlocked(walls,objects,player,position,radius);
      })) return false;
  m_Position=candidate;
  return true;
}
