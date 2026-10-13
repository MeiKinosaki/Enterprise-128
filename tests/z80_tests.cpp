#include "cpu/z80.h"

#include <cassert>
#include <cstdint>
#include <iostream>

using enterprise128::cpu::Z80;

static void basicLoadStoreAndFlags() {
  Z80 cpu;
  cpu.reset();
  const std::uint8_t program[] = {0x3E, 0x2A, 0x3C, 0x32, 0x00, 0x20, 0xAF, 0x76};
  cpu.load(0x0000, program, sizeof(program));
  assert(cpu.step() == 7 && cpu.a == 0x2A);
  assert(cpu.step() == 4 && cpu.a == 0x2B);
  assert(cpu.step() == 13 && cpu.memory[0x2000] == 0x2B);
  assert(cpu.step() == 4 && cpu.a == 0 && (cpu.f & Z80::FlagZ));
  assert(cpu.step() == 4 && cpu.halted);
  assert(cpu.step() == 0);
  assert(cpu.cycles == 32);
}

static void pairsStackCallAndReturn() {
  Z80 cpu;
  cpu.reset();
  // LD BC,1234h; PUSH BC; LD BC,0; POP BC; CALL 000Ah; HALT; subroutine: LD A,55h; RET
  const std::uint8_t program[] = {
    0x01, 0x34, 0x12, 0xC5, 0x01, 0x00, 0x00, 0xC1,
    0xCD, 0x0B, 0x00, 0x76, 0x3E, 0x55, 0xC9
  };
  cpu.load(0, program, sizeof(program));
  assert(cpu.step() == 10 && cpu.b == 0x12 && cpu.c == 0x34);
  assert(cpu.step() == 11);
  assert(cpu.step() == 10 && cpu.b == 0 && cpu.c == 0);
  assert(cpu.step() == 10 && cpu.b == 0x12 && cpu.c == 0x34);
  assert(cpu.step() == 17 && cpu.pc == 0x000B);
  assert(cpu.step() == 4 && cpu.halted);
  // Reset to run subroutine path directly.
  cpu.reset();
  cpu.pc = 0x000C;
  cpu.memory[0x000C] = 0x3E;
  cpu.memory[0x000D] = 0x55;
  cpu.memory[0x000E] = 0xC9;
  cpu.memory[0x1234] = 0;
  assert(cpu.step() == 7 && cpu.a == 0x55);
  // RET target is initialized stack memory; deterministic check of stack behavior
  cpu.memory[0xFFFE] = 0x34;
  cpu.memory[0xFFFF] = 0x12;
  cpu.sp = 0xFFFE;
  cpu.pc = 0x000E;
  assert(cpu.step() == 10 && cpu.pc == 0x1234);
}

static void branchAndUnsupportedOpcode() {
  Z80 cpu;
  cpu.reset();
  const std::uint8_t program[] = {0x06, 0x02, 0x10, 0xFE, 0x76};
  cpu.load(0, program, sizeof(program));
  assert(cpu.step() == 7 && cpu.b == 2);
  assert(cpu.step() == 13 && cpu.b == 1 && cpu.pc == 2);
  assert(cpu.step() == 8 && cpu.b == 0 && cpu.pc == 4);
  assert(cpu.step() == 4 && cpu.halted);

  Z80 unsupported;
  unsupported.memory[0] = 0xFF;
  unsupported.step();
  assert(unsupported.halted && unsupported.unsupportedOpcode == 0xFF);
}

int main() {
  basicLoadStoreAndFlags();
  pairsStackCallAndReturn();
  branchAndUnsupportedOpcode();
  std::cout << "Z80 regression tests passed\n";
  return 0;
}
