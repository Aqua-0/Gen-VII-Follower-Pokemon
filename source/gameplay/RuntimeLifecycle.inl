inline Manager::Manager()
: m_State( STATE_IDLE )
, m_pFactory( NULL )
, m_pTrialModel( NULL )
, m_pRenderingPipeLine( NULL )
, m_pFollowerHeap( NULL )
, m_pFollowerModelHeapParent( NULL )
#if FOLLOWER_CARRIER_THREEGX
, m_pRemoteReplicaExternalHeap( NULL )
, m_RemoteReplicaCapacity( 0 )
#endif
#if FOLLOWER_POKEMON_ENABLE_BALL_TRANSITION
, m_pEffectManager( NULL )
, m_SpecialEffects()
, m_BallTransition()
#endif
, m_SimpleParam()
, m_Position( 0.0f, 0.0f, 0.0f )
#if FOLLOWER_3GX_PERFORMANCE_FEATURES
, m_CachedGroundNormal( 0.0f, 1.0f, 0.0f, 0.0f )
, m_pCachedGroundScene( NULL )
, m_CachedGroundUses( 0 )
, m_HasCachedGround( false )
, m_pInterpolatedAnimationNode( NULL )
, m_AnimationUpdatePhase( 0 )
, m_AnimationRateMode( Gen7Follower3gx::FOLLOWER_ANIMATION_RATE_MODE_COUNT )
, m_InterpolatedAnimationJointCount( 0 )
, m_AnimationUpdateThisFrame( true )
, m_InterpolatedAnimationSampleStarted( false )
, m_HasInterpolatedAnimationPose( false )
#endif
, m_FieldEventStartPlayerPosition( 0.0f, 0.0f, 0.0f )
, m_TrailHead( 0 )
, m_TrailCount( 0 )
, m_RunTransitionFrames( 0 )
, m_NoMoveFrames( FOLLOWER_NO_MOVE_WAIT_FRAMES )
, m_CurrentMotion( PokeTool::MODEL_ANIME_ERROR )
, m_AnimationStepFrame( 1.0f )
, m_NominalRootSpeed( 0.0f )
, m_WalkNominalRootSpeed( 0.0f )
, m_RunNominalRootSpeed( 0.0f )
, m_RootMotionSampleDistance( 0.0f )
, m_RootMotionSampleFrames( 0.0f )
, m_CollisionPlaybackRatio( 1.0f )
, m_ModelFacingYaw( 0.0f )
, m_DiagnosticLoadFailure( DIAGNOSTIC_LOAD_FAILURE_NONE )
, m_DiagnosticParentHeapSource( FOLLOWER_PARENT_HEAP_NONE )
, m_DiagnosticParentHeapAllocatableSize( 0 )
, m_DiagnosticIdleReason( FOLLOWER_IDLE_REASON_NONE )
, m_SuppressedRetryFrames( 0 )
, m_UsesEventDeviceHeap( false )
, m_RunMode( false )
#if FOLLOWER_POKEMON_USE_TRAIL_POLICY
, m_TrailMovementState( TrailMovementPolicy::STATE_WAIT )
, m_TrailPreviousPlayerPosition( 0.0f, 0.0f, 0.0f )
, m_TrailPlayerMovingFrames( 0 )
, m_TrailFacingX( 0.0f )
, m_TrailFacingZ( 1.0f )
, m_TrailHasPreviousPlayerPosition( false )
, m_TrailHasFacingDirection( false )
#endif
, m_IsFactoryInitialized( false )
, m_IsFactorySystemCreated( false )
, m_IsTrialModelCreated( false )
, m_ShouldSuppressAfterTerminate( false )
, m_HasSimpleParam( false )
, m_IsFieldEventSuspended( false )
, m_HasFieldEventStartPlayerPosition( false )
#if FOLLOWER_3GX_DIAGNOSTIC
, m_HeapDiagnosticFrames( 0 )
#endif
#if FOLLOWER_3GX_INTERACTION_FEATURES
, m_InteractionState( INTERACTION_STATE_NONE )
, m_InteractionAnimationPackList()
, m_pInteractionFileManager( NULL )
, m_pInteractionResourceHeap( NULL )
, m_pInteractionMotionPack( NULL )
, m_InteractionBufferSize( 0 )
, m_InteractionRealSize( 0 )
, m_InteractionDataId( 0 )
, m_InteractionHeapFree( 0 )
, m_InteractionResourceCount( 0 )
, m_TalkReaction( 0 )
, m_TalkHopOffset( 0.0f )
, m_InteractionPlayingFrames( 0 )
, m_InteractionDiagnosticResult(
    Gen7Follower3gx::FOLLOWER_INTERACTION_RESULT_NOT_ATTEMPTED
    )
, m_InteractionSelectedMotion(
    Gen7Follower3gx::FOLLOWER_INTERACTION_MOTION_NONE
    )
, m_InteractionRandomState( 0x6d2b79f5U )
, m_InteractionPoseBlendFrame( 0 )
, m_InteractionPoseBlendFrames( 0 )
, m_InteractionReturnGroundBlendFrame( 0 )
, m_InteractionPoseBlendWeight( 1.0f )
, m_InteractionPoseBlendStartGroundOffset( 0.0f )
, m_InteractionReturnGroundBlendStart( 0.0f )
, m_InteractionFacingYaw( 0.0f )
, m_InteractionRespondAvailable( false )
, m_InteractionHappyMask( 0 )
, m_InteractionPackLoaded( false )
, m_InteractionPlayingExternal( false )
, m_InteractionPreferHappy( false )
, m_InteractionCancelPending( false )
, m_InteractionFacingComplete( true )
, m_InteractionCryPending( false )
, m_InteractionPoseBlendCapturePending( false )
, m_InteractionPoseBlendActive( false )
, m_InteractionReturnGroundBlendActive( false )
, m_InteractionEffectType( 0 )
, m_InteractionEffectLifetimeFrames( 0 )
, m_InteractionEffectDelayFrames( 0 )
, m_InteractionEffectElapsedFrames( 0 )
, m_InteractionEffectPlacement(
    Gen7Follower3gx::DIAGNOSTIC_EFFECT_PLACEMENT_MID_BODY
    )
, m_InteractionEffectScale( 1.0f )
, m_InteractionEffectPending( false )
, m_VictiniLuckActivationPending( false )
, m_SpeciesActionKind( SPECIES_ACTION_NONE )
, m_SpeciesActionPhase( SPECIES_ACTION_PHASE_NONE )
, m_SpeciesActionOrigin( 0.0f, 0.0f, 0.0f )
, m_SpeciesActionFrame( 0 )
, m_SpeciesActionTrailHead( 0 )
, m_SpeciesActionTrailCount( 0 )
, m_ShayminFlowerTrailFrames( 0 )
, m_ShayminFlowerIntervalFrames( 0 )
, m_ShayminFlowerAlternate( false )
, m_SpeciesActionModelHidden( false )
, m_NecrozmaLightDrainActive( false )
, m_IllusionOriginalParam()
, m_IllusionTargetParam()
, m_IllusionOverrideActive( false )
, m_IllusionModelSwapPending( false )
, m_IllusionReturning( false )
, m_MountedRideActive( false )
, m_MountedRideCleanup( false )
, m_pRideHiddenPlayer( NULL )
, m_RidePlayerWasVisible( false )
, m_RideMotion( 0xffffffffU )
, m_RideStyle( Gen7Follower3gx::RIDER_TAUROS )
, m_pRideMotionData( NULL )
, m_RideMotionPack()
, m_RideMotionPackLoaded( false )
, m_RidePreviousPosition()
, m_DittoTransformState( DITTO_TRANSFORM_NONE )
, m_pDittoMoveModelManager( NULL )
, m_pDittoCloneHeap( NULL )
, m_pDittoDressUpParam( NULL )
, m_DittoPlayerCharacterId( 0 )
, m_DittoModelId( Field::MoveModel::FIELD_MOVE_MODEL_MAX )
, m_DittoHoldFrames( 0 )
, m_DittoParentHeapFree( 0 )
, m_DittoCloneHeapFree( 0 )
, m_DittoDiagnosticResult(
    Gen7Follower3gx::DITTO_TRANSFORM_RESULT_NOT_ATTEMPTED
    )
, m_DittoCloneMotionId( 0 )
, m_DittoCloneMotionPhase( Gen7Follower3gx::DITTO_CLONE_MOTION_NONE )
, m_DittoRevealPending( false )
, m_DittoWorkReserved( false )
, m_DittoResourceCreated( false )
, m_ManualFormSpecies( 0 )
, m_ManualForm( 0 )
, m_ManualFormPending( 0 )
, m_ManualFormOverrideActive( false )
, m_ManualFormEffectPending( false )
, m_CastformWeatherIndex( 0 )
, m_WeatherFormSpecies( 0 )
, m_WeatherBaseForm( FOLLOWER_CASTFORM_FORM_NORMAL )
, m_WeatherTargetForm( FOLLOWER_CASTFORM_FORM_NORMAL )
, m_WeatherFormOverrideActive( false )
, m_WeatherFormActive( false )
, m_WeatherFormChangePending( false )
, m_WeatherReactionPending( false )
#endif
{
  for( u32 i = 0; i < FOLLOWER_TRAIL_COUNT; ++i )
  {
    m_Trail[i].Set( 0.0f, 0.0f, 0.0f );
  }
}

inline void Manager::Update( Fieldmap* pFieldmap, MyRenderingPipeLine* pRenderingPipeLine, BaseCollisionScene* pTerrainWallScene, BaseCollisionScene* pStaticScene )
{
  if( !pFieldmap || !pRenderingPipeLine ){ return; }

#if FOLLOWER_3GX_DIAGNOSTIC
  if( ++m_HeapDiagnosticFrames >= 30 )
  {
    PokeTool::PokeModelSystem* pPokeModelSystem =
      m_IsFactorySystemCreated && m_State == STATE_ACTIVE && m_pFactory
      ? m_pFactory->GetPokeModelSystem()
      : NULL;
    RecordFollowerDiagnosticHeaps(
      pFieldmap,
      pPokeModelSystem
        ? pPokeModelSystem->GetModelHeapRaw( 0 )
        : NULL,
      m_DiagnosticParentHeapSource,
      m_UsesEventDeviceHeap,
      false
      );
    m_HeapDiagnosticFrames = 0;
  }
#endif

#if FOLLOWER_POKEMON_ENABLE_BALL_TRANSITION
  UpdateBallTransition();
#endif

  BaseCollisionScene* pTerrainGroundScene = GetTerrainGroundScene( pFieldmap );

#if FOLLOWER_3GX_INTERACTION_FEATURES
  if( m_State == STATE_WEATHER_FORM_DELETE_WAIT )
  {
    ContinueWeatherFormChange();
    return;
  }
#endif

  if( m_State == STATE_DELETE_WAIT )
  {
    BeginTerminate();
    return;
  }

  if( m_State == STATE_TERM_WAIT )
  {
    FinishTerminate();
    return;
  }

  if( m_UsesEventDeviceHeap && IsRepelExpiryPending( pFieldmap ) )
  {
    SuppressLoadAttempts( DIAGNOSTIC_LOAD_FAILURE_REPEL_EXPIRY_PENDING );
    return;
  }

  if( m_UsesEventDeviceHeap && !IsEventDeviceBorrowAllowed( pFieldmap ) )
  {
    SuppressLoadAttempts( DIAGNOSTIC_LOAD_FAILURE_EVENT_DEVICE_BUSY );
    return;
  }

  GameSys::GameManager* pGameManager = pFieldmap->GetGameManager();
  GameSys::GameEventManager* pEventManager =
    pGameManager
      ? FOLLOWER_POKEMON_GET_GAME_EVENT_MANAGER( pGameManager )
      : NULL;
  const bool fieldEventExists = pEventManager && pEventManager->IsExists();
  if( fieldEventExists )
  {
    m_DiagnosticIdleReason = FOLLOWER_IDLE_REASON_EVENT_ACTIVE;
    SuspendForFieldEvent( pFieldmap );
    return;
  }
#if FOLLOWER_3GX_INTERACTION_FEATURES
  // The manager can disappear before battle uses its weather token. Clear any leftover token when returning to the field.
  if( m_State == STATE_IDLE && !m_WeatherFormActive )
  {
    Gen7Follower3gx::ClearBattleWeatherHandoff();
  }
#endif
  ResumeAfterFieldEvent( pFieldmap );

#if FOLLOWER_3GX_DIAGNOSTIC
  if( ProcessDiagnosticWeatherSelection( pFieldmap ) )
  {
    return;
  }
#endif
#if FOLLOWER_3GX_INTERACTION_FEATURES
  if( RestoreWeatherBaseFormAfterWeather( pFieldmap ) )
  {
    return;
  }
#endif

  if( m_State == STATE_LOAD_SUPPRESSED )
  {
    if( !TryResumeSuppressedLoad( pFieldmap ) )
    {
      return;
    }
  }

  u32 idleReason = FOLLOWER_IDLE_REASON_NONE;
  const pml::pokepara::PokemonParam* pPokemon =
    FindFirstValidPartyPokemon( pFieldmap, &idleReason );
  if( !pPokemon )
  {
#if FOLLOWER_CARRIER_DEDICATED_ARENA
    Gen7Follower3gx::ClearAreaRide();
#endif
    m_DiagnosticIdleReason = idleReason;
    if( m_State == STATE_ACTIVE || m_State == STATE_MODEL_LOAD_WAIT || m_State == STATE_SYSTEM_CREATE_WAIT )
    {
      BeginTerminate();
    }
    return;
  }
  m_DiagnosticIdleReason = FOLLOWER_IDLE_REASON_NONE;

  PokeTool::SimpleParam simpleParam;
  PokeTool::GetSimpleParam( &simpleParam, pPokemon );
#if FOLLOWER_3GX_INTERACTION_FEATURES
  ApplyWeatherFormOverride( &simpleParam );
#endif

  if( m_State != STATE_IDLE && m_HasSimpleParam && !PokeTool::CompareSimpleParam( m_SimpleParam, simpleParam ) )
  {
    BeginTerminate( true );
    return;
  }

  if( m_State != STATE_IDLE && !m_HasSimpleParam )
  {
    m_SimpleParam = simpleParam;
    m_HasSimpleParam = true;
  }

  switch( m_State )
  {
  case STATE_IDLE:
    m_SimpleParam = simpleParam;
    m_HasSimpleParam = true;
    BeginSystemLoad( pFieldmap, pRenderingPipeLine, pTerrainGroundScene );
    break;

  case STATE_INIT_WAIT:
    if( m_pFactory && m_pFactory->IsLoadComplete() )
    {
      CreateFactorySystem();
    }
    break;

  case STATE_SYSTEM_CREATE_WAIT:
    BeginModelLoad();
    break;

  case STATE_MODEL_LOAD_WAIT:
    if( m_pTrialModel && m_pTrialModel->IsLoadComplete() )
    {
      CreateLoadedModel( pFieldmap );
    }
    break;

  case STATE_ACTIVE:
  {
#if FOLLOWER_CARRIER_THREEGX
#if FOLLOWER_CARRIER_DEDICATED_ARENA
    auto* sizeModel=GetPokeModel();
    if (sizeModel) {
      const f32 scale=Gen7Follower3gx::GetFollowerSpeciesScale(m_SimpleParam.monsNo);
      sizeModel->SetScale(scale,scale,scale);
    }
    if (!m_MountedRideActive && Gen7Follower3gx::HasPendingAreaRide()) {
      auto* data=pGameManager ? FOLLOWER_POKEMON_GET_GAME_DATA(pGameManager) : NULL;
      auto* models=data ? data->GetFieldCharaModelManagerRaw() : NULL;
      auto* work=models ? models->GetFieldMoveModelRaw(Field::MoveModel::FIELD_MOVE_MODEL_PLAYER) : NULL;
      auto* player=work ? work->GetCharaDrawInstanceRaw() : NULL;
      if (player && player->IsVisible() && player->GetModelInstanceNode() &&
          Gen7Follower3gx::ConsumeAreaRide(m_SimpleParam.monsNo,m_SimpleParam.perRand))
        TryStartMountedRide(pFieldmap);
    }
    if (m_MountedRideActive)
      Gen7Follower3gx::RememberAreaRide(m_SimpleParam.monsNo,m_SimpleParam.perRand);
    else if (!Gen7Follower3gx::HasPendingAreaRide()) Gen7Follower3gx::ClearAreaRide();
    UpdatePartyFollowers(pFieldmap,pTerrainGroundScene,pTerrainWallScene,pStaticScene);
#endif
    UpdateRemoteReplicas();
#endif
#if FOLLOWER_3GX_INTERACTION_FEATURES
    UpdateShayminFlowerTrail( pFieldmap );
#endif
    UpdateFollower(
      pFieldmap,
      pTerrainGroundScene,
      pTerrainWallScene,
      pStaticScene
      );
#if FOLLOWER_3GX_PERFORMANCE_FEATURES
    SetPerformanceShadowVisible(
      !Gen7Follower3gx::IsPerformanceOptionEnabled(
        Gen7Follower3gx::PERFORMANCE_OPTION_HIDE_FOLLOWER_SHADOW
        )
      );
#endif
    if( m_pFactory
#if FOLLOWER_3GX_PERFORMANCE_FEATURES
        && ShouldTraversePerformanceAnimation()
#endif
      )
    {
      FOLLOWER_PERF_SCOPE(
        modelAfterTraversePerformance,
        Gen7Follower3gx::PERFORMANCE_ZONE_MODEL
      );
#if FOLLOWER_3GX_INTERACTION_FEATURES
      if( !(m_MountedRideActive && m_RideProfile.mode==Gen7Follower3gx::RIDE_POKEMON_ON_PLAYER) )
#endif
        ApplyFollowerAnimationGroundOffset( GetPokeModel() );
#if FOLLOWER_CARRIER_THREEGX
      ApplyRemoteReplicaGroundOffsets();
#endif
#if !(FOLLOWER_CARRIER_THREEGX && FOLLOWER_3GX_PERFORMANCE_FEATURES)
      m_pFactory->UpdateEntryPresentation();
#endif
    }
#if FOLLOWER_3GX_INTERACTION_FEATURES
    if( m_InteractionReturnGroundBlendActive )
    {
      if( m_InteractionReturnGroundBlendFrame <
          FOLLOWER_INTERACTION_RETURN_BLEND_FRAMES )
      {
        ++m_InteractionReturnGroundBlendFrame;
      }
      else
      {
        m_InteractionReturnGroundBlendActive = false;
      }
    }
#endif
    break;
  }

  default:
    break;
  }
}

inline bool Manager::Terminate( void )
{
#if FOLLOWER_3GX_INTERACTION_FEATURES
  StopMountedRide();
#endif
#if FOLLOWER_POKEMON_ENABLE_BALL_TRANSITION
  m_SpecialEffects.Update();
#if FOLLOWER_3GX_DIAGNOSTIC
  PreserveDiagnosticWeatherSelection();
#endif
  m_SpecialEffects.PrepareForFieldExit();
#endif

  if( m_State == STATE_IDLE || m_State == STATE_LOAD_SUPPRESSED )
  {
#if FOLLOWER_POKEMON_ENABLE_BALL_TRANSITION
    if( m_SpecialEffects.IsDynamicEffectBusy() )
    {
      return false;
    }
#endif
    DeleteFollowerHeap();
    ResetPointers();
    return true;
  }

  const bool waitingForModelDelete =
    m_State == STATE_DELETE_WAIT
#if FOLLOWER_3GX_INTERACTION_FEATURES
    || m_State == STATE_WEATHER_FORM_DELETE_WAIT
#endif
    ;
  if( waitingForModelDelete )
  {
    if( !BeginTerminate() )
    {
      return true;
    }

    if( m_State == STATE_DELETE_WAIT )
    {
      return false;
    }

    return FinishTerminate();
  }

  if( m_State == STATE_INIT_WAIT )
  {
    if( m_pFactory && !m_pFactory->IsLoadComplete() )
    {
      return false;
    }
  }

  if( m_State == STATE_MODEL_LOAD_WAIT )
  {
    if( m_pTrialModel && !m_pTrialModel->IsLoadComplete() )
    {
      return false;
    }
  }

  if( m_State != STATE_TERM_WAIT )
  {
    if( !BeginTerminate() )
    {
      return true;
    }

    if( m_State != STATE_TERM_WAIT )
    {
      return false;
    }
  }

  return FinishTerminate();
}

#if FOLLOWER_POKEMON_ENABLE_BALL_TRANSITION
inline void Manager::AdvanceBallTransitionWhileFieldPaused( void )
{
  if( m_pEffectManager && m_SpecialEffects.IsDynamicEffectBusy() )
  {
    m_pEffectManager->TickAndReap();
  }
}
#endif

inline void Manager::ResetPointers( void )
{
#if FOLLOWER_3GX_INTERACTION_FEATURES
  CancelSpeciesAction();
#endif
  m_State = STATE_IDLE;
  m_pFactory = NULL;
  m_pTrialModel = NULL;
  m_pRenderingPipeLine = NULL;
  m_pFollowerHeap = NULL;
  m_pFollowerModelHeapParent = NULL;
#if FOLLOWER_CARRIER_THREEGX
  for( u32 replicaIndex = 0;
       replicaIndex < FOLLOWER_REMOTE_REPLICA_MAX;
       ++replicaIndex )
  {
    m_RemoteReplicas[replicaIndex] = RemoteReplica();
  }
#endif
#if FOLLOWER_POKEMON_ENABLE_BALL_TRANSITION
  m_BallTransition.Reset();
#if FOLLOWER_3GX_DIAGNOSTIC
  PreserveDiagnosticWeatherSelection();
#endif
  m_SpecialEffects.Reset();
  m_pEffectManager = NULL;
#endif
  m_FieldEventStartPlayerPosition.Set( 0.0f, 0.0f, 0.0f );
#if FOLLOWER_3GX_PERFORMANCE_FEATURES
  m_pCachedGroundScene = NULL;
  m_CachedGroundUses = 0;
  m_HasCachedGround = false;
  ResetInterpolatedAnimationPose();
  m_AnimationUpdatePhase = 0;
  m_AnimationRateMode = Gen7Follower3gx::FOLLOWER_ANIMATION_RATE_MODE_COUNT;
  m_AnimationUpdateThisFrame = true;
#endif
  m_TrailHead = 0;
  m_TrailCount = 0;
  m_RunTransitionFrames = 0;
  m_NoMoveFrames = FOLLOWER_NO_MOVE_WAIT_FRAMES;
  m_BlockedFollowFrames = 0;
  m_FollowSpeed = 0.0f;
  m_PlacementPending = true;
  m_CurrentMotion = PokeTool::MODEL_ANIME_ERROR;
  m_AnimationStepFrame = 1.0f;
  m_NominalRootSpeed = 0.0f;
  m_WalkNominalRootSpeed = 0.0f;
  m_RunNominalRootSpeed = 0.0f;
  m_RootMotionSampleDistance = 0.0f;
  m_RootMotionSampleFrames = 0.0f;
  m_CollisionPlaybackRatio = 1.0f;
  m_ModelFacingYaw = 0.0f;
  m_DiagnosticLoadFailure = DIAGNOSTIC_LOAD_FAILURE_NONE;
  m_DiagnosticParentHeapSource = FOLLOWER_PARENT_HEAP_NONE;
  m_DiagnosticParentHeapAllocatableSize = 0;
  m_SuppressedRetryFrames = 0;
  m_UsesEventDeviceHeap = false;
  m_RunMode = false;
#if FOLLOWER_POKEMON_USE_TRAIL_POLICY
  m_TrailMovementState = TrailMovementPolicy::STATE_WAIT;
  m_TrailPreviousPlayerPosition.Set( 0.0f, 0.0f, 0.0f );
  m_TrailPlayerMovingFrames = 0;
  m_TrailFacingX = 0.0f;
  m_TrailFacingZ = 1.0f;
  m_TrailHasPreviousPlayerPosition = false;
  m_TrailHasFacingDirection = false;
#endif
  m_IsFactoryInitialized = false;
  m_IsFactorySystemCreated = false;
  m_IsTrialModelCreated = false;
  m_ShouldSuppressAfterTerminate = false;
  m_HasSimpleParam = false;
  m_IsFieldEventSuspended = false;
  m_HasFieldEventStartPlayerPosition = false;
#if FOLLOWER_3GX_DIAGNOSTIC
  m_HeapDiagnosticFrames = 0;
#endif
#if FOLLOWER_3GX_INTERACTION_FEATURES
  ResetInteractionMembers();
  // Keep the selected form when reloading the model.
  m_ManualFormPending = 0;
  m_ManualFormEffectPending = false;
  m_CastformWeatherIndex = 0;
  m_WeatherFormSpecies = 0;
  m_WeatherBaseForm = FOLLOWER_CASTFORM_FORM_NORMAL;
  m_WeatherTargetForm = FOLLOWER_CASTFORM_FORM_NORMAL;
  m_WeatherFormOverrideActive = false;
  m_WeatherFormActive = false;
  m_WeatherFormChangePending = false;
  m_WeatherReactionPending = false;
#endif
}

inline bool Manager::BeginSystemLoad( Fieldmap* pFieldmap, MyRenderingPipeLine* pRenderingPipeLine, BaseCollisionScene* pTerrainGroundScene )
{
#if FOLLOWER_CARRIER_DEDICATED_ARENA
  const u32 capacity=Gen7Follower3gx::GetFollowerArenaCapacity();
  ConfigureRemoteReplicas(static_cast<gfl2::heap::HeapBase*>(Follower3gx_GetDedicatedArenaHeap()),
    capacity>1 ? capacity-1 : 0);
  if( m_RemoteReplicaCapacity && (!m_pRemoteReplicaExternalHeap ||
      m_pRemoteReplicaExternalHeap->GetTotalAllocatableSize()<
        FOLLOWER_REMOTE_SYSTEM_HEAP_SIZE+(m_RemoteReplicaCapacity+1)*FOLLOWER_POKEMODEL_HEAP_SIZE) ) {
    SuppressLoadAttempts(DIAGNOSTIC_LOAD_FAILURE_PARENT_HEAP_LOW);
    return false;
  }
#endif
  m_DiagnosticLoadFailure = DIAGNOSTIC_LOAD_FAILURE_NONE;
  u32 parentHeapSource = FOLLOWER_PARENT_HEAP_NONE;
  u32 parentHeapAllocatableSize = 0;
  u32 followerHeapSize = FOLLOWER_MODEL_HEAP_SIZE;
  gfl2::heap::HeapBase* pParentHeap = NULL;
#if FOLLOWER_CARRIER_THREEGX
  const u32 externalRequiredSize = FOLLOWER_REMOTE_SYSTEM_HEAP_SIZE +
    ( m_RemoteReplicaCapacity + 1U ) * FOLLOWER_POKEMODEL_HEAP_SIZE;
  if( m_pRemoteReplicaExternalHeap && m_RemoteReplicaCapacity != 0U &&
      m_pRemoteReplicaExternalHeap->GetTotalAllocatableSize() >=
        externalRequiredSize )
  {
    pParentHeap = m_pRemoteReplicaExternalHeap;
    parentHeapSource = FOLLOWER_PARENT_HEAP_EXTERNAL_APPLICATION;
    parentHeapAllocatableSize =
      pParentHeap->GetTotalAllocatableSize();
    followerHeapSize = FOLLOWER_REMOTE_SYSTEM_HEAP_SIZE;
    m_pFollowerModelHeapParent = pParentHeap;
  }
#endif
  if( !pParentHeap )
  {
    pParentHeap = GetFollowerParentHeap(
      pFieldmap,
      FOLLOWER_MODEL_HEAP_SIZE,
      &parentHeapSource,
      &parentHeapAllocatableSize
      );
  }
  m_DiagnosticParentHeapSource = parentHeapSource;
  m_DiagnosticParentHeapAllocatableSize = parentHeapAllocatableSize;

  if( !pParentHeap || parentHeapAllocatableSize < FOLLOWER_MODEL_HEAP_SIZE )
  {
#if FOLLOWER_CARRIER_DEDICATED_ARENA
    SuppressLoadAttempts( pParentHeap ? DIAGNOSTIC_LOAD_FAILURE_PARENT_HEAP_LOW
                                    : DIAGNOSTIC_LOAD_FAILURE_PARENT_HEAP_MISSING );
    return false;
#else
    pParentHeap = GetFollowerParentHeap(
      pFieldmap,
      FOLLOWER_SYSTEM_HEAP_SIZE,
      &parentHeapSource,
      &parentHeapAllocatableSize
      );
    m_DiagnosticParentHeapSource = parentHeapSource;
    m_DiagnosticParentHeapAllocatableSize = parentHeapAllocatableSize;
    followerHeapSize = FOLLOWER_SYSTEM_HEAP_SIZE;

    if( !pParentHeap )
    {
      SuppressLoadAttempts( DIAGNOSTIC_LOAD_FAILURE_PARENT_HEAP_MISSING );
      return false;
    }
    if( parentHeapAllocatableSize < FOLLOWER_SYSTEM_HEAP_SIZE )
    {
      SuppressLoadAttempts( DIAGNOSTIC_LOAD_FAILURE_PARENT_HEAP_LOW );
      return false;
    }
    if( IsRepelExpiryPending( pFieldmap ) )
    {
      SuppressLoadAttempts( DIAGNOSTIC_LOAD_FAILURE_REPEL_EXPIRY_PENDING );
      return false;
    }
    if( !IsEventDeviceBorrowAllowed( pFieldmap ) )
    {
      SuppressLoadAttempts( DIAGNOSTIC_LOAD_FAILURE_EVENT_DEVICE_BUSY );
      return false;
    }

    m_pFollowerModelHeapParent =
      gfl2::heap::Manager::GetHeapByHeapId( HEAPID_EVENT_DEVICE );
    if( !m_pFollowerModelHeapParent ||
        m_pFollowerModelHeapParent->GetTotalAllocatableSize() < FOLLOWER_POKEMODEL_HEAP_SIZE )
    {
      SuppressLoadAttempts( DIAGNOSTIC_LOAD_FAILURE_EVENT_DEVICE_LOW );
      return false;
    }
    m_UsesEventDeviceHeap = true;
#endif
  }

  if( pParentHeap->GetTotalAllocatableSize() < followerHeapSize )
  {
    SuppressLoadAttempts( DIAGNOSTIC_LOAD_FAILURE_PARENT_HEAP_LOW );
    return false;
  }

#if FOLLOWER_3GX_DIAGNOSTIC
  RecordFollowerDiagnosticHeaps(
    pFieldmap,
    NULL,
    m_DiagnosticParentHeapSource,
    m_UsesEventDeviceHeap,
    true
    );
  m_HeapDiagnosticFrames = 0;
#endif

  m_pFollowerHeap = GFL_CREATE_LOCAL_HEAP_NAME(
    pParentHeap,
    followerHeapSize,
    gfl2::heap::HEAP_TYPE_EXP,
    false,
    "FollowerPokemon"
    );
  if( !m_pFollowerHeap )
  {
    SuppressLoadAttempts( DIAGNOSTIC_LOAD_FAILURE_FOLLOWER_HEAP_CREATE );
    return false;
  }

  if( !m_pFollowerModelHeapParent )
  {
    m_pFollowerModelHeapParent = m_pFollowerHeap;
  }

  m_pFactory = GFL_NEW( m_pFollowerHeap ) TrialModel::FieldModelPool();
  if( !m_pFactory )
  {
    SuppressLoadAttempts( DIAGNOSTIC_LOAD_FAILURE_FACTORY_CREATE );
    return false;
  }

  m_pRenderingPipeLine = pRenderingPipeLine;
#if FOLLOWER_POKEMON_ENABLE_BALL_TRANSITION
  m_pEffectManager = pFieldmap->GetEffectManager();
#endif

  TrialModel::FieldModelPool::SetupParam setupParam;
  setupParam.pFieldmap = pFieldmap;
  setupParam.pPipeLine = pRenderingPipeLine;
  setupParam.pEffectManager = pFieldmap->GetEffectManager();
  setupParam.pColScene = pTerrainGroundScene;
  setupParam.pCameraManager = pFieldmap->GetCameraManager();
  ConfigureFollowerShadow();
  setupParam.pTrialShadow = &m_FollowerShadowData;
  setupParam.pFinderShadow = NULL;
  m_pFactory->BindServices( setupParam );
  const bool createShadow = setupParam.pEffectManager != NULL;
  u32 modelCapacity = 1U;
#if FOLLOWER_CARRIER_THREEGX
  modelCapacity += m_RemoteReplicaCapacity;
#endif
  m_pFactory->BeginLoad(
    m_pFollowerHeap,
    m_pFollowerModelHeapParent,
    static_cast<s32>( modelCapacity ),
    createShadow
    );
  m_IsFactoryInitialized = true;
  m_State = STATE_INIT_WAIT;
  return true;
}

inline bool Manager::CreateFactorySystem( void )
{
  if( !m_pFactory || !m_pFollowerHeap || !m_pFollowerModelHeapParent )
  {
    BeginTerminate();
    return false;
  }

  PokeTool::PokeModelSystem::HeapOption heapOption;
  heapOption.animeType = PokeTool::MODEL_ANIMETYPE_FIELD;
  heapOption.useIdModel = false;
  heapOption.useShadow = false;
  heapOption.useColorShader = false;

  const bool previousEdgeNormalMapEnable = m_pRenderingPipeLine && GFL_BOOL_CAST(
    FOLLOWER_POKEMON_GET_EDGE_NORMAL_MAP_ENABLE( m_pRenderingPipeLine )
    );
  m_pFactory->CreateResources( m_pFollowerModelHeapParent, NULL, &heapOption, true );
  if( m_pRenderingPipeLine )
  {
    FOLLOWER_POKEMON_SET_EDGE_NORMAL_MAP_ENABLE(
      m_pRenderingPipeLine,
      previousEdgeNormalMapEnable
      );
  }
  m_IsFactorySystemCreated = true;
  m_State = STATE_SYSTEM_CREATE_WAIT;
  return true;
}

inline void Manager::BeginModelLoad( void )
{
  if( !m_pFactory || !m_pFollowerHeap || !m_HasSimpleParam )
  {
    return;
  }

  PokeTool::PokeModel::SetupOption setupOption;
  setupOption.dataHeap = m_pFollowerHeap;
  setupOption.workHeap = GetFollowerWorkHeap();
  if( !setupOption.workHeap )
  {
    setupOption.workHeap = m_pFollowerHeap->GetLowerHandle();
  }
  setupOption.animeType = PokeTool::MODEL_ANIMETYPE_FIELD;
  setupOption.useShadow = false;
  setupOption.useIdModel = false;

  m_pTrialModel = m_pFactory->AllocateEntry( m_pFollowerHeap, &m_SimpleParam, setupOption );
  if( !m_pTrialModel )
  {
    SuppressLoadAttempts( DIAGNOSTIC_LOAD_FAILURE_MODEL_CREATE );
    return;
  }

  m_State = STATE_MODEL_LOAD_WAIT;
}

inline void Manager::CreateLoadedModel( Fieldmap* pFieldmap )
{
  if( !m_pTrialModel )
  {
    BeginTerminate();
    return;
  }

  PokeTool::PokeModel* pPokeModel = GetPokeModel();
  if( !pPokeModel )
  {
    BeginTerminate();
    return;
  }

  const gfl2::math::Vector3 playerPosition = pFieldmap->GetPlayerPosition();
#if FOLLOWER_3GX_INTERACTION_FEATURES
  const bool completingModelRecycle =
    m_WeatherFormChangePending || m_IllusionModelSwapPending;
  const f32 initialFacingYaw = completingModelRecycle
    ? m_ModelFacingYaw
    : 0.0f;
#else
  const bool completingModelRecycle = false;
  const f32 initialFacingYaw = 0.0f;
#endif
  if( !completingModelRecycle )
  {
    m_PlacementPending = true;
    m_BlockedFollowFrames = 0;
    m_FollowSpeed = 0.0f;
    m_Position.Set(
      playerPosition.x,
      playerPosition.y,
      playerPosition.z + 100.0f
      );
  }

  m_pTrialModel->CreateRenderResources();
  m_IsTrialModelCreated = true;
  if( !pPokeModel->GetModelInstanceNode() )
  {
    SuppressLoadAttempts( DIAGNOSTIC_LOAD_FAILURE_MODEL_CREATE );
    return;
  }

  m_pTrialModel->ApplyStoredHeight();
  UpdateFollowerShadowScale( pPokeModel );
  m_pTrialModel->EnableAmbientTint( true );

  pPokeModel->SetPosition( GetDisplayPosition( pPokeModel ) );
  pPokeModel->SetRotation( 0.0f, initialFacingYaw, 0.0f );
  m_ModelFacingYaw = initialFacingYaw;
  #if FOLLOWER_CARRIER_DEDICATED_ARENA
  const f32 visualScale=Gen7Follower3gx::GetFollowerSpeciesScale(static_cast<u32>(m_SimpleParam.monsNo));
  pPokeModel->SetScale(visualScale,visualScale,visualScale);
#else
  pPokeModel->SetScale( 1.0f, 1.0f, 1.0f );
#endif
  m_CurrentMotion = PokeTool::MODEL_ANIME_ERROR;
  SetMotion( PokeTool::MODEL_ANIME_FI_WAIT_A );

  bool hideForBallTransition = false;
#if FOLLOWER_POKEMON_ENABLE_BALL_TRANSITION
  if( !completingModelRecycle )
  {
    hideForBallTransition = TryStartBallTransition(
      FollowerBallTransition::KIND_SPAWN
      );
  }
#endif
  m_pTrialModel->ForwardVisibility( !hideForBallTransition );
  pPokeModel->SetVisible( !hideForBallTransition );

  if( !completingModelRecycle )
  {
    SeedTrail( playerPosition );
#if FOLLOWER_POKEMON_USE_TRAIL_POLICY
    ResetTrailMovementPolicy( playerPosition );
#endif
  }
  m_State = STATE_ACTIVE;
#if FOLLOWER_3GX_INTERACTION_FEATURES
  if( m_IllusionModelSwapPending )
  {
    CompleteIllusionModelSwap( pFieldmap );
  }
  else if( m_WeatherFormChangePending )
  {
    CompleteWeatherFormChange( pFieldmap );
  }
#endif
}

inline bool Manager::BeginTerminate( bool playBallTransition )
{
#if FOLLOWER_POKEMON_ENABLE_BALL_TRANSITION
#if FOLLOWER_3GX_DIAGNOSTIC
  PreserveDiagnosticWeatherSelection();
#endif
  m_SpecialEffects.RestoreTemporaryWeather();
#endif
#if FOLLOWER_3GX_INTERACTION_FEATURES
  if( !PrepareInteractionForTerminate() )
  {
#if FOLLOWER_POKEMON_ENABLE_BALL_TRANSITION
    if( !playBallTransition )
    {
      m_SpecialEffects.RequestStopAllDynamicEffects();
    }
#endif
    RetireFollowerEdgeTarget();
    HideFollowerModel();
    m_State = STATE_DELETE_WAIT;
    return true;
  }
#endif
#if FOLLOWER_POKEMON_ENABLE_BALL_TRANSITION
  if( m_State != STATE_DELETE_WAIT )
  {
    if( playBallTransition )
    {
      if( !m_SpecialEffects.IsDynamicEffectBusy() )
      {
        TryStartBallTransition( FollowerBallTransition::KIND_DESPAWN );
      }
      else if( !m_BallTransition.IsBusy() )
      {
        m_SpecialEffects.RequestStopAllDynamicEffects();
      }
    }
    else
    {
      m_SpecialEffects.RequestStopAllDynamicEffects();
    }
  }
#else
  (void)playBallTransition;
#endif

#if FOLLOWER_CARRIER_THREEGX
  if( !ReleaseRemoteReplicas() )
  {
    HideFollowerModel();
    m_State = STATE_DELETE_WAIT;
    return true;
  }
#endif

  if( m_pFactory && m_pTrialModel )
  {
    if( m_State != STATE_DELETE_WAIT )
    {
      RetireFollowerEdgeTarget();
      HideFollowerModel();
      m_State = STATE_DELETE_WAIT;
      return true;
    }

    HideFollowerModel();
#if FOLLOWER_POKEMON_ENABLE_BALL_TRANSITION
    if( m_SpecialEffects.IsDynamicEffectBusy() )
    {
      return true;
    }
#endif
    if( !IsPokeModelGpuIdle() )
    {
      return true;
    }

    if( !m_IsTrialModelCreated )
    {
      // Detach the lighting before freeing a model that only finished part of its setup.
      m_pTrialModel->ClearDrawEnvNode();
    }
    m_pFactory->ReleaseEntry( m_pTrialModel );
    m_pTrialModel = NULL;
    m_IsTrialModelCreated = false;
  }

#if FOLLOWER_POKEMON_ENABLE_BALL_TRANSITION
  if( m_SpecialEffects.IsDynamicEffectBusy() )
  {
    m_State = STATE_DELETE_WAIT;
    return true;
  }
#endif

  if( !m_pFactory )
  {
    DeleteFollowerHeap();
    ResetPointers();
    return false;
  }

  if( m_IsFactoryInitialized )
  {
    const bool previousEdgeNormalMapEnable = m_pRenderingPipeLine && GFL_BOOL_CAST(
      FOLLOWER_POKEMON_GET_EDGE_NORMAL_MAP_ENABLE( m_pRenderingPipeLine )
      );
    m_pFactory->BeginShutdown();
    if( m_pRenderingPipeLine )
    {
      FOLLOWER_POKEMON_SET_EDGE_NORMAL_MAP_ENABLE(
        m_pRenderingPipeLine,
        previousEdgeNormalMapEnable
        );
    }
    m_State = STATE_TERM_WAIT;
    return true;
  }

  GFL_SAFE_DELETE( m_pFactory );
  DeleteFollowerHeap();
  ResetPointers();
  return false;
}

inline bool Manager::FinishTerminate( void )
{
  const bool suppressLoad = m_ShouldSuppressAfterTerminate;
  const u32 diagnosticLoadFailure = m_DiagnosticLoadFailure;
  const u32 diagnosticParentHeapSource = m_DiagnosticParentHeapSource;
  const u32 diagnosticParentHeapAllocatableSize = m_DiagnosticParentHeapAllocatableSize;

  if( !m_pFactory || !m_IsFactoryInitialized )
  {
    DeleteFollowerHeap();
    ResetPointers();
    if( suppressLoad )
    {
      m_DiagnosticLoadFailure = diagnosticLoadFailure;
      m_DiagnosticParentHeapSource = diagnosticParentHeapSource;
      m_DiagnosticParentHeapAllocatableSize = diagnosticParentHeapAllocatableSize;
      m_State = STATE_LOAD_SUPPRESSED;
    }
    return true;
  }

  if( !m_pFactory->IsShutdownComplete() )
  {
    return false;
  }

  m_pFactory->ReleaseStorage();
  GFL_SAFE_DELETE( m_pFactory );
  DeleteFollowerHeap();
  ResetPointers();
  if( suppressLoad )
  {
    m_DiagnosticLoadFailure = diagnosticLoadFailure;
    m_DiagnosticParentHeapSource = diagnosticParentHeapSource;
    m_DiagnosticParentHeapAllocatableSize = diagnosticParentHeapAllocatableSize;
    m_State = STATE_LOAD_SUPPRESSED;
  }
  return true;
}

inline bool Manager::TryResumeSuppressedLoad( Fieldmap* pFieldmap )
{
  if( m_DiagnosticLoadFailure != DIAGNOSTIC_LOAD_FAILURE_PARENT_HEAP_MISSING &&
      m_DiagnosticLoadFailure != DIAGNOSTIC_LOAD_FAILURE_PARENT_HEAP_LOW &&
      m_DiagnosticLoadFailure != DIAGNOSTIC_LOAD_FAILURE_EVENT_DEVICE_BUSY &&
      m_DiagnosticLoadFailure != DIAGNOSTIC_LOAD_FAILURE_EVENT_DEVICE_LOW &&
      m_DiagnosticLoadFailure != DIAGNOSTIC_LOAD_FAILURE_REPEL_EXPIRY_PENDING )
  {
    return false;
  }

  if( m_SuppressedRetryFrames > 0 )
  {
    --m_SuppressedRetryFrames;
    return false;
  }

#if FOLLOWER_CARRIER_THREEGX
  const u32 externalRequiredSize = FOLLOWER_REMOTE_SYSTEM_HEAP_SIZE +
    ( m_RemoteReplicaCapacity + 1U ) * FOLLOWER_POKEMODEL_HEAP_SIZE;
  if( m_pRemoteReplicaExternalHeap && m_RemoteReplicaCapacity != 0U &&
      m_pRemoteReplicaExternalHeap->GetTotalAllocatableSize() >=
        externalRequiredSize )
  {
    m_DiagnosticParentHeapSource =
      FOLLOWER_PARENT_HEAP_EXTERNAL_APPLICATION;
    m_DiagnosticParentHeapAllocatableSize =
      m_pRemoteReplicaExternalHeap->GetTotalAllocatableSize();
    m_DiagnosticLoadFailure = DIAGNOSTIC_LOAD_FAILURE_NONE;
    m_SuppressedRetryFrames = 0;
    m_State = STATE_IDLE;
    return true;
  }
#endif

  u32 parentHeapSource = FOLLOWER_PARENT_HEAP_NONE;
  u32 parentHeapAllocatableSize = 0;
  gfl2::heap::HeapBase* pParentHeap = GetFollowerParentHeap(
    pFieldmap,
    FOLLOWER_MODEL_HEAP_SIZE,
    &parentHeapSource,
    &parentHeapAllocatableSize
    );
  m_DiagnosticParentHeapSource = parentHeapSource;
  m_DiagnosticParentHeapAllocatableSize = parentHeapAllocatableSize;

  bool canLoad = pParentHeap &&
    parentHeapAllocatableSize >= FOLLOWER_MODEL_HEAP_SIZE;
#if !FOLLOWER_CARRIER_DEDICATED_ARENA
  if( !canLoad )
  {
    pParentHeap = GetFollowerParentHeap(
      pFieldmap,
      FOLLOWER_SYSTEM_HEAP_SIZE,
      &parentHeapSource,
      &parentHeapAllocatableSize
      );
    m_DiagnosticParentHeapSource = parentHeapSource;
    m_DiagnosticParentHeapAllocatableSize = parentHeapAllocatableSize;

    gfl2::heap::HeapBase* pEventDeviceHeap =
      gfl2::heap::Manager::GetHeapByHeapId( HEAPID_EVENT_DEVICE );
    canLoad = pParentHeap &&
      parentHeapAllocatableSize >= FOLLOWER_SYSTEM_HEAP_SIZE &&
      !IsRepelExpiryPending( pFieldmap ) &&
      IsEventDeviceBorrowAllowed( pFieldmap ) &&
      pEventDeviceHeap &&
      pEventDeviceHeap->GetTotalAllocatableSize() >= FOLLOWER_POKEMODEL_HEAP_SIZE;
  }

#endif

  if( !canLoad )
  {
    m_SuppressedRetryFrames = FOLLOWER_SUPPRESSED_RETRY_FRAMES;
    return false;
  }

  m_DiagnosticLoadFailure = DIAGNOSTIC_LOAD_FAILURE_NONE;
  m_SuppressedRetryFrames = 0;
  m_State = STATE_IDLE;
  return true;
}

inline void Manager::SuppressLoadAttempts( DiagnosticLoadFailure failure )
{
  const u32 diagnosticParentHeapSource = m_DiagnosticParentHeapSource;
  const u32 diagnosticParentHeapAllocatableSize = m_DiagnosticParentHeapAllocatableSize;
  m_ShouldSuppressAfterTerminate = true;
  if( !BeginTerminate() )
  {
    DeleteFollowerHeap();
    ResetPointers();
    m_DiagnosticLoadFailure = failure;
    m_DiagnosticParentHeapSource = diagnosticParentHeapSource;
    m_DiagnosticParentHeapAllocatableSize = diagnosticParentHeapAllocatableSize;
    m_State = STATE_LOAD_SUPPRESSED;
    return;
  }

  m_DiagnosticLoadFailure = failure;
}

inline void Manager::DeleteFollowerHeap( void )
{
  if( m_pFollowerHeap )
  {
    if( m_pFollowerModelHeapParent == m_pFollowerHeap )
    {
      m_pFollowerModelHeapParent = NULL;
    }
    GFL_DELETE_HEAP( m_pFollowerHeap );
    m_pFollowerHeap = NULL;
  }
}

inline void Manager::HideFollowerModel( void )
{
  PokeTool::PokeModel* pPokeModel = GetPokeModel();
  if( !pPokeModel || !pPokeModel->GetModelInstanceNode() )
  {
    return;
  }

  if( m_pTrialModel )
  {
    m_pTrialModel->ForwardVisibility( false );
  }

  pPokeModel->SetVisible( false );
}

inline void Manager::RetireFollowerEdgeTarget( void )
{
  PokeTool::PokeModel* pPokeModel = GetPokeModel();
  gfl2::renderingengine::scenegraph::instance::ModelInstanceNode* pNode =
    pPokeModel ? pPokeModel->GetModelInstanceNode() : NULL;
  if( !m_pRenderingPipeLine || !pNode )
  {
    return;
  }

  // Remove the outline target first so no new draws get queued while we wait for references to clear.
  poke_3d::renderer::EdgeMapSceneRenderPath* pEdgePath =
    *reinterpret_cast<poke_3d::renderer::EdgeMapSceneRenderPath**>(
      reinterpret_cast<u8*>( m_pRenderingPipeLine ) + 0x84
      );
  if( pEdgePath )
  {
    pEdgePath->RemoveEdgeRenderingTarget( pNode );
  }

}

#if FOLLOWER_POKEMON_ENABLE_BALL_TRANSITION
inline gfl2::heap::HeapBase* Manager::GetFollowerModelResourceHeap(
  void
) const
{
  if( !m_IsFactorySystemCreated || !m_IsTrialModelCreated || !m_pFactory )
  {
    return NULL;
  }

  PokeTool::PokeModelSystem* pPokeModelSystem =
    m_pFactory->GetPokeModelSystem();
  return pPokeModelSystem
    ? pPokeModelSystem->GetModelHeapRaw( 0 )
    : NULL;
}

inline void Manager::ReportFieldEventMemory( Fieldmap* pFieldmap ) const
{
  RideEventMemorySnapshot snapshot={};
  snapshot.species=static_cast<u32>(m_SimpleParam.monsNo);
  snapshot.mounted=m_MountedRideActive ? 1U : 0U;
  snapshot.modelBorrowsEventHeap=m_UsesEventDeviceHeap ? 1U : 0U;
  auto* modelHeap=GetFollowerModelResourceHeap();
  if( modelHeap )
  {
    snapshot.modelSize=modelHeap->GetTotalSize();
    snapshot.modelFree=modelHeap->GetTotalFreeSize();
  }
  if( m_pFollowerHeap )
  {
    snapshot.systemSize=m_pFollowerHeap->GetTotalSize();
    snapshot.systemFree=m_pFollowerHeap->GetTotalFreeSize();
  }
  u32 source=0, largest=0;
  GetFollowerParentHeap(pFieldmap,FOLLOWER_MODEL_HEAP_SIZE,&source,&largest);
  snapshot.independentSource=source;
  snapshot.independentLargest=largest;
  snapshot.fullFollowerRequired=FOLLOWER_MODEL_HEAP_SIZE;
  auto* eventHeap=gfl2::heap::Manager::GetHeapByHeapId(HEAPID_EVENT_DEVICE);
  if( eventHeap ) snapshot.eventLargest=eventHeap->GetTotalAllocatableSize();
  Follower3gx_LogEventMemory(&snapshot);
}

inline FollowerDynamicEffectHeapSelection
Manager::SelectDynamicEffectResourceHeap(
  u32 minimumFree
) const
{
  FollowerDynamicEffectHeapSelection selection;
  const u32 maximumValue = ~static_cast<u32>( 0 );
  const u32 modelMinimumFree =
    minimumFree > maximumValue - FOLLOWER_DYNAMIC_EFFECT_MODEL_HEAP_RESERVE
      ? maximumValue
      : minimumFree + FOLLOWER_DYNAMIC_EFFECT_MODEL_HEAP_RESERVE;

  gfl2::heap::HeapBase* pModelHeap = GetFollowerModelResourceHeap();
  if( pModelHeap )
  {
    selection.source =
      Gen7Follower3gx::DYNAMIC_EFFECT_HEAP_POKEMON_MODEL;
    selection.allocatableSize =
      pModelHeap->GetTotalAllocatableSize();
    selection.minimumFree = modelMinimumFree;
    selection.candidateFound = true;
    if( selection.allocatableSize >= selection.minimumFree )
    {
      selection.pHeap = pModelHeap;
      return selection;
    }
  }

  if( !m_UsesEventDeviceHeap )
  {
#if FOLLOWER_CARRIER_DEDICATED_ARENA
    gfl2::heap::HeapBase* pEventDeviceHeap =
      static_cast<gfl2::heap::HeapBase*>(Follower3gx_GetDedicatedArenaHeap());
    const auto fallbackSource = Gen7Follower3gx::DYNAMIC_EFFECT_HEAP_EXTERNAL_APPLICATION;
#else
    gfl2::heap::HeapBase* pEventDeviceHeap =
      gfl2::heap::Manager::GetHeapByHeapId( HEAPID_EVENT_DEVICE );
    const auto fallbackSource = Gen7Follower3gx::DYNAMIC_EFFECT_HEAP_EVENT_DEVICE;
#endif
    if( pEventDeviceHeap )
    {
      const u32 eventAllocatableSize =
        pEventDeviceHeap->GetTotalAllocatableSize();
      if( eventAllocatableSize >= minimumFree )
      {
        selection.pHeap = pEventDeviceHeap;
        selection.source =
          fallbackSource;
        selection.allocatableSize = eventAllocatableSize;
        selection.minimumFree = minimumFree;
        selection.candidateFound = true;
        return selection;
      }

      const u32 eventShortfall = minimumFree - eventAllocatableSize;
      const u32 selectedShortfall = selection.candidateFound
        ? selection.minimumFree - selection.allocatableSize
        : maximumValue;
      if( !selection.candidateFound || eventShortfall < selectedShortfall )
      {
        selection.source =
          fallbackSource;
        selection.allocatableSize = eventAllocatableSize;
        selection.minimumFree = minimumFree;
        selection.candidateFound = true;
      }
    }
  }

  if( !selection.candidateFound )
  {
    selection.minimumFree = minimumFree;
  }
  return selection;
}

inline gfl2::math::Vector3 Manager::GetBallTransitionPosition(
  PokeTool::PokeModel* pPokeModel
) const
{
  gfl2::math::Vector3 position = GetDisplayPosition( pPokeModel );
  f32 height = 50.0f;
  const PokeTool::PokeSettingData* pSettingData = pPokeModel
    ? pPokeModel->GetSettingData()
    : NULL;
  if( pSettingData && pSettingData->cmHeight > 0 )
  {
    height = static_cast<f32>( pSettingData->cmHeight ) * 0.5f;
    if( height < 30.0f ){ height = 30.0f; }
    if( height > 120.0f ){ height = 120.0f; }
  }
  position.y += height;
  return position;
}

inline bool Manager::TryStartBallTransition(
  FollowerBallTransition::Kind kind
)
{
  const FollowerDynamicEffectHeapSelection heapSelection =
    SelectDynamicEffectResourceHeap(
      FollowerBallTransition::GetMinimumResourceHeapFree()
      );
  const FollowerBallTransition::WorkSlotAvailability slotAvailability =
    FollowerBallTransition::InspectWorkSlots( m_pEffectManager );
  const Effect::EffectManager::WorkType selectedWorkType =
    FollowerBallTransition::SelectWorkType( slotAvailability );
  const bool resourceWasLoaded = m_pEffectManager &&
    GFL_BOOL_CAST( m_pEffectManager->IsDataAvailable(
      Effect::EFFECT_TYPE_B_DEMO
      ) );

#if FOLLOWER_3GX_DIAGNOSTIC
  const Gen7Follower3gx::BallTransitionDiagnosticKind diagnosticKind =
    kind == FollowerBallTransition::KIND_SPAWN
      ? Gen7Follower3gx::BALL_TRANSITION_DIAGNOSTIC_SPAWN
      : Gen7Follower3gx::BALL_TRANSITION_DIAGNOSTIC_RECALL;
#define FOLLOWER_RECORD_BALL_START(result) \
  Gen7Follower3gx::RecordBallTransitionStart( \
    diagnosticKind, \
    result, \
    heapSelection.source, \
    heapSelection.allocatableSize, \
    heapSelection.minimumFree, \
    resourceWasLoaded, \
    slotAvailability.system, \
    slotAvailability.event, \
    slotAvailability.weather, \
    slotAvailability.ride, \
    static_cast<unsigned int>( selectedWorkType ) \
    )
#else
#define FOLLOWER_RECORD_BALL_START(result) ((void)0)
#endif

  if( !m_pEffectManager )
  {
    FOLLOWER_RECORD_BALL_START(
      Gen7Follower3gx::BALL_TRANSITION_START_NO_EFFECT_MANAGER
      );
    return false;
  }

  PokeTool::PokeModel* pPokeModel = GetPokeModel();
  if( !pPokeModel || !pPokeModel->GetModelInstanceNode() )
  {
    FOLLOWER_RECORD_BALL_START(
      Gen7Follower3gx::BALL_TRANSITION_START_NO_MODEL
      );
    return false;
  }

  if( m_SpecialEffects.IsDynamicEffectBusy() )
  {
    FOLLOWER_RECORD_BALL_START(
      Gen7Follower3gx::BALL_TRANSITION_START_BUSY
      );
    return false;
  }

  if( resourceWasLoaded )
  {
    FOLLOWER_RECORD_BALL_START(
      Gen7Follower3gx::BALL_TRANSITION_START_RESOURCE_ALREADY_LOADED
      );
    return false;
  }

  if( !heapSelection.pHeap )
  {
    FOLLOWER_RECORD_BALL_START(
      heapSelection.candidateFound
        ? Gen7Follower3gx::BALL_TRANSITION_START_RESOURCE_HEAP_LOW
        : Gen7Follower3gx::BALL_TRANSITION_START_NO_RESOURCE_HEAP
      );
    return false;
  }

  const Gen7Follower3gx::BallTransitionStartResult result =
    m_BallTransition.Start(
      &m_SpecialEffects,
      m_pEffectManager,
      heapSelection.pHeap,
      GetBallTransitionPosition( pPokeModel ),
      kind,
      heapSelection.minimumFree,
      selectedWorkType
      );
  FOLLOWER_RECORD_BALL_START( result );
#undef FOLLOWER_RECORD_BALL_START
  if( result != Gen7Follower3gx::BALL_TRANSITION_START_STARTED )
  {
    return false;
  }

  const u32 soundVolumePercent =
    Gen7Follower3gx::GetFollowerPokeBallSoundVolumePercent();
  if( soundVolumePercent > 0 )
  {
    Sound::PlaySE(
      FOLLOWER_BALL_TRANSITION_SOUND_ID,
      0,
      FOLLOWER_BALL_TRANSITION_SOUND_CONTROL_ID,
      0
      );
    Sound::ChangeSEVolume(
      FOLLOWER_BALL_TRANSITION_SOUND_ID,
      static_cast<f32>( soundVolumePercent ) / 100.0f,
      0,
      FOLLOWER_BALL_TRANSITION_SOUND_CONTROL_ID
      );
  }
  return true;
}

inline void Manager::UpdateBallTransition( void )
{
  m_SpecialEffects.Update();
#if FOLLOWER_3GX_DIAGNOSTIC
  if( !m_SpecialEffects.IsDynamicEffectBusy() )
  {
    Gen7Follower3gx::ReleaseOwnedDiagnosticEffectModule();
  }
#endif
  if( !m_BallTransition.ConsumeRevealRequest() )
  {
    return;
  }

#if FOLLOWER_3GX_INTERACTION_FEATURES
  if( m_ManualFormEffectPending )
  {
    const pml::FormNo targetForm = m_ManualFormPending;
    m_ManualFormPending = 0;
    m_ManualFormEffectPending = false;
    BeginRawFollowerFormChange( targetForm );
    return;
  }

  if( m_DittoTransformState != DITTO_TRANSFORM_NONE )
  {
    m_DittoRevealPending = true;
    return;
  }
#endif

  if( m_State != STATE_ACTIVE || m_IsFieldEventSuspended )
  {
    return;
  }

  PokeTool::PokeModel* pPokeModel = GetPokeModel();
  if( !pPokeModel || !pPokeModel->GetModelInstanceNode() )
  {
    return;
  }

  pPokeModel->SetVisible( true );
  if( m_pTrialModel )
  {
    m_pTrialModel->ForwardVisibility( true );
  }
}
#endif

#include "FollowerEventVisibility.inl"

inline void Manager::ResumeAfterFieldEvent( Fieldmap* pFieldmap )
{
  if( !m_IsFieldEventSuspended )
  {
    return;
  }

  m_IsFieldEventSuspended = false;
  const bool hasStartPosition = m_HasFieldEventStartPlayerPosition;
  m_HasFieldEventStartPlayerPosition = false;
#if FOLLOWER_3GX_INTERACTION_FEATURES
  // If the mount stayed loaded during dialogue, leave it where it is when dialogue ends.
  if( m_MountedRideActive ) return;
#endif
  if( m_State != STATE_ACTIVE || !pFieldmap )
  {
    return;
  }

  PokeTool::PokeModel* pPokeModel = GetPokeModel();
  if( !pPokeModel )
  {
    return;
  }

  const gfl2::math::Vector3 playerPosition = pFieldmap->GetPlayerPosition();
  const f32 playerMoveX = playerPosition.x - m_FieldEventStartPlayerPosition.x;
  const f32 playerMoveY = playerPosition.y - m_FieldEventStartPlayerPosition.y;
  const f32 playerMoveZ = playerPosition.z - m_FieldEventStartPlayerPosition.z;
  const f32 playerMoveDistanceSq =
    playerMoveX * playerMoveX +
    playerMoveY * playerMoveY +
    playerMoveZ * playerMoveZ;

  if( !hasStartPosition ||
      playerMoveDistanceSq > FOLLOWER_IDLE_DISTANCE * FOLLOWER_IDLE_DISTANCE )
  {
    f32 offsetX = hasStartPosition
      ? m_Position.x - m_FieldEventStartPlayerPosition.x
      : 0.0f;
    f32 offsetZ = hasStartPosition
      ? m_Position.z - m_FieldEventStartPlayerPosition.z
      : 1.0f;
    f32 offsetDistanceSq = offsetX * offsetX + offsetZ * offsetZ;
    if( offsetDistanceSq <= FOLLOWER_COLLISION_EPSILON * FOLLOWER_COLLISION_EPSILON )
    {
      offsetX = 0.0f;
      offsetZ = 1.0f;
      offsetDistanceSq = 1.0f;
    }

    f32 resumeDistance =
      GetFollowerPlayerBodyRadius( pPokeModel ) +
      FOLLOWER_PLAYER_COLLISION_RADIUS + 20.0f;
    if( resumeDistance < 100.0f )
    {
      resumeDistance = 100.0f;
    }
    const f32 resumeScale =
      resumeDistance / gfl2::math::FSqrt( offsetDistanceSq );
    m_Position.Set(
      playerPosition.x + offsetX * resumeScale,
      playerPosition.y,
      playerPosition.z + offsetZ * resumeScale
      );
  }

  ApplyGround( GetTerrainGroundScene( pFieldmap ), &m_Position );
  SeedTrail( playerPosition );
#if FOLLOWER_POKEMON_USE_TRAIL_POLICY
  ResetTrailMovementPolicy( playerPosition );
#endif
  m_RunMode = false;
  m_RunTransitionFrames = 0;
  m_NoMoveFrames = FOLLOWER_NO_MOVE_WAIT_FRAMES;
  m_BlockedFollowFrames = 0;
  m_FollowSpeed = 0.0f;
  m_PlacementPending = true;
  m_AnimationStepFrame = 1.0f;
  m_CollisionPlaybackRatio = 1.0f;
  SetMotion( PokeTool::MODEL_ANIME_FI_WAIT_A );
  pPokeModel->SetAnimationStepFrame( m_AnimationStepFrame );
  pPokeModel->SetPosition( GetDisplayPosition( pPokeModel ) );
  pPokeModel->SetVisible( true );
  if( m_pTrialModel )
  {
    m_pTrialModel->ForwardVisibility( true );
  }
}

inline bool Manager::IsPokeModelGpuIdle( void ) const
{
  PokeTool::PokeModel* pPokeModel = GetPokeModel();
  if( !pPokeModel ){ return true; }
  if( !pPokeModel->CanDelete() ){ return false; }

  gfl2::renderingengine::scenegraph::instance::ModelInstanceNode* pNode = pPokeModel->GetModelInstanceNode();
  if( pNode && pNode->GetReferenceCnt() != 0 )
  {
    return false;
  }

  return true;
}

inline u32 Manager::GetDiagnosticTerminateBlocker( void ) const
{
#if FOLLOWER_POKEMON_ENABLE_BALL_TRANSITION
  if( m_SpecialEffects.IsDynamicEffectBusy() )
  {
    return 1;
  }
#endif

  const bool waitingForModelDelete =
    m_State == STATE_DELETE_WAIT
#if FOLLOWER_3GX_INTERACTION_FEATURES
    || m_State == STATE_WEATHER_FORM_DELETE_WAIT
#endif
    ;
  if( waitingForModelDelete )
  {
    PokeTool::PokeModel* pPokeModel = GetPokeModel();
    if( pPokeModel && !pPokeModel->CanDelete() )
    {
      return 2;
    }

    gfl2::renderingengine::scenegraph::instance::ModelInstanceNode* pNode =
      pPokeModel ? pPokeModel->GetModelInstanceNode() : NULL;
    if( pNode && pNode->GetReferenceCnt() != 0 )
    {
      return 3;
    }
  }

  if( m_State == STATE_TERM_WAIT && m_pFactory &&
      m_IsFactoryInitialized && !m_pFactory->IsShutdownComplete() )
  {
    return 4;
  }

  if( m_State == STATE_MODEL_LOAD_WAIT )
  {
    return 5;
  }

  return 0;
}


inline void Manager::UpdatePresentationAfterTraversal()
{
  if (m_State==STATE_ACTIVE && m_pFactory && !m_IsFieldEventSuspended)
    m_pFactory->UpdateEntryPresentation();
}
