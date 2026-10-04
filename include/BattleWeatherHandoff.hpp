#pragma once

namespace Gen7Follower3gx
{

enum BattleWeatherKind : unsigned char
{
  BATTLE_WEATHER_NONE = 0,
  BATTLE_WEATHER_SHINE = 1,
  BATTLE_WEATHER_RAIN = 2,
  BATTLE_WEATHER_SNOW = 3,
  BATTLE_WEATHER_SAND = 4,
  BATTLE_WEATHER_STORM = 5,
  BATTLE_WEATHER_DAY = 6,
  BATTLE_WEATHER_TURBULENCE = 7,
  BATTLE_WEATHER_MAX,
};

void ArmBattleWeatherHandoff(void* gameManager, unsigned char weather);
void ClearBattleWeatherHandoff(void* gameManager = 0);
bool ConsumeBattleWeatherHandoff(
  void* gameManager,
  unsigned char* weather
  );

} // namespace Gen7Follower3gx
