#pragma once

namespace Gen7Follower3gx
{

struct FreeCameraInputState
{
  signed short circleX;
  signed short circleY;
  int cStickX;
  int cStickY;
  bool holdA;
  bool moveUp;
  bool moveDown;
  bool fast;
  bool reset;
  bool exit;
};

void ReadFreeCameraInput(
  FreeCameraInputState* state
  );
bool IsFreeCameraObjectWritable(const void* object);

} // namespace Gen7Follower3gx
