#include "cpu/z80.h"

#include <cassert>
#include <cstdint>
#include <iostream>

int main() {
  using enterprise128::cpu::Z80;
  Z80 cpu;
  cpu.reset();
  const std::uint8_t program[] = {0x3E, 0x2A, 0x3C, 0x32, 0x00, 0x20, 0xAF, 0x76};
  cpu.load(0x0000, program, sizeof(program));

  assert(cpu.step() == 7 && cpu.a == 0x2A);  // LD A,42
  assert(cpu.step() == 4 && cpu.a == 0x2B);  // INC A
  assert(cpu.step() == 13 && cpu.memory[0x2000] == 0x2B); // LD (2000),A
  assert(cpu.step() == 4 && cpu.a == 0 && (cpu.f & Z80::FlagZ)); // XOR A
  assert(cpu.step() == 4 && cpu.halted); // HALT
  assert(cpu.step() == 0); // stable after HALT
  assert(cpu.cycles == 32);

  Z80 unsupported;
  unsupported.memory[0] = 0xFF;
  unsupported.step();
  assert(unsupported.halted && unsupported.unsupportedOpcode == 0xFF);

  std::cout << "Z80 basic tests passed\n";
  return 0;
}
