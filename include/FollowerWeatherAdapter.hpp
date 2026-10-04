#pragma once

#include "FollowerSpecialEffectController.hpp"

namespace Field
{
namespace FollowerRuntime
{

class FollowerWeatherAdapter
{
public:
  enum StartResult
  {
    START_STARTED,
    START_NO_CONTROLLER,
    START_NO_FIELDMAP,
    START_NOT_OUTDOORS,
    START_NO_WEATHER_CONTROL,
    START_INVALID_WEATHER_CONTROL,
    START_INVALID_WEATHER_KIND,
    START_BUSY,
    START_APPLY_FAILED,
  };

  static StartResult StartTemporaryWeather(
    FollowerSpecialEffectController* pController,
    Fieldmap* pFieldmap,
    weather::WeatherKind weatherKind,
    u32 lifetimeFrames,
    FollowerSpecialEffectController::Owner owner =
      FollowerSpecialEffectController::OWNER_SPECIES_INTERACTION
  )
  {
    if( !pController )
    {
      return START_NO_CONTROLLER;
    }
    if( !pFieldmap )
    {
      return START_NO_FIELDMAP;
    }
    if( !IsWeatherKindValid( static_cast<s32>( weatherKind ) ) )
    {
      return START_INVALID_WEATHER_KIND;
    }

    System::Skybox::Skybox* pSkybox = pFieldmap->GetSkybox();
    if( !pSkybox || !pSkybox->IsEnable() )
    {
      return START_NOT_OUTDOORS;
    }

    weather::WeatherControl* pWeatherControl =
      pFieldmap->GetWeatherControl();
    if( !pWeatherControl )
    {
      return START_NO_WEATHER_CONTROL;
    }
    if( !IsWeatherControlValid( pWeatherControl ) )
    {
      return START_INVALID_WEATHER_CONTROL;
    }

    FollowerSpecialEffectController::TemporaryWeatherRequest request;
    request.context = pFieldmap;
    request.apply = ApplyTemporaryWeather;
    request.restore = RestoreTemporaryWeather;
    request.weatherKind = static_cast<s32>( weatherKind );
    request.lifetimeFrames = lifetimeFrames;
    const FollowerSpecialEffectController::WeatherStartResult result =
      pController->StartTemporaryWeather(
        owner,
        request
        );
    if( result == FollowerSpecialEffectController::WEATHER_START_STARTED )
    {
      return START_STARTED;
    }
    if( result == FollowerSpecialEffectController::WEATHER_START_BUSY )
    {
      return START_BUSY;
    }
    return START_APPLY_FAILED;
  }

  static s32 GetNowWeatherKind( const Fieldmap* pFieldmap )
  {
    const weather::WeatherControl* pWeatherControl =
      GetWeatherControl( pFieldmap );
    return IsWeatherControlValid( pWeatherControl )
      ? static_cast<s32>( pWeatherControl->GetNowWeatherKindRaw() )
      : static_cast<s32>( weather::FORCE_WEATHER_NONE );
  }

  static s32 GetForceWeatherKind( const Fieldmap* pFieldmap )
  {
    const weather::WeatherControl* pWeatherControl =
      GetWeatherControl( pFieldmap );
    return IsWeatherControlValid( pWeatherControl )
      ? static_cast<s32>( pWeatherControl->GetForceWeatherKindRaw() )
      : static_cast<s32>( weather::FORCE_WEATHER_NONE );
  }

private:
  enum
  {
    RESTORE_TOKEN_MAGIC = 0x57465452U,
  };

  static const weather::WeatherControl* GetWeatherControl(
    const Fieldmap* pFieldmap
  )
  {
    return pFieldmap
      ? const_cast<Fieldmap*>( pFieldmap )->GetWeatherControl()
      : NULL;
  }

  static bool IsWeatherKindValid( s32 weatherKind )
  {
    return weatherKind >= static_cast<s32>( weather::SUNNY ) &&
      weatherKind <= static_cast<s32>( weather::DIAMONDDUST );
  }

  static bool IsStoredWeatherKindValid( s32 weatherKind )
  {
    return weatherKind == static_cast<s32>( weather::FORCE_WEATHER_NONE ) ||
      IsWeatherKindValid( weatherKind );
  }

  static bool IsWeatherControlValid(
    const weather::WeatherControl* pWeatherControl
  )
  {
    return pWeatherControl &&
      pWeatherControl->HasValidRequestState() &&
      IsStoredWeatherKindValid(
        static_cast<s32>( pWeatherControl->GetNowWeatherKindRaw() )
        ) &&
      IsStoredWeatherKindValid(
        static_cast<s32>( pWeatherControl->GetForceWeatherKindRaw() )
        );
  }

  static void SetForceWeather(
    weather::WeatherControl* pWeatherControl,
    s32 weatherKind
  )
  {
    if( weatherKind == static_cast<s32>( weather::FORCE_WEATHER_NONE ) )
    {
      pWeatherControl->ClearOverride();
      return;
    }
    pWeatherControl->RequestOverride(
      static_cast<weather::WeatherKind>( weatherKind )
      );
  }

  static bool ApplyTemporaryWeather(
    void* context,
    s32 weatherKind,
    FollowerSpecialEffectController::WeatherRestoreToken* pRestoreToken
  )
  {
    Fieldmap* pFieldmap = static_cast<Fieldmap*>( context );
    if( !pFieldmap || !pRestoreToken || !IsWeatherKindValid( weatherKind ) )
    {
      return false;
    }

    weather::WeatherControl* pWeatherControl =
      pFieldmap->GetWeatherControl();
    if( !IsWeatherControlValid( pWeatherControl ) )
    {
      return false;
    }

    const s32 originalForce = static_cast<s32>(
      pWeatherControl->GetForceWeatherKindRaw()
      );
    pRestoreToken->words[0] = RESTORE_TOKEN_MAGIC;
    pRestoreToken->words[1] = reinterpret_cast<u32>( pFieldmap );
    pRestoreToken->words[2] = reinterpret_cast<u32>( pWeatherControl );
    pRestoreToken->words[3] = static_cast<u32>(
      static_cast<u8>( originalForce )
      );

    pWeatherControl->RequestOverride(
      static_cast<weather::WeatherKind>( weatherKind )
      );
    if( static_cast<s32>( pWeatherControl->GetForceWeatherKindRaw() ) ==
        weatherKind )
    {
      return true;
    }

    SetForceWeather( pWeatherControl, originalForce );
    *pRestoreToken =
      FollowerSpecialEffectController::WeatherRestoreToken();
    return false;
  }

  static void RestoreTemporaryWeather(
    void* context,
    const FollowerSpecialEffectController::WeatherRestoreToken& restoreToken
  )
  {
    Fieldmap* pFieldmap = static_cast<Fieldmap*>( context );
    if( restoreToken.words[0] != RESTORE_TOKEN_MAGIC ||
        !pFieldmap ||
        restoreToken.words[1] != reinterpret_cast<u32>( pFieldmap ) )
    {
      return;
    }

    weather::WeatherControl* pWeatherControl =
      pFieldmap->GetWeatherControl();
    if( !IsWeatherControlValid( pWeatherControl ) ||
        restoreToken.words[2] !=
          reinterpret_cast<u32>( pWeatherControl ) )
    {
      return;
    }

    const s32 originalForce = static_cast<s32>(
      static_cast<s8>( restoreToken.words[3] & 0xffU )
      );
    if( IsStoredWeatherKindValid( originalForce ) )
    {
      SetForceWeather( pWeatherControl, originalForce );
    }
  }
};

} // namespace FollowerRuntime
} // namespace Field
