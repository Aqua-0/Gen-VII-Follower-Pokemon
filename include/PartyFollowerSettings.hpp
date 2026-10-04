#pragma once
namespace Gen7Follower3gx {
const unsigned int PartyFollowerMaximum=3;
struct PartyFollowerSettings {
  unsigned int count;
  unsigned int slots[PartyFollowerMaximum]; // 0 = automatic; 1..6 = party slot.
};
inline bool ValidPartyFollowerSettings(const PartyFollowerSettings& settings) {
  if (settings.count<1 || settings.count>PartyFollowerMaximum) return false;
  for (auto slot : settings.slots) if (slot>6) return false;
  return true;
}
inline void ResolvePartyFollowers(const PartyFollowerSettings& settings,
  const bool (&valid)[6], int (&selected)[PartyFollowerMaximum]) {
  bool used[6]={};
  for (unsigned int follower=0;follower<PartyFollowerMaximum;++follower) {
    selected[follower]=-1;
    if (follower>=settings.count) continue;
    const auto requested=settings.slots[follower];
    for (unsigned int slot=0;slot<6;++slot) {
      if (requested && requested!=slot+1) continue;
      if (!valid[slot] || used[slot]) continue;
      selected[follower]=static_cast<int>(slot); used[slot]=true; break;
    }
  }
}
void InitializePartyFollowerSettings();
PartyFollowerSettings GetPartyFollowerSettings();
bool SavePartyFollowerSettings(const PartyFollowerSettings& settings);
}
