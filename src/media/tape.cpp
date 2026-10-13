#include "media/tape.h"
namespace enterprise128::media {
bool TapeImage::loadBytes(const std::uint8_t* data, std::uint32_t size) {
  if (data == nullptr && size != 0) return false;
  if (size == 0) { bytes_.clear(); return true; }
  bytes_.assign(data, data + size);
  return true;
}
void TapeImage::clear() { bytes_.clear(); }
}  // namespace enterprise128::media
