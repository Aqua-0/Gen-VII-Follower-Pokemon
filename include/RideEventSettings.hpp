#pragma once
namespace Gen7Follower3gx {
enum MountedInteractionMode {
  MOUNTED_INTERACTION_BLOCK,
  MOUNTED_INTERACTION_DISMOUNT,
  MOUNTED_INTERACTION_KEEP,
  MOUNTED_INTERACTION_COUNT
};
enum EventCleanupReason {
  EVENT_CLEANUP_RIDE_POLICY=1U<<0,
  EVENT_CLEANUP_RIDER_RETIRING=1U<<1,
  EVENT_CLEANUP_INTERACTION=1U<<2,
  EVENT_CLEANUP_MOTION_PACK=1U<<3,
  EVENT_CLEANUP_FORM_CHANGE=1U<<4,
  EVENT_CLEANUP_EFFECT=1U<<5,
  EVENT_CLEANUP_EVENT_HEAP=1U<<6,
  EVENT_CLEANUP_REGULAR_POLICY=1U<<7
};
void InitializeRideEventSettings();
MountedInteractionMode GetMountedInteractionMode();
bool KeepFollowerVisibleDuringEvents();
bool SetRideEventSettings(MountedInteractionMode mode, bool keepFollower);
bool PreserveRideBetweenAreas();
bool SetPreserveRideBetweenAreas(bool enabled);
void RememberAreaRide(unsigned int species,unsigned int personality);
void ClearAreaRide();
void MarkRideAreaTransition();
bool ConsumeAreaRide(unsigned int species,unsigned int personality);
bool HasPendingAreaRide();
inline bool ConsumeMountedInteraction(bool cleanup, bool rideChord, MountedInteractionMode mode)
{ return cleanup || rideChord || mode==MOUNTED_INTERACTION_BLOCK; }
inline bool RideRequiresEventRelease(bool active, bool cleanup, MountedInteractionMode mode)
{ return cleanup || (active && mode!=MOUNTED_INTERACTION_KEEP); }
}
