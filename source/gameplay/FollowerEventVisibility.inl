inline bool Manager::CanRetainInteractionForFieldEvent( void ) const
{
#if FOLLOWER_CARRIER_DEDICATED_ARENA && FOLLOWER_3GX_INTERACTION_FEATURES
  // Keep cached reactions during events. Loads in progress and special actors still need cleanup.
  return m_State == STATE_ACTIVE &&
    m_InteractionState != INTERACTION_STATE_LOADING &&
    !m_InteractionCancelPending &&
    m_SpeciesActionKind == SPECIES_ACTION_NONE &&
    m_DittoTransformState == DITTO_TRANSFORM_NONE;
#else
  return false;
#endif
}

inline void Manager::PrepareRetainedEventIdle( void )
{
#if FOLLOWER_3GX_INTERACTION_FEATURES
  if( !CanRetainInteractionForFieldEvent() ) return;
  auto* pokemon = GetPokeModel();
  if( !pokemon ) return;
  // Keep the current idle. A reaction can have the same clip ID, so check which pack it comes from.
  if( m_CurrentMotion != PokeTool::MODEL_ANIME_FI_WAIT_A ||
      m_InteractionState == INTERACTION_STATE_PLAYING || m_InteractionPlayingExternal )
    pokemon->ChangeAnimation(PokeTool::MODEL_ANIME_FI_WAIT_A, true);
  pokemon->SetAnimationIsLoop(true);
  pokemon->SetAnimationStepFrame(1.0f);
  m_CurrentMotion = PokeTool::MODEL_ANIME_FI_WAIT_A;
  m_InteractionState = m_InteractionPackLoaded ? INTERACTION_STATE_READY : INTERACTION_STATE_NONE;
  m_InteractionPlayingFrames = 0;
  m_InteractionPlayingExternal = false;
  m_InteractionCryPending = false;
  m_InteractionPoseBlendCapturePending = false;
  m_InteractionPoseBlendActive = false;
  m_InteractionReturnGroundBlendActive = false;
  ResetPendingInteractionEffect();
#endif
}

inline void Manager::SuspendForFieldEvent( Fieldmap* pFieldmap )
{
  if( !m_IsFieldEventSuspended )
  {
    PrepareRetainedEventIdle();
    m_IsFieldEventSuspended = true;
    m_HasFieldEventStartPlayerPosition = pFieldmap != NULL;
    if( pFieldmap ) m_FieldEventStartPlayerPosition = pFieldmap->GetPlayerPosition();
  }
#if FOLLOWER_CARRIER_THREEGX
  HideRemoteReplicas();
#endif
#if FOLLOWER_3GX_INTERACTION_FEATURES
  if( m_State==STATE_ACTIVE && m_MountedRideActive && pFieldmap &&
      Gen7Follower3gx::GetMountedInteractionMode()==Gen7Follower3gx::MOUNTED_INTERACTION_KEEP &&
      !NeedsFieldEventResourceRelease() )
  {
    // Keep checking ride state and restoring transforms while events run.
#if FOLLOWER_CARRIER_DEDICATED_ARENA && FOLLOWER_CARRIER_THREEGX
    ShowPartyFollowersForEvent(true);
#endif
    UpdateMountedRide(pFieldmap);
    if( m_MountedRideActive )
    {
      if( m_pFactory ) m_pFactory->UpdateEntryPresentation();
      return;
    }
  }
  StopMountedRide();
#endif
#if FOLLOWER_POKEMON_ENABLE_BALL_TRANSITION
#if FOLLOWER_3GX_DIAGNOSTIC
  PreserveDiagnosticWeatherSelection();
#endif
  m_SpecialEffects.RestoreTemporaryWeather();
#endif
#if FOLLOWER_3GX_INTERACTION_FEATURES
  CancelSpeciesAction();
#endif

  bool showFollower=false;
#if FOLLOWER_3GX_INTERACTION_FEATURES
  if( pFieldmap && m_State==STATE_ACTIVE && !m_MountedRideCleanup &&
      Gen7Follower3gx::KeepFollowerVisibleDuringEvents() &&
      !NeedsFieldEventResourceRelease() )
  {
    auto* gameManager=pFieldmap->GetGameManager();
    auto* gameData=gameManager ? FOLLOWER_POKEMON_GET_GAME_DATA(gameManager) : NULL;
    auto* models=gameData ? gameData->GetFieldCharaModelManagerRaw() : NULL;
    auto* playerWork=models ? models->GetFieldMoveModelRaw(Field::MoveModel::FIELD_MOVE_MODEL_PLAYER) : NULL;
    auto* player=playerWork ? playerWork->GetCharaDrawInstanceRaw() : NULL;
    showFollower=player && player->IsVisible();
  }
#endif
  PokeTool::PokeModel* pokemon=GetPokeModel();
  if( showFollower && pokemon )
  {
    // Let idle loop in place instead of restarting it every frame.
    SetMotion(PokeTool::MODEL_ANIME_FI_WAIT_A);
    pokemon->SetAnimationIsLoop(true);
    pokemon->SetAnimationStepFrame(1.0f);
#if FOLLOWER_3GX_PERFORMANCE_FEATURES
    m_AnimationUpdateThisFrame = true;
    ResetInterpolatedAnimationPose();
#endif
#if FOLLOWER_CARRIER_DEDICATED_ARENA && FOLLOWER_CARRIER_THREEGX
    ShowPartyFollowersForEvent(true);
#endif
    if( m_pFactory )
    {
      m_pFactory->TickEntries();
      ApplyFollowerAnimationGroundOffset(pokemon);
      m_pFactory->UpdateEntryPresentation();
    }
    pokemon->SetVisible(true);
    if( m_pTrialModel ) m_pTrialModel->ForwardVisibility(true);
  }
  else HideFollowerModel();
#if FOLLOWER_3GX_INTERACTION_FEATURES
  auto* clone=GetDittoPlayerCloneModel();
  if( clone ) clone->SetVisible(false);
#endif
}

