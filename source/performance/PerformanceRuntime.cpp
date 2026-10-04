#include "PerformanceDiagnostics.hpp"

#if !FOLLOWER_3GX_DIAGNOSTIC

namespace Gen7Follower3gx
{
namespace
{

PerformanceOptionMask g_TransientPerformanceOptions = 0;

} // namespace

PerformanceOptionMask GetTransientPerformanceOptions()
{
  return __atomic_load_n(
    &g_TransientPerformanceOptions,
    __ATOMIC_RELAXED
    );
}

void ClearTransientPerformanceOptions()
{
  __atomic_store_n(
    &g_TransientPerformanceOptions,
    0U,
    __ATOMIC_RELAXED
    );
}

void SetTransientPerformanceOptionEnabled(
  PerformanceOption option,
  bool enabled
  )
{
  const PerformanceOptionMask bit = GetPerformanceOptionBit(option);
  if (bit == 0)
  {
    return;
  }

  if (enabled)
  {
    __atomic_fetch_or(
      &g_TransientPerformanceOptions,
      bit,
      __ATOMIC_RELAXED
      );
  }
  else
  {
    __atomic_fetch_and(
      &g_TransientPerformanceOptions,
      ~bit,
      __ATOMIC_RELAXED
      );
  }
}

} // namespace Gen7Follower3gx

#endif
