#pragma once

namespace Gen7Follower3gx
{

enum DiagnosticWeatherSelection
{
  DIAGNOSTIC_WEATHER_MAP_DEFAULT = -1,
  DIAGNOSTIC_WEATHER_SUNNY = 0,
  DIAGNOSTIC_WEATHER_CLOUDY = 1,
  DIAGNOSTIC_WEATHER_RAIN = 2,
  DIAGNOSTIC_WEATHER_THUNDERSTORM = 3,
  DIAGNOSTIC_WEATHER_SNOW = 4,
  DIAGNOSTIC_WEATHER_SNOWSTORM = 5,
  DIAGNOSTIC_WEATHER_DRY = 6,
  DIAGNOSTIC_WEATHER_SANDSTORM = 7,
  DIAGNOSTIC_WEATHER_MIST = 8,
  DIAGNOSTIC_WEATHER_SUNSHOWER = 9,
  DIAGNOSTIC_WEATHER_DIAMOND_DUST = 10,
};

void QueueDiagnosticWeatherSelection(int selection);
bool ConsumeDiagnosticWeatherSelection(int* selection);
int GetDiagnosticWeatherSelection();
const char* GetDiagnosticWeatherSelectionName(int selection);
void RequeueDiagnosticWeatherSelection();
void ResetDiagnosticWeatherSelection();

} // namespace Gen7Follower3gx
