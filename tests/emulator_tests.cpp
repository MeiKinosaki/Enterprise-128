#include "machine/emulator.h"

#include <cassert>
#include <cstdint>
#include <iostream>

static void testMappedInstructionAndDataBus() {
  enterprise128::machine::Emulator emulator;
  emulator.reset();
  // LD A,10h; INC A; LD (4000h),A; HALT
  const std::uint8_t program[] = {0x3E, 0x10, 0x3C, 0x32, 0x00, 0x40, 0x76};
  assert(emulator.loadProgram(0x0000, program, sizeof(program)));
  assert(!emulator.loadProgram(0xFFFF, program, sizeof(program)));
  assert(emulator.runInstructions(20) == 4);
  assert(emulator.cpu().halted && emulator.cpu().a == 0x11);
  assert(emulator.memory().read(0x4000) == 0x11);

  emulator.memory().mapWindow(0, 4);
  assert(emulator.memory().read(0x0000) == 0);
  // Fetch must now see bank 4's zero byte (NOP), not the LD A,n in bank 0.
  emulator.cpu().halted = false;
  emulator.cpu().pc = 0;
  const auto oldA = emulator.cpu().a;
  assert(emulator.stepInstruction() == 4);
  assert(emulator.cpu().pc == 1 && emulator.cpu().a == oldA);
  emulator.memory().mapWindow(0, 0);
  assert(emulator.memory().read(0x0000) == 0x3E);
}

static void testStackUsesMappedBus() {
  enterprise128::machine::Emulator emulator;
  emulator.reset();
  // LD A,22h; CALL 0008h; HALT; padding; INC A; RET
  const std::uint8_t program[] = {
      0x3E, 0x22, 0xCD, 0x08, 0x00, 0x76, 0x00, 0x00, 0x3C, 0xC9};
  assert(emulator.loadProgram(0, program, sizeof(program)));
  assert(emulator.runInstructions(10) == 5);
  assert(emulator.cpu().halted && emulator.cpu().a == 0x23);
  assert(emulator.cpu().pc == 6);
  // CALL pushed the return address 0005h through the mapped stack window.
  assert(emulator.memory().read(0xFFFD) == 0x05);
  assert(emulator.memory().read(0xFFFE) == 0x00);
}

static void testIntegratedPortBusSurvivesReset() {
  enterprise128::machine::Emulator emulator;
  std::uint16_t lastReadPort = 0;
  std::uint16_t lastWritePort = 0;
  std::uint8_t lastWrittenValue = 0;
  emulator.setPortBus(
      [&](std::uint16_t port) {
        lastReadPort = port;
        return static_cast<std::uint8_t>(0xA6);
      },
      [&](std::uint16_t port, std::uint8_t value) {
        lastWritePort = port;
        lastWrittenValue = value;
      });
  emulator.reset();
  // LD A,12h; OUT (34h),A; IN A,(78h); HALT
  const std::uint8_t program[] = {0x3E, 0x12, 0xD3, 0x34, 0xDB, 0x78, 0x76};
  assert(emulator.loadProgram(0, program, sizeof(program)));
  assert(emulator.runInstructions(10) == 4);
  assert(lastWritePort == 0x1234 && lastWrittenValue == 0x12);
  assert(lastReadPort == 0x1278); // IN's high port byte is the pre-read A value
  assert(emulator.cpu().a == 0xA6 && emulator.cpu().halted);
}

static void testSubsystemResetAndSmoke() {
  enterprise128::machine::Emulator emulator;
  emulator.reset();
  emulator.video().setPixel(3, 4, 6);
  assert(emulator.video().pixel(3, 4) == 6);
  emulator.audio().writeRegister(2, 0x44);
  assert(emulator.audio().readRegister(2) == 0x44);
  emulator.controller().set(enterprise128::input::Controller::Fire1, true);
  emulator.keyboard().setKey(1, 2, true);
  assert(emulator.controller().pressed(enterprise128::input::Controller::Fire1));
  assert(emulator.keyboard().keyPressed(1, 2));
  const std::uint8_t tapeBytes[] = {0x10, 0x20};
  assert(emulator.tape().loadBytes(tapeBytes, sizeof(tapeBytes)));
  assert(emulator.tape().bytes().size() == 2);
  emulator.reset();
  assert(emulator.cpu().pc == 0 && !emulator.cpu().halted);
  assert(emulator.memory().read(0x0000) == 0);
  assert(emulator.video().pixel(3, 4) == 0);
  assert(emulator.tape().empty());
  assert(!emulator.keyboard().keyPressed(1, 2) && emulator.keyboard().readRow(1) == 0xFF);
}

int main() {
  testMappedInstructionAndDataBus();
  testStackUsesMappedBus();
  testSubsystemResetAndSmoke();
  testIntegratedPortBusSurvivesReset();
  std::cout << "Integrated machine bus tests passed\n";
}
