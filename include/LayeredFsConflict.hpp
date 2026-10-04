#pragma once

#include <string>

namespace Gen7Follower3gx
{

class CroModuleView;
struct GameProfile;

void InitializeLayeredFsConflictDetection();
void InspectEmulatorFieldRoForConflict(
  const GameProfile& profile,
  const CroModuleView& fieldRo
);
bool HasLayeredFsConflict();
bool ConsumeLayeredFsConflictWarning();
std::string BuildLayeredFsConflictMessage();

} // namespace Gen7Follower3gx
