#include "machine/memory.h"

namespace enterprise128::machine {

Memory::Memory() { reset(); }

void Memory::reset() {
  for (auto& bank : ram_) bank.fill(0);
  windows_ = {{0, 1, 2, 3}};
}

void Memory::mapWindow(std::uint8_t window, std::uint8_t ramBank) {
  if (window < windows_.size() && ramBank < kBankCount) windows_[window] = ramBank;
}

std::uint8_t Memory::read(std::uint16_t address) const {
  const auto window = static_cast<std::uint8_t>(address / kBankSize);
  const auto offset = static_cast<std::size_t>(address % kBankSize);
  return ram_[windows_[window]][offset];
}

void Memory::write(std::uint16_t address, std::uint8_t value) {
  const auto window = static_cast<std::uint8_t>(address / kBankSize);
  const auto offset = static_cast<std::size_t>(address % kBankSize);
  ram_[windows_[window]][offset] = value;
}

std::uint8_t Memory::mappedBank(std::uint8_t window) const {
  return window < windows_.size() ? windows_[window] : 0xFF;
}

}  // namespace enterprise128::machine
