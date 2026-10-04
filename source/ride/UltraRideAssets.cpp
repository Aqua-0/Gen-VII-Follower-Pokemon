#include "MountedRideFeedback.hpp"
#include "SolgaleoRideProfiles.hpp"
#include <CTRPluginFramework/System/File.hpp>
#include <cstdlib>
#include <malloc.h>

extern "C" void* Follower3gx_LoadUltraRidePack(unsigned int characterId, bool lunala)
{
  if (characterId > 1U) { return NULL; }
  const auto& profile = SolgaleoRidePacks[characterId+(lunala ? 2 : 0)];
  CTRPluginFramework::File file;
  if (CTRPluginFramework::File::Open(file, profile.path,
        CTRPluginFramework::File::READ) != 0 || file.GetSize() != profile.size)
  {
    return NULL;
  }
  void* data = memalign(128, profile.size);
  if (!data) { return NULL; }
  if (file.Read(data, profile.size) != 0)
  {
    std::free(data);
    return NULL;
  }
  unsigned int checksum = 2166136261U;
  const auto* bytes = static_cast<const unsigned char*>(data);
  for (unsigned int i = 0; i < profile.size; ++i)
  {
    checksum = (checksum ^ bytes[i]) * 16777619U;
  }
  if (checksum != profile.checksum)
  {
    std::free(data);
    return NULL;
  }
  return data;
}

extern "C" void Follower3gx_FreeUltraRidePack(void* data)
{
  std::free(data);
}
