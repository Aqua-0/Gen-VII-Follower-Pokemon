// This changes how riding looks. The game still uses walking controls and collision.
inline void Manager::RestoreRidePlayerVisibility( void )
{
  m_RiderBoneControl.Restore();
  m_RiderStabilizer.Restore();
  if( m_pRideHiddenPlayer )
  {
    m_pRideHiddenPlayer->SetVisible( m_RidePlayerWasVisible );
    m_pRideHiddenPlayer = NULL;
  }
}

inline void Manager::StopMountedRide( void )
{
  Follower3gx_ClearRidePresentation();
  RestoreRidePlayerVisibility();
  m_RiderStabilizer.Reset();
  if( m_MountedRideActive )
  {
    m_MountedRideActive = false;
    m_MountedRideCleanup = m_RideProfile.mode!=Gen7Follower3gx::RIDE_POKEMON_ON_PLAYER;
    poke_3d::model::BaseModel* rider = GetDittoPlayerCloneModel();
    if( rider && m_MountedRideCleanup ){ rider->SetVisible( false ); }
    if( !m_MountedRideCleanup ) {
      m_pDittoMoveModelManager=NULL;
      SeedTrail(m_RidePreviousPosition);
#if FOLLOWER_POKEMON_USE_TRAIL_POLICY
      ResetTrailMovementPolicy(m_RidePreviousPosition);
#endif
    }
    if( m_pTrialModel ) m_pTrialModel->SetShadowVisible(
#if FOLLOWER_3GX_PERFORMANCE_FEATURES
      !Gen7Follower3gx::IsPerformanceOptionEnabled(Gen7Follower3gx::PERFORMANCE_OPTION_HIDE_FOLLOWER_SHADOW)
#else
      true
#endif
      );
  }
}

inline bool Manager::TryStartMountedRide( Fieldmap* fieldmap )
{
  if( m_MountedRideActive || m_MountedRideCleanup ||
      m_DittoWorkReserved || m_DittoTransformState != DITTO_TRANSFORM_NONE ||
      m_InteractionState == INTERACTION_STATE_LOADING ||
      m_InteractionState == INTERACTION_STATE_PLAYING ||
      m_SpeciesActionKind != SPECIES_ACTION_NONE )
  {
    Follower3gx_NotifyRide( "Follower ride: another action is busy" );
    return false;
  }
  m_RideProfile=Gen7Follower3gx::GetRideProfile(static_cast<u32>(m_SimpleParam.monsNo));
  if( !m_RideProfile.enabled )
  {
    Follower3gx_NotifyRide("Riding is disabled in this Pokemon's ride profile");
    return false;
  }
  if( m_RideProfile.mode==Gen7Follower3gx::RIDE_POKEMON_ON_PLAYER )
  {
    auto* game=fieldmap ? fieldmap->GetGameManager() : NULL;
    auto* data=game ? FOLLOWER_POKEMON_GET_GAME_DATA(game) : NULL;
    auto* models=data ? data->GetFieldCharaModelManagerRaw() : NULL;
    auto* work=models ? models->GetFieldMoveModelRaw(Field::MoveModel::FIELD_MOVE_MODEL_PLAYER) : NULL;
    auto* player=work ? work->GetCharaDrawInstanceRaw() : NULL;
    if( !player || !player->GetModelInstanceNode() || !GetPokeModel() ) return false;
    Gen7Follower3gx::PublishRideModelBones(true,m_RideProfile.species,player->GetModelInstanceNode());
    Gen7Follower3gx::PublishRideModelBones(false,m_RideProfile.species,GetPokeModel()->GetModelInstanceNode());
    m_RideAttachmentJoint=Gen7Follower3gx::FindRideJoint(player->GetModelInstanceNode(),m_RideProfile.carryAttachment);
    if( m_RideProfile.carryAttachment[0] && m_RideAttachmentJoint<0 ) {
      Follower3gx_NotifyRide("Carry bone unavailable; choose a player bone or player origin");
      return false;
    }
    m_pDittoMoveModelManager=models;
    m_MountedRideActive=true;
    m_MountedRideCleanup=false;
    m_RidePreviousPosition=fieldmap->GetPlayerPosition();
    SetMotion(PokeTool::MODEL_ANIME_FI_WAIT_A);
    Follower3gx_NotifyRide("Pokemon carried - L+R+A to put down");
    return true;
  }
  Follower3gx_TraceRide("ride: allocating player clone");
  if( !PreparePlayerCloneAllocation( fieldmap ) )
  {
    Follower3gx_NotifyRide( "Follower ride: rider allocation failed" );
    return false;
  }

  // Mark the model as ours before creating it so cleanup can finish even if creation fails partway.
  m_MountedRideCleanup = true;
  Follower3gx_TraceRide("ride: creating player clone");
  if( !CreateDittoPlayerClone( false ) )
  {
    Follower3gx_NotifyRide( "Follower ride: rider model creation failed" );
    return false;
  }
  Follower3gx_TraceRide("ride: player clone created");
  poke_3d::model::BaseModel* rider = GetDittoPlayerCloneModel();
  m_RideStyle = static_cast<Gen7Follower3gx::RiderAnimationStyle>(m_RideProfile.style);
  Gen7Follower3gx::PublishRideModelBones(true,m_RideProfile.species,rider->GetModelInstanceNode());
  auto* mountNode=GetPokeModel() ? GetPokeModel()->GetModelInstanceNode() : NULL;
  Gen7Follower3gx::PublishRideModelBones(false,m_RideProfile.species,mountNode);
  m_RideAttachmentJoint=Gen7Follower3gx::FindRideJoint(mountNode,m_RideProfile.attachment);
  m_RidePelvisJoint=Gen7Follower3gx::FindRideJoint(rider->GetModelInstanceNode(),"Waist");
  if( m_RideProfile.attachment[0] && (m_RideAttachmentJoint<0 || m_RidePelvisJoint<0) )
  {
    rider->SetVisible(false);
    Follower3gx_NotifyRide("Ride bone unavailable on this model; edit the attachment or use fixed seat");
    return false;
  }
  if( Gen7Follower3gx::IsExternalRiderStyle(m_RideStyle) )
  {
    Follower3gx_TraceRide("ride: loading external animations");
    m_pRideMotionData = Follower3gx_LoadUltraRidePack( m_DittoPlayerCharacterId, m_RideStyle == Gen7Follower3gx::RIDER_LUNALA );
    if( !m_pRideMotionData || !m_pFactory || !m_pFactory->GetAllocator() ||
        m_pDittoCloneHeap->GetTotalAllocatableSize() < 0x10000U )
    {
      rider->SetVisible( false );
      Follower3gx_NotifyRide( "Follower ride: missing Ultra rider pack or low rider memory" );
      return false;
    }
    m_RideMotionPack.Initialize( m_pDittoCloneHeap, 1 );
    m_RideMotionPack.LoadData( 0, m_pFactory->GetAllocator(), m_pDittoCloneHeap,
      reinterpret_cast<char*>( m_pRideMotionData ) );
    m_RideMotionPackLoaded = true;
    if( !m_RideMotionPack.GetResourceNode( 0, 0 ) ||
        !m_RideMotionPack.GetResourceNode( 0, 1 ) )
    {
      rider->SetVisible( false );
      Follower3gx_NotifyRide( "Follower ride: Ultra rider resource unavailable" );
      return false;
    }
  }
  else
  {
    const u32 idle = Gen7Follower3gx::GetRiderIdleMotion(m_RideStyle);
    if( !rider->IsAnimationExist( idle ) || !rider->IsAnimationExist( idle + 1 ) )
    {
      rider->SetVisible( false );
      Follower3gx_NotifyRide( "Follower ride: selected rider animations unavailable" );
      return false;
    }
  }
  if( m_RideStyle == Gen7Follower3gx::RIDER_TAUROS && m_RideProfile.stabilize )
  {
    rider->ChangeAnimation( 150U );
    rider->SetAnimationFrame( 0.0f );
    rider->SetAnimationStepFrame( 0.0f );
    rider->UpdateAnimation();
    m_RiderStabilizer.Capture( rider->GetModelInstanceNode() );
    rider->SetAnimationStepFrame( 1.0f );
  }
  m_MountedRideCleanup = false;
  m_MountedRideActive = true;
  m_RideMotion = 0xffffffffU;
  m_RidePreviousPosition = fieldmap->GetPlayerPosition();
  m_InteractionFacingYaw = m_ModelFacingYaw;
  Follower3gx_TraceRide("ride: ready");
  return true;
}

inline bool Manager::UpdateMountedRide( Fieldmap* fieldmap )
{
  if( m_MountedRideCleanup )
  {
    if( !ReleaseDittoPlayerClone() ){ return true; }
    m_MountedRideCleanup = false;
    SeedTrail( fieldmap->GetPlayerPosition() );
#if FOLLOWER_POKEMON_USE_TRAIL_POLICY
    ResetTrailMovementPolicy( fieldmap->GetPlayerPosition() );
#endif
    return false;
  }
  if( !m_MountedRideActive ){ return false; }
  if( m_RideProfile.mode==Gen7Follower3gx::RIDE_POKEMON_ON_PLAYER )
    return UpdateCarriedPokemon(fieldmap);

  poke_3d::model::BaseModel* rider = GetDittoPlayerCloneModel();
  Field::MoveModel::FieldMoveModel* playerWork = m_pDittoMoveModelManager
    ? m_pDittoMoveModelManager->GetFieldMoveModelRaw(
        Field::MoveModel::FIELD_MOVE_MODEL_PLAYER ) : NULL;
  poke_3d::model::BaseModel* player = playerWork
    ? playerWork->GetCharaDrawInstanceRaw() : NULL;
  PokeTool::PokeModel* pokemon = GetPokeModel();
  if( !player || !player->GetModelInstanceNode() || !rider || !pokemon ||
      !player->IsVisible() )
  {
    Follower3gx_NotifyRide( "Follower ride: player hidden or model unavailable" );
    StopMountedRide();
    return true;
  }

  const gfl2::math::Vector3 position = fieldmap->GetPlayerPosition();
  const f32 dx = position.x - m_RidePreviousPosition.x;
  const f32 dz = position.z - m_RidePreviousPosition.z;
  const f32 distanceSq = dx * dx + dz * dz;
  // If the mount teleports, don't drag the rider across the map or doorway.
  if( distanceSq > 300.0f * 300.0f )
  {
    StopMountedRide();
    return true;
  }
  const bool moving = distanceSq > 0.25f * 0.25f;
  if( moving ){ m_ModelFacingYaw = static_cast<f32>( atan2( dx, dz ) ); }
  m_RidePreviousPosition = position;
  m_Position = position;
  const PokeTool::MODEL_ANIME motion = !moving
    ? PokeTool::MODEL_ANIME_FI_WAIT_A
    : distanceSq > 64.0f ? PokeTool::MODEL_ANIME_RUN01
                        : PokeTool::MODEL_ANIME_WALK01;
  SetMotion( motion );
  pokemon->SetAnimationStepFrame( 1.0f );
  pokemon->SetRotation( 0.0f, m_ModelFacingYaw, 0.0f );
  pokemon->SetPosition( GetDisplayPosition( pokemon ) );
  pokemon->SetVisible( true );
  if( m_pTrialModel ){ m_pTrialModel->ForwardVisibility( true ); }
#if FOLLOWER_3GX_PERFORMANCE_FEATURES
  // Update the rider every field frame, even with the Lite preset.
  m_AnimationUpdateThisFrame = true;
  ResetInterpolatedAnimationPose();
#endif
  if( m_pFactory ){ m_pFactory->TickEntries(); }

  const u32 riderMotion = moving ? 1U : 0U;
  if( riderMotion != m_RideMotion )
  {
    if( m_RideMotion == 0xffffffffU )
    {
      Follower3gx_NotifyRide( "Follower mounted - L+R+A to dismount" );
    }
    if( Gen7Follower3gx::IsExternalRiderStyle(m_RideStyle) )
    {
      rider->ChangeAnimationByResourceNode(
        m_RideMotionPack.GetResourceNode( 0, riderMotion ) );
    }
    else
    {
      const u32 idle = Gen7Follower3gx::GetRiderIdleMotion(m_RideStyle);
      rider->ChangeAnimation( idle + riderMotion );
    }
    rider->SetAnimationLoop( true );
    rider->SetAnimationFrame( 0.0f );
    rider->SetAnimationStepFrame( 1.0f );
    m_RideMotion = riderMotion;
  }
  // Seat offsets follow the mount's direction, regardless of which way the rider faces.
  gfl2::math::Vector3 riderPosition=position;
  const f32 sine=sinf(m_ModelFacingYaw), cosine=cosf(m_ModelFacingYaw);
  riderPosition.x+=cosine*m_RideProfile.seat[0]+sine*m_RideProfile.seat[2];
  riderPosition.y+=m_RideProfile.seat[1];
  riderPosition.z+=cosine*m_RideProfile.seat[2]-sine*m_RideProfile.seat[0];
  rider->SetPosition(riderPosition);
  rider->SetRotation(0.0f,m_ModelFacingYaw+m_RideProfile.yaw*0.01745329252f,0.0f);
  rider->SetVisible(true);
  auto* riderNode=rider->GetModelInstanceNode();
  if( m_RideStyle==Gen7Follower3gx::RIDER_TAUROS && m_RideProfile.stabilize )
    m_RiderStabilizer.Apply(riderNode,m_RideProfile.waistMotion,m_RideProfile.thighInward);
  m_RiderBoneControl.Apply(riderNode,m_RideProfile);
  gfl2::renderingengine::scenegraph::SceneGraphManager::TraverseModelFast(
    reinterpret_cast<gfl2::renderingengine::scenegraph::DagNode*>(riderNode));
  if( m_RideAttachmentJoint>=0 && m_RidePelvisJoint>=0 )
  {
    auto* mountNode=pokemon->GetModelInstanceNode();
    gfl2::renderingengine::scenegraph::SceneGraphManager::TraverseModelFast(
      reinterpret_cast<gfl2::renderingengine::scenegraph::DagNode*>(mountNode));
    const auto anchor=mountNode->GetJointInstanceNode(m_RideAttachmentJoint)->GetWorldPositionRaw();
    const auto pelvis=riderNode->GetJointInstanceNode(m_RidePelvisJoint)->GetWorldPositionRaw();
    // Place the pelvis at the bone, then add the offset in the mount's facing direction. Ignore bone tilt.
    riderPosition.x+=anchor.x+(riderPosition.x-position.x)-pelvis.x;
    riderPosition.y+=anchor.y+m_RideProfile.seat[1]-pelvis.y;
    riderPosition.z+=anchor.z+(riderPosition.z-position.z)-pelvis.z;
    rider->SetPosition(riderPosition);
    gfl2::renderingengine::scenegraph::SceneGraphManager::TraverseModelFast(
      reinterpret_cast<gfl2::renderingengine::scenegraph::DagNode*>(riderNode));
  }
  Follower3gx_SetRidePresentation(playerWork,player,rider,true,
    m_IsFieldEventSuspended ? 1.0f : m_RideProfile.movementSpeed);
  m_RidePlayerWasVisible = player->IsVisible();
  m_pRideHiddenPlayer = player;
  player->SetVisible( false );
  return true;
}

inline bool Manager::UpdateCarriedPokemon( Fieldmap* fieldmap )
{
  auto* work=m_pDittoMoveModelManager ? m_pDittoMoveModelManager->GetFieldMoveModelRaw(
    Field::MoveModel::FIELD_MOVE_MODEL_PLAYER) : NULL;
  auto* player=work ? work->GetCharaDrawInstanceRaw() : NULL;
  auto* pokemon=GetPokeModel();
  if( !player || !player->IsVisible() || !player->GetModelInstanceNode() || !pokemon ) {
    StopMountedRide(); return false;
  }
  const auto position=fieldmap->GetPlayerPosition();
  const float dx=position.x-m_RidePreviousPosition.x, dz=position.z-m_RidePreviousPosition.z;
  if( dx*dx+dz*dz>300.0f*300.0f ) { StopMountedRide(); return false; }
  m_RidePreviousPosition=position;
  m_Position=position;
  auto* node=player->GetModelInstanceNode();
  m_ModelFacingYaw=node->GetFacingYawRaw();
  auto anchor=position;
  if( m_RideAttachmentJoint>=0 ) {
    gfl2::renderingengine::scenegraph::SceneGraphManager::TraverseModelFast(
      reinterpret_cast<gfl2::renderingengine::scenegraph::DagNode*>(node));
    anchor=node->GetJointInstanceNode(m_RideAttachmentJoint)->GetWorldPositionRaw();
  }
  const float sine=sinf(m_ModelFacingYaw), cosine=cosf(m_ModelFacingYaw);
  anchor.x+=cosine*m_RideProfile.carryOffset[0]+sine*m_RideProfile.carryOffset[2];
  anchor.y+=m_RideProfile.carryOffset[1];
  anchor.z+=cosine*m_RideProfile.carryOffset[2]-sine*m_RideProfile.carryOffset[0];
  SetMotion(PokeTool::MODEL_ANIME_FI_WAIT_A);
  pokemon->SetAnimationIsLoop(true);
  pokemon->SetAnimationStepFrame(1.0f);
  pokemon->SetPosition(anchor);
  const float radians=0.01745329252f;
  pokemon->SetRotation(m_RideProfile.carryRotation[0]*radians,
    m_ModelFacingYaw+m_RideProfile.carryRotation[1]*radians,m_RideProfile.carryRotation[2]*radians);
  pokemon->SetVisible(true);
  if( m_pTrialModel ) { m_pTrialModel->ForwardVisibility(true); m_pTrialModel->SetShadowVisible(false); }
#if FOLLOWER_3GX_PERFORMANCE_FEATURES
  m_AnimationUpdateThisFrame=true;
  ResetInterpolatedAnimationPose();
#endif
  if( m_pFactory ) m_pFactory->TickEntries();
  Follower3gx_SetRidePresentation(work,player,NULL,false,
    m_IsFieldEventSuspended ? 1.0f : m_RideProfile.movementSpeed);
  return true;
}
