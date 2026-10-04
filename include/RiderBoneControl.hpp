#pragma once
#include "RideProfile.hpp"

namespace Gen7Follower3gx {
template<class Node> int FindRideJoint(Node* node, const char* name)
{
  if (!node || !name[0] || node->GetJointNum()>256) return -1;
  for (unsigned int i=0; i<node->GetJointNum(); ++i) {
    const char* candidate=node->GetJointInstanceNode(i)->GetName();
    if (candidate && std::strcmp(candidate,name)==0) return static_cast<int>(i);
  }
  return -1;
}
inline void RotateRiderQuaternion(float* q, const float* degrees)
{
  // Apply X, then Y, then Z rotations in parent space by pre-multiplying each one.
  for (int axis=0; axis<3; ++axis) {
    const float angle=degrees[axis]*0.00872664626f;
    const float s=std::sin(angle), c=std::cos(angle);
    float v[3]={0,0,0}; v[axis]=s;
    const float x=q[0], y=q[1], z=q[2], w=q[3];
    q[0]=c*x+v[0]*w+v[1]*z-v[2]*y;
    q[1]=c*y-v[0]*z+v[1]*w+v[2]*x;
    q[2]=c*z+v[0]*y-v[1]*x+v[2]*w;
    q[3]=c*w-v[0]*x-v[1]*y-v[2]*z;
  }
}
// Undo the custom edits before the stabilizer, in the reverse order they were applied.
template<class Node, class Pose> class RiderBoneControl {
public:
  RiderBoneControl() : node_(NULL), count_(0) {}
  void Restore() {
    if (node_) for (unsigned int i=count_; i>0; --i)
      node_->GetJointInstanceNode(indices_[i-1])->SetLocalSrtRaw(original_[i-1]);
    count_=0; node_=NULL;
  }
  bool Apply(Node* node, const RideProfile& profile) {
    if (node_ || !node) return false;
    node_=node;
    for (const auto& edit : profile.bones) {
      const int index=FindRideJoint(node,edit.name);
      if (index<0) continue;
      auto* joint=node->GetJointInstanceNode(index);
      indices_[count_]=index;
      original_[count_]=joint->GetLocalSrtRaw();
      Pose pose=original_[count_++];
      for (int axis=0; axis<3; ++axis) pose.translate[axis]+=edit.translation[axis];
      RotateRiderQuaternion(pose.rotation,edit.rotation);
      joint->SetLocalSrtRaw(pose);
    }
    return count_!=0;
  }
private:
  Node* node_;
  unsigned int count_;
  int indices_[RideBoneEditCount];
  Pose original_[RideBoneEditCount];
};
}
