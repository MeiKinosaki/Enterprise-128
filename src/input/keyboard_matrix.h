#pragma once

#include <array>
#include <cstdint>

namespace enterprise128::input {

// Generic active-low 8x8 keyboard matrix scaffold.
// Row/column assignments are intentionally machine-neutral until verified
// against the Enterprise 128 keyboard wiring.
class KeyboardMatrix {
 public:
  static constexpr std::uint8_t RowCount = 8;
  static constexpr std::uint8_t ColumnCount = 8;

  void reset();
  void setKey(std::uint8_t row, std::uint8_t column, bool pressed);
  bool keyPressed(std::uint8_t row, std::uint8_t column) const;
  std::uint8_t readRow(std::uint8_t row) const;

 private:
  std::array<std::uint8_t, RowCount> rows_{};
};

}  // namespace enterprise128::input
