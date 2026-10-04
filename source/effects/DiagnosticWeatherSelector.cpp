#include "DiagnosticWeatherSelector.hpp"

#if FOLLOWER_3GX_DIAGNOSTIC

namespace Gen7Follower3gx
{
namespace
{

volatile int g_DiagnosticWeatherSelection =
  DIAGNOSTIC_WEATHER_MAP_DEFAULT;
volatile unsigned int g_DiagnosticWeatherSelectionPending = 0;

bool IsDiagnosticWeatherSelectionValid(int selection)
{
  return selection >= DIAGNOSTIC_WEATHER_MAP_DEFAULT &&
    selection <= DIAGNOSTIC_WEATHER_DIAMOND_DUST;
}

} // namespace

void QueueDiagnosticWeatherSelection(int selection)
{
  if (!IsDiagnosticWeatherSelectionValid(selection))
  {
    return;
  }

  __atomic_store_n(
    &g_DiagnosticWeatherSelection,
    selection,
    __ATOMIC_RELAXED
    );
  __atomic_store_n(
    &g_DiagnosticWeatherSelectionPending,
    1U,
    __ATOMIC_RELEASE
    );
}

bool ConsumeDiagnosticWeatherSelection(int* selection)
{
  if (!selection ||
      __atomic_exchange_n(
        &g_DiagnosticWeatherSelectionPending,
        0U,
        __ATOMIC_ACQUIRE
        ) == 0U)
  {
    return false;
  }

  *selection = __atomic_load_n(
    &g_DiagnosticWeatherSelection,
    __ATOMIC_RELAXED
    );
  return IsDiagnosticWeatherSelectionValid(*selection);
}

int GetDiagnosticWeatherSelection()
{
  return __atomic_load_n(
    &g_DiagnosticWeatherSelection,
    __ATOMIC_ACQUIRE
    );
}

const char* GetDiagnosticWeatherSelectionName(int selection)
{
  switch (selection)
  {
  case DIAGNOSTIC_WEATHER_SUNNY:
    return "Sunny";
  case DIAGNOSTIC_WEATHER_CLOUDY:
    return "Cloudy";
  case DIAGNOSTIC_WEATHER_RAIN:
    return "Rain";
  case DIAGNOSTIC_WEATHER_THUNDERSTORM:
    return "Thunderstorm";
  case DIAGNOSTIC_WEATHER_SNOW:
    return "Snow";
  case DIAGNOSTIC_WEATHER_SNOWSTORM:
    return "Snowstorm";
  case DIAGNOSTIC_WEATHER_DRY:
    return "Dry";
  case DIAGNOSTIC_WEATHER_SANDSTORM:
    return "Sandstorm";
  case DIAGNOSTIC_WEATHER_MIST:
    return "Mist";
  case DIAGNOSTIC_WEATHER_SUNSHOWER:
    return "Sunshower";
  case DIAGNOSTIC_WEATHER_DIAMOND_DUST:
    return "Diamond dust";
  default:
    return "Map default";
  }
}

void RequeueDiagnosticWeatherSelection()
{
  if (GetDiagnosticWeatherSelection() != DIAGNOSTIC_WEATHER_MAP_DEFAULT)
  {
    __atomic_store_n(
      &g_DiagnosticWeatherSelectionPending,
      1U,
      __ATOMIC_RELEASE
      );
  }
}

void ResetDiagnosticWeatherSelection()
{
  __atomic_store_n(
    &g_DiagnosticWeatherSelection,
    static_cast<int>(DIAGNOSTIC_WEATHER_MAP_DEFAULT),
    __ATOMIC_RELAXED
    );
  __atomic_store_n(
    &g_DiagnosticWeatherSelectionPending,
    0U,
    __ATOMIC_RELEASE
    );
}

} // namespace Gen7Follower3gx

#endif
