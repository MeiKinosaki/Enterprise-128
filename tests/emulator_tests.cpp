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

static void testSubsystemResetAndSmoke() {
  enterprise128::machine::Emulator emulator;
  emulator.reset();
  emulator.video().setPixel(3, 4, 6);
  assert(emulator.video().pixel(3, 4) == 6);
  emulator.audio().writeRegister(2, 0x44);
  assert(emulator.audio().readRegister(2) == 0x44);
  emulator.controller().set(enterprise128::input::Controller::Fire1, true);
  assert(emulator.controller().pressed(enterprise128::input::Controller::Fire1));
  const std::uint8_t tapeBytes[] = {0x10, 0x20};
  assert(emulator.tape().loadBytes(tapeBytes, sizeof(tapeBytes)));
  assert(emulator.tape().bytes().size() == 2);
  emulator.reset();
  assert(emulator.cpu().pc == 0 && !emulator.cpu().halted);
  assert(emulator.memory().read(0x0000) == 0);
  assert(emulator.video().pixel(3, 4) == 0);
  assert(emulator.tape().empty());
}

int main() {
  testMappedInstructionAndDataBus();
  testStackUsesMappedBus();
  testSubsystemResetAndSmoke();
  std::cout << "Integrated machine bus tests passed\n";
}
