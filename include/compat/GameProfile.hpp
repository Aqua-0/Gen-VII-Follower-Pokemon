#pragma once

#include <3ds/types.h>

#include "RetailFunctions.hpp"

namespace Gen7Follower3gx
{

enum class GameFamily : u8
{
  Regular,
  Ultra,
};

struct HookProfile
{
  u32 callsiteTextOffset;
  u32 targetTextOffset;
  u32 expectedCallInstruction;
  u32 expectedTargetInstructions[2];
};

struct LifecycleProfile
{
  u32 startModuleAddress;
  u32 startModuleInstructions[2];
  u32 disposeModuleAddress;
  u32 disposeModuleInstructions[2];
};

struct StaticHookProfile
{
  u32 targetAddress;
  u32 expectedTargetInstructions[2];
};

struct GameProfile
{
  const char* name;
  u32 titleIdLow;
  u16 minimumVersion;
  u16 maximumVersion;
  GameFamily family;
  const char* staticCrsSha256;
  const char* fieldRoSha256;
  const char* fieldDebugSha256;
  u32 retailAddresses[RetailFunctionCount];
  u32 retailEntryInstructions[RetailFunctionCount];
  u32 gfglSingletonAddress;
  LifecycleProfile lifecycle;
  StaticHookProfile simpleLightPickUpHook;
  StaticHookProfile initialSpecFixHook;
  HookProfile updateHook;
  HookProfile terminateHook;
  HookProfile surfHook;
  HookProfile eventCheckHook;
};

const GameProfile* SelectGameProfile(u64 titleId, u16 version);
const GameProfile* GetGameProfiles(u32& count);

} // namespace Gen7Follower3gx
