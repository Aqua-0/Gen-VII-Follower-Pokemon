#include "FreeCamera.hpp"
#include "RelativeCameraPose.hpp"

#include "FollowerFeatures.hpp"


#include <math.h>

#include "FreeCameraInput.hpp"
#include "compat/GameAbi.hpp"

namespace Gen7Follower3gx
{
namespace
{

const float CAMERA_MIN_DISTANCE = 1.0f;
const float CAMERA_MAX_DISTANCE = 100000.0f;
const float CAMERA_MOVE_SPEEDS[] = {0.5f, 2.0f, 5.0f, 10.0f};
const float CAMERA_LOOK_SPEED = 0.025f;
const float CAMERA_FAST_LOOK_SPEED = 0.05f;
const float CAMERA_PITCH_LIMIT = 1.483529864f;
const float CIRCLE_PAD_MAX = 156.0f;
const int CIRCLE_PAD_DEAD_ZONE = 15;

struct FreeCameraState
{
  bool active;
  bool overlayApplied;
  void* fieldmap;
  poke_3d::model::BaseCamera* camera;
  gfl2::ui::Device* stickDevice;
  gfl2::ui::Device* buttonDevice;
  bool stickWasRunning;
  bool buttonWasRunning;
  gfl2::math::Vector3 restorePosition;
  gfl2::math::Vector3 restoreTarget;
  gfl2::math::Vector3 position;
  float targetDistance;
  float yaw;
  float pitch;
};

FreeCameraState g_State = {};
struct SavedCameraPose {
  bool valid;
  void* fieldmap; // Only used to identify the object. We never dereference it.
  gfl2::math::Vector3 position;
  float targetDistance, yaw, pitch;
};
SavedCameraPose g_Saved = {};
bool g_Enabled = false;
bool g_Parked = false;
bool g_FollowGame = false;
bool g_CaptureFollow = false;
bool g_FollowValid = false;
RelativeCameraPose g_FollowPose;
bool g_RearmControls = false;
unsigned int g_Speed=1;
bool g_ResetRequested=false;
bool g_ExitArmed=false;
bool g_ResetHeld=false;

float Clamp(float value, float minimum, float maximum)
{
  if (value < minimum)
  {
    return minimum;
  }
  if (value > maximum)
  {
    return maximum;
  }
  return value;
}

bool IsFiniteCoordinate(float value)
{
  return value == value && value > -10000000.0f && value < 10000000.0f;
}

bool IsFiniteVector(const gfl2::math::Vector3& value)
{
  return IsFiniteCoordinate(value.x) &&
    IsFiniteCoordinate(value.y) &&
    IsFiniteCoordinate(value.z);
}

float NormalizeCirclePadAxis(int value)
{
  if (value > -CIRCLE_PAD_DEAD_ZONE && value < CIRCLE_PAD_DEAD_ZONE)
  {
    return 0.0f;
  }
  return Clamp(static_cast<float>(value) / CIRCLE_PAD_MAX, -1.0f, 1.0f);
}

void ClearActiveState()
{
  g_State.active = false;
  g_State.overlayApplied = false;
  g_State.fieldmap = NULL;
  g_State.camera = NULL;
  g_State.stickDevice = NULL;
  g_State.buttonDevice = NULL;
  g_State.stickWasRunning = false;
  g_State.buttonWasRunning = false;
  g_State.restorePosition.Set(0.0f, 0.0f, 0.0f);
  g_State.restoreTarget.Set(0.0f, 0.0f, 0.0f);
  g_State.position.Set(0.0f, 0.0f, 0.0f);
  g_State.targetDistance = 0.0f;
  g_State.yaw = 0.0f;
  g_State.pitch = 0.0f;
}

void RestoreGameInput()
{
  if (IsFreeCameraObjectWritable(g_State.stickDevice))
  {
    g_State.stickDevice->SetDeviceRunningEnable(g_State.stickWasRunning);
  }
  if (IsFreeCameraObjectWritable(g_State.buttonDevice))
  {
    g_State.buttonDevice->SetDeviceRunningEnable(g_State.buttonWasRunning);
  }
}

void SuppressGameInput()
{
  if (IsFreeCameraObjectWritable(g_State.stickDevice))
  {
    g_State.stickDevice->SetDeviceRunningEnable(false);
  }
  if (IsFreeCameraObjectWritable(g_State.buttonDevice))
  {
    g_State.buttonDevice->SetDeviceRunningEnable(false);
  }
}

void Deactivate(bool restoreCamera, bool remember = true)
{
  if (!g_State.active)
  {
    return;
  }

  if (remember) {
    g_Saved.valid=true;
    g_Saved.fieldmap=g_State.fieldmap;
    g_Saved.position=g_State.position;
    g_Saved.targetDistance=g_State.targetDistance;
    g_Saved.yaw=g_State.yaw;
    g_Saved.pitch=g_State.pitch;
  }
  RestoreGameInput();
  if (restoreCamera &&
      IsFreeCameraObjectWritable(g_State.camera) &&
      IsFiniteVector(g_State.restorePosition) &&
      IsFiniteVector(g_State.restoreTarget))
  {
    const gfl2::math::Vector3 up(0.0f, 1.0f, 0.0f);
    g_State.camera->SetupCameraLookAt(
      g_State.restorePosition,
      g_State.restoreTarget,
      up
      );
  }
  ClearActiveState();
}

void RestoreNativeCameraForUpdate()
{
  if (g_State.active && g_State.overlayApplied && IsFreeCameraObjectWritable(g_State.camera)) {
    const gfl2::math::Vector3 up(0,1,0);
    g_State.camera->SetupCameraLookAt(g_State.restorePosition,g_State.restoreTarget,up);
    g_State.overlayApplied=false;
  }
}
bool NativeAngles(float& yaw,float& pitch,float& distance)
{
  const float dx=g_State.restoreTarget.x-g_State.restorePosition.x;
  const float dy=g_State.restoreTarget.y-g_State.restorePosition.y;
  const float dz=g_State.restoreTarget.z-g_State.restorePosition.z;
  const float horizontal=sqrtf(dx*dx+dz*dz);
  distance=sqrtf(horizontal*horizontal+dy*dy);
  if (!(distance>=CAMERA_MIN_DISTANCE && distance<=CAMERA_MAX_DISTANCE)) return false;
  yaw=atan2f(dx,dz); pitch=atan2f(dy,horizontal);
  return true;
}
void CaptureFollowPose()
{
  float yaw,pitch,distance;
  if (!NativeAngles(yaw,pitch,distance)) return;
  g_FollowPose.Capture(g_State.restorePosition,yaw,pitch,distance,
    g_State.position,g_State.yaw,g_State.pitch,g_State.targetDistance);
  g_FollowValid=true;
}

bool ResolveFieldObjects(
  void* fieldmap,
  poke_3d::model::BaseCamera** camera,
  gfl2::ui::Device** stickDevice,
  gfl2::ui::Device** buttonDevice
  )
{
  Field::Fieldmap* const map = static_cast<Field::Fieldmap*>(fieldmap);
  Field::Camera::CameraManager* const cameraManager = map
    ? map->GetCameraManager()
    : NULL;
  Field::Camera::CameraUnit* const cameraUnit = cameraManager
    ? cameraManager->GetMainViewCamera()
    : NULL;
  *camera = cameraUnit ? cameraUnit->GetBaseCamera() : NULL;

  GameSys::GameManager* const gameManager = map
    ? map->GetGameManager()
    : NULL;
  gfl2::ui::DeviceManager* const deviceManager = gameManager
    ? gameManager->GetUiDeviceManager()
    : NULL;
  gfl2::ui::VectorDevice* const stick = deviceManager
    ? deviceManager->GetStick(gfl2::ui::DeviceManager::STICK_STANDARD)
    : NULL;
  gfl2::ui::Button* const button = deviceManager
    ? deviceManager->GetButton(gfl2::ui::DeviceManager::BUTTON_STANDARD)
    : NULL;
  *stickDevice = reinterpret_cast<gfl2::ui::Device*>(stick);
  *buttonDevice = reinterpret_cast<gfl2::ui::Device*>(button);

  return IsFreeCameraObjectWritable(*camera) &&
    IsFreeCameraObjectWritable(*stickDevice) &&
    IsFreeCameraObjectWritable(*buttonDevice);
}

bool Activate(
  void* fieldmap,
  poke_3d::model::BaseCamera* camera,
  gfl2::ui::Device* stickDevice,
  gfl2::ui::Device* buttonDevice
  )
{
  const gfl2::math::Vector3 position = camera->GetPosition(false);
  const gfl2::math::Vector3 target = camera->GetTargetPosition();
  if (!IsFiniteVector(position) || !IsFiniteVector(target))
  {
    return false;
  }

  const float deltaX = target.x - position.x;
  const float deltaY = target.y - position.y;
  const float deltaZ = target.z - position.z;
  const float horizontalDistance = sqrtf(
    deltaX * deltaX + deltaZ * deltaZ
    );
  const float distance = sqrtf(
    horizontalDistance * horizontalDistance + deltaY * deltaY
    );
  if (!(distance >= CAMERA_MIN_DISTANCE && distance <= CAMERA_MAX_DISTANCE))
  {
    return false;
  }

  FreeCameraInputState input={};
  ReadFreeCameraInput(&input);
  g_ExitArmed=!input.exit; // Ignore B if it's still held from closing the menu.
  g_ResetHeld=input.reset;
  g_State.active = true;
  g_State.fieldmap = fieldmap;
  g_State.camera = camera;
  g_State.stickDevice = stickDevice;
  g_State.buttonDevice = buttonDevice;
  g_State.stickWasRunning = stickDevice->IsDeviceRunning();
  g_State.buttonWasRunning = buttonDevice->IsDeviceRunning();
  g_State.restorePosition = position;
  g_State.restoreTarget = target;
  g_State.position = position;
  g_State.targetDistance = distance;
  g_State.yaw = atan2f(deltaX, deltaZ);
  g_State.pitch = atan2f(deltaY, horizontalDistance);
  if (g_Saved.valid && g_Saved.fieldmap==fieldmap) {
    g_State.position=g_Saved.position;
    g_State.targetDistance=g_Saved.targetDistance;
    g_State.yaw=g_Saved.yaw;
    g_State.pitch=g_Saved.pitch;
  }
  if (!IsFreeCameraParked()) SuppressGameInput();
  return true;
}

void ApplyInputAndCamera()
{
  FreeCameraInputState input = {};
  if (!IsFreeCameraParked()) ReadFreeCameraInput(&input);
  const float circleX = NormalizeCirclePadAxis(input.circleX);
  const float circleY = NormalizeCirclePadAxis(input.circleY);

  float lookX = static_cast<float>(input.cStickX);
  float lookY = static_cast<float>(input.cStickY);

  const bool useCirclePadForLook =
    lookX == 0.0f && lookY == 0.0f &&
    input.holdA;
  if (useCirclePadForLook)
  {
    lookX = circleX;
    lookY = circleY;
  }

  const float lookSpeed = input.fast
    ? CAMERA_FAST_LOOK_SPEED
    : CAMERA_LOOK_SPEED;
  g_State.yaw -= lookX * lookSpeed;
  g_State.pitch = Clamp(
    g_State.pitch + lookY * lookSpeed,
    -CAMERA_PITCH_LIMIT,
    CAMERA_PITCH_LIMIT
    );

  const float cosPitch = cosf(g_State.pitch);
  const gfl2::math::Vector3 forward(
    sinf(g_State.yaw) * cosPitch,
    sinf(g_State.pitch),
    cosf(g_State.yaw) * cosPitch
    );
  // SetupCameraLookAt uses an OpenGL-style basis: screen-right is forward x up.
  const gfl2::math::Vector3 right(
    -cosf(g_State.yaw),
    0.0f,
    sinf(g_State.yaw)
    );

  float vertical = 0.0f;
  if (input.moveUp)
  {
    vertical += 1.0f;
  }
  if (input.moveDown)
  {
    vertical -= 1.0f;
  }

  const float moveX = useCirclePadForLook ? 0.0f : circleX;
  const float moveForward = useCirclePadForLook ? 0.0f : circleY;
  const float moveSpeed = CAMERA_MOVE_SPEEDS[GetFreeCameraSpeed()] * (input.fast ? 5.0f : 1.0f);
  g_State.position.x +=
    (right.x * moveX + forward.x * moveForward) * moveSpeed;
  g_State.position.y +=
    (forward.y * moveForward + vertical) * moveSpeed;
  g_State.position.z +=
    (right.z * moveX + forward.z * moveForward) * moveSpeed;

  const gfl2::math::Vector3 target(
    g_State.position.x + forward.x * g_State.targetDistance,
    g_State.position.y + forward.y * g_State.targetDistance,
    g_State.position.z + forward.z * g_State.targetDistance
    );
  const gfl2::math::Vector3 up(0.0f, 1.0f, 0.0f);
  g_State.camera->SetupCameraLookAt(g_State.position, target, up);
  g_State.overlayApplied=true;
}

} // namespace

void FollowGameCamera() {
  const bool capture=!g_FollowValid || (g_State.active && !IsFollowingGameCamera());
  __atomic_store_n(&g_CaptureFollow,capture,__ATOMIC_RELAXED);
  __atomic_store_n(&g_FollowGame,true,__ATOMIC_RELAXED);
  __atomic_store_n(&g_Parked,true,__ATOMIC_RELAXED);
  SetFreeCameraEnabled(true);
}
bool IsFollowingGameCamera() { return __atomic_load_n(&g_FollowGame,__ATOMIC_RELAXED); }
void SetFreeCameraParked(bool parked) {
  __atomic_store_n(&g_FollowGame,false,__ATOMIC_RELAXED);
  __atomic_store_n(&g_CaptureFollow,false,__ATOMIC_RELAXED);
  __atomic_store_n(&g_Parked,parked,__ATOMIC_RELAXED);
  if (!parked) __atomic_store_n(&g_RearmControls,true,__ATOMIC_RELAXED);
}
bool IsFreeCameraParked() { return __atomic_load_n(&g_Parked,__ATOMIC_RELAXED); }
void RequestFreeCameraReset() { __atomic_store_n(&g_ResetRequested,true,__ATOMIC_RELAXED); }
void SetFreeCameraSpeed(unsigned int preset) { if (preset<4) __atomic_store_n(&g_Speed,preset,__ATOMIC_RELAXED); }
unsigned int GetFreeCameraSpeed() { return __atomic_load_n(&g_Speed,__ATOMIC_RELAXED); }
void InitializeFreeCamera()
{
  g_Saved.valid=false;
  ClearActiveState();
}

void ShutdownFreeCamera()
{
  Deactivate(true,false);
  g_Saved.valid=false;
}

void SetFreeCameraEnabled(bool enabled)
{
  __atomic_store_n(&g_Enabled,enabled,__ATOMIC_RELAXED);
}

bool IsFreeCameraEnabled()
{
  return __atomic_load_n(&g_Enabled,__ATOMIC_RELAXED);
}

bool IsFreeCameraActive()
{
  return g_State.active && !IsFreeCameraParked(); // Let the player interact while the camera stays in place.
}

void PrepareFreeCamera(void* fieldmap, bool freeField)
{
  RestoreNativeCameraForUpdate();
  const bool requestedReset=__atomic_exchange_n(&g_ResetRequested,false,__ATOMIC_RELAXED);
  if (requestedReset) {
    Deactivate(true,false); g_Saved.valid=false;
    g_FollowPose=RelativeCameraPose(); g_FollowValid=true;
    __atomic_store_n(&g_CaptureFollow,false,__ATOMIC_RELAXED);
  }
  if (g_Saved.valid && g_Saved.fieldmap!=fieldmap) g_Saved.valid=false;
  if (!IsFreeCameraEnabled() || !freeField || !fieldmap)
  {
    Deactivate(true);
    return;
  }

  FreeCameraInputState input={};
  ReadFreeCameraInput(&input);
  if (__atomic_exchange_n(&g_RearmControls,false,__ATOMIC_RELAXED)) {
    g_ExitArmed=!input.exit;
    g_ResetHeld=input.reset;
  }
  if (g_State.active && !IsFreeCameraParked()) {
    if (!input.exit) g_ExitArmed=true;
    if (input.exit && g_ExitArmed) {
      SetFreeCameraParked(true);
      RestoreGameInput();
      return;
    }
    if (input.reset && !g_ResetHeld) {
      Deactivate(true,false);
      g_Saved.valid=false;
    }
    g_ResetHeld=input.reset;
  }

  poke_3d::model::BaseCamera* camera = NULL;
  gfl2::ui::Device* stickDevice = NULL;
  gfl2::ui::Device* buttonDevice = NULL;
  if (!ResolveFieldObjects(
        fieldmap,
        &camera,
        &stickDevice,
        &buttonDevice
        ))
  {
    Deactivate(true);
    return;
  }

  if (g_State.active &&
      (g_State.fieldmap != fieldmap || g_State.camera != camera ||
       g_State.stickDevice != stickDevice ||
       g_State.buttonDevice != buttonDevice))
  {
    Deactivate(true);
  }
  if (!g_State.active &&
      !Activate(fieldmap, camera, stickDevice, buttonDevice))
  {
    return;
  }
  if (__atomic_exchange_n(&g_CaptureFollow,false,__ATOMIC_RELAXED)) CaptureFollowPose();
  if (IsFreeCameraParked()) RestoreGameInput();
  else SuppressGameInput();
}

void UpdateFreeCamera(void* fieldmap)
{
  if (!g_State.active || g_State.fieldmap != fieldmap ||
      !IsFreeCameraObjectWritable(g_State.camera))
  {
    return;
  }
  if (!g_State.overlayApplied) {
    const auto position=g_State.camera->GetPosition(false);
    const auto target=g_State.camera->GetTargetPosition();
    if (!IsFiniteVector(position) || !IsFiniteVector(target)) return;
    g_State.restorePosition=position; g_State.restoreTarget=target;
  }
  if (IsFollowingGameCamera() && g_FollowValid) {
    float yaw,pitch,distance;
    if (!NativeAngles(yaw,pitch,distance)) return;
    g_State.position=g_FollowPose.Position(g_State.restorePosition,yaw);
    g_State.yaw=yaw+g_FollowPose.yaw;
    g_State.pitch=Clamp(pitch+g_FollowPose.pitch,-CAMERA_PITCH_LIMIT,CAMERA_PITCH_LIMIT);
    g_State.targetDistance=Clamp(distance*g_FollowPose.distanceScale,CAMERA_MIN_DISTANCE,CAMERA_MAX_DISTANCE);
  }
  ApplyInputAndCamera();
}

void TerminateFreeCameraField(void* fieldmap)
{
  if (g_Saved.fieldmap==fieldmap) g_Saved.valid=false;
  if (g_State.active && g_State.fieldmap == fieldmap)
  {
    Deactivate(true,false);
  }
}

} // namespace Gen7Follower3gx
