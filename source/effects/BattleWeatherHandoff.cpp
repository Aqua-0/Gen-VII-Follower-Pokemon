#include "BattleWeatherHandoff.hpp"

namespace Gen7Follower3gx
{
namespace
{

void* g_GameManager = 0;
unsigned char g_Weather = BATTLE_WEATHER_NONE;

} // namespace

void ArmBattleWeatherHandoff(void* gameManager, unsigned char weather)
{
  if (!gameManager || weather <= BATTLE_WEATHER_NONE ||
      weather >= BATTLE_WEATHER_MAX)
  {
    ClearBattleWeatherHandoff();
    return;
  }

  g_GameManager = gameManager;
  g_Weather = weather;
}

void ClearBattleWeatherHandoff(void* gameManager)
{
  if (gameManager && gameManager != g_GameManager)
  {
    return;
  }

  g_Weather = BATTLE_WEATHER_NONE;
  g_GameManager = 0;
}

bool ConsumeBattleWeatherHandoff(
  void* gameManager,
  unsigned char* weather
  )
{
  if (!weather || !gameManager || gameManager != g_GameManager ||
      g_Weather <= BATTLE_WEATHER_NONE || g_Weather >= BATTLE_WEATHER_MAX)
  {
    return false;
  }

  *weather = g_Weather;
  ClearBattleWeatherHandoff(gameManager);
  return true;
}

} // namespace Gen7Follower3gx
