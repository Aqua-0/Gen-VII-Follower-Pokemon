inline bool Manager::TryCycleCastformWeather( Fieldmap* pFieldmap )
{
#if !FOLLOWER_POKEMON_ENABLE_BALL_TRANSITION
  (void)pFieldmap;
  return false;
#else
  weather::WeatherKind weatherKind = weather::SUNNY;
  pml::FormNo targetForm = FOLLOWER_CASTFORM_FORM_SUN;
  u8 battleWeather = Gen7Follower3gx::BATTLE_WEATHER_SHINE;
  switch( m_CastformWeatherIndex % FOLLOWER_CASTFORM_WEATHER_COUNT )
  {
  case 1:
    weatherKind = weather::RAIN;
    targetForm = FOLLOWER_CASTFORM_FORM_RAIN;
    battleWeather = Gen7Follower3gx::BATTLE_WEATHER_RAIN;
    break;
  case 2:
    weatherKind = weather::SNOW;
    targetForm = FOLLOWER_CASTFORM_FORM_SNOW;
    battleWeather = Gen7Follower3gx::BATTLE_WEATHER_SNOW;
    break;
  case 3:
    weatherKind = weather::SANDSTORM;
    targetForm = FOLLOWER_CASTFORM_FORM_NORMAL;
    battleWeather = Gen7Follower3gx::BATTLE_WEATHER_SAND;
    break;
  default:
    break;
  }

  return TryStartWeatherForm(
    pFieldmap,
    weatherKind,
    targetForm,
    battleWeather,
    true
    );
#endif
}

inline bool Manager::TryStartPrimalWeather(
  Fieldmap* pFieldmap,
  u32 species
)
{
#if !FOLLOWER_POKEMON_ENABLE_BALL_TRANSITION
  (void)pFieldmap;
  (void)species;
  return false;
#else
  if( species == FOLLOWER_KYOGRE_SPECIES )
  {
    return TryStartWeatherForm(
      pFieldmap,
      weather::THUNDERSTORM,
      FOLLOWER_PRIMAL_FORM,
      Gen7Follower3gx::BATTLE_WEATHER_STORM,
      false
      );
  }
  if( species == FOLLOWER_GROUDON_SPECIES )
  {
    return TryStartWeatherForm(
      pFieldmap,
      weather::DRY,
      FOLLOWER_PRIMAL_FORM,
      Gen7Follower3gx::BATTLE_WEATHER_DAY,
      false
      );
  }
  return false;
#endif
}

inline bool Manager::TryStartWeatherForm(
  Fieldmap* pFieldmap,
  weather::WeatherKind weatherKind,
  pml::FormNo targetForm,
  u8 battleWeather,
  bool advanceCastformCycle
)
{
#if !FOLLOWER_POKEMON_ENABLE_BALL_TRANSITION
  (void)pFieldmap;
  (void)weatherKind;
  (void)targetForm;
  (void)battleWeather;
  (void)advanceCastformCycle;
  return false;
#else
#if FOLLOWER_3GX_DIAGNOSTIC
  Gen7Follower3gx::ResetDiagnosticWeatherSelection();
#endif
  GameSys::GameManager* pGameManager = pFieldmap
    ? pFieldmap->GetGameManager()
    : NULL;
  Gen7Follower3gx::ClearBattleWeatherHandoff( pGameManager );
  if( m_SpecialEffects.IsTemporaryWeatherActive() )
  {
    m_SpecialEffects.RestoreTemporaryWeather();
  }

  Gen7Follower3gx::CastformWeatherDiagnosticSnapshot snapshot = {};
  snapshot.requestedWeather = static_cast<u32>( weatherKind );
  snapshot.previousWeather =
    FollowerWeatherAdapter::GetNowWeatherKind( pFieldmap );
  snapshot.previousForceWeather =
    FollowerWeatherAdapter::GetForceWeatherKind( pFieldmap );

  if( Gen7Follower3gx::IsPerformanceOptionEnabled(
        Gen7Follower3gx::PERFORMANCE_OPTION_DISABLE_WEATHER
        ) )
  {
    snapshot.result =
      Gen7Follower3gx::CASTFORM_WEATHER_RESULT_PERFORMANCE_DISABLED;
    Gen7Follower3gx::UpdateCastformWeatherDiagnostics(
      snapshot,
      true,
      false
      );
    if( m_WeatherFormActive )
    {
      m_WeatherFormActive = false;
      m_WeatherReactionPending = false;
      BeginWeatherFormChange( m_WeatherBaseForm, false );
    }
    return false;
  }

  const FollowerWeatherAdapter::StartResult startResult =
    FollowerWeatherAdapter::StartTemporaryWeather(
      &m_SpecialEffects,
      pFieldmap,
      weatherKind,
      FOLLOWER_WEATHER_LIFETIME_FRAMES
      );
  switch( startResult )
  {
  case FollowerWeatherAdapter::START_STARTED:
    snapshot.result = Gen7Follower3gx::CASTFORM_WEATHER_RESULT_STARTED;
    break;
  case FollowerWeatherAdapter::START_NOT_OUTDOORS:
    snapshot.result =
      Gen7Follower3gx::CASTFORM_WEATHER_RESULT_NOT_OUTDOORS;
    break;
  case FollowerWeatherAdapter::START_NO_WEATHER_CONTROL:
  case FollowerWeatherAdapter::START_NO_CONTROLLER:
  case FollowerWeatherAdapter::START_NO_FIELDMAP:
    snapshot.result = Gen7Follower3gx::CASTFORM_WEATHER_RESULT_NO_CONTROL;
    break;
  case FollowerWeatherAdapter::START_INVALID_WEATHER_CONTROL:
    snapshot.result =
      Gen7Follower3gx::CASTFORM_WEATHER_RESULT_INVALID_CONTROL;
    break;
  case FollowerWeatherAdapter::START_INVALID_WEATHER_KIND:
    snapshot.result =
      Gen7Follower3gx::CASTFORM_WEATHER_RESULT_INVALID_KIND;
    break;
  case FollowerWeatherAdapter::START_BUSY:
    snapshot.result = Gen7Follower3gx::CASTFORM_WEATHER_RESULT_BUSY;
    break;
  default:
    snapshot.result = Gen7Follower3gx::CASTFORM_WEATHER_RESULT_APPLY_FAILED;
    break;
  }

  const bool started =
    startResult == FollowerWeatherAdapter::START_STARTED;
  if( started )
  {
    if( !m_WeatherFormActive )
    {
      m_WeatherFormSpecies = static_cast<u32>( m_SimpleParam.monsNo );
      m_WeatherBaseForm = m_SimpleParam.formNo;
    }
    m_WeatherFormActive = true;
    m_WeatherFormOverrideActive = true;
    m_WeatherTargetForm = targetForm;
    m_WeatherReactionPending = true;
    Gen7Follower3gx::ArmBattleWeatherHandoff(
      pGameManager,
      battleWeather
      );
    BeginWeatherFormChange( targetForm, true );
    if( advanceCastformCycle )
    {
      m_CastformWeatherIndex =
        (m_CastformWeatherIndex + 1) % FOLLOWER_CASTFORM_WEATHER_COUNT;
    }
  }
  else if( m_WeatherFormActive )
  {
    m_WeatherFormActive = false;
    m_WeatherReactionPending = false;
    BeginWeatherFormChange( m_WeatherBaseForm, false );
  }
  Gen7Follower3gx::UpdateCastformWeatherDiagnostics(
    snapshot,
    true,
    started
    );
  return started;
#endif
}

#if FOLLOWER_3GX_DIAGNOSTIC
inline bool Manager::ProcessDiagnosticWeatherSelection(
  Fieldmap* pFieldmap
)
{
#if !FOLLOWER_POKEMON_ENABLE_BALL_TRANSITION
  (void)pFieldmap;
  return false;
#else
  int selection = Gen7Follower3gx::DIAGNOSTIC_WEATHER_MAP_DEFAULT;
  if( !Gen7Follower3gx::ConsumeDiagnosticWeatherSelection( &selection ) )
  {
    return false;
  }

  Gen7Follower3gx::CastformWeatherDiagnosticSnapshot snapshot = {};
  snapshot.requestedWeather = selection >= 0
    ? static_cast<u32>( selection )
    : 0xffffffffU;
  snapshot.previousWeather =
    FollowerWeatherAdapter::GetNowWeatherKind( pFieldmap );
  snapshot.previousForceWeather =
    FollowerWeatherAdapter::GetForceWeatherKind( pFieldmap );

  Gen7Follower3gx::ClearBattleWeatherHandoff(
    pFieldmap ? pFieldmap->GetGameManager() : NULL
    );
  if( m_SpecialEffects.IsTemporaryWeatherActive() )
  {
    m_SpecialEffects.RestoreTemporaryWeather();
  }

  bool beganBaseFormChange = false;
  if( m_WeatherFormActive )
  {
    m_WeatherFormActive = false;
    m_WeatherReactionPending = false;
    beganBaseFormChange =
      BeginWeatherFormChange( m_WeatherBaseForm, false );
  }

  if( selection == Gen7Follower3gx::DIAGNOSTIC_WEATHER_MAP_DEFAULT )
  {
    snapshot.result = Gen7Follower3gx::CASTFORM_WEATHER_RESULT_RESTORED;
    Gen7Follower3gx::UpdateCastformWeatherDiagnostics(
      snapshot,
      true,
      true
      );
    return beganBaseFormChange;
  }

  if( Gen7Follower3gx::IsPerformanceOptionEnabled(
        Gen7Follower3gx::PERFORMANCE_OPTION_DISABLE_WEATHER
        ) )
  {
    snapshot.result =
      Gen7Follower3gx::CASTFORM_WEATHER_RESULT_PERFORMANCE_DISABLED;
    Gen7Follower3gx::ResetDiagnosticWeatherSelection();
    Gen7Follower3gx::UpdateCastformWeatherDiagnostics(
      snapshot,
      true,
      false
      );
    return beganBaseFormChange;
  }

  const FollowerWeatherAdapter::StartResult startResult =
    FollowerWeatherAdapter::StartTemporaryWeather(
      &m_SpecialEffects,
      pFieldmap,
      static_cast<weather::WeatherKind>( selection ),
      0,
      FollowerSpecialEffectController::OWNER_DIAGNOSTIC_WEATHER
      );
  switch( startResult )
  {
  case FollowerWeatherAdapter::START_STARTED:
    snapshot.result = Gen7Follower3gx::CASTFORM_WEATHER_RESULT_STARTED;
    break;
  case FollowerWeatherAdapter::START_NOT_OUTDOORS:
    snapshot.result =
      Gen7Follower3gx::CASTFORM_WEATHER_RESULT_NOT_OUTDOORS;
    break;
  case FollowerWeatherAdapter::START_NO_WEATHER_CONTROL:
  case FollowerWeatherAdapter::START_NO_CONTROLLER:
  case FollowerWeatherAdapter::START_NO_FIELDMAP:
    snapshot.result = Gen7Follower3gx::CASTFORM_WEATHER_RESULT_NO_CONTROL;
    break;
  case FollowerWeatherAdapter::START_INVALID_WEATHER_CONTROL:
    snapshot.result =
      Gen7Follower3gx::CASTFORM_WEATHER_RESULT_INVALID_CONTROL;
    break;
  case FollowerWeatherAdapter::START_INVALID_WEATHER_KIND:
    snapshot.result =
      Gen7Follower3gx::CASTFORM_WEATHER_RESULT_INVALID_KIND;
    break;
  case FollowerWeatherAdapter::START_BUSY:
    snapshot.result = Gen7Follower3gx::CASTFORM_WEATHER_RESULT_BUSY;
    break;
  default:
    snapshot.result = Gen7Follower3gx::CASTFORM_WEATHER_RESULT_APPLY_FAILED;
    break;
  }

  const bool started =
    startResult == FollowerWeatherAdapter::START_STARTED;
  if( !started )
  {
    Gen7Follower3gx::ResetDiagnosticWeatherSelection();
  }
  Gen7Follower3gx::UpdateCastformWeatherDiagnostics(
    snapshot,
    true,
    started
    );
  return beganBaseFormChange;
#endif
}

inline void Manager::PreserveDiagnosticWeatherSelection( void )
{
#if FOLLOWER_POKEMON_ENABLE_BALL_TRANSITION
  if( m_SpecialEffects.IsOwnerActive(
        FollowerSpecialEffectController::OWNER_DIAGNOSTIC_WEATHER
        ) )
  {
    Gen7Follower3gx::RequeueDiagnosticWeatherSelection();
  }
#endif
}
#endif

inline void Manager::ApplyWeatherFormOverride(
  PokeTool::SimpleParam* pParam
)
{
  if( !pParam )
  {
    return;
  }

  if( m_IllusionOverrideActive )
  {
    *pParam = m_IllusionTargetParam;
    return;
  }

  const u32 species = static_cast<u32>( pParam->monsNo );
  if( m_ManualFormOverrideActive )
  {
    if( species == m_ManualFormSpecies )
    {
      pParam->formNo = m_ManualForm;
    }
    else
    {
      m_ManualFormSpecies = 0;
      m_ManualForm = 0;
      m_ManualFormOverrideActive = false;
    }
  }

  if( m_WeatherFormOverrideActive &&
      static_cast<u32>( pParam->monsNo ) == m_WeatherFormSpecies )
  {
    pParam->formNo = m_WeatherTargetForm;
  }
}

inline bool Manager::BeginRawFollowerFormChange(
  pml::FormNo targetForm
)
{
  if( m_WeatherFormActive || m_State != STATE_ACTIVE || !m_pFactory ||
      !m_pTrialModel )
  {
    return false;
  }

  const u32 species = static_cast<u16>( m_SimpleParam.monsNo );
  const u32 previousWeatherSpecies = m_WeatherFormSpecies;
  const pml::FormNo previousWeatherBaseForm = m_WeatherBaseForm;
  m_WeatherFormSpecies = species;
  // Clear the temporary weather after recycling the model, but keep the manually selected form.
  m_WeatherBaseForm = targetForm;
  if( !BeginWeatherFormChange( targetForm, false ) )
  {
    m_WeatherFormSpecies = previousWeatherSpecies;
    m_WeatherBaseForm = previousWeatherBaseForm;
    return false;
  }

  m_ManualFormSpecies = species;
  m_ManualForm = targetForm;
  m_ManualFormOverrideActive = true;
  return true;
}

inline bool Manager::TryCycleRawFollowerForm( void )
{
  // Keep showing the weather form until the previous weather has finished restoring.
  if( m_WeatherFormActive || m_ManualFormEffectPending ||
      m_State != STATE_ACTIVE || !m_pFactory || !m_pTrialModel )
  {
    return true;
  }

  PokeTool::PokeModelSystem* pModelSystem =
    m_pFactory->GetPokeModelSystem();
  const u32 species = static_cast<u16>( m_SimpleParam.monsNo );
  const PokeTool::PokeModelSystem::POKE_MNG_DATA* pMngData =
    pModelSystem ? pModelSystem->GetMngDataRaw( species ) : NULL;
  if( !pMngData ||
      !(pMngData->flags &
        PokeTool::PokeModelSystem::POKE_MNG_FLG_EXIST_FORM_CHANGE) )
  {
    return true;
  }

  const u32 femaleDataCount =
    (pMngData->flags &
     PokeTool::PokeModelSystem::POKE_MNG_FLG_EXIST_FEMALE) ? 1U : 0U;
  const u32 dataCount = static_cast<u32>( pMngData->dataNum );
  if( dataCount <= femaleDataCount + 1U )
  {
    return true;
  }
  const u32 formCount = dataCount - femaleDataCount;
  const u32 currentForm = static_cast<u8>( m_SimpleParam.formNo );
  const pml::FormNo targetForm = static_cast<pml::FormNo>(
    (currentForm + 1U) % formCount
    );

#if FOLLOWER_POKEMON_ENABLE_BALL_TRANSITION
  if( !Gen7Follower3gx::IsPerformanceOptionEnabled(
        Gen7Follower3gx::PERFORMANCE_OPTION_DISABLE_EFFECTS
        ) &&
      TryStartBallTransition( FollowerBallTransition::KIND_SPAWN ) )
  {
    m_ManualFormPending = targetForm;
    m_ManualFormEffectPending = true;
    return true;
  }
#endif

  BeginRawFollowerFormChange( targetForm );
  return true;
}

inline bool Manager::BeginWeatherFormChange(
  pml::FormNo targetForm,
  bool playReaction
)
{
  if( m_State != STATE_ACTIVE || !m_pFactory || !m_pTrialModel ||
      static_cast<u32>( m_SimpleParam.monsNo ) != m_WeatherFormSpecies )
  {
    return false;
  }

  m_WeatherTargetForm = targetForm;
  m_WeatherFormOverrideActive = true;
  m_WeatherReactionPending = playReaction;
  if( m_SimpleParam.formNo == targetForm )
  {
    // If the form doesn't need changing, let the normal interaction play its reaction now.
    m_WeatherReactionPending = false;
    if( !m_WeatherFormActive &&
        targetForm == m_WeatherBaseForm )
    {
      m_WeatherFormOverrideActive = false;
    }
    return false;
  }

  if( !PrepareInteractionForTerminate() )
  {
    // Keep the old form if an interaction is still loading and blocks the model swap.
    m_WeatherTargetForm = m_SimpleParam.formNo;
    m_WeatherFormOverrideActive = m_WeatherFormActive;
    m_WeatherReactionPending = false;
    return false;
  }

  m_WeatherFormChangePending = true;
  RetireFollowerEdgeTarget();
  HideFollowerModel();
  m_State = STATE_WEATHER_FORM_DELETE_WAIT;
  return true;
}

inline void Manager::ContinueWeatherFormChange( void )
{
  if( (!m_WeatherFormChangePending && !m_IllusionModelSwapPending) ||
      !m_pFactory || !m_pTrialModel )
  {
    m_WeatherFormChangePending = false;
    m_IllusionModelSwapPending = false;
    BeginTerminate();
    return;
  }

  HideFollowerModel();
  if( !IsPokeModelGpuIdle() )
  {
    return;
  }

  m_pFactory->ReleaseEntry( m_pTrialModel );
  m_pTrialModel = NULL;
  m_IsTrialModelCreated = false;
  if( m_IllusionModelSwapPending )
  {
    m_SimpleParam = m_IllusionTargetParam;
  }
  else
  {
    m_SimpleParam.formNo = m_WeatherTargetForm;
  }
  m_CurrentMotion = PokeTool::MODEL_ANIME_ERROR;
  m_AnimationStepFrame = 1.0f;
  m_NominalRootSpeed = 0.0f;
  m_WalkNominalRootSpeed = 0.0f;
  m_RunNominalRootSpeed = 0.0f;
  m_RootMotionSampleDistance = 0.0f;
  m_RootMotionSampleFrames = 0.0f;
#if FOLLOWER_3GX_PERFORMANCE_FEATURES
  ResetInterpolatedAnimationPose();
  m_AnimationUpdatePhase = 0;
  m_AnimationUpdateThisFrame = true;
#endif
  BeginModelLoad();
}

inline bool Manager::RestoreWeatherBaseFormAfterWeather( Fieldmap* pFieldmap )
{
#if !FOLLOWER_POKEMON_ENABLE_BALL_TRANSITION
  return false;
#else
  if( m_State != STATE_ACTIVE || !m_WeatherFormActive ||
      m_SpecialEffects.IsTemporaryWeatherActive() )
  {
    return false;
  }

  Gen7Follower3gx::ClearBattleWeatherHandoff(
    pFieldmap ? pFieldmap->GetGameManager() : NULL
    );
  m_WeatherFormActive = false;
  m_WeatherReactionPending = false;
  return BeginWeatherFormChange( m_WeatherBaseForm, false );
#endif
}

inline void Manager::CompleteWeatherFormChange( Fieldmap* pFieldmap )
{
  m_WeatherFormChangePending = false;
  if( !m_WeatherFormActive &&
      m_WeatherTargetForm == m_WeatherBaseForm )
  {
    m_WeatherFormOverrideActive = false;
  }

  if( !m_WeatherReactionPending || !pFieldmap )
  {
    m_WeatherReactionPending = false;
    return;
  }

  PokeTool::PokeModel* pPokeModel = GetPokeModel();
  if( !pPokeModel )
  {
    m_WeatherReactionPending = false;
    return;
  }

  m_WeatherReactionPending = false;
  m_InteractionFacingYaw = m_ModelFacingYaw;
  m_InteractionFacingComplete = false;
  UpdateInteractionFacing( pPokeModel, pFieldmap->GetPlayerPosition() );
  m_InteractionPreferHappy = true;
  BeginInteractionMotionLoad( pFieldmap );
}

