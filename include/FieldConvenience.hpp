#pragma once
namespace Gen7Follower3gx {
struct GameProfile;
void BindPcEvent(const GameProfile& profile);
void UnbindPcEvent();
bool RequestOpenPc();
unsigned int PcRequestState();
void SetPcRequestState(unsigned int state);
}
extern "C" bool Follower3gx_StartPcEvent(void* gameManager);
