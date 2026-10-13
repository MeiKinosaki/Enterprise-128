#pragma once

#include <array>
#include <cstdint>

namespace enterprise128::cpu {

// Deliberately small first-pass Z80 core. Unsupported opcodes stop execution
// and are reported through halted/unsupportedOpcode for deterministic tests.
struct Z80 {
  std::uint8_t a = 0, f = 0;
  std::uint8_t b = 0, c = 0, d = 0, e = 0, h = 0, l = 0;
  std::uint16_t sp = 0xFFFF, pc = 0;
  std::array<std::uint8_t, 65536> memory{};
  bool halted = false;
  std::uint8_t unsupportedOpcode = 0;
  std::uint64_t cycles = 0;

  void reset();
  std::uint32_t step();
  void load(std::uint16_t address, const std::uint8_t* data, std::uint32_t size);

  static constexpr std::uint8_t FlagZ = 0x40;
  static constexpr std::uint8_t FlagN = 0x02;
};

}  // namespace enterprise128::cpu
