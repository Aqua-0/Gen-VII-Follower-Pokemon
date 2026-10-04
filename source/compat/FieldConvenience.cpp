#include "types.h"
#include "Diagnostics.hpp"
#include "FieldConvenience.hpp"
#include "PcRuntimeProfiles.hpp"
#include "GameProfile.hpp"
#include "MountedRideFeedback.hpp"
#include <CTRPluginFramework/System/Process.hpp>
#include <cstring>
#include <cstdio>
#include <3ds/svc.h>
namespace Gen7Follower3gx {
namespace {
const PcRuntimeProfile* pcProfile=nullptr;
unsigned int pcRequest=0;
// Use functions from static code because the parent event outlives the field module.
const PcRuntimeProfile* activeProfile=nullptr;
void* activeApp=nullptr;
unsigned int appVtable[9]={}; // The table starts with two ABI words, followed by seven virtual functions.
unsigned int lastSequence=~0U;
void LogPc(const char* phase,unsigned int sequence)
{
#if FOLLOWER_3GX_DIAGNOSTIC
  char text[128];
  const int length=std::snprintf(text,sizeof(text),"[FollowerPC] %s sequence=%u\n",phase,sequence);
  if (length>0) svcOutputDebugString(text,length<static_cast<int>(sizeof(text)) ? length : sizeof(text)-1);
#else
  (void)phase;
  (void)sequence;
#endif
}
unsigned int PcAppMain(void* app,void* gameManager)
{
  auto* sequence=reinterpret_cast<unsigned int*>(static_cast<unsigned char*>(app)+0x4c);
  if (*sequence!=lastSequence) { lastSequence=*sequence; LogPc("field/app transition",*sequence); }
  // Replace the app launch at state 3. Let the game handle closing and restoring the field.
  if (*sequence==3) {
    void* box=reinterpret_cast<void*(*)(void*)>(activeProfile->targets[0].address)(gameManager);
    if (box) reinterpret_cast<void(*)(void*,unsigned int,unsigned int)>(activeProfile->targets[1].address)(box,0,0);
    else Follower3gx_NotifyRide("PC allocation failed; returning to the field");
    *sequence=4;
    LogPc(box ? "storage event queued" : "storage allocation failed",*sequence);
    return 0; // Keep the parent event running while storage is open.
  }
  return reinterpret_cast<unsigned int(*)(void*,void*)>(activeProfile->targets[3].address)(app,gameManager);
}
void PcAppEnd(void* app,void* gameManager)
{
  const auto* nativeTable=reinterpret_cast<const unsigned int*>(activeProfile->appVtable);
  reinterpret_cast<void(*)(void*,void*)>(nativeTable[5])(app,gameManager);
  *reinterpret_cast<unsigned int*>(app)=activeProfile->appVtable;
  activeApp=nullptr;
  activeProfile=nullptr;
  LogPc("returned to field",8);
}
}
void BindPcEvent(const GameProfile& profile)
{
  UnbindPcEvent();
  for (const auto& candidate : PcRuntimeProfiles) {
    if (std::strcmp(candidate.name,profile.name)) continue;
    for (const auto& target : candidate.targets) {
      if (!CTRPluginFramework::Process::CheckAddress(target.address,MEMPERM_READ|MEMPERM_EXECUTE) ||
          !CTRPluginFramework::Process::CheckAddress(target.address+8,MEMPERM_READ|MEMPERM_EXECUTE)) return;
      const auto* words=reinterpret_cast<const volatile unsigned int*>(target.address);
      for (unsigned int i=0;i<3;++i) if (words[i]!=target.expected[i]) return;
    }
    if (!CTRPluginFramework::Process::CheckAddress(candidate.appVtable-8,MEMPERM_READ) ||
        !CTRPluginFramework::Process::CheckAddress(candidate.appVtable+24,MEMPERM_READ)) return;
    const auto* table=reinterpret_cast<const unsigned int*>(candidate.appVtable);
    if (table[4]!=candidate.targets[3].address) return;
    pcProfile=&candidate;
  }
}
void UnbindPcEvent() { pcProfile=nullptr; SetPcRequestState(0); }
bool RequestOpenPc()
{
  if (!pcProfile || activeApp) return false;
  unsigned int expected=0;
  return __atomic_compare_exchange_n(&pcRequest,&expected,1,false,__ATOMIC_RELAXED,__ATOMIC_RELAXED);
}
unsigned int PcRequestState() { return __atomic_load_n(&pcRequest,__ATOMIC_RELAXED); }
void SetPcRequestState(unsigned int state) { __atomic_store_n(&pcRequest,state,__ATOMIC_RELAXED); }
}
extern "C" bool Follower3gx_StartPcEvent(void* gameManager)
{
  using namespace Gen7Follower3gx;
  if (!gameManager || !pcProfile) return false;
  if (activeApp) return false;
  void* manager=*reinterpret_cast<void**>(static_cast<unsigned char*>(gameManager)+0x20);
  if (!manager) return false;
  void* heap=reinterpret_cast<void*(*)(void*)>(pcProfile->targets[4].address)(manager);
  if (!heap) return false;
  void* app=reinterpret_cast<void*(*)(void*,int)>(pcProfile->targets[5].address)(heap,0xa8);
  if (!app) return false;
  std::memset(app,0,0xa8);
  reinterpret_cast<void*(*)(void*,void*)>(pcProfile->targets[2].address)(app,heap);
  auto* bytes=static_cast<unsigned char*>(app);
  bytes[0x1c]=0; // Assumed neutral caller mode. Still needs checking.
  bytes[0x48]=1; // Assumed to restore the field when storage closes.
  bytes[0x58]=0; // Assumed normal close mode. Resource retention still needs checking.
  const auto* nativeTable=reinterpret_cast<const unsigned int*>(pcProfile->appVtable);
  std::memcpy(appVtable,nativeTable-2,sizeof(appVtable));
  appVtable[2+4]=reinterpret_cast<unsigned int>(&PcAppMain);
  appVtable[2+5]=reinterpret_cast<unsigned int>(&PcAppEnd);
  activeProfile=pcProfile;
  activeApp=app;
  lastSequence=~0U;
  *reinterpret_cast<unsigned int**>(app)=appVtable+2;
  reinterpret_cast<void(*)(void*,void*)>(pcProfile->targets[6].address)(manager,app);
  LogPc("field close queued",0);
  return true;
}
