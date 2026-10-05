#pragma once

namespace Gen7Follower3gx {
template <typename Attributes, typename Color>
class FieldLightingOverride
{
public:
  void Apply(Attributes* attributes)
  {
    if (!attributes || m_Attributes) return;
    m_Attributes=attributes;
    m_LightSet=attributes->m_LightSetNo;
    m_Tint=attributes->m_ConstantColor[5];
    attributes->m_LightSetNo=0;
    attributes->m_ConstantColor[5]=Color{255,255,255,255};
  }
  void Restore()
  {
    if (!m_Attributes) return;
    m_Attributes->m_LightSetNo=m_LightSet;
    m_Attributes->m_ConstantColor[5]=m_Tint;
    m_Attributes=nullptr;
  }
private:
  Attributes* m_Attributes=nullptr;
  signed char m_LightSet=0;
  Color m_Tint{};
};
}
