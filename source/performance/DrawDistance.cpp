#include "DrawDistance.hpp"

#include "FollowerSettings.hpp"
#include "compat/GameAbi.hpp"

namespace Gen7Follower3gx
{
namespace
{

const unsigned int CAMERA_STATE_CAPACITY = 3;

struct CameraState
{
  poke_3d::model::BaseCamera* camera;
  float retailFar;
};

CameraState g_CameraStates[CAMERA_STATE_CAPACITY] = {};
unsigned int g_CameraStateCount = 0;

bool IsCameraAccessible(poke_3d::model::BaseCamera* camera)
{
  return camera != NULL;
}

CameraState* FindCameraState(poke_3d::model::BaseCamera* camera)
{
  for (unsigned int i = 0; i < g_CameraStateCount; ++i)
  {
    if (g_CameraStates[i].camera == camera)
    {
      return &g_CameraStates[i];
    }
  }
  return NULL;
}

CameraState* CaptureCamera(poke_3d::model::BaseCamera* camera)
{
  CameraState* state = FindCameraState(camera);
  if (state)
  {
    return state;
  }
  if (!IsCameraAccessible(camera) ||
      g_CameraStateCount >= CAMERA_STATE_CAPACITY)
  {
    return NULL;
  }

  const float retailFar = camera->GetFar(false);
  if (!(retailFar > 0.0f && retailFar < 1000000.0f))
  {
    return NULL;
  }

  state = &g_CameraStates[g_CameraStateCount++];
  state->camera = camera;
  state->retailFar = retailFar;
  return state;
}

void ApplyToCamera(
  Field::Camera::CameraUnit* cameraUnit,
  FollowerDrawDistanceMode mode
  )
{
  if (!cameraUnit)
  {
    return;
  }
  poke_3d::model::BaseCamera* const camera = cameraUnit->GetBaseCamera();
  CameraState* const state = CaptureCamera(camera);
  if (!state)
  {
    return;
  }
  camera->SetFar(state->retailFar * GetFollowerDrawDistanceScale(mode));
}

} // namespace

void InitializeDrawDistance()
{
  g_CameraStateCount = 0;
}

void UpdateDrawDistance(void* fieldmap)
{
  if (!fieldmap)
  {
    return;
  }

  const FollowerDrawDistanceMode mode = GetFollowerDrawDistanceMode();
  if (mode == FOLLOWER_DRAW_DISTANCE_FULL)
  {
    RestoreDrawDistance();
    return;
  }

  Field::Fieldmap* const map = static_cast<Field::Fieldmap*>(fieldmap);
  Field::Camera::CameraManager* const cameraManager = map->GetCameraManager();
  if (!cameraManager)
  {
    return;
  }

  Field::Camera::CameraUnit* const gameplayCamera =
    cameraManager->GetMainGamePlayCamera();
  Field::Camera::CameraUnit* const viewCamera =
    cameraManager->GetMainViewCamera();
  ApplyToCamera(gameplayCamera, mode);
  if (viewCamera != gameplayCamera)
  {
    ApplyToCamera(viewCamera, mode);
  }
}

void RestoreDrawDistance()
{
  for (unsigned int i = 0; i < g_CameraStateCount; ++i)
  {
    CameraState& state = g_CameraStates[i];
    if (IsCameraAccessible(state.camera))
    {
      state.camera->SetFar(state.retailFar);
    }
    state.camera = NULL;
    state.retailFar = 0.0f;
  }
  g_CameraStateCount = 0;
}

void ShutdownDrawDistance()
{
  RestoreDrawDistance();
}

} // namespace Gen7Follower3gx
