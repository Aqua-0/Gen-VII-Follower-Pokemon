#pragma once
namespace Gen7Follower3gx {
enum RiderAnimationStyle { RIDER_TAUROS, RIDER_MUDSDALE, RIDER_SOLGALEO, RIDER_SHARPEDO, RIDER_STOUTLAND, RIDER_MACHAMP, RIDER_LAPRAS, RIDER_LUNALA, RIDER_STYLE_COUNT };
inline bool IsExternalRiderStyle(RiderAnimationStyle style)
{ return style==RIDER_SOLGALEO || style==RIDER_LUNALA; }
inline unsigned int GetRiderIdleMotion(RiderAnimationStyle style)
{
  switch (style) {
  case RIDER_TAUROS: return 150;
  case RIDER_SHARPEDO: return 170;
  case RIDER_STOUTLAND: return 190;
  case RIDER_MUDSDALE: return 210;
  case RIDER_MACHAMP: return 230;
  case RIDER_LAPRAS: return 250;
  default: return 0xffffffffU;
  }
}
void InitializeRiderAnimationSettings();
RiderAnimationStyle GetRiderAnimationStyle();
bool SetRiderAnimationStyle(RiderAnimationStyle style);
const char* GetRiderAnimationStyleName(RiderAnimationStyle style);
}
