#include "FollowerTalk.hpp"
#include "FollowerTalkProfiles.hpp"
#include "GameProfile.hpp"
#include "types.h"
#include <CTRPluginFramework/System/Process.hpp>
#include <CTRPluginFramework/System/Controller.hpp>
#include <cstring>
#include <cstdio>
#include <3ds/svc.h>

namespace Gen7Follower3gx {
namespace {
unsigned int targets[12] = {};
unsigned int request = 0;
unsigned int activeReaction = 0;
bool cancelRequested = false;
bool textInstalled = false;
bool windowHidden = false;
bool dismissArmed = false;
bool waitForDismissRelease = false;
unsigned int lastSequence = ~0U;
alignas(8) unsigned char window[0x74] = {};
alignas(8) unsigned char textBuffer[0x10] = {};

template<class T> T& Field(void* object, unsigned int offset)
{
  return *reinterpret_cast<T*>(static_cast<unsigned char*>(object) + offset);
}

template<class Function> Function Call(unsigned int index)
{
  return reinterpret_cast<Function>(targets[index]);
}

void CloseWindow()
{
  Call<void(*)(void*)>(2)(window);
  Call<void(*)(void*)>(5)(textBuffer);
  const char released[] = "[FollowerTalk] window resources released\n";
  svcOutputDebugString(released, sizeof(released)-1);
  activeReaction = 0;
  textInstalled = false;
  windowHidden = false;
  dismissArmed = false;
  cancelRequested = false;
}

bool DismissKeyHeld()
{
  const u32 keys = CTRPluginFramework::Controller::GetKeysDown();
  return (keys & (CTRPluginFramework::Key::A | CTRPluginFramework::Key::B)) != 0;
}

void ReadDismissInput()
{
  const bool held = DismissKeyHeld();
  if (!textInstalled || cancelRequested)
  {
    dismissArmed = false;
    return;
  }
  if (!held)
  {
    dismissArmed = true;
  }
  else if (dismissArmed)
  {
    dismissArmed = false;
    cancelRequested = true;
    waitForDismissRelease = true;
  }
}

bool AdvanceWindow()
{
  if (windowHidden)
  {
    CloseWindow();
    return false;
  }
  ReadDismissInput();
  const bool finished = Call<bool(*)(void*)>(1)(window);
  const unsigned int sequence = Field<unsigned int>(window, 0x5c);
  if (sequence != lastSequence)
  {
    lastSequence = sequence;
    char line[80];
    const int length = std::snprintf(line, sizeof(line),
      "[FollowerTalk] sequence=%u cancel=%u\n", sequence, cancelRequested ? 1U : 0U);
    if (length > 0 && length < static_cast<int>(sizeof(line))) svcOutputDebugString(line, length);
  }
  void* inner = Field<void*>(window, 0x24);
  if (inner && !textInstalled && !cancelRequested)
  {
    static const char* lines[] = {
      "", "Your Pokemon seems happy\nto see you!",
      "Your Pokemon is bouncing\nwith excitement!",
      "Your Pokemon wants to stay\nby your side."
    };
    unsigned short text[96] = {};
    const char* line = lines[activeReaction];
    for (unsigned int i = 0; line[i] && i + 1 < 96; ++i)
      text[i] = static_cast<unsigned char>(line[i]);
    Call<void(*)(void*,const unsigned short*)>(6)(textBuffer, text);
    Call<void(*)(void*,const void*)>(7)(inner, textBuffer);
    Call<void(*)(void*,unsigned int,unsigned char)>(10)(inner, 1, 0);
    Call<void(*)(void*)>(8)(inner);
    textInstalled = true;
  }
  // The native loader must finish its archive closes before destruction.
  if (Field<unsigned int>(window, 0x5c) == 4 &&
      (cancelRequested || (textInstalled && finished)))
  {
    if (inner) Call<void(*)(void*)>(11)(inner);
    windowHidden = true;
    const char hidden[] = "[FollowerTalk] window hidden; release next update\n";
    svcOutputDebugString(hidden, sizeof(hidden)-1);
    return true;
  }
  if (inner) Call<void(*)(void*)>(9)(inner);
  return true;
}
}

void BindFollowerTalk(const GameProfile& profile, unsigned int textBase)
{
  if (activeReaction) return;
  std::memset(targets, 0, sizeof(targets));
  for (const auto& candidate : FollowerTalkProfiles)
  {
    if (std::strcmp(candidate.name, profile.name)) continue;
    unsigned int resolved[12];
    for (unsigned int i = 0; i < 12; ++i)
    {
      const auto& target = candidate.targets[i];
      resolved[i] = target.address + (i < 3 ? textBase : 0);
      if (!CTRPluginFramework::Process::CheckAddress(resolved[i], MEMPERM_READ|MEMPERM_EXECUTE) ||
          !CTRPluginFramework::Process::CheckAddress(resolved[i]+8, MEMPERM_READ|MEMPERM_EXECUTE)) return;
      const auto* words = reinterpret_cast<const volatile unsigned int*>(resolved[i]);
      for (unsigned int j = 0; j < 3; ++j)
        if (words[j] != target.expected[j]) return;
    }
    std::memcpy(targets, resolved, sizeof(targets));
    return;
  }
}

void UnbindFollowerTalk()
{
  request = 0;
  waitForDismissRelease = false;
  if (!activeReaction) std::memset(targets, 0, sizeof(targets));
}

void ShowFollowerTalk(unsigned int reaction)
{
  if (!activeReaction) request = reaction <= 3 ? reaction : 0;
}

void ClearFollowerTalk()
{
  request = 0;
  if (activeReaction) cancelRequested = true;
}

bool DrainFollowerTalk()
{
  ClearFollowerTalk();
  waitForDismissRelease = false;
  return activeReaction && AdvanceWindow();
}

bool UpdateFollowerTalk(void* fieldmap)
{
  if (activeReaction)
  {
    AdvanceWindow();
    return true;
  }
  if (waitForDismissRelease)
  {
    waitForDismissRelease = DismissKeyHeld();
    return true;
  }
  const unsigned int reaction = request;
  request = 0;
  if (!reaction || !targets[0] || !fieldmap || !Field<void*>(fieldmap, 0xe0)) return false;
  void* heap = Call<void*(*)(int)>(3)(11);
  if (!heap) return false;
  const auto* heapTable = *static_cast<unsigned int**>(heap);
  if (reinterpret_cast<unsigned int(*)(void*)>(heapTable[0x2c/4])(heap) < 0xc0000U)
    return false;
  std::memset(window, 0, sizeof(window));
  std::memset(textBuffer, 0, sizeof(textBuffer));
  Call<void*(*)(void*,unsigned int,void*)>(4)(textBuffer, 96, heap);
  Call<void*(*)(void*,void*)>(0)(window, fieldmap);
  activeReaction = reaction;
  windowHidden = false;
  dismissArmed = false;
  waitForDismissRelease = false;
  lastSequence = ~0U;
  cancelRequested = false;
  textInstalled = false;
  AdvanceWindow();
  return true;
}
}
