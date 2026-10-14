#pragma once

#include <array>
#include <cstdint>
#include <functional>

namespace enterprise128::cpu {

class Z80 {
 public:
  static constexpr std::uint8_t FlagS = 0x80;
  static constexpr std::uint8_t FlagZ = 0x40;
  static constexpr std::uint8_t FlagY = 0x20;
  static constexpr std::uint8_t FlagH = 0x10;
  static constexpr std::uint8_t FlagX = 0x08;
  static constexpr std::uint8_t FlagPV = 0x04;
  static constexpr std::uint8_t FlagN = 0x02;
  static constexpr std::uint8_t FlagC = 0x01;

  std::uint8_t a{}, f{}, b{}, c{}, d{}, e{}, h{}, l{};
  std::uint16_t sp{0xFFFF}, pc{};
  bool halted{};
  std::uint8_t unsupportedOpcode{};
  std::uint64_t cycles{};
  std::array<std::uint8_t, 65536> memory{};

  using ReadCallback = std::function<std::uint8_t(std::uint16_t)>;
  using WriteCallback = std::function<void(std::uint16_t, std::uint8_t)>;
  using PortReadCallback = std::function<std::uint8_t(std::uint16_t)>;
  using PortWriteCallback = std::function<void(std::uint16_t, std::uint8_t)>;

  void setBus(ReadCallback read, WriteCallback write);
  void setIoBus(PortReadCallback read, PortWriteCallback write);
  void clearBus();
  void reset();
  void load(std::uint16_t address, const std::uint8_t* data, std::uint32_t size);
  std::uint32_t step();

 private:
  ReadCallback busRead_{};
  WriteCallback busWrite_{};
  PortReadCallback portRead_{};
  PortWriteCallback portWrite_{};
  std::uint8_t read8(std::uint16_t address) const;
  void write8(std::uint16_t address, std::uint8_t value);
};

}  // namespace enterprise128::cpu
