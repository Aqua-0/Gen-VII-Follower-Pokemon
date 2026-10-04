#pragma once

#include "CroModule.hpp"
#include "GameProfile.hpp"

namespace Gen7Follower3gx
{

bool InstallFieldHooks(const GameProfile& profile, const CroModuleView& fieldRo);
void RemoveFieldHooks();
bool AreFieldHooksInstalled();

} // namespace Gen7Follower3gx
