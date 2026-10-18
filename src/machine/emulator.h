#pragma once

#include "audio/dave.h"
#include "cpu/z80.h"
#include "input/controller.h"
#include "input/keyboard_matrix.h"
#include "machine/memory.h"
#include "media/tape.h"
#include "video/nick.h"

#include <cstddef>
#include <cstdint>
#include <functional>

namespace enterprise128::machine {

// System-level facade for staged integration. reset() connects Z80 fetches and
// data/stack accesses to generic banked RAM through read/write callbacks.
class Emulator {
 public:
  using PortReadCallback = cpu::Z80::PortReadCallback;
  using PortWriteCallback = cpu::Z80::PortWriteCallback;

  void setPortBus(PortReadCallback read, PortWriteCallback write);
  void reset();
  bool loadProgram(std::uint16_t address, const std::uint8_t* bytes, std::uint32_t size);
  std::uint32_t stepInstruction();
  std::size_t runInstructions(std::size_t limit);

  cpu::Z80& cpu() { return cpu_; }
  const cpu::Z80& cpu() const { return cpu_; }
  Memory& memory() { return memory_; }
  video::Nick& video() { return video_; }
  audio::Dave& audio() { return audio_; }
  input::Controller& controller() { return controller_; }
  input::KeyboardMatrix& keyboard() { return keyboard_; }
  media::TapeImage& tape() { return tape_; }

 private:
  cpu::Z80 cpu_{};
  Memory memory_{};
  video::Nick video_{};
  audio::Dave audio_{};
  input::Controller controller_{};
  input::KeyboardMatrix keyboard_{};
  media::TapeImage tape_{};
  PortReadCallback portRead_{};
  PortWriteCallback portWrite_{};
};

}  // namespace enterprise128::machine
