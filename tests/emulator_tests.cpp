#include "machine/emulator.h"

#include <cassert>
#include <cstdint>
#include <iostream>

int main() {
  enterprise128::machine::Emulator emulator;
  emulator.reset();

  const std::uint8_t program[] = {0x3E, 0x10, 0x3C, 0x32, 0x00, 0x40, 0x76};
  assert(emulator.loadProgram(0x0000, program, sizeof(program)));
  assert(!emulator.loadProgram(0xFFFF, program, sizeof(program)));
  assert(emulator.runInstructions(20) == 4);
  assert(emulator.cpu().halted);
  assert(emulator.cpu().a == 0x11);
  assert(emulator.cpu().memory[0x4000] == 0x11);

  emulator.memory().write(0x0100, 0xA5);
  assert(emulator.memory().read(0x0100) == 0xA5);
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
  assert(emulator.memory().read(0x0100) == 0);
  assert(emulator.video().pixel(3, 4) == 0);
  assert(emulator.tape().empty());
  std::cout << "Integrated machine facade tests passed\n";
}
