#include "RiderAnimationSettings.hpp"
#include <CTRPluginFramework/System/File.hpp>
namespace Gen7Follower3gx {
namespace {
unsigned int g_Style = RIDER_TAUROS;
const unsigned int Magic = 0x52494445;
struct Record { unsigned int magic, version, style, check; };
const char Path[] = "FollowerRide.cfg";
}
void InitializeRiderAnimationSettings()
{
  CTRPluginFramework::File file;
  Record record = {};
  if (CTRPluginFramework::File::Open(file, Path, CTRPluginFramework::File::READ) == 0 &&
      file.GetSize() == sizeof(record) && file.Read(&record, sizeof(record)) == 0 &&
      record.magic == Magic && record.version == 1 && record.style < RIDER_STYLE_COUNT &&
      record.check == (record.magic ^ record.version ^ record.style))
  {
    __atomic_store_n(&g_Style, record.style, __ATOMIC_RELAXED);
  }
}
RiderAnimationStyle GetRiderAnimationStyle()
{
  return static_cast<RiderAnimationStyle>(__atomic_load_n(&g_Style, __ATOMIC_RELAXED));
}
bool SetRiderAnimationStyle(RiderAnimationStyle style)
{
  if (style < RIDER_TAUROS || style >= RIDER_STYLE_COUNT) { return false; }
  Record record = {Magic, 1, static_cast<unsigned int>(style), 0};
  record.check = record.magic ^ record.version ^ record.style;
  CTRPluginFramework::File file;
  if (CTRPluginFramework::File::Open(file, Path, CTRPluginFramework::File::WRITE |
        CTRPluginFramework::File::CREATE | CTRPluginFramework::File::TRUNCATE |
        CTRPluginFramework::File::SYNC) != 0 ||
      file.Write(&record, sizeof(record)) != 0) { return false; }
  __atomic_store_n(&g_Style, static_cast<unsigned int>(style), __ATOMIC_RELAXED);
  return true;
}
const char* GetRiderAnimationStyleName(RiderAnimationStyle style)
{
  switch (style) {
  case RIDER_TAUROS: return "Tauros";
  case RIDER_MUDSDALE: return "Mudsdale";
  case RIDER_SOLGALEO: return "Solgaleo";
  case RIDER_SHARPEDO: return "Sharpedo";
  case RIDER_STOUTLAND: return "Stoutland";
  case RIDER_MACHAMP: return "Machamp";
  case RIDER_LAPRAS: return "Lapras";
  case RIDER_LUNALA: return "Lunala";
  default: return "Unknown";
  }
}
}
