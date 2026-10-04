#include "CroModule.hpp"

#include "Diagnostics.hpp"
#include "types.h"
#include <CTRPluginFramework/System/Process.hpp>

namespace Gen7Follower3gx
{
namespace
{

enum
{
  CroHeaderMagicOffset = 0x80,
  CroHeaderModuleNameOffset = 0xc0,
  CroHeaderModuleNameSizeOffset = 0xc4,
  CroHeaderSegmentTableOffset = 0xc8,
  CroHeaderSegmentCountOffset = 0xcc,
  CroSegmentSize = 12,
  CroSegmentCode = 0,
  CroMagicCro0 = 0x304f5243,
  CroMagicFixed = 0x44584946,
};

u32 ReadU32(u32 address)
{
  return *reinterpret_cast<const volatile u32*>(address);
}

bool IsReadable(u32 address, u32 size)
{
  if (size == 0 || address + size < address)
  {
    return false;
  }
  return CTRPluginFramework::Process::CheckAddress(address, MEMPERM_READ) &&
    CTRPluginFramework::Process::CheckAddress(
      address + size - 1,
      MEMPERM_READ
      );
}

bool NameEquals(u32 address, u32 size, const char* expected)
{
  if (!expected)
  {
    return false;
  }

  u32 expectedSize = 1;
  while (expected[expectedSize - 1] != '\0')
  {
    if (expectedSize >= 64)
    {
      return false;
    }
    ++expectedSize;
  }
  if (size != expectedSize || !IsReadable(address, size))
  {
    return false;
  }

  const char* name = reinterpret_cast<const char*>(address);
  for (u32 index = 0; index < expectedSize; ++index)
  {
    if (name[index] != expected[index])
    {
      return false;
    }
  }
  return true;
}

} // namespace

bool CroModuleNameEquals(void* module, const char* expectedName)
{
  const u32 base = reinterpret_cast<u32>(module);
  if (!module || !IsReadable(base + CroHeaderMagicOffset, 0x50))
  {
    return false;
  }

  const u32 magic = ReadU32(base + CroHeaderMagicOffset);
  if (magic != CroMagicCro0 && magic != CroMagicFixed)
  {
    return false;
  }

  return NameEquals(
    ReadU32(base + CroHeaderModuleNameOffset),
    ReadU32(base + CroHeaderModuleNameSizeOffset),
    expectedName
    );
}

CroModuleView::CroModuleView()
: m_Module(NULL)
, m_TextBase(0)
, m_TextSize(0)
{
}

bool CroModuleView::Initialize(void* module)
{
  return Initialize(module, "FieldRo");
}

bool CroModuleView::Initialize(void* module, const char* expectedName)
{
  Reset();
  const u32 base = reinterpret_cast<u32>(module);
  if (!module || !IsReadable(base + CroHeaderMagicOffset, 0x50))
  {
    FOLLOWER_3GX_TRACE("CRO header unreadable");
    return false;
  }

  const u32 magic = ReadU32(base + CroHeaderMagicOffset);
  if (magic != CroMagicCro0 && magic != CroMagicFixed)
  {
    FOLLOWER_3GX_TRACE_VALUE("CRO bad magic", magic);
    return false;
  }

  const u32 nameAddress = ReadU32(base + CroHeaderModuleNameOffset);
  const u32 nameSize = ReadU32(base + CroHeaderModuleNameSizeOffset);
  if (!NameEquals(nameAddress, nameSize, expectedName))
  {
    FOLLOWER_3GX_TRACE_VALUE("CRO name size", nameSize);
    if (IsReadable(nameAddress, 8))
    {
      FOLLOWER_3GX_TRACE_VALUE("CRO name word0", ReadU32(nameAddress));
      FOLLOWER_3GX_TRACE_VALUE("CRO name word1", ReadU32(nameAddress + 4));
    }
    return false;
  }

  const u32 segmentTable = ReadU32(base + CroHeaderSegmentTableOffset);
  const u32 segmentCount = ReadU32(base + CroHeaderSegmentCountOffset);
  if (segmentCount == 0 || segmentCount > 16 ||
      !IsReadable(segmentTable, segmentCount * CroSegmentSize))
  {
    FOLLOWER_3GX_TRACE_VALUE("CRO bad segment count", segmentCount);
    FOLLOWER_3GX_TRACE_VALUE("CRO segment table", segmentTable);
    return false;
  }

  for (u32 index = 0; index < segmentCount; ++index)
  {
    const u32 entry = segmentTable + index * CroSegmentSize;
    const u32 segmentAddress = ReadU32(entry);
    const u32 segmentSize = ReadU32(entry + 4);
    const u32 segmentType = ReadU32(entry + 8);
    if (segmentType != CroSegmentCode)
    {
      continue;
    }
    if (segmentSize == 0 || !IsReadable(segmentAddress, segmentSize))
    {
      FOLLOWER_3GX_TRACE_VALUE("CRO text unreadable", segmentAddress);
      FOLLOWER_3GX_TRACE_VALUE("CRO text size", segmentSize);
      return false;
    }

    m_Module = module;
    m_TextBase = segmentAddress;
    m_TextSize = segmentSize;
    FOLLOWER_3GX_TRACE_VALUE("FieldRo text", m_TextBase);
    FOLLOWER_3GX_TRACE_VALUE("FieldRo text size", m_TextSize);
    return true;
  }

  return false;
}

void CroModuleView::Reset()
{
  m_Module = NULL;
  m_TextBase = 0;
  m_TextSize = 0;
}

} // namespace Gen7Follower3gx
