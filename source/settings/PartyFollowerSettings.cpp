#include "PartyFollowerSettings.hpp"
#include <CTRPluginFramework/System/File.hpp>
namespace Gen7Follower3gx {
namespace {
unsigned int packed=1;
struct Record { unsigned int magic, version, flags, checksum; };
const unsigned int Magic=0x50465253U;
const char Path[]="FollowerParty.cfg";
PartyFollowerSettings Decode(unsigned int flags) {
  PartyFollowerSettings settings={flags&3U,{}};
  for (unsigned int i=0;i<PartyFollowerMaximum;++i) settings.slots[i]=(flags>>(2+3*i))&7U;
  return settings;
}
}
void InitializePartyFollowerSettings() {
  Record record={}; CTRPluginFramework::File file;
  if (CTRPluginFramework::File::Open(file,Path,CTRPluginFramework::File::READ)==0 &&
      file.GetSize()==sizeof(record) && file.Read(&record,sizeof(record))==0 &&
      record.magic==Magic && record.version==1 && record.flags<2048 &&
      record.checksum==(record.magic^record.version^record.flags) &&
      ValidPartyFollowerSettings(Decode(record.flags)))
    __atomic_store_n(&packed,record.flags,__ATOMIC_RELAXED);
}
PartyFollowerSettings GetPartyFollowerSettings() { return Decode(__atomic_load_n(&packed,__ATOMIC_RELAXED)); }
bool SavePartyFollowerSettings(const PartyFollowerSettings& settings) {
  if (!ValidPartyFollowerSettings(settings)) return false;
  unsigned int flags=settings.count;
  for (unsigned int i=0;i<PartyFollowerMaximum;++i) flags|=settings.slots[i]<<(2+3*i);
  Record record={Magic,1,flags,Magic^1U^flags}; CTRPluginFramework::File file;
  if (CTRPluginFramework::File::Open(file,Path,CTRPluginFramework::File::WRITE |
      CTRPluginFramework::File::CREATE | CTRPluginFramework::File::TRUNCATE |
      CTRPluginFramework::File::SYNC)!=0 || file.Write(&record,sizeof(record))!=0 || file.Flush()!=0) return false;
  __atomic_store_n(&packed,flags,__ATOMIC_RELAXED); return true;
}
}
