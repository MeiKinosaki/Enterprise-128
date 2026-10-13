#include "input/controller.h"
namespace enterprise128::input {
void Controller::reset() { buttons_.fill(false); }
void Controller::set(Button button, bool pressedValue) {
  if (button < ButtonCount) buttons_[button] = pressedValue;
}
bool Controller::pressed(Button button) const {
  return button < ButtonCount ? buttons_[button] : false;
}
}  // namespace enterprise128::input
