#include "video/nick.h"
namespace enterprise128::video {
void Nick::reset() {
  pixels_.fill(0);
  // Neutral grayscale fallback, not claimed to be the Enterprise hardware palette.
  for (unsigned i = 0; i < PaletteSize; ++i) {
    const auto level = static_cast<std::uint8_t>(i * 17);
    palette_[i] = {level, level, level};
  }
}
void Nick::setPixel(unsigned x, unsigned y, std::uint8_t color) {
  if (x < Width && y < Height) pixels_[y * Width + x] = static_cast<std::uint8_t>(color & 0x0F);
}
std::uint8_t Nick::pixel(unsigned x, unsigned y) const {
  return (x < Width && y < Height) ? pixels_[y * Width + x] : 0;
}
void Nick::setPaletteColor(unsigned index, Rgb color) {
  if (index < PaletteSize) palette_[index] = color;
}
Nick::Rgb Nick::paletteColor(unsigned index) const {
  return index < PaletteSize ? palette_[index] : Rgb{};
}
}  // namespace enterprise128::video
