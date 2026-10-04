#pragma once

#include "FollowerFeatures.hpp"

namespace Gen7Follower3gx
{

struct GameProfile;

struct VictiniLuckDiagnosticSnapshot
{
  unsigned int active;
  unsigned int hooksEnabled;
  unsigned int remainingTenths;
  unsigned int targetRolls;
  unsigned int activationCount;
  unsigned int encounterCount;
  unsigned int pokemonCount;
  unsigned int intruderCount;
  unsigned int previousRolls;
  unsigned int boostedRolls;
};

void InitializeVictiniLuck();
void ShutdownVictiniLuck();
bool InstallVictiniLuckHooks(const GameProfile& profile);
void SuspendVictiniLuckHooksForRetailBind();
void ActivateVictiniLuck();
VictiniLuckDiagnosticSnapshot GetVictiniLuckDiagnostics();

} // namespace Gen7Follower3gx
