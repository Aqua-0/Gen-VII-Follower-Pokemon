#pragma once

#include "CroModule.hpp"
#include "GameProfile.hpp"

namespace Gen7Follower3gx
{

bool BindRetailApi(const GameProfile& profile, const CroModuleView& fieldRo);
void UnbindRetailApi();
bool IsRetailApiBound();

} // namespace Gen7Follower3gx
