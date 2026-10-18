#pragma once
#include <array>
#include <cstdint>
namespace enterprise128::video {
class Nick {
 public:
  static constexpr unsigned Width = 320, Height = 256;
  struct Rgb { std::uint8_t r{}, g{}, b{}; };
  static constexpr unsigned PaletteSize = 16;
  void reset();
  void setPixel(unsigned x, unsigned y, std::uint8_t color);
  std::uint8_t pixel(unsigned x, unsigned y) const;
  void setPaletteColor(unsigned index, Rgb color);
  Rgb paletteColor(unsigned index) const;
  const std::array<std::uint8_t, Width * Height>& framebuffer() const { return pixels_; }
 private:
  std::array<std::uint8_t, Width * Height> pixels_{};
  std::array<Rgb, PaletteSize> palette_{};
};
}  // namespace enterprise128::video
