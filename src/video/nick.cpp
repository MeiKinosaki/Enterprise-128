#include "video/nick.h"
namespace enterprise128::video {
void Nick::reset() { pixels_.fill(0); }
void Nick::setPixel(unsigned x, unsigned y, std::uint8_t color) {
  if (x < Width && y < Height) pixels_[y * Width + x] = static_cast<std::uint8_t>(color & 0x0F);
}
std::uint8_t Nick::pixel(unsigned x, unsigned y) const {
  return (x < Width && y < Height) ? pixels_[y * Width + x] : 0;
}
}  // namespace enterprise128::video
