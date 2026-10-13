#include "machine/emulator.h"

namespace enterprise128::machine {

void Emulator::reset() {
  cpu_.reset();
  memory_.reset();
  video_.reset();
  audio_.reset();
  controller_.reset();
  tape_.clear();
  cpu_.setBus(
      [this](std::uint16_t address) { return memory_.read(address); },
      [this](std::uint16_t address, std::uint8_t value) { memory_.write(address, value); });
}

bool Emulator::loadProgram(std::uint16_t address, const std::uint8_t* bytes, std::uint32_t size) {
  if (bytes == nullptr && size != 0) return false;
  if (size > 65536u - static_cast<std::uint32_t>(address)) return false;
  if (size == 0) return true;
  // Load into the currently mapped machine RAM, not the CPU's private test RAM.
  for (std::uint32_t i = 0; i < size; ++i) {
    memory_.write(static_cast<std::uint16_t>(address + i), bytes[i]);
  }
  return true;
}

std::uint32_t Emulator::stepInstruction() {
  return cpu_.step();
}

std::size_t Emulator::runInstructions(std::size_t limit) {
  std::size_t executed = 0;
  while (executed < limit && !cpu_.halted) {
    const auto used = cpu_.step();
    if (used == 0) break;
    ++executed;
  }
  return executed;
}

}  // namespace enterprise128::machine
