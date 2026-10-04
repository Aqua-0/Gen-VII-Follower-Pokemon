#pragma once
#include <cmath>
#include <cstring>

// Restore the rider before the next animation update so the edits don't keep adding up.
template<class Node, class Pose>
class RiderPoseStabilizer
{
public:
  RiderPoseStabilizer() : node_(NULL), changed_(false) { for (int& index : indices_) index=-1; }
  void Reset()
  {
    Restore();
    node_=NULL;
    for (int& index : indices_) index=-1;
  }
  void Capture(Node* node)
  {
    Reset();
    if (!node || node->GetJointNum()>256) { return; }
    node_=node;
    for (unsigned int i=0; i<node->GetJointNum(); ++i)
    {
      auto* joint=node->GetJointInstanceNode(i);
      const char* name=joint ? joint->GetName() : NULL;
      if (!name) { continue; }
      const int slot=std::strcmp(name,"_")==0 ? 0 :
        std::strcmp(name,"Waist")==0 ? 1 :
        std::strcmp(name,"LThigh")==0 ? 2 :
        std::strcmp(name,"RThigh")==0 ? 3 : -1;
      if (slot<0) { continue; }
      indices_[slot]=static_cast<int>(i);
      reference_[slot]=joint->GetLocalSrtRaw();
    }
  }
  bool Apply(Node* node, float waistMotion=15.0f, float thighInward=12.0f)
  {
    if (!node || node!=node_ || changed_) { return false; }
    for (int slot=0; slot<4; ++slot)
    {
      if (indices_[slot]<0) { continue; }
      auto* joint=node->GetJointInstanceNode(indices_[slot]);
      original_[slot]=joint->GetLocalSrtRaw();
      Pose pose=original_[slot];
      // Keep the seat at the same height when idle and moving so the rider doesn't jump.
      if (slot<2)
        for (int axis=0; axis<3; ++axis)
          pose.translate[axis]=reference_[slot].translate[axis];
      if (slot==1)
      {
        const float amount=waistMotion*0.01f;
        float dot=0, norm=0;
        for (int axis=0; axis<4; ++axis)
          dot+=reference_[slot].rotation[axis]*pose.rotation[axis];
        for (int axis=0; axis<4; ++axis)
        {
          pose.rotation[axis]=(1.0f-amount)*reference_[slot].rotation[axis]+
            amount*(dot<0 ? -pose.rotation[axis] : pose.rotation[axis]);
          norm+=pose.rotation[axis]*pose.rotation[axis];
        }
        if (norm>0.000001f)
          for (int axis=0; axis<4; ++axis) pose.rotation[axis]/=std::sqrt(norm);
      }
      if (slot>=2)
      {
        // For Tauros, parent-space Z turns the thighs inward without changing knee bends.
        const float halfAngle=thighInward*0.00872664626f;
        const float sine=(slot==2 ? -1.0f : 1.0f)*std::sin(halfAngle);
        const float cosine=std::cos(halfAngle);
        const float x=pose.rotation[0], y=pose.rotation[1];
        const float z=pose.rotation[2], w=pose.rotation[3];
        pose.rotation[0]=cosine*x-sine*y;
        pose.rotation[1]=cosine*y+sine*x;
        pose.rotation[2]=cosine*z+sine*w;
        pose.rotation[3]=cosine*w-sine*z;
      }
      joint->SetLocalSrtRaw(pose);
      changed_=true;
    }
    return changed_;
  }
  void Restore()
  {
    if (!changed_ || !node_) { return; }
    for (int slot=0; slot<4; ++slot)
      if (indices_[slot]>=0)
        node_->GetJointInstanceNode(indices_[slot])->SetLocalSrtRaw(original_[slot]);
    changed_=false;
  }
private:
  Node* node_;
  int indices_[4];
  Pose reference_[4], original_[4];
  bool changed_;
};
