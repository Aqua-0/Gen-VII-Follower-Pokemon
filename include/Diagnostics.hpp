#pragma once

#include "types.h"

#ifndef FOLLOWER_3GX_DIAGNOSTIC
#define FOLLOWER_3GX_DIAGNOSTIC 0
#endif

namespace Gen7Follower3gx
{

#if FOLLOWER_3GX_DIAGNOSTIC
void DiagnosticTrace(const char* message);
void DiagnosticTraceValue(const char* label, u32 value);

#define FOLLOWER_3GX_TRACE(message) \
  ::Gen7Follower3gx::DiagnosticTrace(message)
#define FOLLOWER_3GX_TRACE_VALUE(label, value) \
  ::Gen7Follower3gx::DiagnosticTraceValue(label, value)
#else
#define FOLLOWER_3GX_TRACE(message) ((void)0)
#define FOLLOWER_3GX_TRACE_VALUE(label, value) ((void)0)
#endif

} // namespace Gen7Follower3gx
