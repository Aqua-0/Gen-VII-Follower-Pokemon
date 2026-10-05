#include "FollowMode.hpp"
#include <CTRPluginFramework/System/File.hpp>

namespace Gen7Follower3gx {
namespace {
const unsigned int Magic=0x464d4f44;
struct Record { unsigned int magic, version, generation, closeEnabled, check; };
const char* Paths[]={"FollowerFollowMode.a.cfg","FollowerFollowMode.b.cfg"};
unsigned int g_CloseEnabled=0U;
unsigned int g_Generation=0;
unsigned int Check(const Record& record)
{
  return record.magic ^ record.version ^ record.generation ^ record.closeEnabled;
}
bool Read(unsigned int slot, Record& record)
{
  CTRPluginFramework::File file;
  return CTRPluginFramework::File::Open(file,Paths[slot],CTRPluginFramework::File::READ)==0 &&
    file.GetSize()==sizeof(record) && file.Read(&record,sizeof(record))==0 &&
    record.magic==Magic && record.version==1 && record.check==Check(record) &&
    record.closeEnabled<=1U;
}
}
void InitializeFollowMode()
{
  Record first{}, second{};
  const bool hasFirst=Read(0,first), hasSecond=Read(1,second);
  const Record* latest=hasFirst ? &first : nullptr;
  if (hasSecond && (!latest || static_cast<int>(second.generation-latest->generation)>0))
    latest=&second;
  g_Generation=latest ? latest->generation : 0;
  __atomic_store_n(&g_CloseEnabled,latest ? latest->closeEnabled : 0U,__ATOMIC_RELAXED);
}
bool IsCloseFollowEnabled()
{
  return __atomic_load_n(&g_CloseEnabled,__ATOMIC_RELAXED);
}
bool SetCloseFollowEnabled(bool enabled)
{
  const unsigned int closeEnabled=enabled ? 1U : 0U;
  if (closeEnabled==IsCloseFollowEnabled()) return true;
  Record record{Magic,1,g_Generation+1,closeEnabled,0};
  record.check=Check(record);
  CTRPluginFramework::File file;
  if (CTRPluginFramework::File::Open(file,Paths[record.generation & 1],
      CTRPluginFramework::File::WRITE | CTRPluginFramework::File::CREATE |
      CTRPluginFramework::File::TRUNCATE | CTRPluginFramework::File::SYNC)!=0 ||
      file.Write(&record,sizeof(record))!=0 || file.Flush()!=0) return false;
  g_Generation=record.generation;
  __atomic_store_n(&g_CloseEnabled,closeEnabled,__ATOMIC_RELAXED);
  return true;
}
}
