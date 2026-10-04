inline gfl2::renderingengine::scenegraph::instance::ModelInstanceNode* Manager::GetFollowerModelInstanceNode( void ) const
{
  if( m_State != STATE_ACTIVE ){ return NULL; }
  if( m_IsFieldEventSuspended )
  {
#if FOLLOWER_3GX_INTERACTION_FEATURES
    // Keep scene and bone updates running during dialogue.
    if( NeedsFieldEventResourceRelease() ||
        (!m_MountedRideActive && !Gen7Follower3gx::KeepFollowerVisibleDuringEvents()) )
      return NULL;
#else
    return NULL;
#endif
  }

  PokeTool::PokeModel* pPokeModel = GetPokeModel();
  if( !pPokeModel ){ return NULL; }
  return pPokeModel->GetModelInstanceNode();
}

#if FOLLOWER_CARRIER_THREEGX
inline bool Manager::GetNetworkState(
  Gen7Follower3gx::FollowerNetworkState* pState
  ) const
{
  if( !pState ){ return false; }
  *pState = Gen7Follower3gx::FollowerNetworkState();
  if( m_State != STATE_ACTIVE || m_IsFieldEventSuspended ||
      !m_HasSimpleParam )
  {
    return false;
  }

  PokeTool::PokeModel* pPokeModel = GetPokeModel();
  gfl2::renderingengine::scenegraph::instance::ModelInstanceNode* pNode =
    pPokeModel ? pPokeModel->GetModelInstanceNode() : NULL;
  if( !pPokeModel || !pNode )
  {
    return false;
  }

  pState->species = static_cast<unsigned short>( m_SimpleParam.monsNo );
  pState->form = static_cast<unsigned short>( m_SimpleParam.formNo );
  pState->sex = static_cast<unsigned char>( m_SimpleParam.sex );
  pState->isRare = m_SimpleParam.isRare ? 1U : 0U;
  pState->personality = m_SimpleParam.perRand;
  pState->x = m_Position.x;
  pState->y = m_Position.y;
  pState->z = m_Position.z;
  pState->yaw = m_ModelFacingYaw;
  pState->locomotion =
    m_CurrentMotion == static_cast<s32>( PokeTool::MODEL_ANIME_RUN01 )
      ? Gen7Follower3gx::FOLLOWER_NETWORK_LOCOMOTION_RUN
      : ( m_CurrentMotion == static_cast<s32>( PokeTool::MODEL_ANIME_WALK01 )
          ? Gen7Follower3gx::FOLLOWER_NETWORK_LOCOMOTION_WALK
          : Gen7Follower3gx::FOLLOWER_NETWORK_LOCOMOTION_IDLE );
  pState->flags = Gen7Follower3gx::FOLLOWER_NETWORK_FLAG_VALID;
  if( pNode->IsVisible() )
  {
    pState->flags |= Gen7Follower3gx::FOLLOWER_NETWORK_FLAG_VISIBLE;
  }
  return true;
}

#if FOLLOWER_CARRIER_DEDICATED_ARENA
inline void Manager::UpdatePartyFollowers(Fieldmap* fieldmap,BaseCollisionScene* ground,
  BaseCollisionScene* walls,BaseCollisionScene* objects)
{
  auto* game=fieldmap ? fieldmap->GetGameManager() : NULL;
  auto* data=game ? FOLLOWER_POKEMON_GET_GAME_DATA(game) : NULL;
  const auto* party=data ? data->GetPlayerPartyConst() : NULL;
  bool valid[6]={};
  if( party ) for( u32 slot=0;slot<party->GetMemberCount() && slot<6;++slot ) {
    const auto* pokemon=party->GetMemberPointerConst(slot);
    valid[slot]=pokemon && !pokemon->IsNull() && !pokemon->IsEgg(pml::pokepara::CHECK_BOTH_EGG);
  }
  int selected[Gen7Follower3gx::PartyFollowerMaximum];
  Gen7Follower3gx::ResolvePartyFollowers(Gen7Follower3gx::GetPartyFollowerSettings(),valid,selected);
  auto leader=m_Position;
  f32 leaderYaw=m_ModelFacingYaw;
  f32 leaderRadius=GetFollowerPlayerBodyRadius(GetPokeModel());
  for( u32 index=0;index<m_RemoteReplicaCapacity;++index ) {
    Gen7Follower3gx::FollowerNetworkState state={};
    if( index+1>=Gen7Follower3gx::PartyFollowerMaximum || selected[index+1]<0 || !party ) {
      m_RemoteReplicas[index].partyMotion.Reset();
      SetRemoteReplicaState(index,state); continue;
    }
    PokeTool::SimpleParam identity;
    PokeTool::GetSimpleParam(&identity,party->GetMemberPointerConst(selected[index+1]));
    state.species=identity.monsNo; state.form=identity.formNo; state.sex=identity.sex;
    state.isRare=identity.isRare; state.personality=identity.perRand;
    state.flags=Gen7Follower3gx::FOLLOWER_NETWORK_FLAG_VALID|Gen7Follower3gx::FOLLOWER_NETWORK_FLAG_VISIBLE;
    auto& replica=m_RemoteReplicas[index];
    auto* model=replica.modelCreated && replica.pTrialModel ? replica.pTrialModel->GetPokeModel() : NULL;
    if (model) {
      const f32 scale=Gen7Follower3gx::GetFollowerSpeciesScale(identity.monsNo);
      model->SetScale(scale,scale,scale);
    }
    const f32 radius=model ? GetFollowerPlayerBodyRadius(model) : 40.0f;
    const f32 spacing=leaderRadius+radius+45.0f;
    auto target=replica.hasPose ? replica.currentPosition : leader;
    const f32 dx=leader.x-target.x, dz=leader.z-target.z;
    const f32 distance=gfl2::math::FSqrt(dx*dx+dz*dz);
    state.yaw=replica.hasPose ? replica.currentYaw : leaderYaw;
    state.locomotion=Gen7Follower3gx::FOLLOWER_NETWORK_LOCOMOTION_IDLE;
    if( !replica.hasPose || distance>Gen7Follower3gx::GetFollowerWarpDistance() ) {
      replica.partyMotion.Reset();
      target=leader;
      target.x-=sinf(leaderYaw)*spacing;
      target.z-=cosf(leaderYaw)*spacing;
      if( !ApplyGround(ground,&target) || IsWallBlocked(walls,objects,leader,target,
          model ? GetFollowerCollisionRadius(model) : 20.0f) ) {
        state.flags=0; SetRemoteReplicaState(index,state); continue;
      }
      // Spawns and warps should move straight to the new position, without sliding across the map.
      replica.hasPose=false;
    } else {
      const f32 advance=replica.partyMotion.Plan(leader.x,leader.z,distance-spacing,
        Gen7Follower3gx::GetFollowerWalkSpeed(),Gen7Follower3gx::GetFollowerRunSpeed());
      bool moved=false;
      if (advance>0 && distance>0) {
        auto candidate=target;
        candidate.x+=dx/distance*advance;
        candidate.z+=dz/distance*advance;
        if( ApplyGround(ground,&candidate) &&
            !IsWallBlocked(walls,objects,target,candidate,model ? GetFollowerCollisionRadius(model) : 20.0f) ) {
          target=candidate;
          state.yaw=static_cast<f32>(atan2(dx,dz));
          moved=true;
        }
      }
      replica.partyMotion.Commit(moved);
      state.locomotion=replica.partyMotion.motion;
    }
    state.x=target.x; state.y=target.y; state.z=target.z;
    SetRemoteReplicaState(index,state);
    leader=replica.hasPose ? replica.currentPosition : target;
    leaderYaw=state.yaw; leaderRadius=radius;
  }
}
inline void Manager::ShowPartyFollowersForEvent(bool visible)
{
  for( u32 index=0;index<m_RemoteReplicaCapacity;++index ) {
    auto& replica=m_RemoteReplicas[index];
    if( !replica.modelCreated || !replica.pTrialModel ) continue;
    auto* pokemon=replica.pTrialModel->GetPokeModel();
    if( !pokemon ) continue;
    const bool show=visible && replica.requestPresent;
    SetRemoteReplicaMotion(replica,PokeTool::MODEL_ANIME_FI_WAIT_A);
    pokemon->SetAnimationStepFrame(1.0f);
    pokemon->SetVisible(show); replica.pTrialModel->ForwardVisibility(show);
  }
}
#endif

inline void Manager::ConfigureRemoteReplicas(
  gfl2::heap::HeapBase* pExternalHeap,
  u32 replicaCapacity
  )
{
  if( m_State != STATE_IDLE &&
      ( pExternalHeap != m_pRemoteReplicaExternalHeap ||
        replicaCapacity != m_RemoteReplicaCapacity ) )
  {
    return;
  }
  m_pRemoteReplicaExternalHeap = pExternalHeap;
  m_RemoteReplicaCapacity = pExternalHeap
    ? ( replicaCapacity < FOLLOWER_REMOTE_REPLICA_MAX
        ? replicaCapacity
        : FOLLOWER_REMOTE_REPLICA_MAX )
    : 0U;
}

inline void Manager::SetRemoteReplicaState(
  u32 replicaIndex,
  const Gen7Follower3gx::FollowerNetworkState& state
  )
{
  if( replicaIndex >= m_RemoteReplicaCapacity )
  {
    return;
  }
  RemoteReplica& replica = m_RemoteReplicas[replicaIndex];
  const bool valid =
    ( state.flags & Gen7Follower3gx::FOLLOWER_NETWORK_FLAG_VALID ) != 0U &&
    state.species != 0U;
  if( !valid )
  {
    replica.requestPresent = false;
    return;
  }
  replica.requested = state;
  replica.requestPresent = true;
  replica.missingFrames = 0;
}

inline gfl2::renderingengine::scenegraph::instance::ModelInstanceNode*
Manager::GetRemoteReplicaModelInstanceNode( u32 replicaIndex ) const
{
  if( replicaIndex >= m_RemoteReplicaCapacity )
  {
    return NULL;
  }
  const RemoteReplica& replica = m_RemoteReplicas[replicaIndex];
  PokeTool::PokeModel* pPokeModel =
    replica.modelCreated && replica.pTrialModel
      ? replica.pTrialModel->GetPokeModel()
      : NULL;
  return pPokeModel ? pPokeModel->GetModelInstanceNode() : NULL;
}

inline void Manager::GetRemoteReplicaDiagnostics(
  Gen7Follower3gx::FollowerReplicaDiagnostics* pDiagnostics
  ) const
{
  if( !pDiagnostics )
  {
    return;
  }
  *pDiagnostics = Gen7Follower3gx::FollowerReplicaDiagnostics();
  pDiagnostics->capacity = m_RemoteReplicaCapacity;
  for( u32 replicaIndex = 0;
       replicaIndex < m_RemoteReplicaCapacity;
       ++replicaIndex )
  {
    const RemoteReplica& replica = m_RemoteReplicas[replicaIndex];
    const u32 bit = 1U << replicaIndex;
    pDiagnostics->species[replicaIndex] = replica.requested.species;
    pDiagnostics->form[replicaIndex] = replica.requested.form;
    pDiagnostics->locomotion[replicaIndex] =
      replica.requested.locomotion;
    if( replica.requestPresent )
    {
      pDiagnostics->requestedMask |= bit;
    }
    if( replica.pTrialModel && !replica.modelCreated )
    {
      pDiagnostics->loadingMask |= bit;
      pDiagnostics->lifecycle[replicaIndex] =
        Gen7Follower3gx::FOLLOWER_REPLICA_LOADING;
    }
    else if( replica.pTrialModel && replica.modelCreated )
    {
      pDiagnostics->activeMask |= bit;
      pDiagnostics->lifecycle[replicaIndex] =
        Gen7Follower3gx::FOLLOWER_REPLICA_ACTIVE;
    }
    else if( replica.requestPresent )
    {
      pDiagnostics->lifecycle[replicaIndex] =
        Gen7Follower3gx::FOLLOWER_REPLICA_REQUESTED;
    }
    if( replica.pTrialModel && !replica.requestPresent )
    {
      pDiagnostics->releasingMask |= bit;
      pDiagnostics->lifecycle[replicaIndex] =
        Gen7Follower3gx::FOLLOWER_REPLICA_RELEASING;
    }
    if( replica.requestPresent && replica.modelCreated &&
        ( replica.requested.flags &
          Gen7Follower3gx::FOLLOWER_NETWORK_FLAG_VISIBLE ) != 0U )
    {
      pDiagnostics->visibleMask |= bit;
    }
  }
}

inline bool Manager::IsRemoteReplicaIdentityCurrent(
  const RemoteReplica& replica
  ) const
{
  return replica.pTrialModel &&
    static_cast<u32>( replica.simpleParam.monsNo ) ==
      static_cast<u32>( replica.requested.species ) &&
    static_cast<u32>( replica.simpleParam.formNo ) ==
      static_cast<u32>( replica.requested.form ) &&
    static_cast<u32>( replica.simpleParam.sex ) ==
      static_cast<u32>( replica.requested.sex ) &&
    replica.simpleParam.isRare == ( replica.requested.isRare != 0U ) &&
    replica.simpleParam.perRand == replica.requested.personality;
}

inline bool Manager::BeginRemoteReplicaLoad( RemoteReplica& replica )
{
  if( !m_pFactory || !m_pFollowerHeap || replica.pTrialModel ||
      !replica.requestPresent )
  {
    return false;
  }
  if( replica.retryFrames != 0U )
  {
    --replica.retryFrames;
    return false;
  }

  replica.simpleParam = PokeTool::SimpleParam();
  replica.simpleParam.monsNo = static_cast<MonsNo>(
    replica.requested.species
    );
  replica.simpleParam.formNo = static_cast<pml::FormNo>(
    replica.requested.form
    );
  replica.simpleParam.sex = static_cast<pml::Sex>(
    replica.requested.sex
    );
  replica.simpleParam.isRare = replica.requested.isRare != 0U;
  replica.simpleParam.isEgg = false;
  replica.simpleParam.perRand = replica.requested.personality;

  PokeTool::PokeModel::SetupOption setupOption;
  setupOption.dataHeap = m_pFollowerHeap;
  setupOption.workHeap =
    GetFollowerWorkHeap();
  if( !setupOption.workHeap )
  {
    setupOption.workHeap = m_pFollowerHeap->GetLowerHandle();
  }
  setupOption.animeType = PokeTool::MODEL_ANIMETYPE_FIELD;
  // Give replicas shadows. They still don't have collision or pathfinding.
  setupOption.useShadow = true;
  setupOption.useIdModel = false;
  replica.pTrialModel = m_pFactory->AllocateEntry(
    m_pFollowerHeap,
    &replica.simpleParam,
    setupOption
    );
  if( !replica.pTrialModel )
  {
    replica.retryFrames = FOLLOWER_SUPPRESSED_RETRY_FRAMES;
  }
  return true;
}

inline bool Manager::ReleaseRemoteReplica( RemoteReplica& replica )
{
  if( !replica.pTrialModel )
  {
    return true;
  }
  replica.pTrialModel->ForwardVisibility( false );
  PokeTool::PokeModel* pPokeModel = replica.pTrialModel->GetPokeModel();
  if( pPokeModel )
  {
    pPokeModel->SetVisible( false );
  }
  if( !replica.pTrialModel->IsLoadComplete() || !m_pFactory )
  {
    return false;
  }
  if( replica.modelCreated && pPokeModel )
  {
    if( !pPokeModel->CanDelete() )
    {
      return false;
    }
    gfl2::renderingengine::scenegraph::instance::ModelInstanceNode* pNode =
      pPokeModel->GetModelInstanceNode();
    if( pNode && pNode->GetReferenceCnt() != 0 )
    {
      return false;
    }
  }
  else
  {
    replica.pTrialModel->ClearDrawEnvNode();
  }

  const Gen7Follower3gx::FollowerNetworkState requested =
    replica.requested;
  const bool requestPresent = replica.requestPresent;
  const u32 missingFrames = replica.missingFrames;
  m_pFactory->ReleaseEntry( replica.pTrialModel );
  replica = RemoteReplica();
  replica.requested = requested;
  replica.requestPresent = requestPresent;
  replica.missingFrames = missingFrames;
  return true;
}

inline bool Manager::ReleaseRemoteReplicas( void )
{
  bool released = true;
  for( u32 replicaIndex = 0;
       replicaIndex < FOLLOWER_REMOTE_REPLICA_MAX;
       ++replicaIndex )
  {
    RemoteReplica& replica = m_RemoteReplicas[replicaIndex];
    replica.requestPresent = false;
    released = ReleaseRemoteReplica( replica ) && released;
  }
  return released;
}

inline void Manager::SetRemoteReplicaMotion(
  RemoteReplica& replica,
  PokeTool::MODEL_ANIME motion
  )
{
  PokeTool::PokeModel* pPokeModel = replica.pTrialModel
    ? replica.pTrialModel->GetPokeModel()
    : NULL;
  if( !pPokeModel )
  {
    return;
  }
  if( !pPokeModel->IsAvailableAnimationDirect(
        static_cast<int>( motion )
        ) )
  {
    motion = PokeTool::MODEL_ANIME_FI_WAIT_A;
  }
  if( replica.currentMotion == static_cast<s32>( motion ) )
  {
    return;
  }
  if( replica.currentMotion == PokeTool::MODEL_ANIME_ERROR )
  {
    pPokeModel->ChangeAnimation( motion );
  }
  else
  {
    pPokeModel->ChangeAnimationSmooth(
      motion,
      FOLLOWER_MOTION_BLEND_FRAMES
      );
  }
  pPokeModel->SetAnimationIsLoop( true );
  pPokeModel->SetAnimationStepFrame( 1.0f );
  replica.currentMotion = static_cast<s32>( motion );
}

inline void Manager::UpdateRemoteReplicas( void )
{
  bool beganLoad = false;
  for( u32 replicaIndex = 0;
       replicaIndex < m_RemoteReplicaCapacity;
       ++replicaIndex )
  {
    RemoteReplica& replica = m_RemoteReplicas[replicaIndex];
    if( !replica.requestPresent )
    {
      if( replica.pTrialModel )
      {
        replica.pTrialModel->ForwardVisibility( false );
        PokeTool::PokeModel* pPokeModel =
          replica.pTrialModel->GetPokeModel();
        if( pPokeModel ){ pPokeModel->SetVisible( false ); }
        if( replica.missingFrames <
              FOLLOWER_REMOTE_REPLICA_RELEASE_FRAMES )
        {
          ++replica.missingFrames;
        }
        else
        {
          ReleaseRemoteReplica( replica );
        }
      }
      continue;
    }

    replica.missingFrames = 0;
    if( replica.pTrialModel &&
        !IsRemoteReplicaIdentityCurrent( replica ) )
    {
      ReleaseRemoteReplica( replica );
      continue;
    }
    if( !replica.pTrialModel )
    {
      if( !beganLoad )
      {
        beganLoad = BeginRemoteReplicaLoad( replica );
      }
      continue;
    }
    if( !replica.modelCreated )
    {
      if( !replica.pTrialModel->IsLoadComplete() )
      {
        continue;
      }
      PokeTool::PokeModel* pPokeModel =
        replica.pTrialModel->GetPokeModel();
      replica.pTrialModel->CreateRenderResources();
      replica.modelCreated = true;
      if( !pPokeModel || !pPokeModel->GetModelInstanceNode() )
      {
        ReleaseRemoteReplica( replica );
        replica.retryFrames = FOLLOWER_SUPPRESSED_RETRY_FRAMES;
        continue;
      }
      replica.pTrialModel->ApplyStoredHeight();
      replica.pTrialModel->EnableAmbientTint( true );
      replica.pTrialModel->SetShadowVisible( true );
      replica.currentPosition.Set(
        replica.requested.x,
        replica.requested.y,
        replica.requested.z
        );
      replica.currentYaw = replica.requested.yaw;
      replica.hasPose = true;
      #if FOLLOWER_CARRIER_DEDICATED_ARENA
      const f32 visualScale=Gen7Follower3gx::GetFollowerSpeciesScale(replica.requested.species);
      pPokeModel->SetScale(visualScale,visualScale,visualScale);
#else
      pPokeModel->SetScale(1.0f,1.0f,1.0f);
#endif
      replica.currentMotion = PokeTool::MODEL_ANIME_ERROR;
    }

    PokeTool::PokeModel* pPokeModel =
      replica.pTrialModel->GetPokeModel();
    if( !pPokeModel || !pPokeModel->GetModelInstanceNode() )
    {
      ReleaseRemoteReplica( replica );
      continue;
    }

    const f32 deltaX = replica.requested.x - replica.currentPosition.x;
    const f32 deltaY = replica.requested.y - replica.currentPosition.y;
    const f32 deltaZ = replica.requested.z - replica.currentPosition.z;
    const f32 distanceSq =
      deltaX * deltaX + deltaY * deltaY + deltaZ * deltaZ;
    const f32 teleportDistanceSq =
      FOLLOWER_REMOTE_REPLICA_TELEPORT_DISTANCE *
      FOLLOWER_REMOTE_REPLICA_TELEPORT_DISTANCE;
    if( !replica.hasPose || distanceSq > teleportDistanceSq )
    {
      replica.currentPosition.Set(
        replica.requested.x,
        replica.requested.y,
        replica.requested.z
        );
      replica.currentYaw = replica.requested.yaw;
      replica.hasPose = true;
    }
    else
    {
#if FOLLOWER_CARRIER_DEDICATED_ARENA
      const f32 interpolation=1.0f;
#else
      const f32 interpolation=FOLLOWER_REMOTE_REPLICA_INTERPOLATION;
#endif
      replica.currentPosition.x +=
        deltaX * interpolation;
      replica.currentPosition.y +=
        deltaY * interpolation;
      replica.currentPosition.z +=
        deltaZ * interpolation;
      f32 yawDelta = replica.requested.yaw - replica.currentYaw;
      while( yawDelta > FOLLOWER_REMOTE_REPLICA_PI )
      {
        yawDelta -= FOLLOWER_REMOTE_REPLICA_TWO_PI;
      }
      while( yawDelta < -FOLLOWER_REMOTE_REPLICA_PI )
      {
        yawDelta += FOLLOWER_REMOTE_REPLICA_TWO_PI;
      }
      replica.currentYaw +=
        yawDelta * FOLLOWER_REMOTE_REPLICA_INTERPOLATION;
    }

    pPokeModel->SetPosition( replica.currentPosition );
    pPokeModel->SetRotation( 0.0f, replica.currentYaw, 0.0f );
    PokeTool::MODEL_ANIME motion = PokeTool::MODEL_ANIME_FI_WAIT_A;
    if( replica.requested.locomotion ==
          Gen7Follower3gx::FOLLOWER_NETWORK_LOCOMOTION_WALK )
    {
      motion = PokeTool::MODEL_ANIME_WALK01;
    }
    else if( replica.requested.locomotion ==
               Gen7Follower3gx::FOLLOWER_NETWORK_LOCOMOTION_RUN )
    {
      motion = PokeTool::MODEL_ANIME_RUN01;
    }
    SetRemoteReplicaMotion( replica, motion );
    const bool visible =
      ( replica.requested.flags &
        Gen7Follower3gx::FOLLOWER_NETWORK_FLAG_VISIBLE ) != 0U;
    replica.pTrialModel->ForwardVisibility( visible );
    pPokeModel->SetVisible( visible );
  }
}

inline void Manager::ApplyRemoteReplicaGroundOffsets( void )
{
  for( u32 replicaIndex = 0;
       replicaIndex < m_RemoteReplicaCapacity;
       ++replicaIndex )
  {
    RemoteReplica& replica = m_RemoteReplicas[replicaIndex];
    PokeTool::PokeModel* pPokeModel =
      replica.modelCreated && replica.pTrialModel
        ? replica.pTrialModel->GetPokeModel()
        : NULL;
    if( !pPokeModel )
    {
      continue;
    }
    gfl2::math::Vector3 displayPosition = replica.currentPosition;
    displayPosition.y += GetFollowerAnimationGroundOffset( pPokeModel );
    static_cast<poke_3d::model::PokemonModel*>( pPokeModel )
      ->SetPosition( displayPosition );
  }
}

inline void Manager::HideRemoteReplicas( void )
{
  for( u32 replicaIndex = 0;
       replicaIndex < m_RemoteReplicaCapacity;
       ++replicaIndex )
  {
    RemoteReplica& replica = m_RemoteReplicas[replicaIndex];
    if( !replica.pTrialModel )
    {
      continue;
    }
    replica.pTrialModel->ForwardVisibility( false );
    PokeTool::PokeModel* pPokeModel =
      replica.pTrialModel->GetPokeModel();
    if( pPokeModel ){ pPokeModel->SetVisible( false ); }
  }
}
#endif

#if FOLLOWER_3GX_PERFORMANCE_FEATURES
inline void Manager::SetPerformanceShadowVisible( bool visible )
{
  if( m_pTrialModel )
  {
    m_pTrialModel->SetShadowVisible( visible
#if FOLLOWER_3GX_INTERACTION_FEATURES
      && !(m_MountedRideActive && m_RideProfile.mode==Gen7Follower3gx::RIDE_POKEMON_ON_PLAYER)
#endif
      );
  }
#if FOLLOWER_CARRIER_THREEGX
  for( u32 replicaIndex = 0;
       replicaIndex < m_RemoteReplicaCapacity;
       ++replicaIndex )
  {
    RemoteReplica& replica = m_RemoteReplicas[replicaIndex];
    if( replica.pTrialModel )
    {
      replica.pTrialModel->SetShadowVisible( visible );
    }
  }
#endif
}
#endif

