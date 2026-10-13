#pragma once

#include "audio/dave.h"
#include "cpu/z80.h"
#include "input/controller.h"
#include "machine/memory.h"
#include "media/tape.h"
#include "video/nick.h"

#include <cstddef>
#include <cstdint>

namespace enterprise128::machine {

// System-level facade for staged integration. The CPU still executes from its
// own flat test memory; Memory is not yet wired into Z80 fetch/read/write.
class Emulator {
 public:
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
  media::TapeImage& tape() { return tape_; }

 private:
  cpu::Z80 cpu_{};
  Memory memory_{};
  video::Nick video_{};
  audio::Dave audio_{};
  input::Controller controller_{};
  media::TapeImage tape_{};
};

}  // namespace enterprise128::machine
