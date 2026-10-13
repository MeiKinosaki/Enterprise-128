#include "audio/dave.h"
namespace enterprise128::audio {
void Dave::reset() { registers_.fill(0); }
void Dave::writeRegister(std::uint8_t index, std::uint8_t value) {
  if (index < registers_.size()) registers_[index] = value;
}
std::uint8_t Dave::readRegister(std::uint8_t index) const {
  return index < registers_.size() ? registers_[index] : 0xFF;
}
std::int16_t Dave::nextSample() { return 0; }
}  // namespace enterprise128::audio
