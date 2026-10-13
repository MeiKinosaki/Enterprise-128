#pragma once
#include <cstdint>
#include <vector>
namespace enterprise128::media {
class TapeImage {
 public:
  bool loadBytes(const std::uint8_t* data, std::uint32_t size);
  void clear();
  const std::vector<std::uint8_t>& bytes() const { return bytes_; }
  bool empty() const { return bytes_.empty(); }
 private:
  std::vector<std::uint8_t> bytes_;
};
}  // namespace enterprise128::media
