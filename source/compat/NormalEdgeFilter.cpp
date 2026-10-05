#include "NormalEdgeFilter.hpp"
#include "EdgeFilterProfiles.hpp"
#include "GameProfile.hpp"
#include "types.h"
#include <CTRPluginFramework/System/Hook.hpp>
#include <CTRPluginFramework/System/Process.hpp>
#include <cstring>

extern "C" bool FollowerCarrier_ShouldEraseEdgeMaterialRegular(const void* material);
extern "C" bool FollowerCarrier_ShouldEraseEdgeMaterialUltra(const void* material);
namespace Gen7Follower3gx {
namespace {
CTRPluginFramework::Hook vertexHook, pixelHook;
bool installed=false;
bool (*shouldErase)(const void*)=nullptr;

int* MaterialEdgeType(void* tag)
{
  if (!tag) return nullptr;
  void* material=*reinterpret_cast<void**>(static_cast<unsigned char*>(tag)+0x48);
  if (!material) return nullptr;
  void* resource=*reinterpret_cast<void**>(static_cast<unsigned char*>(material)+0x28);
  return resource ? reinterpret_cast<int*>(static_cast<unsigned char*>(resource)+0x25c) : nullptr;
}

bool FilterEdgeVertex(void* driver, void* state, void* tag)
{
  int* type=MaterialEdgeType(tag);
  EdgeMaterialOverride override(type, type && shouldErase && shouldErase(type));
  return CTRPluginFramework::HookContext::GetCurrent().OriginalFunction<bool>(driver,state,tag);
}

void FilterEdgePixel(void* driver, void* state, void* tag)
{
  int* type=MaterialEdgeType(tag);
  EdgeMaterialOverride override(type, type && shouldErase && shouldErase(type));
  CTRPluginFramework::HookContext::GetCurrent().OriginalFunction<void>(driver,state,tag);
}
}

bool InstallNormalEdgeFilter(const GameProfile& profile)
{
  RemoveNormalEdgeFilter();
  for (const auto& candidate : EdgeFilterProfiles)
  {
    if (std::strcmp(candidate.name,profile.name)) continue;
    for (const auto& target : candidate.targets)
    {
      if (!CTRPluginFramework::Process::CheckAddress(target.address,MEMPERM_READ|MEMPERM_EXECUTE) ||
          !CTRPluginFramework::Process::CheckAddress(target.address+8,MEMPERM_READ|MEMPERM_EXECUTE)) return false;
      const auto* words=reinterpret_cast<const volatile unsigned int*>(target.address);
      for (unsigned int i=0;i<3;++i) if (words[i]!=target.expected[i]) return false;
    }
    shouldErase = profile.family==GameFamily::Regular
      ? FollowerCarrier_ShouldEraseEdgeMaterialRegular : FollowerCarrier_ShouldEraseEdgeMaterialUltra;
    using CTRPluginFramework::HookResult;
    if (vertexHook.InitializeForMitm(candidate.targets[0].address,
        reinterpret_cast<u32>(&FilterEdgeVertex)).Enable()!=HookResult::Success ||
        pixelHook.InitializeForMitm(candidate.targets[1].address,
        reinterpret_cast<u32>(&FilterEdgePixel)).Enable()!=HookResult::Success)
    {
      RemoveNormalEdgeFilter();
      return false;
    }
    installed=true;
    return true;
  }
  return false;
}
void RemoveNormalEdgeFilter()
{
  installed=false;
  pixelHook.Disable();
  vertexHook.Disable();
  shouldErase=nullptr;
}
bool IsNormalEdgeFilterInstalled() { return installed; }
}
