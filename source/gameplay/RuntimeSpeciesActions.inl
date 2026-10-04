inline void Manager::ResetDittoTransformMembers( void )
{
  m_DittoTransformState = DITTO_TRANSFORM_NONE;
  m_pDittoMoveModelManager = NULL;
  m_pDittoCloneHeap = NULL;
  m_pDittoDressUpParam = NULL;
  m_DittoPlayerCharacterId = 0;
  m_DittoModelId = Field::MoveModel::FIELD_MOVE_MODEL_MAX;
  m_DittoHoldFrames = 0;
  m_DittoParentHeapFree = 0;
  m_DittoCloneHeapFree = 0;
  m_DittoDiagnosticResult =
    Gen7Follower3gx::DITTO_TRANSFORM_RESULT_NOT_ATTEMPTED;
  m_DittoCloneMotionId = 0;
  m_DittoCloneMotionPhase = Gen7Follower3gx::DITTO_CLONE_MOTION_NONE;
  m_DittoRevealPending = false;
  m_DittoWorkReserved = false;
  m_DittoResourceCreated = false;
}

inline void Manager::ResetSpeciesActionMembers( void )
{
  m_SpeciesActionKind = SPECIES_ACTION_NONE;
  m_SpeciesActionPhase = SPECIES_ACTION_PHASE_NONE;
  m_SpeciesActionOrigin.Set( 0.0f, 0.0f, 0.0f );
  m_SpeciesActionFrame = 0;
  m_SpeciesActionTrailHead = 0;
  m_SpeciesActionTrailCount = 0;
  m_ShayminFlowerTrailFrames = 0;
  m_ShayminFlowerIntervalFrames = 0;
  m_ShayminFlowerAlternate = false;
  m_SpeciesActionModelHidden = false;
  m_NecrozmaLightDrainActive = false;
  m_IllusionOverrideActive = false;
  m_IllusionModelSwapPending = false;
  m_IllusionReturning = false;
}

inline void Manager::CancelSpeciesAction( void )
{
  Gen7Follower3gx::SetTransientPerformanceOptionEnabled(
    Gen7Follower3gx::PERFORMANCE_OPTION_DISABLE_BLOOM,
    false
    );
  Gen7Follower3gx::SetTransientPerformanceOptionEnabled(
    Gen7Follower3gx::PERFORMANCE_OPTION_SIMPLE_WORLD_LIGHTING,
    false
    );
  ResetSpeciesActionMembers();
}

inline void Manager::SetSpeciesActionModelVisible( bool visible )
{
  PokeTool::PokeModel* pPokeModel = GetPokeModel();
  if( pPokeModel )
  {
    pPokeModel->SetVisible( visible );
  }
  if( m_pTrialModel )
  {
    m_pTrialModel->ForwardVisibility( visible );
  }
  m_SpeciesActionModelHidden = !visible;
}

inline bool Manager::TryStartSpeciesActionEffect(
  PokeTool::PokeModel* pPokeModel,
  Fieldmap* pFieldmap,
  Effect::Type effectType,
  u32 lifetimeFrames,
  u32 placement,
  f32 scale
)
{
  if( m_InteractionEffectPending )
  {
    return false;
  }

  const u32 savedType = m_InteractionEffectType;
  const u32 savedLifetime = m_InteractionEffectLifetimeFrames;
  const u32 savedDelay = m_InteractionEffectDelayFrames;
  const u32 savedElapsed = m_InteractionEffectElapsedFrames;
  const u32 savedPlacement = m_InteractionEffectPlacement;
  const f32 savedScale = m_InteractionEffectScale;
  const bool savedPending = m_InteractionEffectPending;
  const bool savedVictiniLuck = m_VictiniLuckActivationPending;

  m_InteractionEffectType = static_cast<u32>( effectType );
  m_InteractionEffectLifetimeFrames = lifetimeFrames;
  m_InteractionEffectDelayFrames = 0;
  m_InteractionEffectElapsedFrames = 0;
  m_InteractionEffectPlacement = placement;
  m_InteractionEffectScale = scale;
  m_InteractionEffectPending = false;
  const bool started = TryStartPendingInteractionEffect(
    pPokeModel,
    pFieldmap
    );

  m_InteractionEffectType = savedType;
  m_InteractionEffectLifetimeFrames = savedLifetime;
  m_InteractionEffectDelayFrames = savedDelay;
  m_InteractionEffectElapsedFrames = savedElapsed;
  m_InteractionEffectPlacement = savedPlacement;
  m_InteractionEffectScale = savedScale;
  m_InteractionEffectPending = savedPending;
  m_VictiniLuckActivationPending = savedVictiniLuck;
  return started;
}

inline bool Manager::SelectIllusionTarget(
  Fieldmap* pFieldmap,
  PokeTool::SimpleParam* pTarget
) const
{
  if( !pFieldmap || !pTarget )
  {
    return false;
  }

  GameSys::GameManager* pGameManager = pFieldmap->GetGameManager();
  GameSys::GameData* pGameData = pGameManager
    ? FOLLOWER_POKEMON_GET_GAME_DATA( pGameManager )
    : NULL;
  const pml::PokeParty* pParty = pGameData
    ? pGameData->GetPlayerPartyConst()
    : NULL;
  if( !pParty )
  {
    return false;
  }

  const u32 memberCount = pParty->GetMemberCount();
  bool skippedFollower = false;
  for( u32 i = 0; i < memberCount; ++i )
  {
    const pml::pokepara::PokemonParam* pPokemon =
      pParty->GetMemberPointerConst( i );
    if( !pPokemon || pPokemon->IsNull() ||
        pPokemon->IsEgg( pml::pokepara::CHECK_BOTH_EGG ) )
    {
      continue;
    }

    if( !skippedFollower )
    {
      skippedFollower = true;
      continue;
    }

    PokeTool::SimpleParam candidate;
    PokeTool::GetSimpleParam( &candidate, pPokemon );
    *pTarget = candidate;
    return true;
  }
  return false;
}

inline bool Manager::BeginIllusionModelSwap(
  const PokeTool::SimpleParam& target,
  bool returning
)
{
  if( m_State != STATE_ACTIVE || !m_pFactory || !m_pTrialModel ||
      m_SpecialEffects.IsDynamicEffectBusy() )
  {
    return false;
  }
  if( !PrepareInteractionForTerminate( true ) )
  {
    return false;
  }

  m_IllusionTargetParam = target;
  m_IllusionOverrideActive = true;
  m_IllusionModelSwapPending = true;
  m_IllusionReturning = returning;
  m_SpeciesActionPhase = SPECIES_ACTION_PHASE_WAIT_MODEL;
  SetSpeciesActionModelVisible( false );
  m_State = STATE_WEATHER_FORM_DELETE_WAIT;
  return true;
}

inline void Manager::CompleteIllusionModelSwap( Fieldmap* pFieldmap )
{
  m_IllusionModelSwapPending = false;
  SetSpeciesActionModelVisible( true );
  if( m_IllusionReturning )
  {
    m_IllusionReturning = false;
    m_IllusionOverrideActive = false;
    FinishSpeciesAction( pFieldmap );
    return;
  }

  m_SpeciesActionPhase = SPECIES_ACTION_PHASE_ACTIVE;
  m_SpeciesActionFrame = 0;
  Sound::PlayVoice(
    0,
    m_SimpleParam.monsNo,
    m_SimpleParam.formNo,
    Sound::VOICE_TYPE_DEFAULT,
    false,
    0
    );
}

inline bool Manager::TryStartSpeciesAction(
  Fieldmap* pFieldmap,
  u32 species
)
{
  if( m_SpeciesActionKind != SPECIES_ACTION_NONE ||
      m_State != STATE_ACTIVE || !pFieldmap )
  {
    return m_SpeciesActionKind != SPECIES_ACTION_NONE;
  }

  PokeTool::PokeModel* pPokeModel = GetPokeModel();
  if( !pPokeModel || !pPokeModel->GetModelInstanceNode() )
  {
    return false;
  }

  SpeciesActionKind action = SPECIES_ACTION_NONE;
  if( species == FOLLOWER_ZORUA_SPECIES ||
      species == FOLLOWER_ZOROARK_SPECIES )
  {
    PokeTool::SimpleParam target;
    if( !SelectIllusionTarget( pFieldmap, &target ) )
    {
      return false;
    }
    m_IllusionOriginalParam = m_SimpleParam;
    m_IllusionTargetParam = target;
    action = SPECIES_ACTION_ILLUSION;
  }
  else if( species == FOLLOWER_DARKRAI_SPECIES ||
           species == FOLLOWER_MARSHADOW_SPECIES )
  {
    action = SPECIES_ACTION_SHADOW_STEP;
  }
  else if( species == FOLLOWER_CELEBI_SPECIES )
  {
    action = SPECIES_ACTION_TIME_REWIND;
  }
  else if( species == FOLLOWER_HOOPA_SPECIES )
  {
    action = SPECIES_ACTION_PORTAL_BLINK;
  }
  else if( species == FOLLOWER_SHAYMIN_SPECIES )
  {
    action = SPECIES_ACTION_FLOWER_TRAIL;
  }
  else if( species == FOLLOWER_NECROZMA_SPECIES )
  {
    action = SPECIES_ACTION_LIGHT_DRAIN;
  }
  if( action == SPECIES_ACTION_NONE )
  {
    return false;
  }

  m_SpeciesActionKind = action;
  m_SpeciesActionPhase = action == SPECIES_ACTION_ILLUSION
    ? SPECIES_ACTION_PHASE_WAIT_EFFECT
    : SPECIES_ACTION_PHASE_ACTIVE;
  m_SpeciesActionOrigin = m_Position;
  m_SpeciesActionFrame = 0;
  m_SpeciesActionTrailHead = m_TrailHead;
  m_SpeciesActionTrailCount = m_TrailCount;
  m_SpeciesActionModelHidden = false;
  SetMotion( PokeTool::MODEL_ANIME_FI_WAIT_A );
  pPokeModel->SetAnimationStepFrame( 1.0f );

  Effect::Type effectType = Effect::EFFECT_TYPE_DEMO_FIREWORK_PURPLE;
  u32 effectPlacement =
    Gen7Follower3gx::DIAGNOSTIC_EFFECT_PLACEMENT_MID_BODY;
  u32 effectFrames = FOLLOWER_SPECIES_ACTION_EFFECT_FRAMES;
  f32 effectScale = 1.0f;
  switch( action )
  {
  case SPECIES_ACTION_ILLUSION:
    effectType = Effect::EFFECT_TYPE_KAIRIKY_ROCK_SMOKE;
    effectPlacement =
      Gen7Follower3gx::DIAGNOSTIC_EFFECT_PLACEMENT_GROUND;
    effectFrames = 24;
    break;
  case SPECIES_ACTION_SHADOW_STEP:
    effectType = Effect::EFFECT_TYPE_DEMO_FOG;
    effectPlacement =
      Gen7Follower3gx::DIAGNOSTIC_EFFECT_PLACEMENT_GROUND;
    effectFrames = 36;
    break;
  case SPECIES_ACTION_TIME_REWIND:
    effectType = Effect::EFFECT_TYPE_DEMO_FLOWER_YELLOW;
    effectFrames = 54;
    break;
  case SPECIES_ACTION_FLOWER_TRAIL:
    effectType = Effect::EFFECT_TYPE_DEMO_FLOWER_PINK;
    effectFrames = 48;
    effectScale = 1.5f;
    m_ShayminFlowerTrailFrames = FOLLOWER_SHAYMIN_FLOWER_TRAIL_FRAMES;
    m_ShayminFlowerIntervalFrames = FOLLOWER_SHAYMIN_FLOWER_INTERVAL_FRAMES;
    break;
  case SPECIES_ACTION_LIGHT_DRAIN:
    Gen7Follower3gx::SetTransientPerformanceOptionEnabled(
      Gen7Follower3gx::PERFORMANCE_OPTION_DISABLE_BLOOM,
      true
      );
    Gen7Follower3gx::SetTransientPerformanceOptionEnabled(
      Gen7Follower3gx::PERFORMANCE_OPTION_SIMPLE_WORLD_LIGHTING,
      true
      );
    m_NecrozmaLightDrainActive = true;
    break;
  default:
    break;
  }
  TryStartSpeciesActionEffect(
    pPokeModel,
    pFieldmap,
    effectType,
    effectFrames,
    effectPlacement,
    effectScale
    );
  return true;
}

inline void Manager::FinishSpeciesAction( Fieldmap* pFieldmap )
{
  if( m_NecrozmaLightDrainActive )
  {
    Gen7Follower3gx::SetTransientPerformanceOptionEnabled(
      Gen7Follower3gx::PERFORMANCE_OPTION_DISABLE_BLOOM,
      false
      );
    Gen7Follower3gx::SetTransientPerformanceOptionEnabled(
      Gen7Follower3gx::PERFORMANCE_OPTION_SIMPLE_WORLD_LIGHTING,
      false
      );
    m_NecrozmaLightDrainActive = false;
  }

  SetSpeciesActionModelVisible( true );
  m_SpeciesActionKind = SPECIES_ACTION_NONE;
  m_SpeciesActionPhase = SPECIES_ACTION_PHASE_NONE;
  m_SpeciesActionFrame = 0;
  m_SpeciesActionModelHidden = false;
  PokeTool::PokeModel* pPokeModel = GetPokeModel();
  if( pPokeModel )
  {
    SetMotion( PokeTool::MODEL_ANIME_FI_WAIT_A );
    pPokeModel->SetAnimationStepFrame( 1.0f );
    pPokeModel->SetPosition( GetDisplayPosition( pPokeModel ) );
  }
  if( pFieldmap )
  {
    const gfl2::math::Vector3 playerPosition =
      pFieldmap->GetPlayerPosition();
    SeedTrail( playerPosition );
#if FOLLOWER_POKEMON_USE_TRAIL_POLICY
    ResetTrailMovementPolicy( playerPosition );
#endif
  }
}

inline bool Manager::UpdateSpeciesAction(
  Fieldmap* pFieldmap,
  BaseCollisionScene* pTerrainGroundScene
)
{
  if( m_SpeciesActionKind == SPECIES_ACTION_NONE )
  {
    return false;
  }
  PokeTool::PokeModel* pPokeModel = GetPokeModel();
  if( !pFieldmap || !pPokeModel )
  {
    CancelSpeciesAction();
    return false;
  }

#if FOLLOWER_3GX_PERFORMANCE_FEATURES
  ResetInterpolatedAnimationPose();
  m_AnimationUpdatePhase = 0;
  m_AnimationUpdateThisFrame = true;
#endif

  const gfl2::math::Vector3 playerPosition =
    pFieldmap->GetPlayerPosition();
  if( m_SpeciesActionKind == SPECIES_ACTION_ILLUSION )
  {
    if( m_SpeciesActionPhase == SPECIES_ACTION_PHASE_WAIT_EFFECT )
    {
      if( !m_SpecialEffects.IsDynamicEffectBusy() &&
          !BeginIllusionModelSwap( m_IllusionTargetParam, false ) )
      {
        m_IllusionOverrideActive = false;
        FinishSpeciesAction( pFieldmap );
      }
      return true;
    }
    if( m_SpeciesActionPhase == SPECIES_ACTION_PHASE_RETURN_EFFECT )
    {
      if( !m_SpecialEffects.IsDynamicEffectBusy() &&
          !BeginIllusionModelSwap( m_IllusionOriginalParam, true ) )
      {
        m_IllusionOverrideActive = false;
        FinishSpeciesAction( pFieldmap );
      }
      return true;
    }
    if( m_SpeciesActionPhase == SPECIES_ACTION_PHASE_WAIT_MODEL )
    {
      return true;
    }

    ++m_SpeciesActionFrame;
    m_ModelFacingYaw += 0.10f;
    if( m_ModelFacingYaw >= FOLLOWER_INTERACTION_TWO_PI )
    {
      m_ModelFacingYaw -= FOLLOWER_INTERACTION_TWO_PI;
    }
    pPokeModel->SetRotation( 0.0f, m_ModelFacingYaw, 0.0f );
    if( m_SpeciesActionFrame >= FOLLOWER_ILLUSION_HOLD_FRAMES )
    {
      TryStartSpeciesActionEffect(
        pPokeModel,
        pFieldmap,
        Effect::EFFECT_TYPE_KAIRIKY_ROCK_SMOKE,
        24,
        Gen7Follower3gx::DIAGNOSTIC_EFFECT_PLACEMENT_GROUND
        );
      m_SpeciesActionPhase = SPECIES_ACTION_PHASE_RETURN_EFFECT;
    }
  }
  else if( m_SpeciesActionKind == SPECIES_ACTION_SHADOW_STEP )
  {
    ++m_SpeciesActionFrame;
    if( m_SpeciesActionFrame == 6 )
    {
      SetSpeciesActionModelVisible( false );
    }
    if( m_SpeciesActionFrame == 12 )
    {
      u32 previousIndex = m_TrailHead + FOLLOWER_TRAIL_COUNT - 6;
      previousIndex %= FOLLOWER_TRAIL_COUNT;
      f32 forwardX = playerPosition.x - m_Trail[previousIndex].x;
      f32 forwardZ = playerPosition.z - m_Trail[previousIndex].z;
      f32 lengthSq = forwardX * forwardX + forwardZ * forwardZ;
      if( lengthSq <= FOLLOWER_COLLISION_EPSILON )
      {
        forwardX = playerPosition.x - m_Position.x;
        forwardZ = playerPosition.z - m_Position.z;
        lengthSq = forwardX * forwardX + forwardZ * forwardZ;
      }
      if( lengthSq <= FOLLOWER_COLLISION_EPSILON )
      {
        forwardX = 0.0f;
        forwardZ = 1.0f;
        lengthSq = 1.0f;
      }
      const f32 inverseLength = 1.0f / gfl2::math::FSqrt( lengthSq );
      const f32 distance = GetFollowerPlayerBodyRadius( pPokeModel ) +
        FOLLOWER_PLAYER_COLLISION_RADIUS + 45.0f;
      m_Position.Set(
        playerPosition.x - forwardX * inverseLength * distance,
        playerPosition.y,
        playerPosition.z - forwardZ * inverseLength * distance
        );
      ApplyGround( pTerrainGroundScene, &m_Position );
    }
    if( m_SpeciesActionFrame == 18 )
    {
      SetSpeciesActionModelVisible( true );
      Sound::PlayVoice(
        0, m_SimpleParam.monsNo, m_SimpleParam.formNo,
        Sound::VOICE_TYPE_DEFAULT, false, 0
        );
    }
    if( m_SpeciesActionFrame >= 48 )
    {
      FinishSpeciesAction( pFieldmap );
      return true;
    }
  }
  else if( m_SpeciesActionKind == SPECIES_ACTION_TIME_REWIND )
  {
    const u32 totalFrames = FOLLOWER_TIME_REWIND_PATH_FRAMES * 2;
    u32 pathFrame = m_SpeciesActionFrame;
    if( pathFrame >= FOLLOWER_TIME_REWIND_PATH_FRAMES )
    {
      pathFrame = totalFrames - 1 - pathFrame;
    }
    u32 delay = FOLLOWER_TRAIL_DELAY + pathFrame;
    if( m_SpeciesActionTrailCount > 0 &&
        delay >= m_SpeciesActionTrailCount )
    {
      delay = m_SpeciesActionTrailCount - 1;
    }
    const u32 index = (
      m_SpeciesActionTrailHead + FOLLOWER_TRAIL_COUNT - delay
      ) % FOLLOWER_TRAIL_COUNT;
    const gfl2::math::Vector3 previous = m_Position;
    m_Position = m_Trail[index];
    ApplyGround( pTerrainGroundScene, &m_Position );
    const f32 moveX = m_Position.x - previous.x;
    const f32 moveZ = m_Position.z - previous.z;
    if( moveX * moveX + moveZ * moveZ > FOLLOWER_COLLISION_EPSILON )
    {
      m_ModelFacingYaw = static_cast<f32>( atan2( moveX, moveZ ) );
      pPokeModel->SetRotation( 0.0f, m_ModelFacingYaw, 0.0f );
    }
    SetMotion( PokeTool::MODEL_ANIME_WALK01 );
    pPokeModel->SetAnimationStepFrame( 0.85f );
    ++m_SpeciesActionFrame;
    if( m_SpeciesActionFrame >= totalFrames )
    {
      FinishSpeciesAction( pFieldmap );
      return true;
    }
  }
  else if( m_SpeciesActionKind == SPECIES_ACTION_PORTAL_BLINK )
  {
    ++m_SpeciesActionFrame;
    const bool hideFrame = m_SpeciesActionFrame == 6 ||
      m_SpeciesActionFrame == 24 ||
      m_SpeciesActionFrame == 42 ||
      m_SpeciesActionFrame == 60;
    if( hideFrame )
    {
      SetSpeciesActionModelVisible( false );
    }
    const bool revealFrame = m_SpeciesActionFrame == 10 ||
      m_SpeciesActionFrame == 28 ||
      m_SpeciesActionFrame == 46 ||
      m_SpeciesActionFrame == 64;
    if( revealFrame )
    {
      u32 previousIndex = m_TrailHead + FOLLOWER_TRAIL_COUNT - 6;
      previousIndex %= FOLLOWER_TRAIL_COUNT;
      f32 forwardX = playerPosition.x - m_Trail[previousIndex].x;
      f32 forwardZ = playerPosition.z - m_Trail[previousIndex].z;
      f32 lengthSq = forwardX * forwardX + forwardZ * forwardZ;
      if( lengthSq <= FOLLOWER_COLLISION_EPSILON )
      {
        forwardX = 0.0f;
        forwardZ = 1.0f;
        lengthSq = 1.0f;
      }
      const f32 inverseLength = 1.0f / gfl2::math::FSqrt( lengthSq );
      forwardX *= inverseLength;
      forwardZ *= inverseLength;
      const f32 rightX = forwardZ;
      const f32 rightZ = -forwardX;
      const f32 distance = GetFollowerPlayerBodyRadius( pPokeModel ) + 95.0f;
      if( m_SpeciesActionFrame == 10 )
      {
        m_Position.Set(
          playerPosition.x + rightX * distance,
          playerPosition.y,
          playerPosition.z + rightZ * distance
          );
      }
      else if( m_SpeciesActionFrame == 28 )
      {
        m_Position.Set(
          playerPosition.x - rightX * distance,
          playerPosition.y,
          playerPosition.z - rightZ * distance
          );
      }
      else if( m_SpeciesActionFrame == 46 )
      {
        m_Position.Set(
          playerPosition.x + forwardX * distance,
          playerPosition.y,
          playerPosition.z + forwardZ * distance
          );
      }
      else
      {
        m_Position = GetTrailTarget();
      }
      ApplyGround( pTerrainGroundScene, &m_Position );
      SetSpeciesActionModelVisible( true );
      TryStartSpeciesActionEffect(
        pPokeModel,
        pFieldmap,
        Effect::EFFECT_TYPE_DEMO_FIREWORK_PURPLE,
        18,
        Gen7Follower3gx::DIAGNOSTIC_EFFECT_PLACEMENT_MID_BODY
        );
    }
    if( m_SpeciesActionFrame >= 72 )
    {
      FinishSpeciesAction( pFieldmap );
      return true;
    }
  }
  else if( m_SpeciesActionKind == SPECIES_ACTION_FLOWER_TRAIL )
  {
    ++m_SpeciesActionFrame;
    if( m_SpeciesActionFrame == 8 )
    {
      Sound::PlayVoice(
        0, m_SimpleParam.monsNo, m_SimpleParam.formNo,
        Sound::VOICE_TYPE_DEFAULT, false, 0
        );
    }
    if( m_SpeciesActionFrame >= 24 )
    {
      FinishSpeciesAction( pFieldmap );
      return true;
    }
  }
  else if( m_SpeciesActionKind == SPECIES_ACTION_LIGHT_DRAIN )
  {
    ++m_SpeciesActionFrame;
    if( m_SpeciesActionFrame == 10 )
    {
      SetSpeciesActionModelVisible( false );
    }
    if( m_SpeciesActionFrame == 22 )
    {
      SetSpeciesActionModelVisible( true );
      Sound::PlayVoice(
        0, m_SimpleParam.monsNo, m_SimpleParam.formNo,
        Sound::VOICE_TYPE_DEFAULT, false, 0
        );
    }
    if( m_SpeciesActionFrame >= 72 )
    {
      FinishSpeciesAction( pFieldmap );
      return true;
    }
  }

  ApplyGround( pTerrainGroundScene, &m_Position );
  pPokeModel->SetPosition( GetDisplayPosition( pPokeModel ) );
  if( !m_SpeciesActionModelHidden )
  {
    SetSpeciesActionModelVisible( true );
  }
  if( m_pFactory )
  {
    m_pFactory->TickEntries();
  }
  return true;
}

inline void Manager::UpdateShayminFlowerTrail( Fieldmap* pFieldmap )
{
  if( m_ShayminFlowerTrailFrames == 0 || !pFieldmap ||
      static_cast<u32>( m_SimpleParam.monsNo ) != FOLLOWER_SHAYMIN_SPECIES )
  {
    return;
  }
  --m_ShayminFlowerTrailFrames;
  if( m_ShayminFlowerIntervalFrames > 0 )
  {
    --m_ShayminFlowerIntervalFrames;
  }
  if( m_ShayminFlowerIntervalFrames != 0 ||
      !IsLocomotionMotion(
        static_cast<PokeTool::MODEL_ANIME>( m_CurrentMotion )
        ) ||
      m_InteractionEffectPending ||
      m_SpecialEffects.IsDynamicEffectBusy() )
  {
    return;
  }

  PokeTool::PokeModel* pPokeModel = GetPokeModel();
  const Effect::Type effectType = m_ShayminFlowerAlternate
    ? Effect::EFFECT_TYPE_DEMO_FLOWER_YELLOW
    : Effect::EFFECT_TYPE_DEMO_FLOWER_PINK;
  if( TryStartSpeciesActionEffect(
        pPokeModel,
        pFieldmap,
        effectType,
        24,
        Gen7Follower3gx::DIAGNOSTIC_EFFECT_PLACEMENT_GROUND
        ) )
  {
    m_ShayminFlowerAlternate = !m_ShayminFlowerAlternate;
    m_ShayminFlowerIntervalFrames =
      FOLLOWER_SHAYMIN_FLOWER_INTERVAL_FRAMES;
  }
}

inline poke_3d::model::BaseModel*
Manager::GetDittoPlayerCloneModel( void ) const
{
  if( !m_DittoWorkReserved || !m_pDittoMoveModelManager ||
      m_DittoModelId >= Field::MoveModel::FieldMoveModelManager::GetModelCount() )
  {
    return NULL;
  }

  Field::MoveModel::FieldMoveModel* pMoveModel =
    m_pDittoMoveModelManager->GetFieldMoveModelRaw(
      static_cast<Field::MoveModel::FIELD_MOVE_MODEL_ID>( m_DittoModelId )
      );
  return pMoveModel ? pMoveModel->GetCharaDrawInstanceRaw() : NULL;
}

inline void Manager::PublishDittoTransformDiagnostics(
  Gen7Follower3gx::DittoTransformDiagnosticResult result,
  bool incrementAttempt,
  bool incrementSuccess
) const
{
  Gen7Follower3gx::DittoTransformDiagnosticSnapshot snapshot = {};
  switch( m_DittoTransformState )
  {
  case DITTO_TRANSFORM_FLASH_IN:
  case DITTO_TRANSFORM_WAIT_FLASH_IN_RELEASE:
    snapshot.state = Gen7Follower3gx::DITTO_TRANSFORM_STATE_FLASH_IN;
    break;
  case DITTO_TRANSFORM_CLONE_HOLD:
    snapshot.state = Gen7Follower3gx::DITTO_TRANSFORM_STATE_CLONE_HOLD;
    break;
  case DITTO_TRANSFORM_FLASH_OUT:
    snapshot.state = Gen7Follower3gx::DITTO_TRANSFORM_STATE_FLASH_OUT;
    break;
  case DITTO_TRANSFORM_CLEANUP:
    snapshot.state = Gen7Follower3gx::DITTO_TRANSFORM_STATE_CLEANUP;
    break;
  default:
    snapshot.state = Gen7Follower3gx::DITTO_TRANSFORM_STATE_NONE;
    break;
  }
  snapshot.result = static_cast<u32>( result );
  snapshot.modelId = m_DittoModelId;
  snapshot.parentHeapFree = m_DittoParentHeapFree;
  snapshot.cloneHeapFree = m_DittoCloneHeapFree;
  snapshot.motionId = m_DittoCloneMotionId;
  snapshot.motionPhase = static_cast<u32>( m_DittoCloneMotionPhase );
  poke_3d::model::BaseModel* pClone = GetDittoPlayerCloneModel();
  if( pClone && pClone->GetModelInstanceNode() )
  {
    const f32 motionFrame = pClone->GetAnimationFrame();
    const f32 motionEndFrame = pClone->GetAnimationEndFrame();
    snapshot.motionFrame = motionFrame > 0.0f
      ? static_cast<u32>( motionFrame )
      : 0;
    snapshot.motionEndFrame = motionEndFrame > 0.0f
      ? static_cast<u32>( motionEndFrame )
      : 0;
  }
  Gen7Follower3gx::UpdateDittoTransformDiagnostics(
    snapshot,
    incrementAttempt,
    incrementSuccess
    );
}

