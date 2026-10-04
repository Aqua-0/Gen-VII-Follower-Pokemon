#include "types.h"
#include "RidePresentation.hpp"
#include "RideRuntimeProfiles.hpp"
#include "GameProfile.hpp"
#include <CTRPluginFramework/System/Hook.hpp>
#include <CTRPluginFramework/System/Process.hpp>
#include <cmath>
#include <cstdio>
#include <3ds.h>
#include <cstring>
namespace Gen7Follower3gx {
namespace {
CTRPluginFramework::Hook hooks[4];
void* playerWork=nullptr;
void* playerModel=nullptr;
void* riderModel=nullptr;
bool mounted=false;
float speed=1;
unsigned int accessorOffset=0;
void* shadowModels[2]={};
bool shadowVisibility[2]={};
// The game writes Vector3 through r0. The model goes in r1 and the frame in s0.
struct WalkVector { float x,y,z; };
void Visible(void* model,bool value) {
  auto table=*reinterpret_cast<unsigned int**>(model);
  reinterpret_cast<void(*)(void*,unsigned int)>(table[5])(model,value);
}
bool IsVisible(void* model) {
  auto table=*reinterpret_cast<unsigned int**>(model);
  return reinterpret_cast<unsigned int(*)(void*)>(table[6])(model)!=0;
}
extern "C" void RideWalkSpeed(WalkVector* result,void* model,float frame) {
  CTRPluginFramework::HookContext::GetCurrent().OriginalFunction<void>(result,model,frame);
  if (model==playerModel && playerWork && std::isfinite(speed)) {
    const auto* state=reinterpret_cast<const unsigned char*>(playerWork);
    // Check the event-request count and scripted-action flag before changing movement speed.
    if (!*reinterpret_cast<const unsigned int*>(state+0xd8) &&
        !*reinterpret_cast<const unsigned int*>(state+0xe0)) {
      result->x*=speed; result->z*=speed;
    }
  }
}
extern "C" void RideFootSound(void* accessor,float before,float after,unsigned int animation,unsigned int attribute) {
  if (mounted && playerWork && accessor==reinterpret_cast<unsigned char*>(playerWork)+accessorOffset) return;
  CTRPluginFramework::HookContext::GetCurrent().OriginalFunction<void>(accessor,before,after,animation,attribute);
}
extern "C" void RideStepEffect(void* work) {
  if (mounted && work==playerWork) return;
  CTRPluginFramework::HookContext::GetCurrent().OriginalFunction<void>(work);
}
}
void BeginRideShadowPass() {
  if (!mounted) return;
  shadowModels[0]=playerModel; shadowModels[1]=riderModel;
  for (int i=0;i<2;++i) if (shadowModels[i]) {
    shadowVisibility[i]=IsVisible(shadowModels[i]);
    Visible(shadowModels[i],false);
  }
}
void EndRideShadowPass() {
  for (int i=0;i<2;++i) if (shadowModels[i]) {
    Visible(shadowModels[i],shadowVisibility[i]); shadowModels[i]=nullptr;
  }
}
bool InstallRidePresentationHooks(const GameProfile& profile) {
  RemoveRidePresentationHooks();
  const RideRuntimeProfile* found=nullptr;
  for (const auto& candidate : RideRuntimeProfiles)
    if (std::strcmp(candidate.name,profile.name)==0) found=&candidate;
  if (!found) return false;
  for (const auto& target : found->targets) {
    if (!CTRPluginFramework::Process::CheckAddress(target.address, MEMPERM_READ|MEMPERM_EXECUTE) ||
        !CTRPluginFramework::Process::CheckAddress(target.address+4, MEMPERM_READ|MEMPERM_EXECUTE)) return false;
    const auto* code=reinterpret_cast<const volatile unsigned int*>(target.address);
    if (code[0]!=target.expected[0] || code[1]!=target.expected[1]) return false;
  }
  const unsigned int callbacks[]={reinterpret_cast<unsigned int>(RideWalkSpeed),
    reinterpret_cast<unsigned int>(RideFootSound),reinterpret_cast<unsigned int>(RideStepEffect),
    reinterpret_cast<unsigned int>(RideStepEffect)};
  for (int i=0;i<4;++i) {
    if (hooks[i].InitializeForMitm(found->targets[i].address,callbacks[i]).Enable()!=CTRPluginFramework::HookResult::Success) {
      RemoveRidePresentationHooks(); return false;
    }
  }
  accessorOffset=found->footAccessorOffset;
  char message[160];
  const int length=std::snprintf(message,sizeof(message),
    "[RidePresentation] Active profile=%s speed=horizontal footSoundOffset=%x\n",profile.name,accessorOffset);
  if (length>0) svcOutputDebugString(message,length<static_cast<int>(sizeof(message)) ? length : sizeof(message)-1);
  return true;
}
void RemoveRidePresentationHooks() {
  Follower3gx_ClearRidePresentation();
  for (auto& hook : hooks) hook.Disable();
}
}
extern "C" void Follower3gx_SetRidePresentation(void* work,void* player,void* rider,bool isMounted,float multiplier) {
  Gen7Follower3gx::playerWork=work;
  Gen7Follower3gx::playerModel=player;
  Gen7Follower3gx::riderModel=rider;
  Gen7Follower3gx::mounted=isMounted;
  Gen7Follower3gx::speed=multiplier;
}
extern "C" void Follower3gx_ClearRidePresentation() {
  Gen7Follower3gx::EndRideShadowPass();
  Gen7Follower3gx::playerWork=nullptr;
  Gen7Follower3gx::playerModel=nullptr;
  Gen7Follower3gx::riderModel=nullptr;
  Gen7Follower3gx::mounted=false;
  Gen7Follower3gx::speed=1;
}
