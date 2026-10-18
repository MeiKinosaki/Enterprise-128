#include "input/keyboard_matrix.h"

namespace enterprise128::input {

void KeyboardMatrix::reset() {
  rows_.fill(0xFF);
}

void KeyboardMatrix::setKey(std::uint8_t row, std::uint8_t column, bool pressed) {
  if (row >= RowCount || column >= ColumnCount) return;
  const auto mask = static_cast<std::uint8_t>(1u << column);
  if (pressed) rows_[row] = static_cast<std::uint8_t>(rows_[row] & ~mask);
  else rows_[row] = static_cast<std::uint8_t>(rows_[row] | mask);
}

bool KeyboardMatrix::keyPressed(std::uint8_t row, std::uint8_t column) const {
  if (row >= RowCount || column >= ColumnCount) return false;
  return (rows_[row] & static_cast<std::uint8_t>(1u << column)) == 0;
}

std::uint8_t KeyboardMatrix::readRow(std::uint8_t row) const {
  return row < RowCount ? rows_[row] : 0xFF;
}

}  // namespace enterprise128::input
