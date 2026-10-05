#include "MenuHotkeySettings.hpp"
#include <CTRPluginFramework/System/File.hpp>

namespace Gen7Follower3gx {
namespace {
const unsigned int Magic=0x4d484b59;
struct Record { unsigned int magic, version, generation, keys, check; };
const char* Paths[]={"FollowerMenuHotkey.a.cfg","FollowerMenuHotkey.b.cfg"};
unsigned int g_Keys=DefaultMenuHotkey;
unsigned int g_Generation=0;
unsigned int Check(const Record& record)
{
  return record.magic ^ record.version ^ record.generation ^ record.keys;
}
bool Read(unsigned int slot, Record& record)
{
  CTRPluginFramework::File file;
  return CTRPluginFramework::File::Open(file,Paths[slot],CTRPluginFramework::File::READ)==0 &&
    file.GetSize()==sizeof(record) && file.Read(&record,sizeof(record))==0 &&
    record.magic==Magic && record.version==1 && record.check==Check(record) &&
    IsValidMenuHotkey(record.keys);
}
}
void InitializeMenuHotkeySettings()
{
  Record first{}, second{};
  const bool hasFirst=Read(0,first), hasSecond=Read(1,second);
  const Record* latest=hasFirst ? &first : nullptr;
  if (hasSecond && (!latest || static_cast<int>(second.generation-latest->generation)>0))
    latest=&second;
  g_Generation=latest ? latest->generation : 0;
  __atomic_store_n(&g_Keys,latest ? latest->keys : DefaultMenuHotkey,__ATOMIC_RELAXED);
}
unsigned int GetMenuHotkey()
{
  return __atomic_load_n(&g_Keys,__ATOMIC_RELAXED);
}
bool SetMenuHotkey(unsigned int keys)
{
  if (!IsValidMenuHotkey(keys)) return false;
  if (keys==GetMenuHotkey()) return true;
  Record record{Magic,1,g_Generation+1,keys,0};
  record.check=Check(record);
  CTRPluginFramework::File file;
  if (CTRPluginFramework::File::Open(file,Paths[record.generation & 1],
      CTRPluginFramework::File::WRITE | CTRPluginFramework::File::CREATE |
      CTRPluginFramework::File::TRUNCATE | CTRPluginFramework::File::SYNC)!=0 ||
      file.Write(&record,sizeof(record))!=0 || file.Flush()!=0) return false;
  g_Generation=record.generation;
  __atomic_store_n(&g_Keys,keys,__ATOMIC_RELAXED);
  return true;
}
}
