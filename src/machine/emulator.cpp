#include "machine/emulator.h"

namespace enterprise128::machine {

void Emulator::reset() {
  cpu_.reset();
  memory_.reset();
  video_.reset();
  audio_.reset();
  controller_.reset();
  tape_.clear();
}

bool Emulator::loadProgram(std::uint16_t address, const std::uint8_t* bytes, std::uint32_t size) {
  if (bytes == nullptr && size != 0) return false;
  if (size > 65536u - static_cast<std::uint32_t>(address)) return false;
  cpu_.load(address, bytes, size);
  return true;
}

std::uint32_t Emulator::stepInstruction() {
  return cpu_.step();
}

std::size_t Emulator::runInstructions(std::size_t limit) {
  std::size_t executed = 0;
  while (executed < limit && !cpu_.halted) {
    const auto beforePc = cpu_.pc;
    const auto used = cpu_.step();
    if (used == 0 || cpu_.halted) {
      if (used != 0) ++executed;
      break;
    }
    ++executed;
    // Defensive guard against a malformed CPU implementation that neither
    // advances time nor changes the program counter.
    if (cpu_.pc == beforePc && used == 0) break;
  }
  return executed;
}

}  // namespace enterprise128::machine
