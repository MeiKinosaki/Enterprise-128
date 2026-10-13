#pragma once

#include <array>
#include <cstdint>

namespace enterprise128::machine {

// Development memory abstraction: eight 16 KiB RAM banks (128 KiB total)
// exposed through four 16 KiB CPU windows. This is a bring-up model, not yet
// the complete Enterprise EXOS memory-controller behavior.
class Memory {
 public:
  static constexpr std::size_t kBankSize = 16u * 1024u;
  static constexpr std::size_t kBankCount = 8;
  static constexpr std::size_t kAddressSpace = 65536;

  Memory();
  void reset();
  void mapWindow(std::uint8_t window, std::uint8_t ramBank);
  std::uint8_t read(std::uint16_t address) const;
  void write(std::uint16_t address, std::uint8_t value);
  std::uint8_t mappedBank(std::uint8_t window) const;

 private:
  std::array<std::array<std::uint8_t, kBankSize>, kBankCount> ram_{};
  std::array<std::uint8_t, 4> windows_{{0, 1, 2, 3}};
};

}  // namespace enterprise128::machine
