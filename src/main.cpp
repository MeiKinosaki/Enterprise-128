#include "cpu/z80.h"
#include "machine/memory.h"

#include <cstdint>
#include <iostream>

int main() {
  enterprise128::cpu::Z80 cpu;
  enterprise128::machine::Memory memory;
  const std::uint8_t program[] = {0x3E, 0x2A, 0x3C, 0x76}; // LD A,42; INC A; HALT
  cpu.reset();
  cpu.load(0, program, sizeof(program));
  cpu.step();
  cpu.step();
  cpu.step();
  memory.write(0x1234, cpu.a);

  const bool ok = cpu.halted && cpu.a == 0x2B && memory.read(0x1234) == 0x2B;
  std::cout << "Enterprise128-360 host development build\n"
            << "Target: Xbox 360 (Xenon) - not yet packaged\n"
            << "Prototype CPU instructions: " << (ok ? "PASS" : "FAIL") << '\n'
            << "Banked RAM smoke check: " << (memory.read(0x1234) == 0x2B ? "PASS" : "FAIL") << '\n'
            << "This is a development scaffold, not a complete Enterprise emulator.\n";
  return ok ? 0 : 1;
}
