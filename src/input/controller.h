#pragma once
#include <array>
#include <cstdint>
namespace enterprise128::input {
class Controller {
 public:
  enum Button : std::uint8_t { Up, Down, Left, Right, Fire1, Fire2, ButtonCount };
  void reset();
  void set(Button button, bool pressed);
  bool pressed(Button button) const;
 private:
  std::array<bool, ButtonCount> buttons_{};
};
}  // namespace enterprise128::input
