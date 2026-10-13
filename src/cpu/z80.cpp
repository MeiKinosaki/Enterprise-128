#include "cpu/z80.h"

#include <algorithm>

namespace enterprise128::cpu {

void Z80::reset() {
  a = f = b = c = d = e = h = l = 0;
  sp = 0xFFFF;
  pc = 0;
  halted = false;
  unsupportedOpcode = 0;
  cycles = 0;
}

void Z80::load(std::uint16_t address, const std::uint8_t* data, std::uint32_t size) {
  if (data == nullptr) return;
  const auto available = static_cast<std::uint32_t>(memory.size() - address);
  const auto count = std::min(size, available);
  std::copy(data, data + count, memory.begin() + address);
}

std::uint32_t Z80::step() {
  if (halted) return 0;
  const std::uint8_t op = memory[pc++];
  std::uint32_t used = 0;
  switch (op) {
    case 0x00: // NOP
      used = 4;
      break;
    case 0x3E: // LD A,n
      a = memory[pc++]; used = 7;
      break;
    case 0x06: // LD B,n
      b = memory[pc++]; used = 7;
      break;
    case 0x0E: // LD C,n
      c = memory[pc++]; used = 7;
      break;
    case 0x32: { // LD (nn),A
      const std::uint16_t lo = memory[pc++];
      const std::uint16_t hi = memory[pc++];
      memory[static_cast<std::uint16_t>(lo | (hi << 8))] = a;
      used = 13;
      break;
    }
    case 0x3A: { // LD A,(nn)
      const std::uint16_t lo = memory[pc++];
      const std::uint16_t hi = memory[pc++];
      a = memory[static_cast<std::uint16_t>(lo | (hi << 8))];
      used = 13;
      break;
    }
    case 0x3C: // INC A (carry preserved; S/Z/H/PV are not yet complete)
      ++a;
      f = static_cast<std::uint8_t>((f & 0x01) | (a == 0 ? FlagZ : 0));
      used = 4;
      break;
    case 0x3D: // DEC A
      --a;
      f = static_cast<std::uint8_t>((f & 0x01) | FlagN | (a == 0 ? FlagZ : 0));
      used = 4;
      break;
    case 0xAF: // XOR A
      a = 0; f = FlagZ; used = 4;
      break;
    case 0xC3: { // JP nn
      const std::uint16_t lo = memory[pc++];
      const std::uint16_t hi = memory[pc++];
      pc = static_cast<std::uint16_t>(lo | (hi << 8));
      used = 10;
      break;
    }
    case 0xFE: { // CP n (minimal Z/N result flags)
      const std::uint8_t value = memory[pc++];
      const std::uint8_t result = static_cast<std::uint8_t>(a - value);
      f = static_cast<std::uint8_t>((result == 0 ? FlagZ : 0) | (a < value ? 0x01 : 0) | FlagN);
      used = 7;
      break;
    }
    case 0x76: // HALT
      halted = true; used = 4;
      break;
    default:
      unsupportedOpcode = op;
      halted = true;
      used = 0;
      break;
  }
  cycles += used;
  return used;
}

}  // namespace enterprise128::cpu
