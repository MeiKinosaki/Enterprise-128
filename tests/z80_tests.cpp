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

static void conditionalJumpAndReturn() {
  Z80 cpu;
  cpu.reset();
  // LD A,1; OR A; JP NZ,0008h; HALT; target: CALL 000Ch; HALT; RET NZ
  const std::uint8_t program[] = {
      0x3E, 0x01, 0xB7, 0xC2, 0x08, 0x00, 0x76, 0x00,
      0xCD, 0x0C, 0x00, 0x76, 0xC0};
  cpu.load(0, program, sizeof(program));
  assert(cpu.step() == 7);
  assert(cpu.step() == 4 && (cpu.f & Z80::FlagZ) == 0);
  assert(cpu.step() == 10 && cpu.pc == 8);
  assert(cpu.step() == 17 && cpu.pc == 12);
  assert(cpu.step() == 11 && cpu.pc == 11); // RET NZ, condition true
  assert(cpu.step() == 4 && cpu.halted);

  Z80 retNotTaken;
  retNotTaken.reset();
  const std::uint8_t retProgram[] = {0x3E, 0x00, 0xB7, 0xC0, 0x76};
  retNotTaken.load(0, retProgram, sizeof(retProgram));
  assert(retNotTaken.step() == 7);
  assert(retNotTaken.step() == 4 && (retNotTaken.f & Z80::FlagZ));
  assert(retNotTaken.step() == 5 && retNotTaken.pc == 4);
  assert(retNotTaken.step() == 4 && retNotTaken.halted);
}

static void extendedSixteenBitArithmetic() {
  Z80 cpu;
  cpu.reset();
  // LD HL,7FFFh; LD BC,0001h; SCF; ADC HL,BC -> 8001h
  const std::uint8_t program[] = {
      0x21, 0xFF, 0x7F, 0x01, 0x01, 0x00, 0x37, 0xED, 0x4A, 0x76};
  cpu.load(0, program, sizeof(program));
  assert(cpu.step() == 10 && cpu.h == 0x7F && cpu.l == 0xFF);
  assert(cpu.step() == 10 && cpu.b == 0 && cpu.c == 1);
  assert(cpu.step() == 4 && (cpu.f & Z80::FlagC));
  assert(cpu.step() == 15 && cpu.h == 0x80 && cpu.l == 0x01);
  assert((cpu.f & (Z80::FlagS | Z80::FlagH | Z80::FlagPV)) ==
         (Z80::FlagS | Z80::FlagH | Z80::FlagPV));
  assert(cpu.step() == 4 && cpu.halted);

  Z80 sub;
  sub.reset();
  // HL=0000h; BC=0000h; SCF; SBC HL,BC -> FFFFh
  const std::uint8_t subtract[] = {
      0x21, 0x00, 0x00, 0x01, 0x00, 0x00, 0x37, 0xED, 0x42, 0x76};
  sub.load(0, subtract, sizeof(subtract));
  assert(sub.step() == 10);
  assert(sub.step() == 10);
  assert(sub.step() == 4 && (sub.f & Z80::FlagC));
  assert(sub.step() == 15 && sub.h == 0xFF && sub.l == 0xFF);
  assert((sub.f & (Z80::FlagN | Z80::FlagC | Z80::FlagS)) ==
         (Z80::FlagN | Z80::FlagC | Z80::FlagS));
}

static void sixteenBitTransfersAndExchanges() {
  Z80 cpu;
  cpu.reset();
  // LD HL,1234h; LD DE,ABCDh; EX DE,HL; LD SP,HL; JP (HL)
  const std::uint8_t program[] = {
      0x21, 0x34, 0x12, 0x11, 0xCD, 0xAB, 0xEB, 0xF9, 0xE9};
  cpu.load(0, program, sizeof(program));
  assert(cpu.step() == 10 && cpu.h == 0x12 && cpu.l == 0x34);
  assert(cpu.step() == 10 && cpu.d == 0xAB && cpu.e == 0xCD);
  assert(cpu.step() == 4 && cpu.h == 0xAB && cpu.l == 0xCD &&
         cpu.d == 0x12 && cpu.e == 0x34);
  assert(cpu.step() == 6 && cpu.sp == 0xABCD);
  assert(cpu.step() == 4 && cpu.pc == 0xABCD);

  Z80 exchange;
  exchange.reset();
  exchange.sp = 0x2000;
  exchange.h = 0x12; exchange.l = 0x34;
  exchange.memory[0x2000] = 0x78;
  exchange.memory[0x2001] = 0x56;
  exchange.memory[0] = 0xE3;
  assert(exchange.step() == 19);
  assert(exchange.h == 0x56 && exchange.l == 0x78);
  assert(exchange.memory[0x2000] == 0x34 && exchange.memory[0x2001] == 0x12);
}

static void accumulatorRotateComplementAndCarryFlags() {
  Z80 cpu;
  cpu.reset();
  // LD A,81h; RLA; RRA; CPL; CCF; SCF; HALT
  const std::uint8_t program[] = {
      0x3E, 0x81, 0x17, 0x1F, 0x2F, 0x3F, 0x37, 0x76};
  cpu.load(0, program, sizeof(program));
  assert(cpu.step() == 7 && cpu.a == 0x81);
  assert(cpu.step() == 4 && cpu.a == 0x02 && (cpu.f & Z80::FlagC));
  assert(cpu.step() == 4 && cpu.a == 0x81 && (cpu.f & Z80::FlagC) == 0);
  assert(cpu.step() == 4 && cpu.a == 0x7E);
  assert((cpu.f & (Z80::FlagH | Z80::FlagN)) == (Z80::FlagH | Z80::FlagN));
  assert(cpu.step() == 4 && (cpu.f & Z80::FlagC));
  assert(cpu.step() == 4 && (cpu.f & Z80::FlagC));
  assert(cpu.step() == 4 && cpu.halted);
}

static void blockCopyInstructions() {
  Z80 cpu;
  cpu.reset();
  // LD HL,2000h; LD DE,2100h; LD BC,0002h; LDIR; HALT
  const std::uint8_t program[] = {
      0x21, 0x00, 0x20, 0x11, 0x00, 0x21, 0x01, 0x02, 0x00,
      0xED, 0xB0, 0x76};
  cpu.load(0, program, sizeof(program));
  cpu.memory[0x2000] = 0x41;
  cpu.memory[0x2001] = 0x42;
  assert(cpu.step() == 10);
  assert(cpu.step() == 10);
  assert(cpu.step() == 10);
  // LDIR copies one byte per iteration; the first iteration repeats.
  assert(cpu.step() == 21 && cpu.pc == 9);
  assert(cpu.memory[0x2100] == 0x41 && cpu.memory[0x2101] == 0x00);
  assert(cpu.h == 0x20 && cpu.l == 0x01 && cpu.d == 0x21 && cpu.e == 0x01);
  assert(cpu.b == 0 && cpu.c == 1 && (cpu.f & Z80::FlagPV) != 0);
  // Final iteration copies the second byte and continues past the ED opcode.
  assert(cpu.step() == 16 && cpu.pc == 11);
  assert(cpu.memory[0x2101] == 0x42);
  assert(cpu.h == 0x20 && cpu.l == 0x02 && cpu.d == 0x21 && cpu.e == 0x02);
  assert(cpu.b == 0 && cpu.c == 0 && (cpu.f & Z80::FlagPV) == 0);
  assert(cpu.step() == 4 && cpu.halted);
}

static void cbRotateBitResetAndSet() {
  Z80 cpu;
  cpu.reset();
  // LD B,81h; RLC B; BIT 7,B; RES 0,B; SET 0,B; HALT
  const std::uint8_t program[] = {
      0x06, 0x81, 0xCB, 0x00, 0xCB, 0x78, 0xCB, 0x80, 0xCB, 0xC0, 0x76};
  cpu.load(0, program, sizeof(program));
  assert(cpu.step() == 7 && cpu.b == 0x81);
  assert(cpu.step() == 8 && cpu.b == 0x03 && (cpu.f & Z80::FlagC));
  assert(cpu.step() == 8 && (cpu.f & (Z80::FlagZ | Z80::FlagPV)) ==
         (Z80::FlagZ | Z80::FlagPV) && (cpu.f & Z80::FlagH));
  assert(cpu.step() == 8 && cpu.b == 0x02);
  assert(cpu.step() == 8 && cpu.b == 0x03);
  assert(cpu.step() == 4 && cpu.halted);

  Z80 memory;
  memory.reset();
  memory.h = 0x20; memory.l = 0x00;
  memory.memory[0] = 0xCB; memory.memory[1] = 0x06; // RLC (HL)
  memory.memory[0x2000] = 0x80;
  assert(memory.step() == 15 && memory.memory[0x2000] == 0x01);
  assert((memory.f & Z80::FlagC) != 0);
}

static void immediatePortIoCallbacks() {
  Z80 cpu;
  cpu.reset();
  std::uint16_t writtenPort = 0;
  std::uint8_t writtenValue = 0;
  std::uint16_t readPort = 0;
  cpu.setIoBus(
      [&](std::uint16_t port) { readPort = port; return static_cast<std::uint8_t>(0x5A); },
      [&](std::uint16_t port, std::uint8_t value) { writtenPort = port; writtenValue = value; });
  // LD A,12h; OUT (34h),A; LD A,56h; IN A,(78h); HALT
  const std::uint8_t program[] = {
      0x3E, 0x12, 0xD3, 0x34, 0x3E, 0x56, 0xDB, 0x78, 0x76};
  cpu.load(0, program, sizeof(program));
  assert(cpu.step() == 7 && cpu.a == 0x12);
  assert(cpu.step() == 11 && writtenPort == 0x1234 && writtenValue == 0x12);
  assert(cpu.step() == 7 && cpu.a == 0x56);
  assert(cpu.step() == 11 && readPort == 0x5678 && cpu.a == 0x5A);
  assert(cpu.step() == 4 && cpu.halted);
}

static void registerIndirectPortIo() {
  Z80 cpu;
  cpu.reset();
  std::uint16_t seenPort = 0;
  std::uint8_t seenValue = 0xFF;
  cpu.setIoBus(
      [&](std::uint16_t port) {
        seenPort = port;
        return static_cast<std::uint8_t>(0xA5);
      },
      [&](std::uint16_t port, std::uint8_t value) {
        seenPort = port;
        seenValue = value;
      });
  // BC=1234h; IN D,(C); OUT (C),D; IN (C); OUT (C),0; HALT
  const std::uint8_t program[] = {
      0x01, 0x34, 0x12, 0xED, 0x50, 0xED, 0x51,
      0xED, 0x70, 0xED, 0x71, 0x76};
  cpu.load(0, program, sizeof(program));
  assert(cpu.step() == 10 && cpu.b == 0x12 && cpu.c == 0x34);
  assert(cpu.step() == 12 && cpu.d == 0xA5 && seenPort == 0x1234);
  assert((cpu.f & (Z80::FlagS | Z80::FlagPV | Z80::FlagH)) ==
         (Z80::FlagS | Z80::FlagPV | Z80::FlagH));
  assert(cpu.step() == 12 && seenPort == 0x1234 && seenValue == 0xA5);
  assert(cpu.step() == 12 && seenPort == 0x1234); // IN (C) discards data but updates flags
  assert(cpu.step() == 12 && seenValue == 0); // undocumented OUT (C),0
  assert(cpu.step() == 4 && cpu.halted);
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
  conditionalJumpAndReturn();
  extendedSixteenBitArithmetic();
  sixteenBitTransfersAndExchanges();
  accumulatorRotateComplementAndCarryFlags();
  blockCopyInstructions();
  cbRotateBitResetAndSet();
  immediatePortIoCallbacks();
  registerIndirectPortIo();
  aluRegisterOperationsAndFlags();
  std::cout << "Z80 regression tests passed\n";
  return 0;
}
