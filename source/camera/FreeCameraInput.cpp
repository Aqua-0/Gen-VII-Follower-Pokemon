#include "FreeCameraInput.hpp"

#include "FollowerFeatures.hpp"


#include <CTRPluginFramework/System/Controller.hpp>
#include <CTRPluginFramework/System/Process.hpp>

namespace Gen7Follower3gx
{

void ReadFreeCameraInput(
  FreeCameraInputState* state
  )
{
  if (!state)
  {
    return;
  }

  const u32 keys = CTRPluginFramework::Controller::GetKeysDown();
  const CTRPluginFramework::shortVector circlePad =
    CTRPluginFramework::Controller::GetCirclePadPosition();
  state->circleX = circlePad.x;
  state->circleY = circlePad.y;
  state->cStickX = 0;
  state->cStickY = 0;
  if (keys & (CTRPluginFramework::Key::CStickRight | CTRPluginFramework::Key::DPadRight))
  {
    ++state->cStickX;
  }
  if (keys & (CTRPluginFramework::Key::CStickLeft | CTRPluginFramework::Key::DPadLeft))
  {
    --state->cStickX;
  }
  if (keys & (CTRPluginFramework::Key::CStickUp | CTRPluginFramework::Key::DPadUp))
  {
    ++state->cStickY;
  }
  if (keys & (CTRPluginFramework::Key::CStickDown | CTRPluginFramework::Key::DPadDown))
  {
    --state->cStickY;
  }
  state->holdA = (keys & CTRPluginFramework::Key::A) != 0;
  state->moveUp =
    (keys & (CTRPluginFramework::Key::R | CTRPluginFramework::Key::ZR)) != 0;
  state->moveDown =
    (keys & (CTRPluginFramework::Key::L | CTRPluginFramework::Key::ZL)) != 0;
  state->reset = (keys & CTRPluginFramework::Key::Y) != 0;
  state->exit = (keys & CTRPluginFramework::Key::B) != 0;
  state->fast = (keys & CTRPluginFramework::Key::X) != 0;
}

bool IsFreeCameraObjectWritable(const void* object)
{
  if (!object)
  {
    return false;
  }
  const u32 address = reinterpret_cast<u32>(object);
  return CTRPluginFramework::Process::CheckAddress(address, MEMPERM_READ) &&
    CTRPluginFramework::Process::CheckAddress(address, MEMPERM_WRITE);
}

} // namespace Gen7Follower3gx
