inline bool Manager::ReleaseDittoPlayerClone( void )
{
  if( m_DittoWorkReserved && !m_pDittoMoveModelManager )
  {
    return false;
  }

  poke_3d::model::BaseModel* pClone = GetDittoPlayerCloneModel();
  if( pClone )
  {
    pClone->SetVisible( false );
    gfl2::renderingengine::scenegraph::instance::ModelInstanceNode* pNode =
      pClone->GetModelInstanceNode();
    if( pNode && pNode->GetReferenceCnt() != 0 )
    {
      return false;
    }
  }

  if( m_DittoWorkReserved )
  {
    m_pDittoMoveModelManager->TerminateMoveModelWorkResource(
      static_cast<Field::MoveModel::FIELD_MOVE_MODEL_ID>( m_DittoModelId )
      );
    m_DittoWorkReserved = false;
    m_DittoResourceCreated = false;
  }

  // Keep the resource nodes and data until the rider is gone, then free them before its heap.
  if( m_RideMotionPackLoaded )
  {
    m_RideMotionPack.Finalize();
    m_RideMotionPackLoaded = false;
  }
  if( m_pRideMotionData )
  {
    Follower3gx_FreeUltraRidePack( m_pRideMotionData );
    m_pRideMotionData = NULL;
  }

  if( m_pDittoCloneHeap )
  {
    m_DittoCloneHeapFree =
      m_pDittoCloneHeap->GetTotalAllocatableSize();
    GFL_DELETE_HEAP( m_pDittoCloneHeap );
    m_pDittoCloneHeap = NULL;
  }
  m_pDittoMoveModelManager = NULL;
  m_pDittoDressUpParam = NULL;
  return true;
}

inline bool Manager::CreateDittoPlayerClone( bool hideFollower )
{
  if( !m_pDittoMoveModelManager || !m_pDittoCloneHeap ||
      !m_DittoWorkReserved || !m_pDittoDressUpParam ||
      m_DittoModelId >= Field::MoveModel::FieldMoveModelManager::GetModelCount() )
  {
    return false;
  }

  PokeTool::PokeModel* pPokeModel = GetPokeModel();
  if( !pPokeModel || !pPokeModel->GetModelInstanceNode() )
  {
    return false;
  }

  Field::MoveModel::FieldMoveModelHeaderResource header;
  header.position = GetDisplayPosition( pPokeModel );
  header.characterId = m_DittoPlayerCharacterId;
  header.pDressUpParam = m_pDittoDressUpParam;

  const Field::MoveModel::FIELD_MOVE_MODEL_ID modelId =
    static_cast<Field::MoveModel::FIELD_MOVE_MODEL_ID>( m_DittoModelId );
  gfl2::heap::HeapBase*& pSlotHeap =
    m_pDittoMoveModelManager->GetLocalModelHeapRaw( modelId );
  gfl2::heap::HeapBase* pRetailSlotHeap = pSlotHeap;
  pSlotHeap = m_pDittoCloneHeap;
  const int createState =
    m_pDittoMoveModelManager->InitializeMoveModelResource(
      modelId,
      &header,
      NULL
      );
  pSlotHeap = pRetailSlotHeap;

  poke_3d::model::BaseModel* pClone = GetDittoPlayerCloneModel();
  if( createState != 0 || !pClone || !pClone->GetModelInstanceNode() )
  {
    return false;
  }

  pClone->SetPosition( header.position );
  pClone->SetRotation( 0.0f, m_InteractionFacingYaw, 0.0f );
  pClone->ChangeAnimation( 0 );
  pClone->SetAnimationLoop( true );
  pClone->SetAnimationStepFrame( 1.0f );
  pClone->SetVisible( true );
  m_DittoResourceCreated = true;
  m_DittoCloneHeapFree = m_pDittoCloneHeap->GetTotalAllocatableSize();

  if( hideFollower ){ pPokeModel->SetVisible( false ); }
  if( hideFollower && m_pTrialModel )
  {
    m_pTrialModel->ForwardVisibility( false );
  }
  return true;
}

inline bool Manager::StartDittoPlayerSpinReaction(
  poke_3d::model::BaseModel* pClone
)
{
  m_DittoHoldFrames = 0;
  m_DittoCloneMotionId = 0;
  m_DittoCloneMotionPhase = Gen7Follower3gx::DITTO_CLONE_MOTION_FALLBACK;
  if( !pClone ||
      !pClone->IsAnimationExist( FOLLOWER_DITTO_SPIN_POSE_MOTION ) ||
      !pClone->IsAnimationExist( FOLLOWER_DITTO_SPIN_RECOVER_MOTION ) )
  {
    return false;
  }

  pClone->ChangeAnimation( FOLLOWER_DITTO_SPIN_POSE_MOTION );
  pClone->SetAnimationLoop( false );
  pClone->SetAnimationFrame( 0.0f );
  pClone->SetAnimationStepFrame( 1.0f );
  m_DittoCloneMotionId = FOLLOWER_DITTO_SPIN_POSE_MOTION;
  m_DittoCloneMotionPhase = Gen7Follower3gx::DITTO_CLONE_MOTION_POSE;
  return true;
}

inline bool Manager::PreparePlayerCloneAllocation( Fieldmap* pFieldmap )
{
#if !FOLLOWER_POKEMON_ENABLE_BALL_TRANSITION
  (void)pFieldmap;
  return false;
#else
  ResetDittoTransformMembers();
  GameSys::GameManager* pGameManager = pFieldmap
    ? pFieldmap->GetGameManager()
    : NULL;
  GameSys::GameData* pGameData = pGameManager
    ? FOLLOWER_POKEMON_GET_GAME_DATA( pGameManager )
    : NULL;
  Field::MoveModel::FieldMoveModelManager* pMoveModelManager =
    pGameData ? pGameData->GetFieldCharaModelManagerRaw() : NULL;
  if( !pMoveModelManager )
  {
    m_DittoDiagnosticResult =
      Gen7Follower3gx::DITTO_TRANSFORM_RESULT_NO_MANAGER;
    PublishDittoTransformDiagnostics(
      Gen7Follower3gx::DITTO_TRANSFORM_RESULT_NO_MANAGER,
      true,
      false
      );
    return false;
  }

  Field::MoveModel::FieldMoveModel* pPlayerMoveModel =
    pMoveModelManager->GetFieldMoveModelRaw(
      Field::MoveModel::FIELD_MOVE_MODEL_PLAYER
      );
  poke_3d::model::BaseModel* pPlayerModel = pPlayerMoveModel
    ? pPlayerMoveModel->GetCharaDrawInstanceRaw()
    : NULL;
  if( !pPlayerMoveModel || !pPlayerModel ||
      !pPlayerModel->GetModelInstanceNode() )
  {
    m_DittoDiagnosticResult =
      Gen7Follower3gx::DITTO_TRANSFORM_RESULT_NO_PLAYER;
    PublishDittoTransformDiagnostics(
      Gen7Follower3gx::DITTO_TRANSFORM_RESULT_NO_PLAYER,
      true,
      false
      );
    return false;
  }

  void* pDressUpParam = pPlayerMoveModel->GetDressUpParamRaw();
  if( !pDressUpParam )
  {
    m_DittoDiagnosticResult =
      Gen7Follower3gx::DITTO_TRANSFORM_RESULT_NO_DRESS;
    PublishDittoTransformDiagnostics(
      Gen7Follower3gx::DITTO_TRANSFORM_RESULT_NO_DRESS,
      true,
      false
      );
    return false;
  }

  const u32 modelId =
    pMoveModelManager->GetFieldMoveModelIndexFromFreeSpace();
  if( modelId < Field::MoveModel::FIELD_MOVE_MODEL_NPC_START ||
      modelId > Field::MoveModel::FieldMoveModelManager::GetLastNpcModelId() )
  {
    m_DittoModelId = modelId;
    m_DittoDiagnosticResult =
      Gen7Follower3gx::DITTO_TRANSFORM_RESULT_NO_SLOT;
    PublishDittoTransformDiagnostics(
      Gen7Follower3gx::DITTO_TRANSFORM_RESULT_NO_SLOT,
      true,
      false
      );
    return false;
  }

  u32 parentHeapSource = FOLLOWER_PARENT_HEAP_NONE;
  u32 parentHeapFree = 0;
  gfl2::heap::HeapBase* pParentHeap = GetFollowerParentHeap(
    pFieldmap,
    FOLLOWER_DITTO_CLONE_HEAP_SIZE,
    &parentHeapSource,
    &parentHeapFree
    );
  (void)parentHeapSource;
  m_DittoModelId = modelId;
  m_DittoParentHeapFree = parentHeapFree;
  if( !pParentHeap || parentHeapFree < FOLLOWER_DITTO_CLONE_HEAP_SIZE )
  {
    m_DittoDiagnosticResult =
      Gen7Follower3gx::DITTO_TRANSFORM_RESULT_NO_HEAP;
    PublishDittoTransformDiagnostics(
      Gen7Follower3gx::DITTO_TRANSFORM_RESULT_NO_HEAP,
      true,
      false
      );
    return false;
  }

  gfl2::heap::HeapBase* pCloneHeap = GFL_CREATE_LOCAL_HEAP_NAME(
    pParentHeap,
    FOLLOWER_DITTO_CLONE_HEAP_SIZE,
    gfl2::heap::HEAP_TYPE_EXP,
    false,
    "DittoPlayerClone"
    );
  if( !pCloneHeap )
  {
    m_DittoDiagnosticResult =
      Gen7Follower3gx::DITTO_TRANSFORM_RESULT_HEAP_CREATE_FAILED;
    PublishDittoTransformDiagnostics(
      Gen7Follower3gx::DITTO_TRANSFORM_RESULT_HEAP_CREATE_FAILED,
      true,
      false
      );
    return false;
  }

  m_pDittoMoveModelManager = pMoveModelManager;
  m_pDittoCloneHeap = pCloneHeap;
  m_pDittoDressUpParam = pDressUpParam;
  m_DittoPlayerCharacterId = pPlayerMoveModel->GetCharacterIdRaw();
  m_DittoCloneHeapFree = pCloneHeap->GetTotalAllocatableSize();

  Field::MoveModel::FieldMoveModelHeaderWork headerWork;
  const int workState = pMoveModelManager->InitializeMoveModelWork(
    static_cast<Field::MoveModel::FIELD_MOVE_MODEL_ID>( modelId ),
    &headerWork
    );
  if( workState != 0 )
  {
    m_DittoDiagnosticResult =
      Gen7Follower3gx::DITTO_TRANSFORM_RESULT_WORK_FAILED;
    GFL_DELETE_HEAP( m_pDittoCloneHeap );
    m_pDittoCloneHeap = NULL;
    m_pDittoMoveModelManager = NULL;
    m_pDittoDressUpParam = NULL;
    PublishDittoTransformDiagnostics(
      Gen7Follower3gx::DITTO_TRANSFORM_RESULT_WORK_FAILED,
      true,
      false
      );
    return false;
  }
  m_DittoWorkReserved = true;

  return true;
#endif
}

inline bool Manager::TryStartDittoTransform( Fieldmap* pFieldmap )
{
#if !FOLLOWER_POKEMON_ENABLE_BALL_TRANSITION
  (void)pFieldmap;
  return false;
#else
  if( !PreparePlayerCloneAllocation( pFieldmap ) ){ return false; }

  if( !TryStartBallTransition( FollowerBallTransition::KIND_SPAWN ) )
  {
    m_DittoDiagnosticResult =
      Gen7Follower3gx::DITTO_TRANSFORM_RESULT_FLASH_FAILED;
    ReleaseDittoPlayerClone();
    PublishDittoTransformDiagnostics(
      Gen7Follower3gx::DITTO_TRANSFORM_RESULT_FLASH_FAILED,
      true,
      false
      );
    return false;
  }

  m_DittoTransformState = DITTO_TRANSFORM_FLASH_IN;
  m_DittoDiagnosticResult =
    Gen7Follower3gx::DITTO_TRANSFORM_RESULT_STARTED;
  m_DittoRevealPending = false;
  m_DittoHoldFrames = 0;
  m_InteractionState = INTERACTION_STATE_PLAYING;
  m_InteractionPlayingFrames = 0;
  m_InteractionPlayingExternal = false;
  m_InteractionCryPending = false;
  SetMotion( PokeTool::MODEL_ANIME_FI_WAIT_A );
  PublishDittoTransformDiagnostics(
    Gen7Follower3gx::DITTO_TRANSFORM_RESULT_STARTED,
    true,
    false
    );
  return true;
#endif
}

inline void Manager::FinishDittoTransform(
  Fieldmap* pFieldmap,
  bool completed
)
{
  PokeTool::PokeModel* pPokeModel = GetPokeModel();
  if( pPokeModel )
  {
    pPokeModel->ChangeAnimation( PokeTool::MODEL_ANIME_FI_WAIT_A, true );
    pPokeModel->SetAnimationIsLoop( true );
    pPokeModel->SetAnimationStepFrame( 1.0f );
    pPokeModel->SetPosition( GetDisplayPosition( pPokeModel ) );
    pPokeModel->SetVisible( true );
  }
  if( m_pTrialModel )
  {
    m_pTrialModel->ForwardVisibility( true );
  }
  m_CurrentMotion = PokeTool::MODEL_ANIME_FI_WAIT_A;
  m_InteractionState = m_InteractionPackLoaded
    ? INTERACTION_STATE_READY
    : INTERACTION_STATE_NONE;
  m_InteractionPlayingFrames = 0;
  m_InteractionPlayingExternal = false;
  m_InteractionCryPending = false;
  m_InteractionFacingComplete = true;

  if( pFieldmap )
  {
    const gfl2::math::Vector3 playerPosition =
      pFieldmap->GetPlayerPosition();
    SeedTrail( playerPosition );
#if FOLLOWER_POKEMON_USE_TRAIL_POLICY
    ResetTrailMovementPolicy( playerPosition );
#endif
  }

  const Gen7Follower3gx::DittoTransformDiagnosticResult result = completed
    ? Gen7Follower3gx::DITTO_TRANSFORM_RESULT_COMPLETE
    : static_cast<Gen7Follower3gx::DittoTransformDiagnosticResult>(
        m_DittoDiagnosticResult
        );
  m_DittoTransformState = DITTO_TRANSFORM_NONE;
  m_DittoDiagnosticResult = static_cast<u32>( result );
  PublishDittoTransformDiagnostics( result, false, completed );
  ResetDittoTransformMembers();

  if( completed && pFieldmap && pPokeModel )
  {
    // Wait for the clone to finish cleaning up. The reaction may need memory to load its animation pack.
    m_InteractionPreferHappy = true;
    if( m_InteractionPackLoaded )
    {
      if( !StartPettingInteractionMotion( false ) )
      {
        StartFieldInteractionFallback(
          Gen7Follower3gx::FOLLOWER_INTERACTION_RESULT_NO_KW_MOTION,
          false
          );
      }
    }
    else
    {
      BeginInteractionMotionLoad( pFieldmap );
    }
  }
}

inline bool Manager::UpdateDittoTransform(
  Fieldmap* pFieldmap,
  BaseCollisionScene* pTerrainGroundScene
)
{
  if( m_DittoTransformState == DITTO_TRANSFORM_NONE )
  {
    return false;
  }

  PokeTool::PokeModel* pPokeModel = GetPokeModel();
  if( !pPokeModel || !pFieldmap )
  {
    return true;
  }

  ApplyGround( pTerrainGroundScene, &m_Position );
  const gfl2::math::Vector3 playerPosition =
    pFieldmap->GetPlayerPosition();
  if( m_DittoTransformState == DITTO_TRANSFORM_FLASH_IN ||
      m_DittoTransformState == DITTO_TRANSFORM_WAIT_FLASH_IN_RELEASE )
  {
    UpdateInteractionFacing( pPokeModel, playerPosition );
  }
  pPokeModel->SetPosition( GetDisplayPosition( pPokeModel ) );

  if( m_pFactory )
  {
    FOLLOWER_PERF_SCOPE(
      dittoModelUpdatePerformance,
      Gen7Follower3gx::PERFORMANCE_ZONE_MODEL
      );
    m_pFactory->TickEntries();
  }

#if FOLLOWER_POKEMON_ENABLE_BALL_TRANSITION
  if( m_DittoTransformState == DITTO_TRANSFORM_FLASH_IN &&
      m_DittoRevealPending )
  {
    m_DittoRevealPending = false;
    if( CreateDittoPlayerClone() )
    {
      m_DittoTransformState = DITTO_TRANSFORM_WAIT_FLASH_IN_RELEASE;
      m_DittoDiagnosticResult =
        Gen7Follower3gx::DITTO_TRANSFORM_RESULT_ACTIVE;
      PublishDittoTransformDiagnostics(
        Gen7Follower3gx::DITTO_TRANSFORM_RESULT_ACTIVE,
        false,
        false
        );
    }
    else
    {
      m_DittoTransformState = DITTO_TRANSFORM_CLEANUP;
      m_DittoDiagnosticResult =
        Gen7Follower3gx::DITTO_TRANSFORM_RESULT_CLONE_FAILED;
      pPokeModel->SetVisible( true );
      if( m_pTrialModel ){ m_pTrialModel->ForwardVisibility( true ); }
      PublishDittoTransformDiagnostics(
        Gen7Follower3gx::DITTO_TRANSFORM_RESULT_CLONE_FAILED,
        false,
        false
        );
    }
  }

  if( m_DittoTransformState == DITTO_TRANSFORM_WAIT_FLASH_IN_RELEASE &&
      !m_BallTransition.IsBusy() )
  {
    m_DittoTransformState = DITTO_TRANSFORM_CLONE_HOLD;
    StartDittoPlayerSpinReaction( GetDittoPlayerCloneModel() );
    PublishDittoTransformDiagnostics(
      Gen7Follower3gx::DITTO_TRANSFORM_RESULT_ACTIVE,
      false,
      false
      );
  }

  if( m_DittoTransformState == DITTO_TRANSFORM_CLONE_HOLD )
  {
    poke_3d::model::BaseModel* pClone = GetDittoPlayerCloneModel();
    if( !pClone || !pClone->GetModelInstanceNode() )
    {
      m_DittoTransformState = DITTO_TRANSFORM_CLEANUP;
      m_DittoDiagnosticResult =
        Gen7Follower3gx::DITTO_TRANSFORM_RESULT_CLONE_FAILED;
      pPokeModel->SetVisible( true );
      if( m_pTrialModel ){ m_pTrialModel->ForwardVisibility( true ); }
      PublishDittoTransformDiagnostics(
        Gen7Follower3gx::DITTO_TRANSFORM_RESULT_CLONE_FAILED,
        false,
        false
        );
    }
    else
    {
      bool cloneMotionComplete = false;
      pClone->SetPosition( GetDisplayPosition( pPokeModel ) );
      if( m_DittoCloneMotionPhase ==
            Gen7Follower3gx::DITTO_CLONE_MOTION_FALLBACK )
      {
        const f32 spin = FOLLOWER_INTERACTION_TWO_PI *
          static_cast<f32>( m_DittoHoldFrames ) /
          static_cast<f32>( FOLLOWER_DITTO_CLONE_HOLD_FRAMES );
        pClone->SetRotation(
          0.0f,
          m_InteractionFacingYaw + spin,
          0.0f
          );
        cloneMotionComplete =
          ++m_DittoHoldFrames >= FOLLOWER_DITTO_CLONE_HOLD_FRAMES;
      }
      else
      {
        pClone->SetRotation( 0.0f, m_InteractionFacingYaw, 0.0f );
        ++m_DittoHoldFrames;
        const f32 endFrame = pClone->GetAnimationEndFrame();
        const f32 frame = pClone->GetAnimationFrame();
        const bool reachedEnd = endFrame > 0.0f &&
          frame >= endFrame - FOLLOWER_INTERACTION_END_FRAME_EPSILON;
        const bool reachedSafetyLimit =
          m_DittoHoldFrames >= FOLLOWER_DITTO_CLONE_MOTION_MAX_FRAMES;
        if( reachedEnd || reachedSafetyLimit )
        {
          if( m_DittoCloneMotionPhase ==
                Gen7Follower3gx::DITTO_CLONE_MOTION_POSE )
          {
            pClone->ChangeAnimation( FOLLOWER_DITTO_SPIN_RECOVER_MOTION );
            pClone->SetAnimationLoop( false );
            pClone->SetAnimationFrame( 0.0f );
            pClone->SetAnimationStepFrame( 1.0f );
            m_DittoCloneMotionId = FOLLOWER_DITTO_SPIN_RECOVER_MOTION;
            m_DittoCloneMotionPhase =
              Gen7Follower3gx::DITTO_CLONE_MOTION_RECOVER;
            m_DittoHoldFrames = 0;
          }
          else
          {
            cloneMotionComplete = true;
          }
        }
      }

      if( cloneMotionComplete )
      {
        if( TryStartBallTransition( FollowerBallTransition::KIND_SPAWN ) )
        {
          m_DittoTransformState = DITTO_TRANSFORM_FLASH_OUT;
          m_DittoRevealPending = false;
          PublishDittoTransformDiagnostics(
            Gen7Follower3gx::DITTO_TRANSFORM_RESULT_ACTIVE,
            false,
            false
            );
        }
        else
        {
          pClone->SetVisible( false );
          pPokeModel->SetVisible( true );
          if( m_pTrialModel ){ m_pTrialModel->ForwardVisibility( true ); }
          m_DittoTransformState = DITTO_TRANSFORM_CLEANUP;
          m_DittoDiagnosticResult =
            Gen7Follower3gx::DITTO_TRANSFORM_RESULT_FLASH_FAILED;
          PublishDittoTransformDiagnostics(
            Gen7Follower3gx::DITTO_TRANSFORM_RESULT_FLASH_FAILED,
            false,
            false
            );
        }
      }
    }
  }

  if( m_DittoTransformState == DITTO_TRANSFORM_FLASH_OUT &&
      m_DittoRevealPending )
  {
    m_DittoRevealPending = false;
    poke_3d::model::BaseModel* pClone = GetDittoPlayerCloneModel();
    if( pClone ){ pClone->SetVisible( false ); }
    pPokeModel->SetVisible( true );
    if( m_pTrialModel ){ m_pTrialModel->ForwardVisibility( true ); }
    m_DittoTransformState = DITTO_TRANSFORM_CLEANUP;
    PublishDittoTransformDiagnostics(
      static_cast<Gen7Follower3gx::DittoTransformDiagnosticResult>(
        m_DittoDiagnosticResult
        ),
      false,
      false
      );
  }

  if( m_DittoTransformState == DITTO_TRANSFORM_CLEANUP )
  {
    if( !ReleaseDittoPlayerClone() )
    {
      return true;
    }
    if( m_BallTransition.IsBusy() )
    {
      return true;
    }
    FinishDittoTransform(
      pFieldmap,
      m_DittoDiagnosticResult ==
        Gen7Follower3gx::DITTO_TRANSFORM_RESULT_ACTIVE
      );
  }
#endif
  return true;
}

