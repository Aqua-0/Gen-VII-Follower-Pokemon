#pragma once
#include "RiderAnimationSettings.hpp"
#include "RideableSpecies.hpp"
#include <cmath>
#include <cstring>

namespace Gen7Follower3gx {
const unsigned int RideBoneNameSize=64;
const unsigned int RideBoneEditCount=8;
const unsigned int RideCatalogCapacity=128;
struct RiderBoneEdit {
  char name[RideBoneNameSize];
  float translation[3];
  float rotation[3]; // X/Y/Z angles in degrees, relative to the parent bone. Applied after the animation.
};
enum RideMode { RIDE_PLAYER_ON_POKEMON, RIDE_POKEMON_ON_PLAYER, RIDE_MODE_COUNT };
struct RideProfile {
  unsigned int species, enabled, style, stabilize;
  float seat[3]; // Right, up and forward relative to the direction the mount is facing.
  float yaw, waistMotion, thighInward;
  char attachment[RideBoneNameSize];
  RiderBoneEdit bones[RideBoneEditCount];
  unsigned int mode;
  float movementSpeed;
  float carryOffset[3], carryRotation[3];
  char carryAttachment[RideBoneNameSize];
  float pokemonScale;
};
inline RideProfile DefaultRideProfile(unsigned int species, RiderAnimationStyle style)
{
  RideProfile profile={};
  profile.species=species;
  profile.enabled=1;
  profile.style=style;
  profile.stabilize=1; // Tauros only.
  profile.seat[1]=25;
  profile.waistMotion=15;
  profile.thighInward=12;
  profile.movementSpeed=1.0f;
  profile.pokemonScale=1.0f;
  profile.carryOffset[0]=18;
  profile.carryOffset[1]=75;
  profile.carryOffset[2]=-8;
  return profile;
}
inline bool RideValueInRange(float value, float minimum, float maximum)
{
  return std::isfinite(value) && value>=minimum && value<=maximum;
}
inline bool ValidRideProfile(const RideProfile& profile)
{
  if (!IsRideableFollowerSpecies(profile.species) || profile.enabled>1 ||
      profile.style>=RIDER_STYLE_COUNT || profile.stabilize>1 ||
      !RideValueInRange(profile.pokemonScale,0.25f,3.0f) ||
      profile.mode>=RIDE_MODE_COUNT || !RideValueInRange(profile.movementSpeed,0.25f,2.0f) ||
      !std::memchr(profile.carryAttachment,0,RideBoneNameSize) ||
      !RideValueInRange(profile.yaw,-180,180) ||
      !RideValueInRange(profile.waistMotion,0,100) ||
      !RideValueInRange(profile.thighInward,-45,45) ||
      !std::memchr(profile.attachment,0,RideBoneNameSize)) return false;
  for (int axis=0; axis<3; ++axis)
    if (!RideValueInRange(profile.seat[axis],-500,500) ||
        !RideValueInRange(profile.carryOffset[axis],-500,500) ||
        !RideValueInRange(profile.carryRotation[axis],-180,180)) return false;
  for (const auto& bone : profile.bones) {
    if (!std::memchr(bone.name,0,RideBoneNameSize)) return false;
    for (int axis=0; axis<3; ++axis)
      if (!RideValueInRange(bone.translation[axis],-100,100) ||
          !RideValueInRange(bone.rotation[axis],-180,180)) return false;
  }
  return true;
}
RideProfile GetRideProfile(unsigned int species);
float GetFollowerSpeciesScale(unsigned int species);
bool SaveRideProfile(const RideProfile& profile);
struct RideBoneCatalog {
  unsigned int species, count;
  char names[RideCatalogCapacity][RideBoneNameSize];
};
void PublishRideBoneCatalog(bool rider, const RideBoneCatalog& catalog);
void GetRideBoneCatalog(bool rider, RideBoneCatalog& catalog);
template<class Node> void PublishRideModelBones(bool rider, unsigned int species, Node* node)
{
  // This catalog is too large to put on the game update thread's stack.
  RideBoneCatalog* catalog=new RideBoneCatalog();
  if (!catalog) return;
  catalog->species=species;
  if (node && node->GetJointNum()<=256) {
    catalog->count=node->GetJointNum()<RideCatalogCapacity ? node->GetJointNum() : RideCatalogCapacity;
    for (unsigned int i=0; i<catalog->count; ++i) {
      const char* name=node->GetJointInstanceNode(i)->GetName();
      if (name) std::strncpy(catalog->names[i],name,RideBoneNameSize-1);
    }
  }
  PublishRideBoneCatalog(rider,*catalog);
  delete catalog;
}
}
