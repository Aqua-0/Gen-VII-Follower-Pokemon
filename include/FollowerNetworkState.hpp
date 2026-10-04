#pragma once

namespace Gen7Follower3gx
{

enum FollowerNetworkLocomotion
{
  FOLLOWER_NETWORK_LOCOMOTION_INVALID = 0,
  FOLLOWER_NETWORK_LOCOMOTION_IDLE = 1,
  FOLLOWER_NETWORK_LOCOMOTION_WALK = 2,
  FOLLOWER_NETWORK_LOCOMOTION_RUN = 3,
};

enum FollowerNetworkFlags
{
  FOLLOWER_NETWORK_FLAG_VALID = 1U << 0,
  FOLLOWER_NETWORK_FLAG_VISIBLE = 1U << 1,
};

// This only stores how the replica looks and moves. It owns no resources or save pointers.
struct FollowerNetworkState
{
  unsigned short species;
  unsigned short form;
  unsigned char sex;
  unsigned char isRare;
  unsigned char locomotion;
  unsigned char flags;
  unsigned int personality;
  float x;
  float y;
  float z;
  float yaw;
};

enum FollowerReplicaLifecycleState
{
  FOLLOWER_REPLICA_EMPTY = 0,
  FOLLOWER_REPLICA_REQUESTED = 1,
  FOLLOWER_REPLICA_LOADING = 2,
  FOLLOWER_REPLICA_ACTIVE = 3,
  FOLLOWER_REPLICA_RELEASING = 4,
};

struct FollowerReplicaDiagnostics
{
  unsigned int capacity;
  unsigned int requestedMask;
  unsigned int loadingMask;
  unsigned int activeMask;
  unsigned int visibleMask;
  unsigned int releasingMask;
  unsigned short species[3];
  unsigned short form[3];
  unsigned char locomotion[3];
  unsigned char lifecycle[3];
};

static_assert(
  sizeof(FollowerNetworkState) == 28,
  "Follower network state ABI changed"
  );

} // namespace Gen7Follower3gx
