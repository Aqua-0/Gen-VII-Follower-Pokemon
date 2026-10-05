#pragma once
#include <cmath>
namespace Gen7Follower3gx {
inline float ClearFollowStep(float offsetX, float offsetZ, float moveX, float moveZ, float radius)
{
  const float a=moveX*moveX+moveZ*moveZ;
  if (a<=0.000001f) return 1.0f;
  const float b=offsetX*moveX+offsetZ*moveZ;
  const float c=offsetX*offsetX+offsetZ*offsetZ-radius*radius;
  if (c<0) return b>=0 ? 1.0f : 0.0f;
  if (b>=0) return 1.0f;
  const float discriminant=b*b-a*c;
  if (discriminant<=0) return 1.0f;
  const float entry=(-b-std::sqrt(discriminant))/a;
  if (entry>=1.0f) return 1.0f;
  return entry>0.0f ? entry : 0.0f;
}
}
