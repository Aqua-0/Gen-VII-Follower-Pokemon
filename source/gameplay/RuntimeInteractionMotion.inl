inline void Manager::ResetInteractionMembers( void )
{
  m_InteractionState = INTERACTION_STATE_NONE;
  m_pInteractionFileManager = NULL;
  m_pInteractionResourceHeap = NULL;
  m_pInteractionMotionPack = NULL;
  m_InteractionBufferSize = 0;
  m_InteractionRealSize = 0;
  m_InteractionDataId = 0;
  m_InteractionHeapFree = 0;
  m_InteractionResourceCount = 0;
  m_InteractionPlayingFrames = 0;
  m_InteractionDiagnosticResult =
    Gen7Follower3gx::FOLLOWER_INTERACTION_RESULT_NOT_ATTEMPTED;
  m_InteractionSelectedMotion =
    Gen7Follower3gx::FOLLOWER_INTERACTION_MOTION_NONE;
  m_InteractionPoseBlendFrame = 0;
  m_InteractionPoseBlendFrames = 0;
  m_InteractionReturnGroundBlendFrame = 0;
  m_InteractionPoseBlendWeight = 1.0f;
  m_InteractionPoseBlendStartGroundOffset = 0.0f;
  m_InteractionReturnGroundBlendStart = 0.0f;
  m_InteractionFacingYaw = 0.0f;
  m_InteractionRespondAvailable = false;
  m_InteractionHappyMask = 0;
  m_InteractionPackLoaded = false;
  m_InteractionPlayingExternal = false;
  m_InteractionPreferHappy = false;
  m_InteractionCancelPending = false;
  m_InteractionFacingComplete = true;
  m_InteractionCryPending = false;
  m_InteractionPoseBlendCapturePending = false;
  m_InteractionPoseBlendActive = false;
  m_InteractionReturnGroundBlendActive = false;
  ResetPendingInteractionEffect();
  ResetDittoTransformMembers();
}

inline void Manager::PublishInteractionDiagnostics(
  Gen7Follower3gx::FollowerInteractionDiagnosticResult result,
  Gen7Follower3gx::FollowerInteractionDiagnosticMotion motion,
  bool incrementAttempt,
  bool incrementSuccess
) const
{
  Gen7Follower3gx::FollowerInteractionDiagnosticSnapshot snapshot = {};
  if( m_InteractionCancelPending )
  {
    snapshot.state =
      Gen7Follower3gx::FOLLOWER_INTERACTION_STATE_CANCELED;
  }
  else
  {
    switch( m_InteractionState )
    {
    case INTERACTION_STATE_LOADING:
      snapshot.state =
        Gen7Follower3gx::FOLLOWER_INTERACTION_STATE_LOADING;
      break;
    case INTERACTION_STATE_READY:
      snapshot.state =
        Gen7Follower3gx::FOLLOWER_INTERACTION_STATE_READY;
      break;
    case INTERACTION_STATE_PLAYING:
      snapshot.state =
        Gen7Follower3gx::FOLLOWER_INTERACTION_STATE_PLAYING;
      break;
    default:
      snapshot.state =
        Gen7Follower3gx::FOLLOWER_INTERACTION_STATE_NONE;
      break;
    }
  }
  snapshot.result = static_cast<u32>( result );
  snapshot.dataId = m_InteractionDataId;
  snapshot.heapFree = m_InteractionHeapFree;
  snapshot.bufferSize = m_InteractionBufferSize;
  snapshot.realSize = m_InteractionRealSize;
  snapshot.resourceCount = m_InteractionResourceCount;
  snapshot.respondAvailable = m_InteractionRespondAvailable ? 1U : 0U;
  snapshot.happyAvailable = m_InteractionHappyMask;
  snapshot.selectedMotion = static_cast<u32>( motion );
  Gen7Follower3gx::UpdateFollowerInteractionDiagnostics(
    snapshot,
    incrementAttempt,
    incrementSuccess
    );
}

inline u32 Manager::GetInteractionAnimationDataIndex( void ) const
{
  if( !m_pFactory )
  {
    return 0xffffffffU;
  }

  PokeTool::PokeModelSystem* pModelSystem =
    m_pFactory->GetPokeModelSystem();
  const u32 monsNo = static_cast<u16>( m_SimpleParam.monsNo );
  const PokeTool::PokeModelSystem::POKE_MNG_DATA* pMngData =
    pModelSystem ? pModelSystem->GetMngDataRaw( monsNo ) : NULL;
  if( !pModelSystem || !pMngData || pMngData->dataNum == 0 )
  {
    return 0xffffffffU;
  }

  const u32 originalForm = static_cast<u8>( m_SimpleParam.formNo );
  u32 form = originalForm;
  if( pMngData->dataNum <= form )
  {
    form = 0;
  }

  int dataIndex = pModelSystem->GetDataIdx(
    static_cast<int>( monsNo ),
    static_cast<int>( form ),
    static_cast<int>( m_SimpleParam.sex )
    );
  const int flagDataIndex = pModelSystem->GetDataIdx(
    static_cast<int>( monsNo ),
    static_cast<int>( originalForm ),
    static_cast<int>( m_SimpleParam.sex )
    );
  if( dataIndex < 0 || flagDataIndex < 0 )
  {
    return 0xffffffffU;
  }

  const PokeTool::PokeModelSystem::POKE_FLG_DATA* pFlagData =
    pModelSystem->GetFlgDataRaw( static_cast<u32>( flagDataIndex ) );
  if( pFlagData &&
      ( pFlagData->flags &
        PokeTool::PokeModelSystem::POKE_DATA_FLG_SHARE_ANIME ) )
  {
    dataIndex = pModelSystem->GetDataIdx(
      static_cast<int>( monsNo ),
      static_cast<int>( pFlagData->subForm ),
      0
      );
  }
  if( dataIndex < 0 )
  {
    return 0xffffffffU;
  }

  return static_cast<u32>( dataIndex ) *
    FOLLOWER_INTERACTION_ARCHIVE_MEMBER_STRIDE +
    FOLLOWER_INTERACTION_KW_MEMBER_OFFSET;
}

inline void Manager::StartFieldInteractionFallback(
  Gen7Follower3gx::FollowerInteractionDiagnosticResult reason,
  bool incrementAttempt
)
{
  PokeTool::PokeModel* pPokeModel = GetPokeModel();
  if( !pPokeModel )
  {
    return;
  }

  m_InteractionPoseBlendFrame = 0;
  m_InteractionPoseBlendFrames = 0;
  m_InteractionReturnGroundBlendFrame = 0;
  m_InteractionPoseBlendWeight = 1.0f;
  m_InteractionPoseBlendStartGroundOffset = 0.0f;
  m_InteractionReturnGroundBlendStart = 0.0f;
  m_InteractionPoseBlendCapturePending = false;
  m_InteractionPoseBlendActive = false;
  m_InteractionReturnGroundBlendActive = false;
#if FOLLOWER_3GX_PERFORMANCE_FEATURES
  ResetInterpolatedAnimationPose();
#endif

  PokeTool::MODEL_ANIME motion = PokeTool::MODEL_ANIME_FI_WAIT_B;
  if( !pPokeModel->IsAvailableAnimationDirect(
        static_cast<int>( motion )
        ) )
  {
    motion = PokeTool::MODEL_ANIME_FI_WAIT_A;
  }

  pPokeModel->ChangeAnimation( motion, true );
  pPokeModel->SetAnimationIsLoop( false );
  pPokeModel->SetAnimationFrame( 0.0f );
  pPokeModel->SetAnimationStepFrame( 1.0f );
  m_CurrentMotion = static_cast<s32>( motion );
  m_InteractionState = INTERACTION_STATE_PLAYING;
  m_InteractionPlayingFrames = 0;
  m_InteractionPlayingExternal = false;
  m_InteractionCryPending = true;
  m_InteractionDiagnosticResult = static_cast<u32>( reason );
  m_InteractionSelectedMotion =
    Gen7Follower3gx::FOLLOWER_INTERACTION_MOTION_FIELD;
  PublishInteractionDiagnostics(
    reason,
    Gen7Follower3gx::FOLLOWER_INTERACTION_MOTION_FIELD,
    incrementAttempt,
    true
    );
}

inline bool Manager::BeginInteractionMotionLoad( Fieldmap* pFieldmap )
{
  PokeTool::PokeModelSystem* pModelSystem = m_pFactory
    ? m_pFactory->GetPokeModelSystem()
    : NULL;
  if( !pModelSystem || !m_pFactory->GetAllocator() )
  {
    StartFieldInteractionFallback(
      Gen7Follower3gx::FOLLOWER_INTERACTION_RESULT_NO_SYSTEM,
      true
      );
    return false;
  }

  m_InteractionDataId = GetInteractionAnimationDataIndex();
  m_pInteractionFileManager = pModelSystem->GetAsyncFileManager();
  if( m_InteractionDataId == 0xffffffffU || !m_pInteractionFileManager )
  {
    m_InteractionDataId = 0;
    StartFieldInteractionFallback(
      Gen7Follower3gx::FOLLOWER_INTERACTION_RESULT_NO_SYSTEM,
      true
      );
    return false;
  }

  const gfl2::fs::ArcFile* pArchive =
    m_pInteractionFileManager->GetArcFile(
      FOLLOWER_INTERACTION_ARCHIVE_ID
      );
  u32 storedSize = 0;
  if( !pArchive )
  {
    StartFieldInteractionFallback(
      Gen7Follower3gx::FOLLOWER_INTERACTION_RESULT_NO_SYSTEM,
      true
      );
    return false;
  }
  pArchive->GetDataSize(
    &storedSize,
    m_InteractionDataId,
    NULL
    );
  if( storedSize == 0 || storedSize > 0x70000000U )
  {
    StartFieldInteractionFallback(
      Gen7Follower3gx::FOLLOWER_INTERACTION_RESULT_BAD_PACK,
      true
      );
    return false;
  }

  const u32 requiredHeapSize =
    storedSize + ( storedSize >> 1 ) +
    FOLLOWER_INTERACTION_RESOURCE_HEADROOM;
  // Keep both size estimates so diagnostics can show why a load was rejected.
  m_InteractionBufferSize = storedSize;
  m_InteractionRealSize = requiredHeapSize;
  m_pInteractionResourceHeap = GetFollowerParentHeap(
    pFieldmap,
    requiredHeapSize,
    NULL,
    &m_InteractionHeapFree
    );
  if( !m_pInteractionResourceHeap ||
      m_InteractionHeapFree < requiredHeapSize )
  {
    m_pInteractionResourceHeap = NULL;
    StartFieldInteractionFallback(
      Gen7Follower3gx::FOLLOWER_INTERACTION_RESULT_NO_HEAP,
      true
      );
    return false;
  }

  gfl2::heap::HeapBase* pWorkHeap =
    GetFollowerWorkHeap();
  if( !pWorkHeap )
  {
    pWorkHeap = m_pInteractionResourceHeap;
  }
  gfl2::heap::HeapBase* pWorkLower = pWorkHeap->GetLowerHandle();
  if( !pWorkLower )
  {
    pWorkLower = pWorkHeap;
  }

  m_pInteractionMotionPack = NULL;
  m_InteractionBufferSize = 0;
  m_InteractionRealSize = 0;
  m_InteractionResourceCount = 0;
  m_InteractionRespondAvailable = false;
  m_InteractionHappyMask = 0;
  m_InteractionCancelPending = false;

  gfl2::fs::AsyncFileManager::ArcFileLoadDataReq request;
  request.arcId = static_cast<s32>( FOLLOWER_INTERACTION_ARCHIVE_ID );
  request.datId = static_cast<s32>( m_InteractionDataId );
  request.ppBuf = &m_pInteractionMotionPack;
  request.pBufSize = &m_InteractionBufferSize;
  request.pRealReadSize = &m_InteractionRealSize;
  request.heapForBuf = m_pInteractionResourceHeap;
  request.align = 128;
  request.heapForReq = pWorkLower;
  request.heapForCompressed = pWorkLower;

  m_InteractionState = INTERACTION_STATE_LOADING;
  m_InteractionDiagnosticResult =
    Gen7Follower3gx::FOLLOWER_INTERACTION_RESULT_LOAD_STARTED;
  m_InteractionSelectedMotion =
    Gen7Follower3gx::FOLLOWER_INTERACTION_MOTION_NONE;
  m_pInteractionFileManager->AddArcFileLoadDataReq( request );
  PublishInteractionDiagnostics(
    Gen7Follower3gx::FOLLOWER_INTERACTION_RESULT_LOAD_STARTED,
    Gen7Follower3gx::FOLLOWER_INTERACTION_MOTION_NONE,
    true,
    false
    );
  return true;
}

inline bool Manager::UpdateInteractionFacing(
  PokeTool::PokeModel* pPokeModel,
  const gfl2::math::Vector3& playerPosition
)
{
  if( !pPokeModel )
  {
    return false;
  }

  const f32 dx = playerPosition.x - m_Position.x;
  const f32 dz = playerPosition.z - m_Position.z;
  const f32 distanceSq = dx * dx + dz * dz;
  if( distanceSq <=
      FOLLOWER_COLLISION_EPSILON * FOLLOWER_COLLISION_EPSILON )
  {
    m_InteractionFacingComplete = true;
    return true;
  }

  const f32 targetYaw = static_cast<f32>( atan2( dx, dz ) );
  f32 delta = targetYaw - m_InteractionFacingYaw;
  while( delta > FOLLOWER_INTERACTION_PI )
  {
    delta -= FOLLOWER_INTERACTION_TWO_PI;
  }
  while( delta < -FOLLOWER_INTERACTION_PI )
  {
    delta += FOLLOWER_INTERACTION_TWO_PI;
  }

  const f32 absDelta = delta < 0.0f ? -delta : delta;
  if( absDelta <= FOLLOWER_INTERACTION_TURN_STEP )
  {
    m_InteractionFacingYaw = targetYaw;
    m_InteractionFacingComplete = true;
#if FOLLOWER_POKEMON_USE_TRAIL_POLICY
    const f32 invDistance = 1.0f / gfl2::math::FSqrt( distanceSq );
    m_TrailFacingX = dx * invDistance;
    m_TrailFacingZ = dz * invDistance;
    m_TrailHasFacingDirection = true;
#endif
  }
  else
  {
    m_InteractionFacingYaw += delta < 0.0f
      ? -FOLLOWER_INTERACTION_TURN_STEP
      : FOLLOWER_INTERACTION_TURN_STEP;
    if( m_InteractionFacingYaw > FOLLOWER_INTERACTION_PI )
    {
      m_InteractionFacingYaw -= FOLLOWER_INTERACTION_TWO_PI;
    }
    else if( m_InteractionFacingYaw < -FOLLOWER_INTERACTION_PI )
    {
      m_InteractionFacingYaw += FOLLOWER_INTERACTION_TWO_PI;
    }
    m_InteractionFacingComplete = false;
  }

  pPokeModel->SetRotation( 0.0f, m_InteractionFacingYaw, 0.0f );
  m_ModelFacingYaw = m_InteractionFacingYaw;
  return m_InteractionFacingComplete;
}

inline bool Manager::ValidateInteractionMotionPack( void )
{
  m_InteractionResourceCount = 0;
  m_InteractionRespondAvailable = false;
  m_InteractionHappyMask = 0;
  if( !m_pInteractionMotionPack )
  {
    return false;
  }

  u32 dataSize = m_InteractionRealSize
    ? m_InteractionRealSize
    : m_InteractionBufferSize;
  const u32 binLinkerFixedHeaderSize = sizeof(u16) * 2;
  if( dataSize < binLinkerFixedHeaderSize + sizeof(u32) * 2 ||
      ( m_InteractionBufferSize &&
        m_InteractionRealSize > m_InteractionBufferSize ) )
  {
    return false;
  }
  if( !m_InteractionBufferSize )
  {
    m_InteractionBufferSize = dataSize;
  }

  u8* pData = reinterpret_cast<u8*>(
    m_pInteractionMotionPack
    );
  const u16 signature = *reinterpret_cast<const u16*>( pData );
  const u32 resourceCount = *reinterpret_cast<const u16*>(
    pData + sizeof(u16)
    );
  if( signature != FOLLOWER_INTERACTION_BINLINKER_SIGNATURE ||
      resourceCount == 0 ||
      resourceCount > FOLLOWER_INTERACTION_MAX_BINLINKER_ENTRY_COUNT )
  {
    return false;
  }

  const u32 headerSize = binLinkerFixedHeaderSize +
    sizeof(u32) * ( resourceCount + 1 );
  if( headerSize > dataSize )
  {
    return false;
  }

  const u32* pOffsets = reinterpret_cast<const u32*>(
    pData + binLinkerFixedHeaderSize
    );
  u32 previousOffset = pOffsets[0];
  if( previousOffset < headerSize || previousOffset > dataSize )
  {
    return false;
  }
  for( u32 i = 1; i <= resourceCount; ++i )
  {
    const u32 offset = pOffsets[i];
    if( offset < previousOffset || offset > dataSize )
    {
      return false;
    }
    previousOffset = offset;
  }

  m_InteractionResourceCount = resourceCount;
  m_InteractionRespondAvailable =
    resourceCount > FOLLOWER_INTERACTION_KW_RESPOND &&
    pOffsets[FOLLOWER_INTERACTION_KW_RESPOND + 1] >
      pOffsets[FOLLOWER_INTERACTION_KW_RESPOND];

  const u32 respondOffset = m_InteractionRespondAvailable
    ? pOffsets[FOLLOWER_INTERACTION_KW_RESPOND]
    : 0;
  u32 happyOffsets[3] = { 0, 0, 0 };
  for( u32 i = 0; i < 3; ++i )
  {
    const u32 animationIndex = FOLLOWER_INTERACTION_KW_HAPPY_A + i;
    if( resourceCount > animationIndex &&
        pOffsets[animationIndex + 1] > pOffsets[animationIndex] )
    {
      m_InteractionHappyMask |= 1U << i;
      happyOffsets[i] = pOffsets[animationIndex];
    }
  }

  // Reuse the header for the four reaction offsets. Leave the motion data in place and 128-byte aligned.
  u32* pAnimationPack = reinterpret_cast<u32*>( pData );
  pAnimationPack[0] = FOLLOWER_INTERACTION_ANIMATION_PACK_RESOURCE_COUNT;
  for( u32 i = 0;
       i < FOLLOWER_INTERACTION_ANIMATION_PACK_RESOURCE_COUNT;
       ++i )
  {
    pAnimationPack[i + 1] = 0;
  }
  if( respondOffset )
  {
    pAnimationPack[FOLLOWER_INTERACTION_KW_RESPOND + 1] =
      respondOffset - sizeof(u32);
  }
  for( u32 i = 0; i < 3; ++i )
  {
    if( happyOffsets[i] )
    {
      const u32 animationIndex = FOLLOWER_INTERACTION_KW_HAPPY_A + i;
      pAnimationPack[animationIndex + 1] =
        happyOffsets[i] - sizeof(u32);
    }
  }
  return true;
}

inline bool Manager::StartPettingInteractionMotion(
  bool incrementAttempt
)
{
  if( !m_InteractionPackLoaded || !m_pTrialModel )
  {
    return false;
  }

  gfl2::animation::AnimationPackList* pPackList =
    GetInteractionAnimationPackList();
  PokeTool::PokeModel* pPokeModel = GetPokeModel();
  if( !pPackList || !pPokeModel )
  {
    return false;
  }

  u32 happyIndices[3] = { 0, 0, 0 };
  u32 happyCount = 0;
  for( u32 i = 0; i < 3; ++i )
  {
    if( m_InteractionHappyMask & ( 1U << i ) )
    {
      happyIndices[happyCount++] = FOLLOWER_INTERACTION_KW_HAPPY_A + i;
    }
  }

  u32 selectedIndex = FOLLOWER_INTERACTION_KW_RESPOND;
  bool useHappy = m_InteractionPreferHappy && happyCount != 0;
  if( !useHappy && !m_InteractionRespondAvailable )
  {
    useHappy = happyCount != 0;
  }
  if( useHappy )
  {
    u32 randomValue = m_InteractionRandomState ^ m_InteractionDataId ^
      ( m_TrailHead << 16 ) ^ m_TrailCount;
    if( randomValue == 0 )
    {
      randomValue = 0x6d2b79f5U;
    }
    const u32 rejectionThreshold = ( 0U - happyCount ) % happyCount;
    do
    {
      randomValue ^= randomValue << 13;
      randomValue ^= randomValue >> 17;
      randomValue ^= randomValue << 5;
    }
    while( randomValue < rejectionThreshold );
    m_InteractionRandomState = randomValue;
    selectedIndex = happyIndices[randomValue % happyCount];
  }
  else if( !m_InteractionRespondAvailable )
  {
    return false;
  }

  gfl2::renderingengine::scenegraph::resource::ResourceNode* pResource =
    pPackList->GetResourceNode( 0, selectedIndex );
  if( !pResource )
  {
    return false;
  }

  Gen7Follower3gx::FollowerInteractionDiagnosticResult result =
    Gen7Follower3gx::FOLLOWER_INTERACTION_RESULT_RESPOND;
  Gen7Follower3gx::FollowerInteractionDiagnosticMotion motion =
    Gen7Follower3gx::FOLLOWER_INTERACTION_MOTION_RESPOND;
  if( selectedIndex == FOLLOWER_INTERACTION_KW_HAPPY_A )
  {
    result = Gen7Follower3gx::FOLLOWER_INTERACTION_RESULT_HAPPY_A;
    motion = Gen7Follower3gx::FOLLOWER_INTERACTION_MOTION_HAPPY_A;
  }
  else if( selectedIndex == FOLLOWER_INTERACTION_KW_HAPPY_B )
  {
    result = Gen7Follower3gx::FOLLOWER_INTERACTION_RESULT_HAPPY_B;
    motion = Gen7Follower3gx::FOLLOWER_INTERACTION_MOTION_HAPPY_B;
  }
  else if( selectedIndex == FOLLOWER_INTERACTION_KW_HAPPY_C )
  {
    result = Gen7Follower3gx::FOLLOWER_INTERACTION_RESULT_HAPPY_C;
    motion = Gen7Follower3gx::FOLLOWER_INTERACTION_MOTION_HAPPY_C;
  }

  const f32 startGroundOffset =
    GetFollowerAnimationGroundOffset( pPokeModel );
  m_InteractionPoseBlendFrame = 0;
  m_InteractionPoseBlendFrames = 0;
  m_InteractionReturnGroundBlendFrame = 0;
  m_InteractionPoseBlendWeight = 0.0f;
  m_InteractionPoseBlendStartGroundOffset = startGroundOffset;
  m_InteractionReturnGroundBlendStart = 0.0f;
  m_InteractionPoseBlendCapturePending = false;
  m_InteractionPoseBlendActive = false;
  m_InteractionReturnGroundBlendActive = false;
#if FOLLOWER_3GX_PERFORMANCE_FEATURES
  BeginInterpolatedAnimationSample( pPokeModel->GetModelInstanceNode() );
  m_InteractionPoseBlendCapturePending =
    m_InterpolatedAnimationSampleStarted;
  if( !m_InteractionPoseBlendCapturePending )
  {
    m_InteractionPoseBlendFrames = FOLLOWER_INTERACTION_POSE_BLEND_FRAMES;
    m_InteractionPoseBlendActive = true;
  }
#endif
  pPokeModel->ChangeAnimationResource( pResource, 0 );
  pPokeModel->SetAnimationIsLoop( false );
  pPokeModel->SetAnimationFrame( 0.0f );
  pPokeModel->SetAnimationStepFrame( 1.0f );
  m_CurrentMotion = PokeTool::MODEL_ANIME_ERROR;
  m_InteractionState = INTERACTION_STATE_PLAYING;
  m_InteractionPlayingFrames = 0;
  m_InteractionPlayingExternal = true;
  m_InteractionCryPending = true;
  m_InteractionPreferHappy = !useHappy;
  m_InteractionDiagnosticResult = static_cast<u32>( result );
  m_InteractionSelectedMotion = static_cast<u32>( motion );
  PublishInteractionDiagnostics(
    result,
    motion,
    incrementAttempt,
    true
    );
  return true;
}

inline bool Manager::CompleteInteractionMotionLoad( Fieldmap* pFieldmap )
{
  (void)pFieldmap;
  if( !m_pInteractionFileManager ||
      !m_pInteractionFileManager->IsArcFileLoadDataFinished(
        &m_pInteractionMotionPack
        ) )
  {
    return false;
  }

  if( m_InteractionCancelPending )
  {
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
    if( m_pInteractionMotionPack )
    {
      gfl2::heap::GflHeapFreeMemoryBlock( m_pInteractionMotionPack );
    }
    ResetInteractionMembers();
    return true;
  }

  if( !m_pInteractionMotionPack )
  {
    StartFieldInteractionFallback(
      Gen7Follower3gx::FOLLOWER_INTERACTION_RESULT_LOAD_FAILED,
      false
      );
    return true;
  }
  if( !ValidateInteractionMotionPack() )
  {
    gfl2::heap::GflHeapFreeMemoryBlock( m_pInteractionMotionPack );
    m_pInteractionMotionPack = NULL;
    StartFieldInteractionFallback(
      Gen7Follower3gx::FOLLOWER_INTERACTION_RESULT_BAD_PACK,
      false
      );
    return true;
  }
  if( !m_InteractionRespondAvailable && !m_InteractionHappyMask )
  {
    gfl2::heap::GflHeapFreeMemoryBlock( m_pInteractionMotionPack );
    m_pInteractionMotionPack = NULL;
    StartFieldInteractionFallback(
      Gen7Follower3gx::FOLLOWER_INTERACTION_RESULT_NO_KW_MOTION,
      false
      );
    return true;
  }

  gfl2::animation::AnimationPackList* pPackList =
    GetInteractionAnimationPackList();
  gfl2::gfx::IGLAllocator* pAllocator = m_pFactory
    ? m_pFactory->GetAllocator()
    : NULL;
  if( !pPackList || !pAllocator || !m_pInteractionResourceHeap )
  {
    gfl2::heap::GflHeapFreeMemoryBlock( m_pInteractionMotionPack );
    m_pInteractionMotionPack = NULL;
    StartFieldInteractionFallback(
      Gen7Follower3gx::FOLLOWER_INTERACTION_RESULT_NO_SYSTEM,
      false
      );
    return true;
  }

  pPackList->Initialize( m_pInteractionResourceHeap, 1 );
  pPackList->LoadData(
    0,
    pAllocator,
    m_pInteractionResourceHeap,
    reinterpret_cast<char*>( m_pInteractionMotionPack )
    );
  m_InteractionPackLoaded = true;
  m_InteractionState = INTERACTION_STATE_READY;
  if( !StartPettingInteractionMotion( false ) )
  {
    StartFieldInteractionFallback(
      Gen7Follower3gx::FOLLOWER_INTERACTION_RESULT_NO_KW_MOTION,
      false
      );
  }
  return true;
}

