#include "RideProfile.hpp"
#include <CTRPluginFramework/System/File.hpp>
#include <cstdio>
#include <cstddef>
namespace Gen7Follower3gx {
namespace {
// Don't wait on the game thread here. The menu may have paused it while it was writing.
unsigned int scaleBits[RideSpeciesMax+1]={};
void CacheScale(unsigned int species,float scale) {
  unsigned int bits; std::memcpy(&bits,&scale,4);
  __atomic_store_n(&scaleBits[species],bits,__ATOMIC_RELAXED);
}
RideBoneCatalog catalogs[2]={};
unsigned int catalogSequence[2]={};
struct Record { unsigned int magic, version, generation; RideProfile profile; unsigned int checksum; };
static_assert(sizeof(Record)==sizeof(RideProfile)+16,"Ride record layout");
unsigned int Checksum(const Record& record)
{
  unsigned int hash=2166136261U;
  const auto* bytes=reinterpret_cast<const unsigned char*>(&record);
  for (unsigned int i=0; i<sizeof(Record)-4; ++i) hash=(hash^bytes[i])*16777619U;
  return hash;
}
void Path(char* path, unsigned int species, unsigned int slot)
{
  std::snprintf(path,64,"FollowerRideSpecies_%03u.%c.cfg",species,slot ? 'b' : 'a');
}
bool Read(unsigned int species, unsigned int slot, Record& record)
{
  char path[64]; Path(path,species,slot);
  CTRPluginFramework::File file;
  if (CTRPluginFramework::File::Open(file,path,CTRPluginFramework::File::READ)!=0) return false;
  constexpr unsigned int legacySize=offsetof(RideProfile,mode)+16;
  constexpr unsigned int version2Size=offsetof(RideProfile,pokemonScale)+16;
  const unsigned int size=file.GetSize();
  if (size!=sizeof(record) && size!=legacySize && size!=version2Size) return false;
  unsigned char bytes[sizeof(Record)]={};
  if (file.Read(bytes,size)!=0) return false;
  unsigned int header[3], checksum;
  std::memcpy(header,bytes,sizeof(header));
  std::memcpy(&checksum,bytes+size-4,4);
  unsigned int hash=2166136261U;
  for (unsigned int i=0;i<size-4;++i) hash=(hash^bytes[i])*16777619U;
  if (header[0]!=0x52505246U || !header[2] || checksum!=hash) return false;
  if ((size==legacySize && header[1]==1) || (size==version2Size && header[1]==2)) {
    record.profile=DefaultRideProfile(species,GetRiderAnimationStyle());
    std::memcpy(&record.profile,bytes+12,size-16);
    record.magic=header[0]; record.version=3; record.generation=header[2];
  } else if (size==sizeof(record) && header[1]==3) {
    std::memcpy(&record,bytes,size);
  } else return false;
  return record.profile.species==species && ValidRideProfile(record.profile);
}
int Latest(bool a, bool b, const Record (&records)[2])
{
  return !a ? (b ? 1 : -1) : (!b || records[0].generation>=records[1].generation ? 0 : 1);
}
}
RideProfile GetRideProfile(unsigned int species)
{
  Record records[2]={};
  const bool a=Read(species,0,records[0]), b=Read(species,1,records[1]);
  const int latest=Latest(a,b,records);
  return latest<0 ? DefaultRideProfile(species,GetRiderAnimationStyle()) : records[latest].profile;
}
float GetFollowerSpeciesScale(unsigned int species)
{
  if (!IsRideableFollowerSpecies(species)) return 1.0f;
  unsigned int bits=__atomic_load_n(&scaleBits[species],__ATOMIC_RELAXED);
  if (!bits) { const float scale=GetRideProfile(species).pokemonScale; CacheScale(species,scale); return scale; }
  float scale; std::memcpy(&scale,&bits,4); return scale;
}
bool SaveRideProfile(const RideProfile& profile)
{
  if (!ValidRideProfile(profile)) return false;
  Record records[2]={};
  const bool a=Read(profile.species,0,records[0]), b=Read(profile.species,1,records[1]);
  const int latest=Latest(a,b,records);
  // Write to the older slot so an interrupted save leaves the last good copy intact.
  Record record={}; record.magic=0x52505246U; record.version=3;
  record.generation=latest<0 ? 1 : records[latest].generation+1;
  if (!record.generation) return false;
  record.profile=profile; record.checksum=Checksum(record);
  char path[64]; Path(path,profile.species,latest<0 ? 0 : (latest^1));
  CTRPluginFramework::File file;
  const bool saved=CTRPluginFramework::File::Open(file,path,CTRPluginFramework::File::WRITE |
    CTRPluginFramework::File::CREATE | CTRPluginFramework::File::TRUNCATE |
    CTRPluginFramework::File::SYNC)==0 && file.Write(&record,sizeof(record))==0 && file.Flush()==0;
  if (saved) CacheScale(profile.species,profile.pokemonScale);
  return saved;
}
void PublishRideBoneCatalog(bool rider, const RideBoneCatalog& catalog)
{
  const unsigned int slot=rider ? 1 : 0;
  __atomic_add_fetch(&catalogSequence[slot],1U,__ATOMIC_SEQ_CST);
  auto* destination=reinterpret_cast<unsigned char*>(&catalogs[slot]);
  const auto* source=reinterpret_cast<const unsigned char*>(&catalog);
  for (unsigned int i=0; i<sizeof(catalog); ++i)
    __atomic_store_n(destination+i,source[i],__ATOMIC_SEQ_CST);
  __atomic_add_fetch(&catalogSequence[slot],1U,__ATOMIC_SEQ_CST);
}
void GetRideBoneCatalog(bool rider, RideBoneCatalog& catalog)
{
  const unsigned int slot=rider ? 1 : 0;
  const unsigned int before=__atomic_load_n(&catalogSequence[slot],__ATOMIC_SEQ_CST);
  catalog=RideBoneCatalog{};
  if (before&1U) return; // The writer is paused. Try again after closing the menu.
  auto* destination=reinterpret_cast<unsigned char*>(&catalog);
  const auto* source=reinterpret_cast<const unsigned char*>(&catalogs[slot]);
  for (unsigned int i=0; i<sizeof(catalog); ++i)
    destination[i]=__atomic_load_n(source+i,__ATOMIC_SEQ_CST);
  if (before!=__atomic_load_n(&catalogSequence[slot],__ATOMIC_SEQ_CST))
    catalog=RideBoneCatalog{};
}
}
