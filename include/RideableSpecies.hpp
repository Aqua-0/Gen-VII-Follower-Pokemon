#pragma once

namespace Gen7Follower3gx {
const unsigned int RideSpeciesMax=0xffffU;

inline bool IsRideableFollowerSpecies(unsigned int species)
{
  return species >= 1U && species <= RideSpeciesMax;
}
}
