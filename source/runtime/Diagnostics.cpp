#include "Diagnostics.hpp"

#if FOLLOWER_3GX_DIAGNOSTIC

#include <3ds.h>

namespace Gen7Follower3gx
{
namespace
{

const char Prefix[] = "[Follower3GX] ";

u32 Append(char* output, u32 capacity, u32 position, const char* text)
{
  while (*text && position < capacity)
  {
    output[position++] = *text++;
  }
  return position;
}

void Emit(const char* text, u32 size)
{
  svcOutputDebugString(text, static_cast<s32>(size));
}

} // namespace

void DiagnosticTrace(const char* message)
{
  char output[128];
  u32 position = Append(output, sizeof(output), 0, Prefix);
  position = Append(output, sizeof(output), position, message);
  if (position < sizeof(output))
  {
    output[position++] = '\n';
  }
  Emit(output, position);
}

void DiagnosticTraceValue(const char* label, u32 value)
{
  static const char Hex[] = "0123456789abcdef";
  char output[128];
  u32 position = Append(output, sizeof(output), 0, Prefix);
  position = Append(output, sizeof(output), position, label);
  position = Append(output, sizeof(output), position, "=0x");
  for (s32 shift = 28; shift >= 0 && position < sizeof(output); shift -= 4)
  {
    output[position++] = Hex[(value >> shift) & 0xf];
  }
  if (position < sizeof(output))
  {
    output[position++] = '\n';
  }
  Emit(output, position);
}

} // namespace Gen7Follower3gx

#endif
