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
  // LD BC,1234h; PUSH BC; LD BC,0; POP BC; CALL 000Ch; HALT; LD A,55h; RET
  const std::uint8_t program[] = {
    0x01, 0x34, 0x12, 0xC5, 0x01, 0x00, 0x00, 0xC1,
    0xCD, 0x0C, 0x00, 0x76, 0x3E, 0x55, 0xC9
  };
  cpu.load(0, program, sizeof(program));
  assert(cpu.step() == 10 && cpu.b == 0x12 && cpu.c == 0x34);
  assert(cpu.step() == 11);
  assert(cpu.step() == 10 && cpu.b == 0 && cpu.c == 0);
  assert(cpu.step() == 10 && cpu.b == 0x12 && cpu.c == 0x34);
  assert(cpu.step() == 17 && cpu.pc == 0x000C);
  assert(cpu.step() == 7 && cpu.a == 0x55);
  assert(cpu.step() == 10 && cpu.pc == 0x000B);
  assert(cpu.step() == 4 && cpu.halted);
}

static void aluRegisterOperationsAndFlags() {
  Z80 cpu;
  cpu.reset();
  // LD A,7Fh; LD B,01h; ADD A,B; SUB B; CP 7Fh; XOR A
  const std::uint8_t program[] = {
      0x3E, 0x7F, 0x06, 0x01, 0x80, 0x90, 0xFE, 0x7F, 0xAF, 0x76};
  cpu.load(0, program, sizeof(program));
  assert(cpu.step() == 7 && cpu.a == 0x7F);
  assert(cpu.step() == 7 && cpu.b == 0x01);
  assert(cpu.step() == 4 && cpu.a == 0x80);
  assert((cpu.f & (Z80::FlagS | Z80::FlagH | Z80::FlagPV)) ==
         (Z80::FlagS | Z80::FlagH | Z80::FlagPV));
  assert(cpu.step() == 4 && cpu.a == 0x7F);
  assert((cpu.f & (Z80::FlagN | Z80::FlagH | Z80::FlagPV)) ==
         (Z80::FlagN | Z80::FlagH | Z80::FlagPV));
  assert(cpu.step() == 7 && cpu.a == 0x7F);
  assert((cpu.f & (Z80::FlagZ | Z80::FlagN)) ==
         (Z80::FlagZ | Z80::FlagN));
  assert(cpu.step() == 4 && cpu.a == 0 && (cpu.f & Z80::FlagZ));
  assert(cpu.step() == 4 && cpu.halted);
}

static void conditionalJumpCallAndReturn() {
  Z80 cpu;
  cpu.reset();
  // LD A,0; OR A; CALL Z,0009h; HALT; target: LD A,2Ah; RET
  const std::uint8_t program[] = {
      0x3E, 0x00, 0xB7, 0xCC, 0x09, 0x00, 0x76, 0x00, 0x00, 0x3E, 0x2A, 0xC9};
  cpu.load(0, program, sizeof(program));
  assert(cpu.step() == 7 && cpu.a == 0);
  assert(cpu.step() == 4 && (cpu.f & Z80::FlagZ));
  assert(cpu.step() == 17 && cpu.pc == 9);
  assert(cpu.step() == 7 && cpu.a == 0x2A);
  assert(cpu.step() == 10 && cpu.pc == 6);
  assert(cpu.step() == 4 && cpu.halted);

  Z80 notTaken;
  notTaken.reset();
  const std::uint8_t branch[] = {0x3E, 0x01, 0xB7, 0xCC, 0x08, 0x00, 0x76};
  notTaken.load(0, branch, sizeof(branch));
  assert(notTaken.step() == 7);
  assert(notTaken.step() == 4 && (notTaken.f & Z80::FlagZ) == 0);
  assert(notTaken.step() == 10 && notTaken.pc == 6);
  assert(notTaken.step() == 4 && notTaken.halted);
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
  conditionalJumpCallAndReturn();
  aluRegisterOperationsAndFlags();
  std::cout << "Z80 regression tests passed\n";
  return 0;
}
