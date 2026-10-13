#include "machine/emulator.h"

#include <cstdint>
#include <iostream>

int main() {
  enterprise128::machine::Emulator emulator;
  emulator.reset();
  const std::uint8_t program[] = {0x3E, 0x2A, 0x3C, 0x76}; // LD A,42; INC A; HALT
  const bool loaded = emulator.loadProgram(0, program, sizeof(program));
  const auto executed = loaded ? emulator.runInstructions(16) : 0;
  const bool cpuOk = loaded && emulator.cpu().halted && emulator.cpu().a == 0x2B && executed == 3;

  emulator.memory().write(0x1234, emulator.cpu().a);
  const bool memoryOk = emulator.memory().read(0x1234) == 0x2B;
  emulator.video().setPixel(0, 0, 1);
  const bool videoOk = emulator.video().pixel(0, 0) == 1;

  std::cout << "Enterprise128-360 integrated host prototype\n"
            << "Target: Xbox 360 (Xenon) - not yet packaged\n"
            << "CPU smoke check: " << (cpuOk ? "PASS" : "FAIL") << '\n'
            << "RAM component check: " << (memoryOk ? "PASS" : "FAIL") << '\n'
            << "NICK framebuffer check: " << (videoOk ? "PASS" : "FAIL") << '\n'
            << "Note: CPU fetches still use its private test memory; bus integration is pending.\n";
  return cpuOk && memoryOk && videoOk ? 0 : 1;
}
