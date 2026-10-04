inline bool Manager::IsLocomotionMotion( PokeTool::MODEL_ANIME motion ) const
{
  return motion == PokeTool::MODEL_ANIME_WALK01 ||
         motion == PokeTool::MODEL_ANIME_RUN01;
}

inline f32 Manager::GetFollowerAnimationGroundOffset(
  PokeTool::PokeModel* pPokeModel,
  f32* pRawRootY
  ) const
{
  if( !pPokeModel )
  {
    if( pRawRootY )
    {
      *pRawRootY = 0.0f;
    }
    return 0.0f;
  }

  const gfl2::math::Vector3 animationOffset = pPokeModel->GetWalkSpeed( -1.0f );
  if( pRawRootY )
  {
    *pRawRootY = animationOffset.y;
  }
  f32 groundOffset = animationOffset.y * pPokeModel->GetScale().y *
    pPokeModel->GetAdjustScale();
#if FOLLOWER_3GX_INTERACTION_FEATURES
  if( m_InteractionPoseBlendCapturePending ||
      m_InteractionPoseBlendActive )
  {
    groundOffset = m_InteractionPoseBlendStartGroundOffset +
      ( groundOffset - m_InteractionPoseBlendStartGroundOffset ) *
      m_InteractionPoseBlendWeight;
  }
  if( m_InteractionReturnGroundBlendActive )
  {
    const f32 blendWeight =
      static_cast<f32>( m_InteractionReturnGroundBlendFrame ) /
      static_cast<f32>( FOLLOWER_INTERACTION_RETURN_BLEND_FRAMES );
    groundOffset = m_InteractionReturnGroundBlendStart +
      ( groundOffset - m_InteractionReturnGroundBlendStart ) * blendWeight;
  }
#endif
  return groundOffset;
}

inline f32 Manager::GetFollowerRootMotionStep( PokeTool::PokeModel* pPokeModel ) const
{
  if( !pPokeModel ){ return 0.0f; }

  const gfl2::math::Vector3 rootMotion = pPokeModel->GetWalkSpeed( -1.0f );
  const f32 rootDistance = rootMotion.z < 0.0f ? -rootMotion.z : rootMotion.z;
  const f32 modelScale = pPokeModel->GetScale().z * pPokeModel->GetAdjustScale();
  return rootDistance * modelScale;
}

inline void Manager::UpdateAnimationNominalRootSpeed( f32 rootMotionStep )
{
  if( m_AnimationStepFrame <= FOLLOWER_ROOT_MOTION_EPSILON ||
      m_CurrentMotion == static_cast<s32>( PokeTool::MODEL_ANIME_ERROR ) ||
      !IsLocomotionMotion( static_cast<PokeTool::MODEL_ANIME>( m_CurrentMotion ) ) )
  {
    return;
  }

  m_RootMotionSampleDistance += rootMotionStep;
  m_RootMotionSampleFrames += m_AnimationStepFrame;
  if( m_RootMotionSampleFrames < FOLLOWER_ROOT_MOTION_SAMPLE_FRAMES )
  {
    return;
  }

  const f32 sampledSpeed =
    m_RootMotionSampleDistance / m_RootMotionSampleFrames;
  if( sampledSpeed > FOLLOWER_ROOT_MOTION_EPSILON )
  {
    if( m_NominalRootSpeed <= FOLLOWER_ROOT_MOTION_EPSILON )
    {
      m_NominalRootSpeed = sampledSpeed;
    }
    else
    {
      m_NominalRootSpeed +=
        ( sampledSpeed - m_NominalRootSpeed ) * FOLLOWER_ROOT_MOTION_SAMPLE_RESPONSE;
    }

    if( m_CurrentMotion == static_cast<s32>( PokeTool::MODEL_ANIME_WALK01 ) )
    {
      m_WalkNominalRootSpeed = m_NominalRootSpeed;
    }
    else if( m_CurrentMotion == static_cast<s32>( PokeTool::MODEL_ANIME_RUN01 ) )
    {
      m_RunNominalRootSpeed = m_NominalRootSpeed;
    }
  }

  m_RootMotionSampleDistance = 0.0f;
  m_RootMotionSampleFrames = 0.0f;
}

inline gfl2::math::Vector3 Manager::GetDisplayPosition( PokeTool::PokeModel* pPokeModel ) const
{
  (void)pPokeModel;
  return m_Position;
}

inline void Manager::ApplyFollowerAnimationGroundOffset( PokeTool::PokeModel* pPokeModel ) const
{
  if( !pPokeModel ){ return; }

  // Apply the visual transform after calculating the animation pose.
  gfl2::math::Vector3 displayPosition = GetDisplayPosition( pPokeModel );
#if FOLLOWER_3GX_DIAGNOSTIC
  f32 rawRootY = 0.0f;
  const f32 animationGroundOffset =
    GetFollowerAnimationGroundOffset( pPokeModel, &rawRootY );
#else
  const f32 animationGroundOffset =
    GetFollowerAnimationGroundOffset( pPokeModel );
#endif
  displayPosition.y += animationGroundOffset;
  static_cast<poke_3d::model::PokemonModel*>( pPokeModel )->SetPosition( displayPosition );

#if FOLLOWER_3GX_DIAGNOSTIC
  const PokeTool::PokeSettingData* pSettingData = pPokeModel->GetSettingData();
  Gen7Follower3gx::FollowerGroundDiagnosticSnapshot ground = {};
  ground.species = static_cast<unsigned int>( m_SimpleParam.monsNo );
  ground.form = static_cast<unsigned int>( m_SimpleParam.formNo );
  ground.motion = m_CurrentMotion;
  ground.bodyRadiusTenths =
    static_cast<int>( GetFollowerPlayerBodyRadius( pPokeModel ) * 10.0f );
  ground.rawRootYTenths = static_cast<int>( rawRootY * 10.0f );
  ground.appliedRootYTenths =
    static_cast<int>( animationGroundOffset * 10.0f );
  ground.cmHeight = pSettingData ? pSettingData->cmHeight : 0;
  ground.fieldAdjustHeight =
    pSettingData ? pSettingData->fieldAdjustHeight : 0;
  Gen7Follower3gx::UpdateFollowerGroundDiagnostics( ground );
#endif
}

inline f32 Manager::GetFollowerCollisionRadius( PokeTool::PokeModel* pPokeModel ) const
{
  f32 radius = FOLLOWER_COLLISION_RADIUS_MIN;
  if( pPokeModel && pPokeModel->GetSettingData() )
  {
    const s32 fieldHeight = pPokeModel->GetSettingData()->fieldAdjustHeight;
    if( fieldHeight > 0 )
    {
      radius = static_cast<f32>( fieldHeight ) * FOLLOWER_COLLISION_HEIGHT_RATE;
    }
  }

  if( radius < FOLLOWER_COLLISION_RADIUS_MIN )
  {
    radius = FOLLOWER_COLLISION_RADIUS_MIN;
  }
  else if( radius > FOLLOWER_COLLISION_RADIUS_MAX )
  {
    radius = FOLLOWER_COLLISION_RADIUS_MAX;
  }
  return radius;
}

inline f32 Manager::GetFollowerPlayerBodyRadius(
  PokeTool::PokeModel* pPokeModel
) const
{
  f32 radius = GetFollowerCollisionRadius( pPokeModel );
  const PokeTool::PokeSettingData* pSettingData =
    pPokeModel ? pPokeModel->GetSettingData() : NULL;
  if( pSettingData )
  {
    f32 minX = pSettingData->minX;
    f32 maxX = pSettingData->maxX;
    f32 minZ = pSettingData->minZ;
    f32 maxZ = pSettingData->maxZ;
    if( minX < 0.0f ){ minX = -minX; }
    if( maxX < 0.0f ){ maxX = -maxX; }
    if( minZ < 0.0f ){ minZ = -minZ; }
    if( maxZ < 0.0f ){ maxZ = -maxZ; }

    f32 extentX = minX > maxX ? minX : maxX;
    f32 extentZ = minZ > maxZ ? minZ : maxZ;
    const gfl2::math::Vector3& scale = pPokeModel->GetScale();
    f32 scaleX = scale.x;
    f32 scaleZ = scale.z;
    f32 adjustScale = pPokeModel->GetAdjustScale();
    if( scaleX < 0.0f ){ scaleX = -scaleX; }
    if( scaleZ < 0.0f ){ scaleZ = -scaleZ; }
    if( adjustScale < 0.0f ){ adjustScale = -adjustScale; }
    extentX *= scaleX * adjustScale;
    extentZ *= scaleZ * adjustScale;
    const f32 boundsRadius =
      ( extentX > extentZ ? extentX : extentZ ) *
      FOLLOWER_PLAYER_BODY_BOUNDS_SCALE;
    if( boundsRadius > FOLLOWER_COLLISION_EPSILON )
    {
      radius = boundsRadius;
    }
  }

  if( radius < FOLLOWER_PLAYER_BODY_RADIUS_MIN )
  {
    radius = FOLLOWER_PLAYER_BODY_RADIUS_MIN;
  }
  else if( radius > FOLLOWER_PLAYER_BODY_RADIUS_MAX )
  {
    radius = FOLLOWER_PLAYER_BODY_RADIUS_MAX;
  }
  return radius;
}

inline f32 Manager::GetFollowerIdleGapScale(
  PokeTool::PokeModel* pPokeModel
) const
{
  f32 scale =
    GetFollowerPlayerBodyRadius( pPokeModel ) /
    FOLLOWER_IDLE_GAP_RADIUS_REFERENCE;
  if( scale < FOLLOWER_IDLE_GAP_SCALE_MIN )
  {
    scale = FOLLOWER_IDLE_GAP_SCALE_MIN;
  }
  else if( scale > 1.0f )
  {
    scale = 1.0f;
  }
  return scale;
}

inline void Manager::ConfigureFollowerShadow( void )
{
  m_FollowerShadowData.count = 1;
  m_FollowerShadowData.data[0].monsNo = static_cast<u32>( m_SimpleParam.monsNo );
  m_FollowerShadowData.data[0].form = 65535;
  m_FollowerShadowData.data[0].type = 0;
  m_FollowerShadowData.data[0].scale = FOLLOWER_SHADOW_DEFAULT_SCALE;
  m_FollowerShadowData.data[0].offsetX = 0.0f;
  m_FollowerShadowData.data[0].offsetZ = 0.0f;
}

inline void Manager::UpdateFollowerShadowScale( PokeTool::PokeModel* pPokeModel )
{
  f32 scale = FOLLOWER_SHADOW_DEFAULT_SCALE;
  const PokeTool::PokeSettingData* pSettingData =
    pPokeModel ? pPokeModel->GetSettingData() : NULL;
  if( pSettingData && pSettingData->cmHeight > 0 )
  {
    scale = static_cast<f32>( pSettingData->cmHeight ) *
      FOLLOWER_SHADOW_HEIGHT_RATE;
  }

  if( scale < FOLLOWER_SHADOW_SCALE_MIN )
  {
    scale = FOLLOWER_SHADOW_SCALE_MIN;
  }
  else if( scale > FOLLOWER_SHADOW_SCALE_MAX )
  {
    scale = FOLLOWER_SHADOW_SCALE_MAX;
  }
  m_FollowerShadowData.data[0].scale = scale;
}

inline BaseCollisionScene* Manager::GetTerrainGroundScene( Fieldmap* pFieldmap ) const
{
  if( !pFieldmap ){ return NULL; }

  Terrain::TerrainManager* pTerrainManager = pFieldmap->GetTerrainManager();
  if( !pTerrainManager ){ return NULL; }
  return pTerrainManager->GetCollsionScene();
}

inline void Manager::EnforcePlayerSeparation(
  const gfl2::math::Vector3& playerPosition,
  f32 separationDistance,
  gfl2::math::Vector3* pTarget
) const
{
  if( !pTarget ){ return; }

  const f32 targetDx = pTarget->x - playerPosition.x;
  const f32 targetDz = pTarget->z - playerPosition.z;
  if( targetDx * targetDx + targetDz * targetDz >=
      separationDistance * separationDistance )
  {
    return;
  }

  f32 dx = m_Position.x - playerPosition.x;
  f32 dz = m_Position.z - playerPosition.z;
  f32 distanceSq = dx * dx + dz * dz;
  if( distanceSq <= FOLLOWER_COLLISION_EPSILON * FOLLOWER_COLLISION_EPSILON )
  {
    dx = targetDx;
    dz = targetDz;
    distanceSq = dx * dx + dz * dz;
  }
  if( distanceSq <= FOLLOWER_COLLISION_EPSILON * FOLLOWER_COLLISION_EPSILON )
  {
    dx = 0.0f;
    dz = 1.0f;
    distanceSq = 1.0f;
  }

  const f32 scale = separationDistance / gfl2::math::FSqrt( distanceSq );
  pTarget->x = playerPosition.x + dx * scale;
  pTarget->z = playerPosition.z + dz * scale;
}

inline bool Manager::UpdateRunMotionState( f32 distance )
{
  const bool requestedRunMode = m_RunMode
    ? distance > FOLLOWER_RUN_EXIT_DISTANCE
    : distance > FOLLOWER_RUN_DISTANCE;
  if( requestedRunMode == m_RunMode )
  {
    m_RunTransitionFrames = 0;
    return m_RunMode;
  }

  ++m_RunTransitionFrames;
  if( m_RunTransitionFrames >= FOLLOWER_RUN_TRANSITION_FRAMES )
  {
    m_RunMode = requestedRunMode;
    m_RunTransitionFrames = 0;
  }
  return m_RunMode;
}

#if FOLLOWER_POKEMON_USE_TRAIL_POLICY
inline void Manager::ResetTrailMovementPolicy(
  const gfl2::math::Vector3& playerPosition
)
{
  m_TrailMovementState = TrailMovementPolicy::STATE_WAIT;
  m_TrailPreviousPlayerPosition = playerPosition;
  m_TrailPlayerMovingFrames = 0;
  m_TrailFacingX = 0.0f;
  m_TrailFacingZ = 1.0f;
  m_TrailHasPreviousPlayerPosition = true;
  m_TrailHasFacingDirection = false;
}
#endif

#if FOLLOWER_3GX_PERFORMANCE_FEATURES
inline void Manager::ResetInterpolatedAnimationPose( void )
{
  m_pInterpolatedAnimationNode = NULL;
  m_InterpolatedAnimationJointCount = 0;
  m_InterpolatedAnimationSampleStarted = false;
  m_HasInterpolatedAnimationPose = false;
}

inline bool Manager::CaptureAnimationPose(
  gfl2::renderingengine::scenegraph::instance::ModelInstanceNode* pNode,
  JointLocalSrt* pPose,
  u32* pJointCount
)
const
{
  if( pJointCount )
  {
    *pJointCount = 0;
  }
  if( !pNode || !pPose || !pJointCount )
  {
    return false;
  }

  const u32 jointCount = pNode->GetJointNum();
  if( jointCount == 0 || jointCount > FOLLOWER_INTERPOLATED_POSE_MAX_JOINTS )
  {
    return false;
  }

  for( u32 jointIndex = 0; jointIndex < jointCount; ++jointIndex )
  {
    gfl2::renderingengine::scenegraph::instance::JointInstanceNode* pJoint =
      pNode->GetJointInstanceNode( jointIndex );
    if( !pJoint )
    {
      return false;
    }
    pPose[jointIndex] = pJoint->GetLocalSrtRaw();
  }

  *pJointCount = jointCount;
  return true;
}

inline void Manager::BeginInterpolatedAnimationSample(
  gfl2::renderingengine::scenegraph::instance::ModelInstanceNode* pNode
)
{
  ResetInterpolatedAnimationPose();
  m_pInterpolatedAnimationNode = pNode;
  m_InterpolatedAnimationSampleStarted = CaptureAnimationPose(
    pNode,
    m_InterpolatedAnimationStart,
    &m_InterpolatedAnimationJointCount
    );
  if( !m_InterpolatedAnimationSampleStarted )
  {
    ResetInterpolatedAnimationPose();
  }
}

inline void Manager::FinishInterpolatedAnimationSample(
  gfl2::renderingengine::scenegraph::instance::ModelInstanceNode* pNode,
  f32 weight
)
{
  if( !m_InterpolatedAnimationSampleStarted ||
      pNode != m_pInterpolatedAnimationNode )
  {
    ResetInterpolatedAnimationPose();
    return;
  }

  u32 targetJointCount = 0;
  if( !CaptureAnimationPose(
        pNode,
        m_InterpolatedAnimationTarget,
        &targetJointCount
        ) ||
      targetJointCount != m_InterpolatedAnimationJointCount )
  {
    ResetInterpolatedAnimationPose();
    return;
  }

  m_InterpolatedAnimationSampleStarted = false;
  m_HasInterpolatedAnimationPose = true;
  ApplyInterpolatedAnimationPose( pNode, weight );
}

inline void Manager::ApplyInterpolatedAnimationPose(
  gfl2::renderingengine::scenegraph::instance::ModelInstanceNode* pNode,
  f32 weight
)
{
  if( !m_HasInterpolatedAnimationPose ||
      !pNode ||
      pNode != m_pInterpolatedAnimationNode ||
      pNode->GetJointNum() != m_InterpolatedAnimationJointCount )
  {
    ResetInterpolatedAnimationPose();
    return;
  }

  if( weight < 0.0f )
  {
    weight = 0.0f;
  }
  else if( weight > 1.0f )
  {
    weight = 1.0f;
  }
  const f32 startWeight = 1.0f - weight;

  for( u32 jointIndex = 0;
       jointIndex < m_InterpolatedAnimationJointCount;
       ++jointIndex )
  {
    gfl2::renderingengine::scenegraph::instance::JointInstanceNode* pJoint =
      pNode->GetJointInstanceNode( jointIndex );
    if( !pJoint )
    {
      ResetInterpolatedAnimationPose();
      return;
    }

    JointLocalSrt pose = m_InterpolatedAnimationTarget[jointIndex];
    if( weight < 1.0f )
    {
      const JointLocalSrt& start = m_InterpolatedAnimationStart[jointIndex];
      const JointLocalSrt& target = m_InterpolatedAnimationTarget[jointIndex];
      for( u32 axis = 0; axis < 3; ++axis )
      {
        pose.scale[axis] =
          start.scale[axis] * startWeight + target.scale[axis] * weight;
        pose.translate[axis] =
          start.translate[axis] * startWeight +
          target.translate[axis] * weight;
      }

      const f32 quaternionDot =
        start.rotation[0] * target.rotation[0] +
        start.rotation[1] * target.rotation[1] +
        start.rotation[2] * target.rotation[2] +
        start.rotation[3] * target.rotation[3];
      const f32 targetSign = quaternionDot < 0.0f ? -1.0f : 1.0f;
      f32 quaternionLengthSquared = 0.0f;
      for( u32 component = 0; component < 4; ++component )
      {
        pose.rotation[component] =
          start.rotation[component] * startWeight +
          target.rotation[component] * targetSign * weight;
        quaternionLengthSquared +=
          pose.rotation[component] * pose.rotation[component];
      }

      if( quaternionLengthSquared > FOLLOWER_INTERPOLATED_QUATERNION_EPSILON )
      {
        const f32 inverseLength =
          1.0f / gfl2::math::FSqrt( quaternionLengthSquared );
        for( u32 component = 0; component < 4; ++component )
        {
          pose.rotation[component] *= inverseLength;
        }
      }
      else
      {
        for( u32 component = 0; component < 4; ++component )
        {
          pose.rotation[component] = start.rotation[component];
        }
      }
    }

    pJoint->SetLocalSrtRaw( pose );
  }
}
#endif

