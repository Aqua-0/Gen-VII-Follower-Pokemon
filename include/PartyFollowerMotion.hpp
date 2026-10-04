#pragma once
#include "FollowerNetworkState.hpp"
#include <cmath>
namespace Gen7Follower3gx {
// Keep the animation playing through short stops so it doesn't keep restarting.
struct PartyFollowerMotion {
  bool following=false, running=false, hasLeader=false;
  float leaderX=0, leaderZ=0, leaderSpeed=0;
  unsigned int stoppedFrames=0;
  unsigned char motion=FOLLOWER_NETWORK_LOCOMOTION_IDLE;
  void Reset() { *this=PartyFollowerMotion(); }
  float Plan(float x,float z,float gap,float walkSpeed,float runSpeed) {
    if (hasLeader) {
      const float dx=x-leaderX,dz=z-leaderZ;
      leaderSpeed+=0.25f*(std::sqrt(dx*dx+dz*dz)-leaderSpeed);
    }
    leaderX=x; leaderZ=z; hasLeader=true;
    if (!following && gap>12.0f) following=true;
    if (!following) return 0;
    if (!running && (gap>100.0f || leaderSpeed>walkSpeed*1.2f)) running=true;
    else if (running && gap<50.0f && leaderSpeed<walkSpeed*0.85f) running=false;
    if (gap<=0.05f) return 0;
    // Slow down as the follower closes the gap, and don't exceed the set speed.
    const float cap=running ? runSpeed : walkSpeed;
    const float step=gap*0.2f;
    return step<cap ? step : cap;
  }
  void Commit(bool moved) {
    if (moved) {
      stoppedFrames=0;
      motion=running ? FOLLOWER_NETWORK_LOCOMOTION_RUN : FOLLOWER_NETWORK_LOCOMOTION_WALK;
    } else if (++stoppedFrames>=8) {
      stoppedFrames=8;
      following=false; running=false;
      motion=FOLLOWER_NETWORK_LOCOMOTION_IDLE;
    }
  }
};
}
