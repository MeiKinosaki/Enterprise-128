#pragma once
#include <array>
#include <cstdint>
namespace enterprise128::audio {
class Dave {
 public:
  void reset();
  void writeRegister(std::uint8_t index, std::uint8_t value);
  std::uint8_t readRegister(std::uint8_t index) const;
  // Placeholder output until channel oscillators and timing are implemented.
  std::int16_t nextSample();
 private:
  std::array<std::uint8_t, 64> registers_{};
};
}  // namespace enterprise128::audio
