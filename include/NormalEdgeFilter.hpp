#pragma once
namespace Gen7Follower3gx {
struct GameProfile;
bool InstallNormalEdgeFilter(const GameProfile& profile);
void RemoveNormalEdgeFilter();
bool IsNormalEdgeFilterInstalled();

class EdgeMaterialOverride
{
public:
  EdgeMaterialOverride(int* type, bool erase) : type_(erase ? type : 0), saved_(0)
  {
    if (type_) { saved_=*type_; *type_=3; }
  }
  ~EdgeMaterialOverride() { if (type_) *type_=saved_; }
private:
  int* type_;
  int saved_;
};
}
