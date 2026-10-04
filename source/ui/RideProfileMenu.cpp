#include "RideProfileMenu.hpp"
#include "DecimalKeyboard.hpp"
#include "RideProfile.hpp"
#include "MountedRideFeedback.hpp"
#include <CTRPluginFramework/Menu/Keyboard.hpp>
#include <CTRPluginFramework/Menu/MessageBox.hpp>
#include <CTRPluginFramework/Utils/Utils.hpp>
#include <string>
#include <vector>
namespace Gen7Follower3gx {
namespace {
using CTRPluginFramework::Keyboard;
using CTRPluginFramework::MessageBox;
std::string Number(float value) { return CTRPluginFramework::Utils::Format("%.1f",value); }
int Choose(const std::string& title, const std::vector<std::string>& options, int selected=0)
{
  Keyboard keyboard(title,options);
  keyboard.ChangeSelectedEntry(selected);
  return keyboard.Open();
}
void SaveChange(RideProfile& profile, const RideProfile& previous)
{
  if (std::memcmp(&profile,&previous,sizeof(profile))==0) return;
  if (!SaveRideProfile(profile)) {
    profile=previous;
    MessageBox("Could not save this change. The previous value has been restored.")();
  }
}
bool EditFloat(const char* title, float& value, float minimum, float maximum)
{
  return EditDecimalValue(title,value,minimum,maximum);
}
void ChooseStyle(RideProfile& profile)
{
  std::vector<std::string> names;
  for (int i=0; i<RIDER_STYLE_COUNT; ++i)
    names.push_back(GetRiderAnimationStyleName(static_cast<RiderAnimationStyle>(i)));
  const int selected=Choose("Rider animation for this Pokemon",names,profile.style);
  if (selected>=0) profile.style=static_cast<unsigned int>(selected);
}
bool ChooseBone(bool rider, unsigned int species, char* name)
{
  RideBoneCatalog* catalog=new RideBoneCatalog();
  if (!catalog) return false;
  GetRideBoneCatalog(rider,*catalog);
  std::vector<std::string> names;
  names.push_back(rider ? "Clear this bone edit" : "No bone (fixed seat)");
  names.push_back("Enter bone name manually");
  if (catalog->species==species)
    for (unsigned int i=0; i<catalog->count; ++i)
      if (catalog->names[i][0]) names.push_back(catalog->names[i]);
  delete catalog;
  const int selected=Choose("Choose bone; mount this species once to populate names",names);
  if (selected<0) return false;
  std::string result;
  if (selected==1) {
    Keyboard keyboard("Bone name (case-sensitive, max 63 characters)");
    if (keyboard.Open(result,std::string(name))<0) return false;
    if (result.size()>=RideBoneNameSize) { MessageBox("Bone name is too long")(); return false; }
  } else if (selected>1) result=names[selected];
  std::memset(name,0,RideBoneNameSize);
  std::memcpy(name,result.c_str(),result.size());
  return true;
}
void EditRiderBones(RideProfile& profile)
{
  for (;;) {
    std::vector<std::string> names;
    for (unsigned int i=0; i<RideBoneEditCount; ++i)
      names.push_back(std::to_string(i+1)+": "+(profile.bones[i].name[0] ? profile.bones[i].name : "Unused"));
    const int slot=Choose("Rider bone adjustments (8 slots)",names);
    if (slot<0) return;
    RiderBoneEdit& edit=profile.bones[slot];
    for (;;) {
      const int item=Choose("Rider bone: "+std::string(edit.name),{
        "Choose bone", "Position X: "+Number(edit.translation[0]),
        "Position Y: "+Number(edit.translation[1]),"Position Z: "+Number(edit.translation[2]),
        "Rotation X: "+Number(edit.rotation[0]),"Rotation Y: "+Number(edit.rotation[1]),
        "Rotation Z: "+Number(edit.rotation[2]),"Clear slot"});
      if (item<0) break;
      const RideProfile previous=profile;
      if (item==0) ChooseBone(true,profile.species,edit.name);
      else if (item<=3) EditFloat("Parent-space position",edit.translation[item-1],-100,100);
      else if (item<=6) EditFloat("Parent-space rotation (degrees)",edit.rotation[item-4],-180,180);
      else edit=RiderBoneEdit{};
      SaveChange(profile,previous);
    }
  }
}
}
int ChooseProfileAction(const RideProfile& profile, unsigned int species, int& section, int& row)
{
  const std::vector<std::string> labels = {
      std::string("Riding: ")+(profile.enabled ? "Enabled" : "Disabled"),
      std::string("Animation: ")+GetRiderAnimationStyleName(static_cast<RiderAnimationStyle>(profile.style)),
      "Seat right: "+Number(profile.seat[0]),"Seat up: "+Number(profile.seat[1]),
      "Seat forward: "+Number(profile.seat[2]),"Rider yaw: "+Number(profile.yaw),
      std::string("Attach hips to: ")+(profile.attachment[0] ? profile.attachment : "Fixed seat"),
      std::string("Tauros stabilization: ")+(profile.stabilize ? "On" : "Off"),
      "Tauros waist motion %: "+Number(profile.waistMotion),
      "Tauros thighs inward: "+Number(profile.thighInward),
      "Rider bone adjustments",
      std::string("Mode: ")+(profile.mode==RIDE_POKEMON_ON_PLAYER ? "Pokemon rides player" : "Player rides Pokemon"),
      "Movement speed: "+Number(profile.movementSpeed)+"x",
      std::string("Carry attachment: ")+(profile.carryAttachment[0] ? profile.carryAttachment : "Player origin"),
      "Carry right: "+Number(profile.carryOffset[0]), "Carry up: "+Number(profile.carryOffset[1]),
      "Carry forward: "+Number(profile.carryOffset[2]),
      "Carry pitch: "+Number(profile.carryRotation[0]), "Carry yaw: "+Number(profile.carryRotation[1]),
      "Carry roll: "+Number(profile.carryRotation[2]),
      "Pokemon size: "+Number(profile.pokemonScale)+"x",
      "Reset to defaults"};
  for (;;) {
    const int selected = section>=0 ? section : Choose("Pokemon #"+std::to_string(species)+
      " - changes save immediately; B: back. Remount for ride changes.", {
      labels[0], labels[11], labels[1], labels[12], labels[20],
      "Rider seat and attachment...", "Carried Pokemon position...",
      "Rider bones and stabilization...", labels[21]},row);
    if (selected < 0) return selected;
    const int direct[] = {0, 11, 1, 12, 20, -1, -1, -1, 21};
    if (direct[selected] >= 0) { row=selected; return direct[selected]; }
    if (section<0) { section=selected; row=0; }
    const int seat[] = {2, 3, 4, 5, 6};
    const int carry[] = {13, 14, 15, 16, 17, 18, 19};
    const int bones[] = {10, 7, 8, 9};
    const int* actions = selected == 5 ? seat : (selected == 6 ? carry : bones);
    const unsigned count = selected == 5 ? 5 : (selected == 6 ? 7 : 4);
    std::vector<std::string> options;
    for (unsigned i=0; i<count; ++i) options.push_back(labels[actions[i]]);
    const int action = Choose(selected == 5 ? "Rider seat (right / up / forward)" :
      (selected == 6 ? "Carried Pokemon (relative to player)" : "Rider pose"), options,row);
    if (action >= 0) { row=action; return actions[action]; }
    row=section; section=-1;
  }
}
void EditRideProfile(CTRPluginFramework::MenuEntry*)
{
  RideBoneCatalog* catalog=new RideBoneCatalog();
  if (!catalog) return;
  GetRideBoneCatalog(false,*catalog);
  u32 species=catalog->species;
  delete catalog;
  const std::string speciesPrompt="Species ID (1-"+std::to_string(RideSpeciesMax)+")";
  const int target=Choose("Choose Pokemon to edit",{
    species ? "Last mounted species #"+std::to_string(species) : "Mount a Pokemon to select it here",
    "Enter "+speciesPrompt});
  if (target<0) return;
  if (target==1) {
    Keyboard keyboard(speciesPrompt);
    keyboard.IsHexadecimal(false);
    if (keyboard.Open(species,species ? species : 245U)<0) return;
  }
  if (!IsRideableFollowerSpecies(species)) { MessageBox("Choose a "+speciesPrompt)(); return; }
  RideProfile profile=GetRideProfile(species);
  int section=-1, row=0;
  for (;;) {
    const int item=ChooseProfileAction(profile,species,section,row);
    if (item<0) return;
    const RideProfile previous=profile;
    if (item==0) profile.enabled^=1;
    else if (item==1) ChooseStyle(profile);
    else if (item<=4) EditFloat("Seat offset (world units)",profile.seat[item-2],-500,500);
    else if (item==5) EditFloat("Rider yaw (degrees)",profile.yaw,-180,180);
    else if (item==6) ChooseBone(false,species,profile.attachment);
    else if (item==7) profile.stabilize^=1;
    else if (item==8) EditFloat("Waist rotation retained (%)",profile.waistMotion,0,100);
    else if (item==9) EditFloat("Thighs inward (degrees)",profile.thighInward,-45,45);
    else if (item==10) { EditRiderBones(profile); continue; }
    else if (item==11) profile.mode=(profile.mode+1)%RIDE_MODE_COUNT;
    else if (item==12) EditFloat("Movement speed multiplier",profile.movementSpeed,0.25f,2.0f);
    else if (item==13) ChooseBone(true,species,profile.carryAttachment);
    else if (item>=14 && item<=16) EditFloat("Carry position offset",profile.carryOffset[item-14],-500,500);
    else if (item>=17 && item<=19) EditFloat("Carry rotation (degrees)",profile.carryRotation[item-17],-180,180);
    else if (item==20) EditFloat("Pokemon size (applies on confirm)",profile.pokemonScale,0.25f,3.0f);
    else if (item==21) {
      if (Choose("Reset this Pokemon's settings?",{"Cancel","Reset to defaults"})!=1) continue;
      profile=DefaultRideProfile(species,GetRiderAnimationStyle());
    }
    SaveChange(profile,previous);
  }
}
}
