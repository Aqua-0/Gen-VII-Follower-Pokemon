#pragma once

#include <stddef.h>

typedef signed char s8;
typedef unsigned char u8;
typedef signed short s16;
typedef unsigned short u16;
typedef signed int s32;
typedef unsigned int u32;
typedef float f32;
typedef int b32;

namespace nn {
namespace ro {
class Module;
} // namespace ro
} // namespace nn

#ifndef NULL
#define NULL 0
#endif

#ifndef FOLLOWER_POKEMON_MODEL_LAYOUT_REGULAR
#define FOLLOWER_POKEMON_MODEL_LAYOUT_REGULAR 0
#endif

#define GFL_BOOL_CAST(value) ((value) ? true : false)

extern "C" f32 __hardfp_atan2f(f32 y, f32 x);
#define atan2 __hardfp_atan2f

namespace Sound
{
void PlaySE(u32 soundId, u32 fadeinFrame, s32 controlId, u32 playerId);
void ChangeSEVolume(
  u32 soundId,
  f32 volume,
  u32 changeFrame,
  s32 controlId
  );
}

namespace gfl2 {
namespace math {

struct Vector3
{
  f32 x;
  f32 y;
  f32 z;

  Vector3() : x(0.0f), y(0.0f), z(0.0f) {}
  Vector3(f32 xValue, f32 yValue, f32 zValue)
  : x(xValue), y(yValue), z(zValue) {}
  Vector3(const Vector3& rhs) : x(rhs.x), y(rhs.y), z(rhs.z) {}

  Vector3& operator=(const Vector3& rhs)
  {
    x = rhs.x;
    y = rhs.y;
    z = rhs.z;
    return *this;
  }

  void Set(f32 xValue, f32 yValue, f32 zValue)
  {
    x = xValue;
    y = yValue;
    z = zValue;
  }

  Vector3 operator+(const Vector3& rhs) const
  {
    return Vector3(x + rhs.x, y + rhs.y, z + rhs.z);
  }
};

struct Vector4
{
  f32 x;
  f32 y;
  f32 z;
  f32 w;

  Vector4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}
  Vector4(f32 xValue, f32 yValue, f32 zValue, f32 wValue)
  : x(xValue), y(yValue), z(zValue), w(wValue) {}
};

struct Quaternion
{
  f32 x;
  f32 y;
  f32 z;
  f32 w;

  Quaternion() : x(0.0f), y(0.0f), z(0.0f), w(1.0f) {}
  Quaternion(f32 xValue, f32 yValue, f32 zValue, f32 wValue)
  : x(xValue), y(yValue), z(zValue), w(wValue) {}
};

inline f32 FSqrt(f32 value)
{
  return __builtin_sqrtf(value);
}

class EulerRotation
{
public:
  enum RotationOrder
  {
    RotationOrderXYZ,
    RotationOrderXZY,
    RotationOrderYXZ,
    RotationOrderYZX,
    RotationOrderZXY,
    RotationOrderZYX,
  };
};

} // namespace math
} // namespace gfl2

namespace gfl2 {
namespace heap {

enum HeapType
{
  HEAP_TYPE_EXP = 0,
};

class CtrHeapBase
{
public:
  u32 GetTotalSize()
  {
    typedef u32 (*Function)(CtrHeapBase*);
    void** vtable = *reinterpret_cast<void***>(this);
    return reinterpret_cast<Function>(vtable[0x24 / 4])(this);
  }

  u32 GetTotalFreeSize()
  {
    typedef u32 (*Function)(CtrHeapBase*);
    void** vtable = *reinterpret_cast<void***>(this);
    return reinterpret_cast<Function>(vtable[0x28 / 4])(this);
  }

  u32 GetTotalAllocatableSize()
  {
    typedef u32 (*Function)(CtrHeapBase*);
    void** vtable = *reinterpret_cast<void***>(this);
    return reinterpret_cast<Function>(vtable[0x2c / 4])(this);
  }

  CtrHeapBase* GetLowerHandle()
  {
    typedef CtrHeapBase* (*Function)(CtrHeapBase*);
    void** vtable = *reinterpret_cast<void***>(this);
    return reinterpret_cast<Function>(vtable[0x34 / 4])(this);
  }
};

typedef CtrHeapBase HeapBase;

class Manager
{
public:
  static CtrHeapBase* CreateHeap(
    CtrHeapBase* parent,
    int heapId,
    int size,
    HeapType type,
    bool allocateFromBack,
    const char* name
  );
  static void DeleteHeap(CtrHeapBase* heap);
  static CtrHeapBase* GetHeapByHeapId(int heapId);
};

void GflHeapFreeMemoryBlock(void* memory);

} // namespace heap
} // namespace gfl2

namespace gfl2 {
namespace fs {

struct Result
{
  u32 bitFlags[2];
};

class ArcFile
{
public:
  Result GetDataSize(
    u32* dataSize,
    u32 dataId,
    gfl2::heap::HeapBase* temporaryHeap
    ) const;
};

class AsyncFileManager
{
public:
  struct ArcFileLoadDataReq
  {
    const char* fileName;
    s32 arcId;
    s32 datId;
    u8 lang;
    u8 prio;
    u8 m_Pad0[2];
    void** ppBuf;
    u32* pBufSize;
    u32* pRealReadSize;
    gfl2::heap::HeapBase* heapForBuf;
    u32 align;
    gfl2::heap::HeapBase* heapForReq;
    gfl2::heap::HeapBase* heapForCompressed;
    u8 arcSrcUseSetting;
    u8 m_Pad1[3];
    void* result;
    u32 m_RealReadSize;
    void* m_SyncReqFinishEvent;

    ArcFileLoadDataReq()
    : fileName(NULL)
    , arcId(-1)
    , datId(-1)
    , lang(0xff)
    , prio(16)
    , m_Pad0{0, 0}
    , ppBuf(NULL)
    , pBufSize(NULL)
    , pRealReadSize(NULL)
    , heapForBuf(NULL)
    , align(4)
    , heapForReq(NULL)
    , heapForCompressed(NULL)
    , arcSrcUseSetting(0)
    , m_Pad1{0, 0, 0}
    , result(NULL)
    , m_RealReadSize(0)
    , m_SyncReqFinishEvent(NULL)
    {
    }
  };

  void AddArcFileLoadDataReq(const ArcFileLoadDataReq& request);
  bool IsArcFileLoadDataFinished(void** buffer) const;
  const ArcFile* GetArcFile(u32 archiveId) const;
};

} // namespace fs
} // namespace gfl2

namespace gfl2 {
namespace ro {

enum FixLevel
{
  FIX_LEVEL_0,
  FIX_LEVEL_1,
  FIX_LEVEL_2,
  FIX_LEVEL_3,
};

class RoManager
{
public:
  nn::ro::Module* LoadModule(
    gfl2::fs::AsyncFileManager* fileManager,
    const char* fileName,
    gfl2::heap::HeapBase* loadHeap,
    FixLevel fixLevel
    );
  void StartModule(nn::ro::Module* module, bool linkCheck);
  void DisposeModule(nn::ro::Module* module);
};

} // namespace ro
} // namespace gfl2

void* operator new(unsigned int size, gfl2::heap::CtrHeapBase* heap);
#if FOLLOWER_CARRIER_THREEGX
extern "C" void FollowerCarrier_RetailDelete(void* memory);

template <typename T>
inline void FollowerCarrierSafeDelete(T*& pointer)
{
  if (pointer)
  {
    pointer->~T();
    FollowerCarrier_RetailDelete(pointer);
    pointer = NULL;
  }
}
#else
void operator delete(void* memory) throw();
#endif

#define GFL_CREATE_LOCAL_HEAP_NAME(parent, size, type, back, name) \
  gfl2::heap::Manager::CreateHeap(parent, -2, size, type, back, name)
#define GFL_DELETE_HEAP(heapObject) \
  gfl2::heap::Manager::DeleteHeap(heapObject)
#define GFL_NEW(heap) new (heap)
#if FOLLOWER_CARRIER_THREEGX
#define GFL_SAFE_DELETE(pointer) FollowerCarrierSafeDelete(pointer)
#else
#define GFL_SAFE_DELETE(pointer) \
  do { if (pointer) { delete (pointer); (pointer) = NULL; } } while (0)
#endif

enum
{
  HEAPID_APP_DEVICE = 5,
  HEAPID_FILEREAD = 7,
  HEAPID_DLL_LOAD = 9,
  HEAPID_EVENT_DEVICE = 11,
};

namespace gfl2 {
namespace gfx {

class CtrDisplayNo
{
public:
  enum Enum { LEFT = 0, RIGHT, DOWN, NumberOf };
  CtrDisplayNo() : m_Value(LEFT) {}
  CtrDisplayNo(Enum value) : m_Value(value) {}
private:
  Enum m_Value;
};

class PolygonFace
{
public:
  enum Enum { CCW = 0, CW, NumberOf };
  PolygonFace() : m_Value(CCW) {}
  PolygonFace(Enum value) : m_Value(value) {}
private:
  Enum m_Value;
};

class CompareFunc
{
public:
  enum Enum
  {
    Less = 0,
    LessEqual,
    Equal,
    GreaterEqual,
    Greater,
    NotEqual,
    Always,
    Never,
    NumberOf,
  };
  CompareFunc() : m_Value(Less) {}
  CompareFunc(Enum value) : m_Value(value) {}
private:
  Enum m_Value;
};

class StencilOp
{
public:
  enum Enum
  {
    Keep = 0,
    Zero,
    Replace,
    Increment,
    IncrementWrap,
    Decrement,
    DecrementWrap,
    Invert,
    NumberOf,
  };
  StencilOp() : m_Value(Keep) {}
  StencilOp(Enum value) : m_Value(value) {}
private:
  Enum m_Value;
};

struct Color
{
  f32 r;
  f32 g;
  f32 b;
  f32 a;

  Color(f32 red, f32 green, f32 blue, f32 alpha)
  : r(red), g(green), b(blue), a(alpha) {}
};

struct ColorU8
{
  u8 r;
  u8 g;
  u8 b;
  u8 a;
};

class IGLAllocator;

class DepthStencilStateObject
{
public:
  u8 GetStencilReference() const
  {
    return *reinterpret_cast<const u8*>(
      reinterpret_cast<const u8*>(this) + 0x20
    );
  }

  void SetStencilTestEnable(bool enable)
  {
    typedef void (*Function)(DepthStencilStateObject*, b32);
    Call<Function>(0x20)(this, enable ? 1 : 0);
  }

  void SetStencilFunc(
    PolygonFace face,
    CompareFunc function,
    u8 reference,
    u8 mask
  )
  {
    typedef void (*Function)(
      DepthStencilStateObject*, PolygonFace, CompareFunc, u8, u8
    );
    Call<Function>(0x28)(this, face, function, reference, mask);
  }

  void SetStencilWriteMask(u8 mask)
  {
    typedef void (*Function)(DepthStencilStateObject*, u8);
    Call<Function>(0x2c)(this, mask);
  }

  void SetStencilOp(
    PolygonFace face,
    StencilOp fail,
    StencilOp depthFail,
    StencilOp pass
  )
  {
    typedef void (*Function)(
      DepthStencilStateObject*, PolygonFace, StencilOp, StencilOp, StencilOp
    );
    Call<Function>(0x30)(this, face, fail, depthFail, pass);
  }

  void UpdateState()
  {
    typedef void (*Function)(DepthStencilStateObject*);
    Call<Function>(0x34)(this);
  }

private:
  template <typename Function>
  Function Call(u32 byteOffset)
  {
    void** vtable = *reinterpret_cast<void***>(this);
    return reinterpret_cast<Function>(vtable[byteOffset / 4]);
  }
};

namespace ctr {
class CTRGL
{
public:
  void ClearRenderTargetDepthStencil_(
    u32 target,
    const Color& color,
    f32 depth,
    u8 stencil
  );
};
} // namespace ctr

template <typename Implementation>
class GFGLBase
{
public:
  static Implementation* s_Gp;
};

class GFGL
{
public:
  static void ClearRenderTargetDepthStencil(
    const Color& color,
    f32 depth,
    u8 stencil
  )
  {
    GFGLBase<ctr::CTRGL>::s_Gp->ClearRenderTargetDepthStencil_(
      0, color, depth, stencil
    );
  }
};

} // namespace gfx
} // namespace gfl2

namespace gfl2 {
namespace renderingengine {
namespace scenegraph {

class DagNode
{
public:
  const DagNode* GetParent() const
  {
    return *reinterpret_cast<DagNode* const*>(
      reinterpret_cast<const u8*>(this) + 0x0c
    );
  }
};

class SceneGraphManager
{
public:
  static void TraverseModelFast(DagNode* node);
};

namespace resource {

class ResourceNode;

struct EdgeType
{
  enum
  {
    Default = 0,
    VColor,
    None,
    Erase,
  };
};

class MaterialResourceNode
{
public:
  struct TextureInfo
  {
    struct Attribute
    {
      u32 m_UvSetNo;
      u32 m_MappingType;
      f32 m_ScaleU;
      f32 m_ScaleV;
      f32 m_Rotate;
      f32 m_TranslateU;
      f32 m_TranslateV;
      u32 m_RepeatU;
      u32 m_RepeatV;
      u32 m_MagFilter;
      u32 m_MinFilter;
      f32 m_MipBias;
    };

    u8 m_Pad0[0x50];
    Attribute m_Attribute;
  };

  struct AttributeParam
  {
    u8 m_Pad0[0x19];
    gfl2::gfx::ColorU8 m_ConstantColor[6];
    u8 m_Pad1[0x94 - 0x19 - sizeof(m_ConstantColor)];
    s8 m_LightSetNo;
    s8 m_FogNo;
    s8 m_BumpMapNo;
    u8 m_Pad2[0x9e - 0x97];
    s8 m_PsLightingEnable;
    s8 m_VsLightingEnable;
    s8 m_FogEnable;
    u8 m_Pad3[0xa8 - 0xa1];
  };

  struct UserData
  {
    s32 m_EdgeType;
    u8 m_Pad0[0x14 - sizeof(s32)];
    f32 m_RimScale;
    u8 m_Pad1[0x1c - 0x18];
    f32 m_PhongScale;
    u8 m_Pad2[0x60 - 0x20];
  };
};

} // namespace resource
namespace instance {

struct JointLocalSrt
{
  f32 translate[3];
  f32 scale[3];
  f32 rotation[4];
};

static_assert(sizeof(JointLocalSrt) == 0x28, "Joint local SRT ABI");
static_assert(offsetof(JointLocalSrt, translate) == 0x00, "SRT translate ABI");
static_assert(offsetof(JointLocalSrt, scale) == 0x0c, "SRT scale ABI");
static_assert(offsetof(JointLocalSrt, rotation) == 0x18, "SRT rotation ABI");

class JointInstanceNode
{
public:
  const char* GetName() const
  {
    // JointData starts with the name. Its pointer sits after the two skin matrices.
    return *reinterpret_cast<const char* const*>(
      reinterpret_cast<const u8*>(this) + 0x108);
  }

  gfl2::math::Vector3 GetWorldPositionRaw() const
  {
    // The world Matrix34 sits after the 0x28-byte local scale, rotation and translation.
    const f32* matrix=reinterpret_cast<const f32*>(
      reinterpret_cast<const u8*>(this) + 0x48);
    return gfl2::math::Vector3(matrix[3],matrix[7],matrix[11]);
  }

  const JointLocalSrt& GetLocalSrtRaw() const
  {
    return *reinterpret_cast<const JointLocalSrt*>(
      reinterpret_cast<const u8*>(this) + 0x20
    );
  }

  void SetLocalSrtRaw(const JointLocalSrt& srt)
  {
    *reinterpret_cast<JointLocalSrt*>(
      reinterpret_cast<u8*>(this) + 0x20
    ) = srt;

    // Mark the joint as changed so the next traversal rebuilds its matrices.
    *reinterpret_cast<u32*>(reinterpret_cast<u8*>(this) + 0x78) |= 1U;
  }

  b32 IsNeedRendering() const
  {
    return *reinterpret_cast<const b32*>(
      reinterpret_cast<const u8*>(this) + 0xa4
    );
  }
};

class DrawableNode
{
public:
  void SetVisible(b32 visible);

  b32 IsVisible() const
  {
    return *reinterpret_cast<const b32*>(
      reinterpret_cast<const u8*>(this) + 0xb4
    );
  }

  u32 GetDrawTagNum()
  {
    typedef u32 (*Function)(DrawableNode*);
    return Call<Function>(0x1c)(this);
  }

  void* GetDrawTag(u32 index)
  {
    typedef void* (*Function)(DrawableNode*, u32);
    return Call<Function>(0x24)(this, index);
  }

private:
  template <typename Function>
  Function Call(u32 byteOffset)
  {
    void** vtable = *reinterpret_cast<void***>(this);
    return reinterpret_cast<Function>(vtable[byteOffset / 4]);
  }
};

class MaterialInstanceNode
{
public:
  gfl2::gfx::DepthStencilStateObject* GetDepthStencilStateObject()
  {
    u8* resource = *reinterpret_cast<u8**>(
      reinterpret_cast<u8*>(this) + 0x28
    );
    return resource
      ? *reinterpret_cast<gfl2::gfx::DepthStencilStateObject**>(resource + 0x2d8)
      : NULL;
  }

  resource::MaterialResourceNode::AttributeParam* GetAttributeParam()
  {
    return reinterpret_cast<resource::MaterialResourceNode::AttributeParam*>(
      reinterpret_cast<u8*>(this) + 0x1cc
    );
  }

  resource::MaterialResourceNode::TextureInfo* GetTextureInfo(u32 index)
  {
    return reinterpret_cast<resource::MaterialResourceNode::TextureInfo*>(
      reinterpret_cast<u8*>(this) + 0x4c +
      index * sizeof(resource::MaterialResourceNode::TextureInfo)
    );
  }

  const resource::MaterialResourceNode::UserData& GetUserData() const
  {
    const u8* resourceData = *reinterpret_cast<u8* const*>(
      reinterpret_cast<const u8*>(this) + 0x28
    );
    return *reinterpret_cast<const resource::MaterialResourceNode::UserData*>(
      resourceData + 0x25c
    );
  }
};

class ModelInstanceNode : public DrawableNode
{
public:
  f32 GetFacingYawRaw() const
  {
    const f32* q=reinterpret_cast<const f32*>(reinterpret_cast<const u8*>(this)+0x38);
    return atan2(2.0f*(q[3]*q[1]+q[0]*q[2]),1.0f-2.0f*(q[1]*q[1]+q[2]*q[2]));
  }
  u32 GetJointNum() const
  {
    ModelInstanceNode* reference = *reinterpret_cast<ModelInstanceNode* const*>(
      reinterpret_cast<const u8*>(this) + 0x13c
    );
    return reference
      ? reference->GetJointNum()
      : *reinterpret_cast<const u32*>(
          reinterpret_cast<const u8*>(this) + 0xd0
        );
  }

  JointInstanceNode* GetJointInstanceNode(u32 index)
  {
    ModelInstanceNode* reference = *reinterpret_cast<ModelInstanceNode**>(
      reinterpret_cast<u8*>(this) + 0x13c
    );
    if (reference)
    {
      return reference->GetJointInstanceNode(index);
    }
    u8* joints = *reinterpret_cast<u8**>(
      reinterpret_cast<u8*>(this) + 0x100
    );
    return reinterpret_cast<JointInstanceNode*>(joints + index * 0x10c);
  }

  u32 GetMaterialNum() const
  {
    return *reinterpret_cast<const u32*>(
      reinterpret_cast<const u8*>(this) + 0xc0
    );
  }

  MaterialInstanceNode* GetMaterialInstanceNode(u32 index)
  {
    u8* materials = *reinterpret_cast<u8**>(
      reinterpret_cast<u8*>(this) + 0xe8
    );
    return reinterpret_cast<MaterialInstanceNode*>(materials + index * 0x2b0);
  }

  u32 GetReferenceCnt() const;
};

} // namespace instance
} // namespace scenegraph

namespace renderer {
class DrawManager
{
public:
  void ViewSpaceRenderEnable(b32 enable);
};

class MeshDrawTag
{
public:
  enum { MATERIAL_INSTANCE_NODE_OFFSET = 0x40 };

  scenegraph::instance::MaterialInstanceNode* GetMaterialInstanceNode()
  {
    return *reinterpret_cast<scenegraph::instance::MaterialInstanceNode**>(
      reinterpret_cast<u8*>(this) + MATERIAL_INSTANCE_NODE_OFFSET
    );
  }
};
} // namespace renderer
} // namespace renderingengine
} // namespace gfl2

namespace gfl2 {
namespace animation {

class AnimationPackList
{
public:
  void Initialize(gfl2::heap::HeapBase* heap, u32 packCount);
  void LoadData(
    u32 packNo,
    gfl2::gfx::IGLAllocator* allocator,
    gfl2::heap::HeapBase* heap,
    char* data
  );
  gfl2::renderingengine::scenegraph::resource::ResourceNode*
  GetResourceNode(u32 packNo, u32 animationNo);
  void Finalize();

private:
  void* m_pAnimationPackList;
  u32 m_AnimationPackCount;
};

} // namespace animation
} // namespace gfl2

namespace pml {

typedef u8 FormNo;
enum Sex { SEX_MALE = 0, SEX_FEMALE, SEX_UNKNOWN, SEX_NUM };

namespace pokepara {
enum EggCheckType
{
  CHECK_ONLY_LEGAL_EGG = 0,
  CHECK_ONLY_ILLEGAL_EGG,
  CHECK_BOTH_EGG,
};

class CoreParam
{
public:
  bool IsEgg(EggCheckType type) const;
  bool IsNull() const;
};

typedef CoreParam PokemonParam;
} // namespace pokepara

class PokeParty
{
public:
  u32 GetMemberCount() const;
  const pokepara::PokemonParam* GetMemberPointerConst(u32 index) const;
};

} // namespace pml

enum MonsNo
{
  MONSNO_NULL = 0,
  MONSNO_ABI_MAX = 0xffff,
};

namespace Sound
{
enum VoiceType
{
  VOICE_TYPE_DEFAULT = 0,
  VOICE_TYPE_BATTLE,
  VOICE_TYPE_DOWN,
  VOICE_TYPE_SP00,
  VOICE_TYPE_NUM,
};

void PlayVoice(
  u8 voiceIndex,
  MonsNo monsNo,
  pml::FormNo formNo,
  VoiceType voiceType = VOICE_TYPE_DEFAULT,
  b32 act3D = false,
  u32 userId = 0
);
}

namespace poke_3d {
namespace model {

class DressUpModelResourceManager;

class BaseCamera
{
public:
  void SetFar(f32 farClip);
  f32 GetFar(bool blendFlag = true) const;
  const gfl2::math::Vector3 GetPosition(bool blendFlag = true) const;
  const gfl2::math::Vector3 GetTargetPosition() const;
  void SetupCameraLookAt(
    const gfl2::math::Vector3& cameraPosition,
    const gfl2::math::Vector3& cameraTarget,
    const gfl2::math::Vector3& cameraUpVector
  );
};

class BaseModel
{
public:
  enum AnimationBit
  {
    ANIMATION_BIT_NONE = 0,
    ANIMATION_BIT_JOINT = 1,
    ANIMATION_BIT_MATERIAL = 2,
    ANIMATION_BIT_VISIBILITY = 4,
    ANIMATION_BIT_BLENDSHAPE = 8,
    ANIMATION_BIT_KEY = 16,
    ANIMATION_BIT_ALL = 31,
  };

  void SetPosition(const gfl2::math::Vector3& position);
  void SetRotation(
    f32 x,
    f32 y,
    f32 z,
    gfl2::math::EulerRotation::RotationOrder order =
      gfl2::math::EulerRotation::RotationOrderZYX
  );

  void SetVisible(bool visible)
  {
    typedef void (*Function)(BaseModel*, b32);
    Call<Function>(0x14)(this, visible ? 1 : 0);
  }

  bool IsVisible() const
  {
    // The outfit root stays hidden even when its parts are visible. Use the visibility override here.
    typedef b32 (*Function)(const BaseModel*);
    return const_cast<BaseModel*>(this)->Call<Function>(0x18)(this) != 0;
  }

  void ChangeAnimation(
    u32 animation,
    u32 slot = 0,
    AnimationBit bit = ANIMATION_BIT_ALL
  )
  {
    typedef void (*Function)(BaseModel*, u32, u32, AnimationBit);
    Call<Function>(0x2c)(this, animation, slot, bit);
  }

  void UpdateAnimation()
  {
    typedef f32 (*Function)(BaseModel*, AnimationBit);
    Call<Function>(0x1c)(this, ANIMATION_BIT_ALL);
  }

  void ChangeAnimationByResourceNode(
    gfl2::renderingengine::scenegraph::resource::ResourceNode* resource,
    u32 slot = 0,
    AnimationBit bit = ANIMATION_BIT_JOINT
  )
  {
    typedef void (*Function)(BaseModel*,
      gfl2::renderingengine::scenegraph::resource::ResourceNode*, u32, AnimationBit);
    Call<Function>(0x38)(this, resource, slot, bit);
  }

  bool IsAnimationExist(u32 animation) const
  {
    typedef b32 (*Function)(const BaseModel*, u32);
    return const_cast<BaseModel*>(this)->Call<Function>(0x30)(
      this,
      animation
      ) != 0;
  }

  void SetAnimationLoop(
    bool loop,
    u32 slot = 0,
    AnimationBit bit = ANIMATION_BIT_ALL
  )
  {
    typedef void (*Function)(BaseModel*, b32, u32, AnimationBit);
    Call<Function>(0x40)(this, loop ? 1 : 0, slot, bit);
  }

  f32 GetAnimationFrame(
    u32 slot = 0,
    AnimationBit bit = ANIMATION_BIT_ALL
  ) const;
  f32 GetAnimationEndFrame(
    u32 slot = 0,
    AnimationBit bit = ANIMATION_BIT_ALL
  ) const;

  void SetAnimationFrame(
    f32 frame,
    u32 slot = 0,
    AnimationBit bit = ANIMATION_BIT_ALL
  )
  {
    typedef void (*Function)(BaseModel*, f32, u32, AnimationBit);
    Call<Function>(0x44)(this, frame, slot, bit);
  }

  void SetAnimationStepFrame(
    f32 frame,
    u32 slot = 0,
    AnimationBit bit = ANIMATION_BIT_ALL
  )
  {
    typedef void (*Function)(BaseModel*, f32, u32, AnimationBit);
    Call<Function>(0x48)(this, frame, slot, bit);
  }

  gfl2::renderingengine::scenegraph::instance::ModelInstanceNode*
  GetModelInstanceNode()
  {
    return *reinterpret_cast<
      gfl2::renderingengine::scenegraph::instance::ModelInstanceNode**
    >(reinterpret_cast<u8*>(this) + 0x04);
  }

private:
  template <typename Function>
  Function Call(u32 byteOffset)
  {
    void** vtable = *reinterpret_cast<void***>(this);
    return reinterpret_cast<Function>(vtable[byteOffset / 4]);
  }
};

class PokemonModel : public BaseModel
{
public:
  gfl2::math::Vector3 GetWalkSpeed(f32 frame);
};

} // namespace model

namespace renderer {

class EdgeMapSceneRenderPath
{
public:
  void RemoveEdgeRenderingTarget(
    gfl2::renderingengine::scenegraph::instance::DrawableNode* node
  );
};

} // namespace renderer
} // namespace poke_3d

namespace PokeTool {

enum MODEL_ANIMETYPE
{
  MODEL_ANIMETYPE_BATTLE = 0,
  MODEL_ANIMETYPE_KAWAIGARI,
  MODEL_ANIMETYPE_FIELD,
  MODEL_ANIMETYPE_POKE_FINDER,
  MODEL_ANIMETYPE_APP,
};

enum MODEL_ANIME
{
  MODEL_ANIME_FI_WAIT_A = 0,
  MODEL_ANIME_FI_WAIT_B,
  MODEL_ANIME_WALK01,
  MODEL_ANIME_RUN01,
  MODEL_ANIME_MERGE_WALK01,
  MODEL_ANIME_WAIT_WALK01,
  MODEL_ANIME_WALK_WAIT01,
  MODEL_ANIME_MERGE_RUN01,
  MODEL_ANIME_WAIT_RUN01,
  MODEL_ANIME_RUN_WAIT01,
  MODEL_ANIME_MERGE_MOVE01,
  MODEL_ANIME_WALK_RUN01,
  MODEL_ANIME_RUN_WALK01,
  MODEL_ANIME_ERROR = -1,
};

struct SimpleParam
{
  MonsNo monsNo;
  pml::FormNo formNo;
  pml::Sex sex;
  bool isRare;
  bool isEgg;
  u8 m_Pad[2];
  u32 perRand;

  SimpleParam()
  : monsNo(MONSNO_NULL)
  , formNo(0)
  , sex(pml::SEX_MALE)
  , isRare(false)
  , isEgg(false)
  , m_Pad{0, 0}
  , perRand(0)
  {
  }
};

struct PokeSettingData
{
  s32 cmHeight;
  s32 adjustHeight;
  s32 fieldAdjustHeight;
  u32 size;
  f32 minX;
  f32 minY;
  f32 minZ;
  f32 maxX;
  f32 maxY;
  f32 maxZ;
  u8 m_Pad[0x60 - 40];
};

namespace PokeModelLayout
{
enum
{
  REGULAR_SETTING_DATA_OFFSET = 0x158c,
  REGULAR_UPDATE_FLAG_OFFSET = 0x1b00,
  REGULAR_BASE_SCALE_OFFSET = 0x1b04,
  REGULAR_ADJUST_SCALE_OFFSET = 0x1b10,
  REGULAR_BASE_POSITION_OFFSET = 0x1b14,
  ULTRA_SETTING_DATA_OFFSET = 0x15a4,
  ULTRA_UPDATE_FLAG_OFFSET = 0x1b80,
  ULTRA_BASE_SCALE_OFFSET = 0x1b84,
  ULTRA_ADJUST_SCALE_OFFSET = 0x1b90,
  ULTRA_BASE_POSITION_OFFSET = 0x1b94,
};

#if FOLLOWER_CARRIER_THREEGX
extern "C" b32 FollowerCarrier_IsRegularPokeModelLayout()
  __attribute__((pure));
#endif

inline u32 SelectOffset(u32 regularOffset, u32 ultraOffset)
{
#if FOLLOWER_CARRIER_THREEGX
  return FollowerCarrier_IsRegularPokeModelLayout()
    ? regularOffset
    : ultraOffset;
#elif FOLLOWER_POKEMON_MODEL_LAYOUT_REGULAR
  (void)ultraOffset;
  return regularOffset;
#else
  (void)regularOffset;
  return ultraOffset;
#endif
}

inline u32 SettingDataOffset()
{
  return SelectOffset(REGULAR_SETTING_DATA_OFFSET, ULTRA_SETTING_DATA_OFFSET);
}

inline u32 UpdateFlagOffset()
{
  return SelectOffset(REGULAR_UPDATE_FLAG_OFFSET, ULTRA_UPDATE_FLAG_OFFSET);
}

inline u32 BaseScaleOffset()
{
  return SelectOffset(REGULAR_BASE_SCALE_OFFSET, ULTRA_BASE_SCALE_OFFSET);
}

inline u32 AdjustScaleOffset()
{
  return SelectOffset(REGULAR_ADJUST_SCALE_OFFSET, ULTRA_ADJUST_SCALE_OFFSET);
}

inline u32 BasePositionOffset()
{
  return SelectOffset(REGULAR_BASE_POSITION_OFFSET, ULTRA_BASE_POSITION_OFFSET);
}
} // namespace PokeModelLayout

class PokeModel : public poke_3d::model::PokemonModel
{
public:
  struct SetupOption
  {
    gfl2::heap::HeapBase* dataHeap;
    gfl2::heap::HeapBase* workHeap;
    void* vramAddr;
    MODEL_ANIMETYPE animeType;
    bool useShadow;
    bool useIdModel;
    u8 m_Pad;

    SetupOption()
    : dataHeap(NULL)
    , workHeap(NULL)
    , vramAddr(NULL)
    , animeType(MODEL_ANIMETYPE_BATTLE)
    , useShadow(false)
    , useIdModel(false)
    , m_Pad(0)
    {
    }
  };

  void ChangeAnimation(MODEL_ANIME animation, bool forceReset = false);
  void ChangeAnimationResource(
    gfl2::renderingengine::scenegraph::resource::ResourceNode* resource,
    int animationSlot = 0
  );
  void ChangeAnimationSmooth(
    MODEL_ANIME animation,
    int smoothFrames,
    bool forceReset = false
  );
  bool IsAvailableAnimationDirect(int animation);
  void SetAnimationIsLoop(bool loop);
  bool CanDelete();

  void SetPosition(const gfl2::math::Vector3& position)
  {
    *reinterpret_cast<gfl2::math::Vector3*>(
      reinterpret_cast<u8*>(this) + PokeModelLayout::BasePositionOffset()
    ) = position;
    *reinterpret_cast<bool*>(
      reinterpret_cast<u8*>(this) + PokeModelLayout::UpdateFlagOffset()
    ) = true;
  }

  void SetScale(f32 x, f32 y, f32 z)
  {
    reinterpret_cast<gfl2::math::Vector3*>(
      reinterpret_cast<u8*>(this) + PokeModelLayout::BaseScaleOffset()
    )->Set(x, y, z);
    *reinterpret_cast<bool*>(
      reinterpret_cast<u8*>(this) + PokeModelLayout::UpdateFlagOffset()
    ) = true;
  }

  const gfl2::math::Vector3& GetScale() const
  {
    return *reinterpret_cast<const gfl2::math::Vector3*>(
      reinterpret_cast<const u8*>(this) + PokeModelLayout::BaseScaleOffset()
    );
  }

  f32 GetAdjustScale() const
  {
    return *reinterpret_cast<const f32*>(
      reinterpret_cast<const u8*>(this) + PokeModelLayout::AdjustScaleOffset()
    );
  }

  const PokeSettingData* GetSettingData()
  {
    return reinterpret_cast<const PokeSettingData*>(
      reinterpret_cast<const u8*>(this) + PokeModelLayout::SettingDataOffset()
    );
  }

  void SetVisible(bool visible)
  {
    typedef void (*Function)(PokeModel*, b32);
    void** vtable = *reinterpret_cast<void***>(this);
    reinterpret_cast<Function>(vtable[0x14 / 4])(
      this, visible ? 1 : 0
    );
  }
};

class PokeModelSystem
{
public:
  struct POKE_MNG_DATA
  {
    u16 dataTop;
    u8 dataNum;
    u8 flags;
  };

  struct POKE_FLG_DATA
  {
    u8 flags;
    u8 subForm;
  };

  enum
  {
    POKE_MNG_FLG_EXIST_FEMALE = 2,
    POKE_MNG_FLG_EXIST_FORM_CHANGE = 4,
    POKE_DATA_FLG_SHARE_ANIME = 1,
  };

  struct HeapOption
  {
    MODEL_ANIMETYPE animeType;
    bool useIdModel;
    bool useShadow;
    bool useColorShader;
    int allocSize;
    u8 vramPool;
    u8 m_Pad[3];

    HeapOption()
    : animeType(MODEL_ANIMETYPE_BATTLE)
    , useIdModel(false)
    , useShadow(false)
    , useColorShader(false)
    , allocSize(0)
    , vramPool(0)
    , m_Pad{0, 0, 0}
    {
    }
  };

  gfl2::fs::AsyncFileManager* GetAsyncFileManager()
  {
    return *reinterpret_cast<gfl2::fs::AsyncFileManager**>(this);
  }

  gfl2::heap::HeapBase* GetModelHeapRaw(u32 index)
  {
    const int modelCount = *reinterpret_cast<const int*>(
      reinterpret_cast<const u8*>(this) + 0x10
    );
    if (modelCount <= 0 || index >= static_cast<u32>(modelCount))
    {
      return NULL;
    }

    gfl2::heap::HeapBase** heaps =
      *reinterpret_cast<gfl2::heap::HeapBase***>(
        reinterpret_cast<u8*>(this) + 0x5c
      );
    return heaps ? heaps[index] : NULL;
  }

  const POKE_MNG_DATA* GetMngDataRaw(u32 monsNo) const
  {
    const POKE_MNG_DATA* data = *reinterpret_cast<POKE_MNG_DATA* const*>(
      reinterpret_cast<const u8*>(this) + 0x6c
    );
    return data && monsNo ? &data[monsNo - 1] : NULL;
  }

  const POKE_FLG_DATA* GetFlgDataRaw(u32 dataIndex) const
  {
    const POKE_FLG_DATA* data = *reinterpret_cast<POKE_FLG_DATA* const*>(
      reinterpret_cast<const u8*>(this) + 0x70
    );
    return data ? &data[dataIndex] : NULL;
  }

  int GetDataIdx(int monsNo, int formNo, int sex);
};

void GetSimpleParam(SimpleParam* output, const pml::pokepara::CoreParam* pokemon);
bool CompareSimpleParam(const SimpleParam& lhs, const SimpleParam& rhs);

} // namespace PokeTool

namespace RaycastCustomCallback {
struct HIT_DATA
{
  gfl2::math::Vector4 intersection;
  const void* pTriangle;
};
} // namespace RaycastCustomCallback

class BaseCollisionScene
{
public:
  bool FindNearestMeshHit(
    gfl2::math::Vector4& start,
    gfl2::math::Vector4& end,
    RaycastCustomCallback::HIT_DATA* hit
  ) asm("FollowerNative_CollisionFindNearestMeshHit");
  bool TestCategory1Segment(
    gfl2::math::Vector4& start,
    gfl2::math::Vector4& end
  ) asm("FollowerNative_CollisionTestCategory1Segment");
  bool TestCategory3Segment(
    gfl2::math::Vector4& start,
    gfl2::math::Vector4& end
  ) asm("FollowerNative_CollisionTestCategory3Segment");
};

namespace Field {
namespace MoveModel {

enum FIELD_MOVE_MODEL_ID
{
  FIELD_MOVE_MODEL_PLAYER = 0,
  FIELD_MOVE_MODEL_RIDE = 1,
  FIELD_MOVE_MODEL_NPC_START = 2,
  FIELD_MOVE_MODEL_NPC_END = 33,
  FIELD_MOVE_MODEL_EVENT = 34,
  FIELD_MOVE_MODEL_MAX = 35,
};

enum FIELD_MOVE_CODE_ID
{
  FIELD_MOVE_CODE_NONE = 0,
};

struct FieldMoveModelHeaderWork
{
  u32 eventId;
  s32 moveCodeId;
  gfl2::math::Vector3 posForReturn;
  gfl2::math::Quaternion quaForReturn;
  u16 zoneId;
  u16 padding;

  FieldMoveModelHeaderWork()
  : eventId(0xffffffffU)
  , moveCodeId(FIELD_MOVE_CODE_NONE)
  , posForReturn()
  , quaForReturn()
  , zoneId(0)
  , padding(0)
  {
  }
};

struct FieldMoveModelHeaderResource
{
  gfl2::math::Vector3 position;
  gfl2::math::Quaternion rotation;
  u32 characterId;
  void* pDressUpParam;
  BaseCollisionScene* collisionScenes[18];

  FieldMoveModelHeaderResource()
  : position()
  , rotation()
  , characterId(0)
  , pDressUpParam(NULL)
  , collisionScenes{}
  {
  }
};

static_assert(
  sizeof(FieldMoveModelHeaderWork) == 0x28,
  "Field move-model work header ABI"
  );
static_assert(
  sizeof(FieldMoveModelHeaderResource) == 0x6c,
  "Field move-model resource header ABI"
  );

class FieldMoveModel
{
public:
  u32 GetCharacterIdRaw() const
  {
    return *reinterpret_cast<const u32*>(
      reinterpret_cast<const u8*>(this) + 0x6c
    );
  }

  void* GetDressUpParamRaw() const
  {
    return *reinterpret_cast<void* const*>(
      reinterpret_cast<const u8*>(this) + 0x70
    );
  }

  poke_3d::model::BaseModel* GetCharaDrawInstanceRaw()
  {
    return *reinterpret_cast<poke_3d::model::BaseModel**>(
      reinterpret_cast<u8*>(this) + 0xf8
    );
  }
};

class FieldMoveModelManager
{
public:
  static u32 GetModelCount()
  {
    return PokeTool::PokeModelLayout::SelectOffset(32, 35);
  }

  static u32 GetLastNpcModelId()
  {
    return PokeTool::PokeModelLayout::SelectOffset(30, 33);
  }

  int InitializeMoveModelWork(
    FIELD_MOVE_MODEL_ID modelId,
    const FieldMoveModelHeaderWork* header
  );
  int InitializeMoveModelResource(
    FIELD_MOVE_MODEL_ID modelId,
    const FieldMoveModelHeaderResource* header,
    poke_3d::model::DressUpModelResourceManager* resourceManager = NULL
  );
  void TerminateMoveModelWorkResource(FIELD_MOVE_MODEL_ID modelId);
  u32 GetFieldMoveModelIndexFromFreeSpace();

  FieldMoveModel* GetFieldMoveModelRaw(FIELD_MOVE_MODEL_ID modelId)
  {
    const u32 index = static_cast<u32>(modelId);
    if (index >= GetModelCount())
    {
      return NULL;
    }
    return reinterpret_cast<FieldMoveModel**>(
      reinterpret_cast<u8*>(this) + 0x0c
    )[index];
  }

  gfl2::heap::HeapBase*& GetLocalModelHeapRaw(FIELD_MOVE_MODEL_ID modelId)
  {
    return reinterpret_cast<gfl2::heap::HeapBase**>(
      reinterpret_cast<u8*>(this) +
        PokeTool::PokeModelLayout::SelectOffset(0xd8, 0xe4)
    )[static_cast<u32>(modelId)];
  }
};

} // namespace MoveModel
} // namespace Field

namespace gfl2 {
namespace ui {

enum
{
  BUTTON_LEFT = 1 << 0,
  BUTTON_RIGHT = 1 << 1,
  BUTTON_UP = 1 << 2,
  BUTTON_DOWN = 1 << 3,
  BUTTON_A = 1 << 4,
  BUTTON_B = 1 << 5,
  BUTTON_X = 1 << 6,
  BUTTON_Y = 1 << 7,
  BUTTON_L = 1 << 8,
  BUTTON_R = 1 << 9,
  BUTTON_ZL = 1 << 10,
  BUTTON_ZR = 1 << 11,
  BUTTON_START = 1 << 12,
  BUTTON_SELECT = 1 << 13,
  BUTTON_CROSS = BUTTON_LEFT | BUTTON_RIGHT | BUTTON_UP | BUTTON_DOWN,
};

class Device
{
public:
  bool IsDeviceRunning() const
  {
    // There's no exported getter, so read the byte that the setter writes.
    return *(reinterpret_cast<const u8*>(this) + 0x1c) != 0;
  }
  void SetDeviceRunningEnable(bool enabled);
};

class VectorDevice;

class Button
{
public:
  enum InputStateID
  {
    INPUT_STATE_ASSIGNED = 0,
    INPUT_STATE_ORIGINAL,
    INPUT_STATE_NUM,
  };

  bool IsTrigger(
    u32 buttons,
    InputStateID state = INPUT_STATE_ASSIGNED
  ) const;
  bool IsHold(
    u32 buttons,
    InputStateID state = INPUT_STATE_ASSIGNED
  ) const;
};

class DeviceManager
{
public:
  typedef u8 DeviceId;
  enum STICK_ID
  {
    STICK_STANDARD = 0,
    STICK_NUM,
  };
  enum BUTTON_ID
  {
    BUTTON_STANDARD = 0,
    BUTTON_STICK_EMU,
    BUTTON_NUM,
  };

  VectorDevice* GetStick(DeviceId id) const;
  Button* GetButton(DeviceId id) const;
};

} // namespace ui
} // namespace gfl2

namespace app {
namespace util {
class G2DUtil;
} // namespace util
namespace ui {
class UIView
{
public:
  app::util::G2DUtil* GetG2DUtil() const;
};
} // namespace ui
} // namespace app

namespace print { class MessageWindow; }

namespace app {
namespace util {
class G2DUtil
{
public:
  print::MessageWindow* GetMsgWin();
};
} // namespace util
} // namespace app

namespace App {
namespace Tool {
class TalkWindow : public app::ui::UIView {};
} // namespace Tool
} // namespace App

namespace Field { class Area; }

namespace GameSys {

class GameManager;

enum GMEVENT_RESULT
{
  GMEVENT_RES_CONTINUE = 0,
  GMEVENT_RES_FINISH,
  GMEVENT_RES_OFF,
  GMEVENT_RES_CONTINUE_DIRECT = 33,
};

class GameEvent
{
public:
  GameEvent(gfl2::heap::HeapBase* heap);
  virtual ~GameEvent();
  virtual bool BootChk(GameManager* manager) = 0;
  virtual void InitFunc(GameManager* manager) = 0;
  virtual GMEVENT_RESULT MainFunc(GameManager* manager) = 0;
  virtual void EndFunc(GameManager* manager) = 0;
  virtual bool IsNormalEvent() { return true; }

  GameEvent* GetParentEventPointer() { return m_Parent; }

private:
  GameEvent* m_Parent;
  u32 m_Sequence;
  gfl2::heap::HeapBase* m_WorkHeap;
  u32 m_State;
  void* m_Module;
};

class GameEventManager
{
public:
  GameEvent* GetGameEvent()
  {
    return *reinterpret_cast<GameEvent**>(this);
  }
  bool IsExists() const;
};

class GameData
{
public:
  const pml::PokeParty* GetPlayerPartyConst() const
  {
    return *reinterpret_cast<pml::PokeParty* const*>(
      reinterpret_cast<const u8*>(this) + 0x0c
    );
  }

  Field::Area* GetFieldArea()
  {
    return *reinterpret_cast<Field::Area**>(
      reinterpret_cast<u8*>(this) + 0x50
    );
  }

  Field::MoveModel::FieldMoveModelManager* GetFieldCharaModelManagerRaw()
  {
    return *reinterpret_cast<Field::MoveModel::FieldMoveModelManager**>(
      reinterpret_cast<u8*>(this) + 0xcc
    );
  }
};

class GameManager
{
public:
  gfl2::ui::DeviceManager* GetUiDeviceManager() const;
  gfl2::fs::AsyncFileManager* GetAsyncFileManager() const;
};

} // namespace GameSys

namespace System {
namespace Skybox {

class Skybox
{
public:
  bool IsEnable() const;
};

} // namespace Skybox
} // namespace System

namespace Field {

class Area;
class MyRenderingPipeLine;

namespace Camera {

class CameraUnit
{
public:
  poke_3d::model::BaseCamera* GetBaseCamera()
  {
    return *reinterpret_cast<poke_3d::model::BaseCamera**>(
      reinterpret_cast<u8*>(this) + 0x08
    );
  }
};

class CameraManager
{
public:
  CameraUnit* GetMainGamePlayCamera()
  {
    return GetCameraByUnitNo(Read<s32>(0x04));
  }

  CameraUnit* GetMainViewCamera()
  {
    const s32 viewCamera = Read<s32>(0x08);
    return viewCamera == -1
      ? GetMainGamePlayCamera()
      : GetCameraByUnitNo(viewCamera);
  }

private:
  CameraUnit* GetCameraByUnitNo(s32 unitNo)
  {
    if (unitNo < 0 || unitNo >= 3)
    {
      return NULL;
    }
    return reinterpret_cast<CameraUnit**>(
      reinterpret_cast<u8*>(this) + 0x0c
    )[unitNo];
  }

  template <typename T>
  T Read(u32 offset)
  {
    return *reinterpret_cast<T*>(reinterpret_cast<u8*>(this) + offset);
  }
};

} // namespace Camera
namespace Effect {

enum Type
{
  EFFECT_TYPE_FES_LEVELUP = 30,
  EFFECT_TYPE_FES_SHOP_OPEN = 31,
  EFFECT_TYPE_FES_START_SPLASH = 32,
  EFFECT_TYPE_FES_WARP = 33,
  EFFECT_TYPE_FISHING_BUOY = 35,
  EFFECT_TYPE_RIDE_APPER_LAND = 36,
  EFFECT_TYPE_RIDE_APPER_SEA = 37,
  EFFECT_TYPE_B_DEMO = 41,
  EFFECT_TYPE_FESTIVAL_FIRE = 50,
  EFFECT_TYPE_KAIRIKY_ROCK_SMOKE = 52,
  EFFECT_TYPE_KAIRIKY_ROCK_DOWN = 53,
  EFFECT_TYPE_DEMO_RIDE = 64,
  EFFECT_TYPE_DEMO_TRIAL2 = 65,
  EFFECT_TYPE_DEMO_TRIAL7_1 = 67,
  EFFECT_TYPE_DEMO_TRIAL7_2 = 68,
  EFFECT_TYPE_BAG_EFFECT = 69,
  EFFECT_TYPE_DEMO_TRIAL3 = 70,
  EFFECT_TYPE_DEMO_TRIAL5 = 71,
  EFFECT_TYPE_DEMO_CONCENTRATE = 75,
  EFFECT_TYPE_DEMO_FOG = 76,
  EFFECT_TYPE_DEMO_FLOWER_YELLOW = 77,
  EFFECT_TYPE_DEMO_FLOWER_PINK = 78,
  EFFECT_TYPE_DEMO_FIREWORK_YELLOW = 79,
  EFFECT_TYPE_DEMO_FIREWORK_PINK = 80,
  EFFECT_TYPE_DEMO_FIREWORK_RED = 81,
  EFFECT_TYPE_DEMO_FIREWORK_PURPLE = 82,
  EFFECT_TYPE_DEMO_FLARE_SUN = 83,
  EFFECT_TYPE_DEMO_FLARE_MOON = 84,
  EFFECT_TYPE_EF_PH0301_DEN = 97,
  EFFECT_TYPE_DEMO_NEW_TRIAL5_01 = 98,
  EFFECT_TYPE_DEMO_NEW_TRIAL5_02 = 99,
  EFFECT_TYPE_KAIRIKY_ROCK_SMOKE_L = 100,
  EFFECT_TYPE_KAIRIKY_ROCK_DOWN_L = 101,
  EFFECT_TYPE_EF_PH0301_DEN2 = 102,
  EFFECT_TYPE_UB_KAMI_SLASH = 103,
  EFFECT_TYPE_UB_KAMI_BLACKOUT = 104,
  EFFECT_TYPE_ROTOM_POWER = 105,
  EFFECT_TYPE_DEMO_TRIAL2_2 = 107,
  EFFECT_TYPE_R_ROCKET1 = 116,
  EFFECT_TYPE_EF_BTFES_WARP = 117,
};

class IEffectBase;

class EffectManager
{
public:
  enum WorkType
  {
    WORK_TYPE_SYS = 0,
    WORK_TYPE_EVT,
    WORK_TYPE_WTR,
    WORK_TYPE_RID,
    WORK_TYPE_DEFAULT,
  };

  void LoadData(
    Type type,
    gfl2::heap::HeapBase* heap,
    b32 sync = false
  ) asm("FollowerNative_EffectsLoadData");
  void RegisterResources(
    Type type,
    gfl2::heap::HeapBase* heap
  ) asm("FollowerNative_EffectsRegisterResources");
  b32 IsDataAvailable(Type type) const asm("FollowerNative_EffectsIsDataAvailable");
  void ReleaseResources(
    Type type,
    gfl2::heap::HeapBase* heap,
    bool isTerminate = false
  ) asm("FollowerNative_EffectsReleaseResources");
  IEffectBase* CreateAtPosition(
    Type type,
    const gfl2::math::Vector3& position,
    bool playSound,
    WorkType workType,
    s32* index,
    f32 scale,
    const gfl2::math::Vector3& rotation
  ) asm("FollowerNative_EffectsCreateAtPosition");
  void RequestStop(IEffectBase* effect) asm("FollowerNative_EffectsRequestStop");
  s32 CountUnfinished(Type type) asm("FollowerNative_EffectsCountUnfinished");
  void TickAndReap() asm("FollowerNative_EffectsTickAndReap");
};

} // namespace Effect

namespace weather {

enum WeatherKind
{
  SUNNY,
  CLOUDINESS,
  RAIN,
  THUNDERSTORM,
  SNOW,
  SNOWSTORM,
  DRY,
  SANDSTORM,
  MIST,
  SUNSHOWER,
  DIAMONDDUST,
  FORCE_WEATHER_NONE = -1,
};

class WeatherControl
{
public:
  void RequestOverride(WeatherKind weatherKind) asm("FollowerNative_WeatherRequestOverride");
  void ClearOverride() asm("FollowerNative_WeatherClearOverride");

  bool HasValidRequestState() const
  {
    const u8 reserve = Read<u8>(0x2c);
    const u8 request = Read<u8>(0x2d);
    return reserve <= 1 && request <= 1;
  }

  s8 GetNowWeatherKindRaw() const
  {
    return Read<s8>(0x2f);
  }

  s8 GetForceWeatherKindRaw() const
  {
    return Read<s8>(0x30);
  }

private:
  template <typename T>
  T Read(u32 offset) const
  {
    return *reinterpret_cast<const T*>(
      reinterpret_cast<const u8*>(this) + offset
    );
  }
};

} // namespace weather

class EventWork
{
public:
  void* GetData()
  {
    return reinterpret_cast<u8*>(this) + 4;
  }
};

namespace Terrain {
class TerrainManager
{
public:
  BaseCollisionScene* GetCollsionScene()
  {
    return *reinterpret_cast<BaseCollisionScene**>(
      reinterpret_cast<u8*>(this) + 0x50
    );
  }
};
} // namespace Terrain

class Fieldmap
{
public:
  gfl2::heap::HeapBase* GetHeap()
  {
    return Read<gfl2::heap::HeapBase*>(0x5c);
  }
  GameSys::GameManager* GetGameManager()
  {
    return Read<GameSys::GameManager*>(0x6c);
  }
  Terrain::TerrainManager* GetTerrainManager()
  {
    return Read<Terrain::TerrainManager*>(0xb8);
  }
  Camera::CameraManager* GetCameraManager()
  {
    return Read<Camera::CameraManager*>(0xc0);
  }
  Effect::EffectManager* GetEffectManager()
  {
    return Read<Effect::EffectManager*>(0xd8);
  }
  System::Skybox::Skybox* GetSkybox()
  {
    return Read<System::Skybox::Skybox*>(0xec);
  }
  weather::WeatherControl* GetWeatherControl()
  {
    return Read<weather::WeatherControl*>(0x170);
  }
  const gfl2::math::Vector3& GetPlayerPosition() const
  {
    return *reinterpret_cast<const gfl2::math::Vector3*>(
      reinterpret_cast<const u8*>(this) + 0x104
    );
  }

private:
  template <typename T>
  T Read(u32 offset)
  {
    return *reinterpret_cast<T*>(reinterpret_cast<u8*>(this) + offset);
  }
};

enum RIDE_POKEMON_ID
{
  RIDE_POKEMON_ID_NONE = -1,
  RIDE_POKEMON_ID_ABI = 0,
  RIDE_POKEMON_ID_COUNT = 7,
};

class EventPokemonRideTool
{
public:
  static RIDE_POKEMON_ID GetPokemonRideOnID(GameSys::GameManager* manager);
  static bool IsNaminori(RIDE_POKEMON_ID ride);
};

namespace TrialModel {

class FieldTrialModel
{
public:
  bool IsLoadComplete() asm("FollowerNative_FieldModelIsLoadComplete");
  void CreateRenderResources() asm("FollowerNative_FieldModelCreateRenderResources");
  void ApplyStoredHeight() asm("FollowerNative_FieldModelApplyStoredHeight");
  void EnableAmbientTint(bool enable) asm("FollowerNative_FieldModelEnableAmbientTint");
  void ForwardVisibility(bool visible) asm("FollowerNative_FieldModelForwardVisibility");

  void SetShadowVisible(bool visible)
  {
    // Probably the shadow flag, but this still needs checking in the game.
    *reinterpret_cast<bool*>(
      reinterpret_cast<u8*>(this) + 0x38
    ) = visible;
  }

  PokeTool::PokeModel* GetPokeModel()
  {
    return *reinterpret_cast<PokeTool::PokeModel**>(
      reinterpret_cast<u8*>(this) + 0x20
    );
  }

  gfl2::animation::AnimationPackList* GetAnimationPackList()
  {
    return reinterpret_cast<gfl2::animation::AnimationPackList*>(
      reinterpret_cast<u8*>(this) + 0x30
    );
  }

  void ClearDrawEnvNode()
  {
    *reinterpret_cast<void**>(
      reinterpret_cast<u8*>(this) + 0x3c
    ) = NULL;
  }

private:
  u8 m_Storage[0x44];
};

extern "C" void FollowerNative_ModelPoolInitializeStorage(void*);
extern "C" void FollowerNative_ModelPoolCheckShutdown(void*);

class FieldModelPool
{
public:
  struct SetupParam
  {
    Fieldmap* pFieldmap;
    MyRenderingPipeLine* pPipeLine;
    Effect::EffectManager* pEffectManager;
    BaseCollisionScene* pColScene;
    Camera::CameraManager* pCameraManager;
    void* pTrialShadow;
    void* pFinderShadow;
  };

  FieldModelPool() { FollowerNative_ModelPoolInitializeStorage(this); }
  ~FieldModelPool() { FollowerNative_ModelPoolCheckShutdown(this); }
  void BindServices(const SetupParam& parameter) asm("FollowerNative_ModelPoolBindServices");
  void BeginLoad(
    gfl2::heap::HeapBase* systemHeap,
    gfl2::heap::HeapBase* deviceHeap,
    s32 pokemonCount,
    bool createShadow
  ) asm("FollowerNative_ModelPoolBeginLoad");
  bool IsLoadComplete() asm("FollowerNative_ModelPoolIsLoadComplete");
  void CreateResources(
    gfl2::heap::HeapBase* deviceHeap,
    gfl2::gfx::IGLAllocator* allocator,
    PokeTool::PokeModelSystem::HeapOption* heapOption,
    bool createHeapArray
  ) asm("FollowerNative_ModelPoolCreateResources");
  void BeginShutdown() asm("FollowerNative_ModelPoolBeginShutdown");
  bool IsShutdownComplete() asm("FollowerNative_ModelPoolIsShutdownComplete");
  void ReleaseStorage() asm("FollowerNative_ModelPoolReleaseStorage");
  void TickEntries() asm("FollowerNative_ModelPoolTickEntries");
  void UpdateEntryPresentation() asm("FollowerNative_ModelPoolUpdateEntryPresentation");
  FieldTrialModel* AllocateEntry(
    gfl2::heap::HeapBase* heap,
    const PokeTool::SimpleParam* parameter,
    const PokeTool::PokeModel::SetupOption& option
  ) asm("FollowerNative_ModelPoolAllocateEntry");
  void ReleaseEntry(FieldTrialModel* model) asm("FollowerNative_ModelPoolReleaseEntry");

  PokeTool::PokeModelSystem* GetPokeModelSystem()
  {
    return *reinterpret_cast<PokeTool::PokeModelSystem**>(
      reinterpret_cast<u8*>(this) + 0x00
    );
  }

  gfl2::gfx::IGLAllocator* GetAllocator()
  {
    return *reinterpret_cast<gfl2::gfx::IGLAllocator**>(
      reinterpret_cast<u8*>(this) + 0x28
    );
  }

private:
  u8 m_Storage[0x68];
};

} // namespace TrialModel

namespace FieldWindow { class FieldTalkWindow; }

namespace FieldScript {

struct SystemSingletones
{
  u8 m_Pad0[0x08];
  void* m_pYesNoWin;
  void* m_pListMenu;
  FieldWindow::FieldTalkWindow* m_pFieldTalkWindow[2];
  u8 m_Pad1[0x04];
  void* m_pProc;
};

class SystemWork
{
public:
  SystemSingletones* GetSingletones()
  {
    return reinterpret_cast<SystemSingletones*>(
      reinterpret_cast<u8*>(this) + 0x08
    );
  }
};

class FieldScriptSystem
{
public:
  SystemWork* GetSystemWork()
  {
    return *reinterpret_cast<SystemWork**>(
      reinterpret_cast<u8*>(this) + 0x20
    );
  }
};

} // namespace FieldScript
} // namespace Field

namespace gfl2 {
namespace base {
template <typename T>
class SingletonAccessor
{
public:
  static T* GetInstance();
};
} // namespace base

namespace Fade {

enum DISP
{
  DISP_UPPER = 0,
  DISP_LOWER,
  DISP_CUSTOM_UPPER,
  DISP_CUSTOM_LOWER,
  DISP_DOUBLE,
};

enum FADE_TYPE
{
  FADE_TYPE_NONE = 0,
  FADE_TYPE_ALPHA,
  FADE_TYPE_BALL,
  FADE_TYPE_MASK,
  FADE_TYPE_DIAMOND,
  FADE_TYPE_CIRCLE,
  FADE_TYPE_CROSS,
  FADE_TYPE_BALL_TILT,
};

enum FADE_RESULT
{
  FADE_RESULT_IN = 0,
  FADE_RESULT_OUT,
};

class FadeSuper
{
public:
  FadeSuper() {}
  ~FadeSuper() {}
  virtual void UpdateFunc() = 0;
  virtual void PreDrawFunc() {}
  virtual void DrawFunc(gfl2::gfx::CtrDisplayNo display) = 0;
  virtual void RequestOut(
    FADE_TYPE type,
    const gfl2::math::Vector4* start,
    const gfl2::math::Vector4* end,
    u32 sync,
    bool keepBuffer
  ) = 0;
  virtual void RequestIn(FADE_TYPE type, u32 sync) = 0;
  virtual void ForceOut(const gfl2::math::Vector4* color) = 0;
  virtual bool IsEnd() = 0;
  virtual bool IsEndStatus() = 0;
  virtual void SetPause(bool paused) = 0;
  virtual bool IsPause() const = 0;
  virtual void Reset() = 0;
  virtual void SetAnimeSpeed(f32) {}
  virtual void RequestHomeNix() {}
  virtual bool IsHomeNixRunning() { return false; }
  virtual FADE_RESULT GetFadeResult() const { return FADE_RESULT_IN; }
  virtual void GetColor(
    gfl2::math::Vector4*,
    gfl2::math::Vector4*
  ) const {}
};

class FadeManager;

} // namespace Fade
} // namespace gfl2

#define GFL_SINGLETON_INSTANCE(type) \
  gfl2::base::SingletonAccessor<type>::GetInstance()

#define FIELDTALKWINDOW_MAX 2

static_assert(sizeof(gfl2::math::Vector3) == 0x0c, "Vector3 ABI");
static_assert(
  !__is_trivially_constructible(
    gfl2::math::Vector3,
    const gfl2::math::Vector3&
  ),
  "Vector3 must retain the retail hidden-sret calling convention"
);
static_assert(sizeof(gfl2::math::Vector4) == 0x10, "Vector4 ABI");
static_assert(
  sizeof(
    gfl2::renderingengine::scenegraph::resource::MaterialResourceNode::TextureInfo
  ) == 0x80,
  "Material texture info ABI"
);
static_assert(
  offsetof(
    gfl2::renderingengine::scenegraph::resource::MaterialResourceNode::TextureInfo,
    m_Attribute
  ) == 0x50,
  "Material texture attribute ABI"
);
static_assert(
  gfl2::renderingengine::renderer::MeshDrawTag::
    MATERIAL_INSTANCE_NODE_OFFSET == 0x40,
  "MeshDrawTag material node ABI"
);
static_assert(sizeof(PokeTool::SimpleParam) == 0x0c, "SimpleParam ABI");
static_assert(offsetof(PokeTool::SimpleParam, monsNo) == 0x00, "SimpleParam species ABI");
static_assert(offsetof(PokeTool::SimpleParam, formNo) == 0x02, "SimpleParam form ABI");
static_assert(offsetof(PokeTool::SimpleParam, perRand) == 0x08, "SimpleParam random ABI");
static_assert(offsetof(PokeTool::PokeSettingData, fieldAdjustHeight) == 0x08, "Poke setting ABI");
static_assert(offsetof(PokeTool::PokeSettingData, minX) == 0x10, "Poke setting bounds ABI");
static_assert(offsetof(PokeTool::PokeSettingData, maxZ) == 0x24, "Poke setting bounds ABI");
static_assert(sizeof(PokeTool::PokeModel::SetupOption) == 0x10, "SetupOption ABI");
static_assert(offsetof(PokeTool::PokeModel::SetupOption, animeType) == 0x0c, "Setup anime ABI");
static_assert(sizeof(PokeTool::PokeModelSystem::HeapOption) == 0x0c, "HeapOption ABI");
static_assert(sizeof(PokeTool::PokeModelSystem::POKE_MNG_DATA) == 0x04, "Poke manager data ABI");
static_assert(sizeof(PokeTool::PokeModelSystem::POKE_FLG_DATA) == 0x02, "Poke flag data ABI");
static_assert(sizeof(gfl2::fs::Result) == 0x08, "File result ABI");
static_assert(sizeof(gfl2::fs::AsyncFileManager::ArcFileLoadDataReq) == 0x3c, "Arc load request ABI");
static_assert(offsetof(gfl2::fs::AsyncFileManager::ArcFileLoadDataReq, lang) == 0x0c, "Arc request language ABI");
static_assert(offsetof(gfl2::fs::AsyncFileManager::ArcFileLoadDataReq, ppBuf) == 0x10, "Arc request buffer ABI");
static_assert(offsetof(gfl2::fs::AsyncFileManager::ArcFileLoadDataReq, heapForBuf) == 0x1c, "Arc request heap ABI");
static_assert(offsetof(gfl2::fs::AsyncFileManager::ArcFileLoadDataReq, arcSrcUseSetting) == 0x2c, "Arc request source ABI");
static_assert(offsetof(gfl2::fs::AsyncFileManager::ArcFileLoadDataReq, result) == 0x30, "Arc request result ABI");
static_assert(sizeof(gfl2::animation::AnimationPackList) == 0x08, "Animation pack list ABI");
static_assert(sizeof(Field::TrialModel::FieldModelPool::SetupParam) == 0x1c, "Factory setup ABI");
static_assert(sizeof(Field::TrialModel::FieldModelPool) == 0x68, "Factory ABI");
static_assert(sizeof(RaycastCustomCallback::HIT_DATA) == 0x14, "Raycast hit ABI");
static_assert(sizeof(GameSys::GameEvent) == 0x18, "GameEvent ABI");
static_assert(sizeof(gfl2::Fade::FadeSuper) == 0x04, "FadeSuper ABI");
static_assert(
  offsetof(
    gfl2::renderingengine::scenegraph::resource::MaterialResourceNode::AttributeParam,
    m_ConstantColor
  ) == 0x19,
  "Material constant color ABI"
);
static_assert(
  offsetof(
    gfl2::renderingengine::scenegraph::resource::MaterialResourceNode::AttributeParam,
    m_LightSetNo
  ) == 0x94,
  "Material light set ABI"
);
static_assert(
  offsetof(
    gfl2::renderingengine::scenegraph::resource::MaterialResourceNode::AttributeParam,
    m_BumpMapNo
  ) == 0x96,
  "Material bump map ABI"
);
static_assert(
  offsetof(
    gfl2::renderingengine::scenegraph::resource::MaterialResourceNode::AttributeParam,
    m_PsLightingEnable
  ) == 0x9e,
  "Material pixel lighting ABI"
);
static_assert(
  offsetof(
    gfl2::renderingengine::scenegraph::resource::MaterialResourceNode::AttributeParam,
    m_FogEnable
  ) == 0xa0,
  "Material fog ABI"
);
static_assert(
  offsetof(
    gfl2::renderingengine::scenegraph::resource::MaterialResourceNode::UserData,
    m_RimScale
  ) == 0x14,
  "Material rim scale ABI"
);
static_assert(
  offsetof(
    gfl2::renderingengine::scenegraph::resource::MaterialResourceNode::UserData,
    m_PhongScale
  ) == 0x1c,
  "Material phong scale ABI"
);
static_assert(sizeof(Field::FieldScript::SystemSingletones) == 0x20, "Script singleton ABI");
static_assert(offsetof(Field::FieldScript::SystemSingletones, m_pYesNoWin) == 0x08, "Script yes/no ABI");
static_assert(offsetof(Field::FieldScript::SystemSingletones, m_pFieldTalkWindow) == 0x10, "Script talk ABI");
static_assert(offsetof(Field::FieldScript::SystemSingletones, m_pProc) == 0x1c, "Script proc ABI");
