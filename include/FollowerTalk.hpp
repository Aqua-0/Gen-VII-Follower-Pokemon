#pragma once

namespace Gen7Follower3gx {
struct GameProfile;
void BindFollowerTalk(const GameProfile& profile, unsigned int textBase);
void UnbindFollowerTalk();
bool UpdateFollowerTalk(void* fieldmap);
bool DrainFollowerTalk();
void ShowFollowerTalk(unsigned int reaction);
void ClearFollowerTalk();

inline unsigned int FollowerTalkDuration(unsigned int reaction)
{
  return reaction == 2 ? 44U : 22U;
}

inline float FollowerTalkHop(unsigned int reaction, unsigned int frame)
{
  if (!reaction || frame >= FollowerTalkDuration(reaction)) return 0.0f;
  const unsigned int phase = frame % 22U;
  if (phase >= 18U) return 0.0f;
  const float t = static_cast<float>(phase) / 18.0f;
  return 32.0f * t * (1.0f - t);
}
}
