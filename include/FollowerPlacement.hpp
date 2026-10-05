#pragma once

namespace Gen7Follower3gx {
template <typename Position, typename Validate>
bool FindFollowerPlacement(const Position& player, float distance,
                           Position& result, Validate validate)
{
  const float directions[][2] = {
    {0,1}, {0,-1}, {1,0}, {-1,0},
    {0.707107f,0.707107f}, {-0.707107f,0.707107f},
    {0.707107f,-0.707107f}, {-0.707107f,-0.707107f}
  };
  for (const auto& direction : directions) {
    Position candidate=player;
    candidate.x+=direction[0]*distance;
    candidate.z+=direction[1]*distance;
    if (!validate(candidate)) continue;
    result=candidate;
    return true;
  }
  return false;
}
}
