#include "RideEventSettings.hpp"
#include <CTRPluginFramework/System/File.hpp>
namespace Gen7Follower3gx {
namespace {
unsigned int rideSpecies=0,ridePersonality=0;
bool ridePending=false;
unsigned int settings=0; // Low two bits: interaction mode; bit 2: keep follower; bit 3: preserve area ride.
struct Record { unsigned int magic, version, flags, checksum; };
const unsigned int Magic=0x52455654U;
const char Path[]="FollowerRideEvents.cfg";
}
void InitializeRideEventSettings()
{
  CTRPluginFramework::File file;
  Record record={};
  if (CTRPluginFramework::File::Open(file,Path,CTRPluginFramework::File::READ)==0 &&
      file.GetSize()==sizeof(record) && file.Read(&record,sizeof(record))==0 &&
      record.magic==Magic && record.version==1 && record.flags<=14 &&
      (record.flags&3)<MOUNTED_INTERACTION_COUNT &&
      record.checksum==(record.magic^record.version^record.flags))
    __atomic_store_n(&settings,record.flags,__ATOMIC_RELAXED);
}
MountedInteractionMode GetMountedInteractionMode()
{ return static_cast<MountedInteractionMode>(__atomic_load_n(&settings,__ATOMIC_RELAXED)&3); }
bool KeepFollowerVisibleDuringEvents()
{ return (__atomic_load_n(&settings,__ATOMIC_RELAXED)&4)!=0; }
static bool SaveSettings(MountedInteractionMode mode, bool keepFollower, bool preserve)
{
  if (mode<MOUNTED_INTERACTION_BLOCK || mode>=MOUNTED_INTERACTION_COUNT) return false;
  Record record={Magic,1,static_cast<unsigned int>(mode)|(keepFollower ? 4U : 0U)|(preserve ? 8U : 0U),0};
  record.checksum=record.magic^record.version^record.flags;
  CTRPluginFramework::File file;
  if (CTRPluginFramework::File::Open(file,Path,CTRPluginFramework::File::WRITE |
        CTRPluginFramework::File::CREATE | CTRPluginFramework::File::TRUNCATE |
        CTRPluginFramework::File::SYNC)!=0 ||
      file.Write(&record,sizeof(record))!=0 || file.Flush()!=0) return false;
  __atomic_store_n(&settings,record.flags,__ATOMIC_RELAXED);
  return true;
}

bool PreserveRideBetweenAreas()
{ return (__atomic_load_n(&settings,__ATOMIC_RELAXED)&8)!=0; }
bool SetRideEventSettings(MountedInteractionMode mode,bool keepFollower)
{ return SaveSettings(mode,keepFollower,PreserveRideBetweenAreas()); }
bool SetPreserveRideBetweenAreas(bool enabled)
{ return SaveSettings(GetMountedInteractionMode(),KeepFollowerVisibleDuringEvents(),enabled); }
void RememberAreaRide(unsigned int species,unsigned int personality)
{ rideSpecies=species; ridePersonality=personality; ridePending=false; }
void ClearAreaRide()
{ rideSpecies=0; ridePending=false; }
void MarkRideAreaTransition()
{
  if (PreserveRideBetweenAreas() && rideSpecies) ridePending=true;
  else ClearAreaRide();
}
bool HasPendingAreaRide() { return ridePending; }
bool ConsumeAreaRide(unsigned int species,unsigned int personality)
{
  if (!ridePending) return false;
  const bool match=PreserveRideBetweenAreas() && species==rideSpecies && personality==ridePersonality;
  ClearAreaRide();
  return match;
}
}
