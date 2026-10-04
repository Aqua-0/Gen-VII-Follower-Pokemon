#pragma once

namespace Field
{
namespace FollowerRuntime
{

class FollowerSpecialEffectController
{
public:
  enum Owner
  {
    OWNER_NONE,
    OWNER_BALL_TRANSITION,
    OWNER_SPECIES_INTERACTION,
    OWNER_DIAGNOSTIC_WEATHER,
  };

  enum DynamicStartResult
  {
    DYNAMIC_START_STARTED,
    DYNAMIC_START_INVALID_OWNER,
    DYNAMIC_START_NO_EFFECT_MANAGER,
    DYNAMIC_START_NO_RESOURCE_HEAP,
    DYNAMIC_START_BUSY,
    DYNAMIC_START_RESOURCE_ALREADY_LOADED,
    DYNAMIC_START_RESOURCE_HEAP_LOW,
    DYNAMIC_START_LOAD_FAILED,
    DYNAMIC_START_NO_EFFECT_SLOT,
    DYNAMIC_START_CREATE_FAILED,
  };

  enum WeatherStartResult
  {
    WEATHER_START_STARTED,
    WEATHER_START_INVALID_OWNER,
    WEATHER_START_INVALID_BACKEND,
    WEATHER_START_BUSY,
    WEATHER_START_APPLY_FAILED,
  };

  struct WorkSlotAvailability
  {
    u32 system;
    u32 event;
    u32 weather;
    u32 ride;
  };

  struct DynamicEffectRequest
  {
    DynamicEffectRequest(
      Effect::Type effectType,
      const gfl2::math::Vector3& effectPosition
    )
    : type( effectType )
    , position( effectPosition )
    , rotation( 0.0f, 0.0f, 0.0f )
    , workType( Effect::EffectManager::WORK_TYPE_DEFAULT )
    , minimumResourceHeapFree( 0 )
    , lifetimeFrames( 0 )
    , cueFrame( NO_CUE_FRAME )
    , releaseSettleFrames( DEFAULT_RELEASE_SETTLE_FRAMES )
    , scale( 1.0f )
    , playSound( false )
    {
    }

    Effect::Type type;
    gfl2::math::Vector3 position;
    gfl2::math::Vector3 rotation;
    Effect::EffectManager::WorkType workType;
    u32 minimumResourceHeapFree;
    u32 lifetimeFrames;
    u32 cueFrame;
    u32 releaseSettleFrames;
    f32 scale;
    bool playSound;
  };

  struct WeatherRestoreToken
  {
    WeatherRestoreToken()
    {
      for( u32 i = 0; i < RESTORE_TOKEN_WORDS; ++i )
      {
        words[i] = 0;
      }
    }

    u32 words[4];
  };

  // Save the previous weather in the token so we can restore it. Leave it unchanged if applying fails.
  typedef bool (*ApplyTemporaryWeatherFunction)(
    void* context,
    s32 weatherKind,
    WeatherRestoreToken* restoreToken
    );
  typedef void (*RestoreTemporaryWeatherFunction)(
    void* context,
    const WeatherRestoreToken& restoreToken
    );

  struct TemporaryWeatherRequest
  {
    TemporaryWeatherRequest()
    : context( NULL )
    , apply( NULL )
    , restore( NULL )
    , weatherKind( -1 )
    , lifetimeFrames( 0 )
    {
    }

    void* context;
    ApplyTemporaryWeatherFunction apply;
    RestoreTemporaryWeatherFunction restore;
    s32 weatherKind;
    u32 lifetimeFrames;
  };

  enum
  {
    NO_CUE_FRAME = 0xffffffffU,
    DEFAULT_RELEASE_SETTLE_FRAMES = 3,
  };

  static WorkSlotAvailability InspectWorkSlots(
    const Effect::EffectManager* pEffectManager
  )
  {
    WorkSlotAvailability result = { 0, 0, 0, 0 };
    if( !pEffectManager )
    {
      return result;
    }

    const void* const* slots = reinterpret_cast<const void* const*>(
      reinterpret_cast<const u8*>( pEffectManager ) + sizeof(void*)
      );
    for( u32 i = 0; i < 30; ++i )
    {
      if( !slots[i] ){ ++result.system; }
    }
    for( u32 i = 30; i < 36; ++i )
    {
      if( !slots[i] ){ ++result.weather; }
    }
    for( u32 i = 36; i < 40; ++i )
    {
      if( !slots[i] ){ ++result.ride; }
    }
    for( u32 i = 40; i < 44; ++i )
    {
      if( !slots[i] ){ ++result.event; }
    }
    return result;
  }

  static Effect::EffectManager::WorkType SelectWorkType(
    const WorkSlotAvailability& availability
  )
  {
    if( availability.event )
    {
      return Effect::EffectManager::WORK_TYPE_EVT;
    }
    if( availability.system )
    {
      return Effect::EffectManager::WORK_TYPE_SYS;
    }
    if( availability.ride )
    {
      return Effect::EffectManager::WORK_TYPE_RID;
    }
    return Effect::EffectManager::WORK_TYPE_DEFAULT;
  }

  FollowerSpecialEffectController()
  : m_pEffectManager( NULL )
  , m_pResourceHeap( NULL )
  , m_pEffect( NULL )
  , m_DynamicPhase( DYNAMIC_PHASE_IDLE )
  , m_DynamicOwner( OWNER_NONE )
  , m_DynamicType( static_cast<Effect::Type>( -1 ) )
  , m_DynamicFrame( 0 )
  , m_DynamicLifetimeFrames( 0 )
  , m_DynamicCueFrame( NO_CUE_FRAME )
  , m_ReleaseSettleFrame( 0 )
  , m_ReleaseSettleFrames( DEFAULT_RELEASE_SETTLE_FRAMES )
  , m_DynamicCuePending( false )
  , m_DynamicCueReady( false )
  , m_OwnsDynamicResource( false )
  , m_WeatherContext( NULL )
  , m_RestoreWeather( NULL )
  , m_WeatherOwner( OWNER_NONE )
  , m_WeatherKind( -1 )
  , m_WeatherFramesRemaining( 0 )
  , m_WeatherHasDeadline( false )
  , m_WeatherActive( false )
  {
  }

  DynamicStartResult StartDynamicEffect(
    Owner owner,
    Effect::EffectManager* pEffectManager,
    gfl2::heap::HeapBase* pResourceHeap,
    const DynamicEffectRequest& request
  )
  {
    if( owner == OWNER_NONE )
    {
      return DYNAMIC_START_INVALID_OWNER;
    }
    if( IsDynamicEffectBusy() )
    {
      return DYNAMIC_START_BUSY;
    }
    if( !pEffectManager )
    {
      return DYNAMIC_START_NO_EFFECT_MANAGER;
    }
    if( !pResourceHeap )
    {
      return DYNAMIC_START_NO_RESOURCE_HEAP;
    }

    Effect::EffectManager::WorkType workType = request.workType;
    if( workType == Effect::EffectManager::WORK_TYPE_DEFAULT )
    {
      workType = SelectWorkType( InspectWorkSlots( pEffectManager ) );
    }
    if( workType == Effect::EffectManager::WORK_TYPE_DEFAULT )
    {
      return DYNAMIC_START_NO_EFFECT_SLOT;
    }

    if( pEffectManager->IsDataAvailable( request.type ) )
    {
      return DYNAMIC_START_RESOURCE_ALREADY_LOADED;
    }
    if( pResourceHeap->GetTotalAllocatableSize() <
        request.minimumResourceHeapFree )
    {
      return DYNAMIC_START_RESOURCE_HEAP_LOW;
    }

    pEffectManager->LoadData(
      request.type,
      pResourceHeap,
      1
      );
    if( !pEffectManager->IsDataAvailable( request.type ) )
    {
      return DYNAMIC_START_LOAD_FAILED;
    }

    pEffectManager->RegisterResources(
      request.type,
      pResourceHeap
      );

    m_pEffectManager = pEffectManager;
    m_pResourceHeap = pResourceHeap;
    m_DynamicOwner = owner;
    m_DynamicType = request.type;
    m_OwnsDynamicResource = true;
    m_pEffect = pEffectManager->CreateAtPosition(
      request.type,
      request.position,
      request.playSound,
      workType,
      NULL,
      request.scale,
      request.rotation
      );
    if( !m_pEffect )
    {
      ReleaseDynamicResource();
      ClearDynamicState();
      return DYNAMIC_START_CREATE_FAILED;
    }

    m_DynamicPhase = DYNAMIC_PHASE_PLAYING;
    m_DynamicFrame = 0;
    m_DynamicLifetimeFrames = request.lifetimeFrames;
    m_DynamicCueFrame = request.cueFrame;
    m_ReleaseSettleFrame = 0;
    m_ReleaseSettleFrames = request.releaseSettleFrames > 0
      ? request.releaseSettleFrames
      : DEFAULT_RELEASE_SETTLE_FRAMES;
    m_DynamicCuePending = request.cueFrame != NO_CUE_FRAME &&
      request.cueFrame > 0;
    m_DynamicCueReady = request.cueFrame == 0;
    return DYNAMIC_START_STARTED;
  }

  WeatherStartResult StartTemporaryWeather(
    Owner owner,
    const TemporaryWeatherRequest& request
  )
  {
    if( owner == OWNER_NONE )
    {
      return WEATHER_START_INVALID_OWNER;
    }
    if( !request.context || !request.apply || !request.restore )
    {
      return WEATHER_START_INVALID_BACKEND;
    }
    if( m_WeatherActive )
    {
      return WEATHER_START_BUSY;
    }

    WeatherRestoreToken restoreToken;
    if( !request.apply(
          request.context,
          request.weatherKind,
          &restoreToken
          ) )
    {
      return WEATHER_START_APPLY_FAILED;
    }

    m_WeatherContext = request.context;
    m_RestoreWeather = request.restore;
    m_WeatherRestoreToken = restoreToken;
    m_WeatherOwner = owner;
    m_WeatherKind = request.weatherKind;
    m_WeatherFramesRemaining = request.lifetimeFrames;
    m_WeatherHasDeadline = request.lifetimeFrames > 0;
    m_WeatherActive = true;
    return WEATHER_START_STARTED;
  }

  void Update( void )
  {
    UpdateTemporaryWeather();

    if( m_DynamicPhase == DYNAMIC_PHASE_PLAYING )
    {
      ++m_DynamicFrame;
      if( m_DynamicCuePending && m_DynamicFrame >= m_DynamicCueFrame )
      {
        m_DynamicCuePending = false;
        m_DynamicCueReady = true;
      }
      if( m_DynamicLifetimeFrames > 0 &&
          m_DynamicFrame >= m_DynamicLifetimeFrames )
      {
        RequestStopDynamicEffect( m_DynamicOwner );
      }
      return;
    }

    if( m_DynamicPhase != DYNAMIC_PHASE_DELETE_WAIT )
    {
      return;
    }

    if( m_pEffectManager &&
        m_pEffectManager->CountUnfinished( m_DynamicType ) > 0 )
    {
      m_ReleaseSettleFrame = 0;
      return;
    }

    // Stopping the effect doesn't free it immediately. Give it a few more updates.
    ++m_ReleaseSettleFrame;
    if( m_ReleaseSettleFrame < m_ReleaseSettleFrames )
    {
      return;
    }

    ReleaseDynamicResource();
    ClearDynamicState();
  }

  void RequestStopDynamicEffect( Owner owner )
  {
    if( m_DynamicPhase != DYNAMIC_PHASE_PLAYING ||
        owner == OWNER_NONE || owner != m_DynamicOwner )
    {
      return;
    }

    if( m_pEffectManager && m_pEffect )
    {
      m_pEffectManager->RequestStop( m_pEffect );
    }
    m_pEffect = NULL;
    m_DynamicPhase = DYNAMIC_PHASE_DELETE_WAIT;
    m_DynamicFrame = 0;
    m_ReleaseSettleFrame = 0;
    m_DynamicCuePending = false;
    m_DynamicCueReady = false;
  }

  void RequestStopAllDynamicEffects( void )
  {
    RequestStopDynamicEffect( m_DynamicOwner );
  }

  bool ConsumeDynamicCue( Owner owner )
  {
    if( !m_DynamicCueReady || owner != m_DynamicOwner )
    {
      return false;
    }
    m_DynamicCueReady = false;
    return true;
  }

  void RestoreTemporaryWeather( void )
  {
    if( !m_WeatherActive )
    {
      return;
    }

    void* context = m_WeatherContext;
    RestoreTemporaryWeatherFunction restore = m_RestoreWeather;
    const WeatherRestoreToken restoreToken = m_WeatherRestoreToken;
    ClearWeatherState();
    restore( context, restoreToken );
  }

  void CancelOwner( Owner owner )
  {
    RequestStopDynamicEffect( owner );
    if( m_WeatherActive && owner == m_WeatherOwner )
    {
      RestoreTemporaryWeather();
    }
  }

  void PrepareForFieldExit( void )
  {
    RestoreTemporaryWeather();
    RequestStopAllDynamicEffects();
  }

  bool Reset( void )
  {
    RestoreTemporaryWeather();
    if( IsDynamicEffectBusy() )
    {
      return false;
    }
    ClearDynamicState();
    ClearWeatherState();
    return true;
  }

  bool IsDynamicEffectBusy( void ) const
  {
    return m_DynamicPhase != DYNAMIC_PHASE_IDLE;
  }

  bool IsTemporaryWeatherActive( void ) const
  {
    return m_WeatherActive;
  }

  bool IsBusy( void ) const
  {
    return IsDynamicEffectBusy() || IsTemporaryWeatherActive();
  }

  bool IsOwnerActive( Owner owner ) const
  {
    return IsDynamicOwnerActive( owner ) ||
      (m_WeatherActive && owner == m_WeatherOwner);
  }

  bool IsDynamicOwnerActive( Owner owner ) const
  {
    return IsDynamicEffectBusy() && owner == m_DynamicOwner;
  }

  bool IsUsingDynamicResourceHeap( void ) const
  {
    return m_OwnsDynamicResource;
  }

  gfl2::heap::HeapBase* GetDynamicResourceHeap( void ) const
  {
    return m_OwnsDynamicResource ? m_pResourceHeap : NULL;
  }

  Owner GetDynamicOwner( void ) const
  {
    return m_DynamicOwner;
  }

  Effect::IEffectBase* GetDynamicEffect( Owner owner ) const
  {
    return m_DynamicPhase == DYNAMIC_PHASE_PLAYING &&
      owner == m_DynamicOwner
      ? m_pEffect
      : NULL;
  }

  s32 GetTemporaryWeatherKind( void ) const
  {
    return m_WeatherKind;
  }

private:
  enum DynamicPhase
  {
    DYNAMIC_PHASE_IDLE,
    DYNAMIC_PHASE_PLAYING,
    DYNAMIC_PHASE_DELETE_WAIT,
  };

  enum
  {
    RESTORE_TOKEN_WORDS = 4,
  };

  void UpdateTemporaryWeather( void )
  {
    if( !m_WeatherActive || !m_WeatherHasDeadline )
    {
      return;
    }
    if( m_WeatherFramesRemaining > 0 )
    {
      --m_WeatherFramesRemaining;
    }
    if( m_WeatherFramesRemaining == 0 )
    {
      RestoreTemporaryWeather();
    }
  }

  void ReleaseDynamicResource( void )
  {
    if( m_OwnsDynamicResource && m_pEffectManager && m_pResourceHeap )
    {
      m_pEffectManager->ReleaseResources(
        m_DynamicType,
        m_pResourceHeap,
        false
        );
    }
    m_OwnsDynamicResource = false;
  }

  void ClearDynamicState( void )
  {
    m_pEffectManager = NULL;
    m_pResourceHeap = NULL;
    m_pEffect = NULL;
    m_DynamicPhase = DYNAMIC_PHASE_IDLE;
    m_DynamicOwner = OWNER_NONE;
    m_DynamicType = static_cast<Effect::Type>( -1 );
    m_DynamicFrame = 0;
    m_DynamicLifetimeFrames = 0;
    m_DynamicCueFrame = NO_CUE_FRAME;
    m_ReleaseSettleFrame = 0;
    m_ReleaseSettleFrames = DEFAULT_RELEASE_SETTLE_FRAMES;
    m_DynamicCuePending = false;
    m_DynamicCueReady = false;
    m_OwnsDynamicResource = false;
  }

  void ClearWeatherState( void )
  {
    m_WeatherContext = NULL;
    m_RestoreWeather = NULL;
    m_WeatherRestoreToken = WeatherRestoreToken();
    m_WeatherOwner = OWNER_NONE;
    m_WeatherKind = -1;
    m_WeatherFramesRemaining = 0;
    m_WeatherHasDeadline = false;
    m_WeatherActive = false;
  }

  Effect::EffectManager* m_pEffectManager;
  gfl2::heap::HeapBase* m_pResourceHeap;
  Effect::IEffectBase* m_pEffect;
  DynamicPhase m_DynamicPhase;
  Owner m_DynamicOwner;
  Effect::Type m_DynamicType;
  u32 m_DynamicFrame;
  u32 m_DynamicLifetimeFrames;
  u32 m_DynamicCueFrame;
  u32 m_ReleaseSettleFrame;
  u32 m_ReleaseSettleFrames;
  bool m_DynamicCuePending;
  bool m_DynamicCueReady;
  bool m_OwnsDynamicResource;

  void* m_WeatherContext;
  RestoreTemporaryWeatherFunction m_RestoreWeather;
  WeatherRestoreToken m_WeatherRestoreToken;
  Owner m_WeatherOwner;
  s32 m_WeatherKind;
  u32 m_WeatherFramesRemaining;
  bool m_WeatherHasDeadline;
  bool m_WeatherActive;
};

} // namespace FollowerRuntime
} // namespace Field
