#pragma once
#include <cerrno>
#include <cmath>
#include <cstdlib>
#include <string>

namespace Gen7Follower3gx {
inline bool ParseDecimalInput(const std::string& input, float& value)
{
  bool digit=false, point=false;
  for (std::size_t i=0; i<input.size(); ++i) {
    const char character=input[i];
    if (i==0 && (character=='-' || character=='+')) continue;
    if (character=='.' && !point) { point=true; continue; }
    if (character<'0' || character>'9') return false;
    digit=true;
  }
  if (!digit) return false;
  errno=0;
  char* end=nullptr;
  const float parsed=std::strtof(input.c_str(),&end);
  if (errno==ERANGE || end!=input.c_str()+input.size() || !std::isfinite(parsed)) return false;
  value=parsed;
  return true;
}
}
