#pragma once

namespace Gen7Follower3gx
{

bool CroModuleNameEquals(void* module, const char* expectedName);

class CroModuleView
{
public:
  CroModuleView();

  bool Initialize(void* module);
  bool Initialize(void* module, const char* expectedName);
  void Reset();

  void* Module() const { return m_Module; }
  unsigned int TextBase() const { return m_TextBase; }
  unsigned int TextSize() const { return m_TextSize; }

private:
  void* m_Module;
  unsigned int m_TextBase;
  unsigned int m_TextSize;
};

} // namespace Gen7Follower3gx
