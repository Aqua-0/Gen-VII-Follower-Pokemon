#include "GameProfile.hpp"

namespace Gen7Follower3gx
{

#include "GameProfiles.inc"

const GameProfile* GetGameProfiles(u32& count)
{
  count = sizeof(kGameProfiles) / sizeof(kGameProfiles[0]);
  return kGameProfiles;
}

const GameProfile* SelectGameProfile(u64 titleId, u16 version)
{
  const u32 titleIdLow = static_cast<u32>(titleId);
  u32 count = 0;
  const GameProfile* profiles = GetGameProfiles(count);
  for (u32 index = 0; index < count; ++index)
  {
    const GameProfile& profile = profiles[index];
    if (profile.titleIdLow == titleIdLow &&
        version >= profile.minimumVersion &&
        version <= profile.maximumVersion)
    {
      return &profile;
    }
  }
  return NULL;
}

} // namespace Gen7Follower3gx
