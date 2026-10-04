#include "compat/GameAbi.hpp"

template <>
gfl2::gfx::ctr::CTRGL*
gfl2::gfx::GFGLBase<gfl2::gfx::ctr::CTRGL>::s_Gp = NULL;

extern "C" void Follower3gx_SetGfglSingleton(void* implementation)
{
  gfl2::gfx::GFGLBase<gfl2::gfx::ctr::CTRGL>::s_Gp =
    static_cast<gfl2::gfx::ctr::CTRGL*>(implementation);
}

extern "C" f32 __hardfp_atan2f(f32 y, f32 x)
{
  return __builtin_atan2f(y, x);
}
