#include "DecimalKeyboard.hpp"
#include "DecimalInput.hpp"
#include <CTRPluginFramework/Menu/Keyboard.hpp>
#include <CTRPluginFramework/Menu/MessageBox.hpp>
#include <CTRPluginFramework/Utils/Utils.hpp>

namespace Gen7Follower3gx {
namespace {
bool ValidateDecimalText(const void* input, std::string& error)
{
  float value=0;
  if (ParseDecimalInput(*static_cast<const std::string*>(input),value)) return true;
  error="Enter a decimal number.";
  return false;
}
}
bool EditDecimalValue(const std::string& title, float& value, float minimum, float maximum)
{
  using namespace CTRPluginFramework;
  std::string text=Utils::Format("%.9f",static_cast<double>(value));
  while (text.back()=='0') text.pop_back();
  if (text.back()=='.') text.pop_back();
  for (;;) {
    Keyboard keyboard(title+Utils::Format(" (%.2f to %.2f)",minimum,maximum));
    keyboard.SetStartingPage(Keyboard::QwertyKeyboardPage::SYMBOLS_PAGE1);
    keyboard.SetMaxLength(32);
    keyboard.SetCompareCallback(ValidateDecimalText);
    if (keyboard.Open(text,text)<0) return false;
    float result=0;
    if (!ParseDecimalInput(text,result) || result<minimum || result>maximum) {
      MessageBox("Enter a number within the allowed range.")();
      continue;
    }
    value=result;
    return true;
  }
}
}
