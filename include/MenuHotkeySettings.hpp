#pragma once
namespace Gen7Follower3gx {
const unsigned int DefaultMenuHotkey=(1U<<2)|(1U<<3);
const unsigned int AlternateMenuHotkey=(1U<<9)|(1U<<7)|(1U<<2);
inline bool IsValidMenuHotkey(unsigned int keys)
{
  return keys && !(keys & ~0xcfffU) &&
    (keys & 0x30U)!=0x30U && (keys & 0xc0U)!=0xc0U;
}
void InitializeMenuHotkeySettings();
unsigned int GetMenuHotkey();
bool SetMenuHotkey(unsigned int keys);
}
