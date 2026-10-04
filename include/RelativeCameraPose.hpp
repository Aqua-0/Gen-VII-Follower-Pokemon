#pragma once
#include <cmath>
namespace Gen7Follower3gx {
struct RelativeCameraPose {
  float right=0,up=0,forward=0,yaw=0,pitch=0,distanceScale=1;
  template<class Vector>
  void Capture(const Vector& nativePosition,float nativeYaw,float nativePitch,float nativeDistance,
      const Vector& position,float viewYaw,float viewPitch,float distance) {
    const float dx=position.x-nativePosition.x,dz=position.z-nativePosition.z;
    right=-std::cos(nativeYaw)*dx+std::sin(nativeYaw)*dz;
    up=position.y-nativePosition.y;
    forward=std::sin(nativeYaw)*dx+std::cos(nativeYaw)*dz;
    yaw=viewYaw-nativeYaw;
    pitch=viewPitch-nativePitch;
    distanceScale=distance/nativeDistance;
  }
  template<class Vector>
  Vector Position(const Vector& nativePosition,float nativeYaw) const {
    return Vector(nativePosition.x-std::cos(nativeYaw)*right+std::sin(nativeYaw)*forward,
      nativePosition.y+up,
      nativePosition.z+std::sin(nativeYaw)*right+std::cos(nativeYaw)*forward);
  }
};
}
