#include "cpu/z80.h"

#include <algorithm>
#include <utility>

namespace enterprise128::cpu {
namespace {
constexpr std::uint8_t S = 0x80, Z = 0x40, H = 0x10, PV = 0x04, N = 0x02, C = 0x01;
bool parityEven(std::uint8_t v) {
  bool odd = false;
  while (v) { odd = !odd; v &= static_cast<std::uint8_t>(v - 1); }
  return !odd;
}
std::uint8_t szp(std::uint8_t v) {
  return static_cast<std::uint8_t>((v & S) | (v == 0 ? Z : 0) | (parityEven(v) ? PV : 0));
}
}

void Z80::setBus(ReadCallback read, WriteCallback write) { busRead_ = std::move(read); busWrite_ = std::move(write); }
void Z80::clearBus() { busRead_ = {}; busWrite_ = {}; }
void Z80::setIoBus(PortReadCallback read, PortWriteCallback write) { portRead_ = std::move(read); portWrite_ = std::move(write); }
std::uint8_t Z80::read8(std::uint16_t address) const { return busRead_ ? busRead_(address) : memory[address]; }
void Z80::write8(std::uint16_t address, std::uint8_t value) { if (busWrite_) busWrite_(address, value); else memory[address] = value; }

void Z80::reset() {
  a = f = b = c = d = e = h = l = 0;
  sp = 0xFFFF; pc = 0; halted = false; unsupportedOpcode = 0; cycles = 0;
}

void Z80::load(std::uint16_t address, const std::uint8_t* data, std::uint32_t size) {
  if (!data) return;
  const auto available = static_cast<std::uint32_t>(memory.size() - address);
  const auto count = std::min(size, available);
  std::copy(data, data + count, memory.begin() + address);
}

std::uint32_t Z80::step() {
  if (halted) return 0;
  const std::uint8_t op = read8(pc++);
  std::uint32_t used = 0;
  auto fetch8 = [this]() { return read8(pc++); };
  auto fetch16 = [&fetch8]() {
    const std::uint16_t lo = fetch8();
    const std::uint16_t hi = fetch8();
    return static_cast<std::uint16_t>(lo | (hi << 8));
  };
  auto getR = [this](unsigned r) -> std::uint8_t {
    switch (r & 7) {
      case 0: return b; case 1: return c; case 2: return d; case 3: return e;
      case 4: return h; case 5: return l; case 7: return a;
      default: return read8(static_cast<std::uint16_t>((h << 8) | l));
    }
  };
  auto setR = [this](unsigned r, std::uint8_t v) {
    switch (r & 7) {
      case 0: b = v; break; case 1: c = v; break; case 2: d = v; break; case 3: e = v; break;
      case 4: h = v; break; case 5: l = v; break; case 7: a = v; break;
      default: write8(static_cast<std::uint16_t>((h << 8) | l), v); break;
    }
  };
  auto getPair = [this](unsigned p) -> std::uint16_t {
    switch (p & 3) {
      case 0: return static_cast<std::uint16_t>((b << 8) | c);
      case 1: return static_cast<std::uint16_t>((d << 8) | e);
      case 2: return static_cast<std::uint16_t>((h << 8) | l);
      default: return sp;
    }
  };
  auto setPair = [this](unsigned p, std::uint16_t v) {
    switch (p & 3) {
      case 0: b = static_cast<std::uint8_t>(v >> 8); c = static_cast<std::uint8_t>(v); break;
      case 1: d = static_cast<std::uint8_t>(v >> 8); e = static_cast<std::uint8_t>(v); break;
      case 2: h = static_cast<std::uint8_t>(v >> 8); l = static_cast<std::uint8_t>(v); break;
      default: sp = v; break;
    }
  };
  auto push = [this](std::uint16_t v) {
    write8(--sp, static_cast<std::uint8_t>(v >> 8));
    write8(--sp, static_cast<std::uint8_t>(v));
  };
  auto pop = [this]() {
    const std::uint16_t lo = read8(sp++);
    const std::uint16_t hi = read8(sp++);
    return static_cast<std::uint16_t>(lo | (hi << 8));
  };
  auto condition = [this](unsigned cc) {
    switch (cc & 7) {
      case 0: return (f & Z) == 0; case 1: return (f & Z) != 0;
      case 2: return (f & C) == 0; case 3: return (f & C) != 0;
      case 4: return (f & PV) == 0; case 5: return (f & PV) != 0;
      case 6: return (f & S) == 0; default: return (f & S) != 0;
    }
  };

  // Regular LD r,r' matrix and HALT.
  if (op >= 0x40 && op <= 0x7F) {
    if (op == 0x76) { halted = true; used = 4; }
    else { setR((op >> 3) & 7, getR(op & 7)); used = (((op >> 3) & 7) == 6 || (op & 7) == 6) ? 7 : 4; }
  } else if ((op & 0xC7) == 0x06) { // LD r,n
    setR((op >> 3) & 7, fetch8());
    used = ((op >> 3) & 7) == 6 ? 10 : 7;
  } else if ((op & 0xCF) == 0x01) { // LD rr,nn
    setPair((op >> 4) & 3, fetch16()); used = 10;
  } else if ((op & 0xCF) == 0x03) { // INC rr
    setPair((op >> 4) & 3, static_cast<std::uint16_t>(getPair((op >> 4) & 3) + 1)); used = 6;
  } else if ((op & 0xCF) == 0x0B) { // DEC rr
    setPair((op >> 4) & 3, static_cast<std::uint16_t>(getPair((op >> 4) & 3) - 1)); used = 6;
  } else if ((op & 0xC7) == 0x04) { // INC r
    const unsigned r = (op >> 3) & 7;
    const std::uint8_t before = getR(r), result = static_cast<std::uint8_t>(before + 1);
    setR(r, result);
    f = static_cast<std::uint8_t>((f & C) | (result & S) | (result == 0 ? Z : 0) |
        ((before & 0x0F) == 0x0F ? H : 0) | (before == 0x7F ? PV : 0));
    used = r == 6 ? 11 : 4;
  } else if ((op & 0xC7) == 0x05) { // DEC r
    const unsigned r = (op >> 3) & 7;
    const std::uint8_t before = getR(r), result = static_cast<std::uint8_t>(before - 1);
    setR(r, result);
    f = static_cast<std::uint8_t>((f & C) | N | (result & S) | (result == 0 ? Z : 0) |
        ((before & 0x0F) == 0 ? H : 0) | (before == 0x80 ? PV : 0));
    used = r == 6 ? 11 : 4;
  } else if ((op & 0xCF) == 0xC5) { // PUSH qq (AF in slot 3)
    const unsigned p = (op >> 4) & 3;
    const std::uint16_t value = p == 3 ? static_cast<std::uint16_t>((a << 8) | f) : getPair(p);
    push(value); used = 11;
  } else if ((op & 0xCF) == 0xC1) { // POP qq
    const unsigned p = (op >> 4) & 3;
    const std::uint16_t value = pop();
    if (p == 3) { a = static_cast<std::uint8_t>(value >> 8); f = static_cast<std::uint8_t>(value); }
    else setPair(p, value);
    used = 10;
  } else if (op >= 0x80 && op <= 0xBF) { // ADD/ADC/SUB/SBC/AND/XOR/OR/CP A,r
    const unsigned operation = (op >> 3) & 7;
    const std::uint8_t value = getR(op & 7);
    const std::uint8_t oldA = a;
    const unsigned carryIn = ((operation == 1 || operation == 3) && (f & C)) ? 1u : 0u;
    if (operation <= 3 || operation == 7) {
      const bool subtract = operation >= 2;
      const unsigned wide = subtract
          ? static_cast<unsigned>(oldA) - static_cast<unsigned>(value) - carryIn
          : static_cast<unsigned>(oldA) + static_cast<unsigned>(value) + carryIn;
      const std::uint8_t result = static_cast<std::uint8_t>(wide);
      const bool halfCarry = subtract
          ? ((oldA ^ value ^ result) & 0x10) != 0
          : ((oldA ^ value ^ result) & 0x10) != 0;
      const bool overflow = subtract
          ? (((oldA ^ value) & (oldA ^ result) & 0x80) != 0)
          : (((~(oldA ^ value)) & (oldA ^ result) & 0x80) != 0);
      const bool carry = subtract
          ? static_cast<unsigned>(oldA) < static_cast<unsigned>(value) + carryIn
          : wide > 0xFFu;
      f = static_cast<std::uint8_t>((subtract ? N : 0) | (result & S) |
          (result == 0 ? Z : 0) | (halfCarry ? H : 0) |
          (overflow ? PV : 0) | (carry ? C : 0));
      if (operation != 7) a = result; // CP updates flags only.
    } else {
      if (operation == 4) { a = static_cast<std::uint8_t>(a & value); f = static_cast<std::uint8_t>(szp(a) | H); }
      else if (operation == 5) { a = static_cast<std::uint8_t>(a ^ value); f = szp(a); }
      else { a = static_cast<std::uint8_t>(a | value); f = szp(a); }
    }
    used = (op & 7) == 6 ? 7 : 4;
  } else {
    switch (op) {
      case 0x00: used = 4; break; // NOP
      case 0x02: write8(getPair(0), a); used = 7; break; // LD (BC),A
      case 0x0A: a = read8(getPair(0)); used = 7; break;
      case 0x12: write8(getPair(1), a); used = 7; break;
      case 0x1A: a = read8(getPair(1)); used = 7; break;
      case 0x22: { const auto addr = fetch16(); write8(addr, l); write8(static_cast<std::uint16_t>(addr + 1), h); used = 16; break; }
      case 0x2A: { const auto addr = fetch16(); l = read8(addr); h = read8(static_cast<std::uint16_t>(addr + 1)); used = 16; break; }
      case 0x32: { const auto addr = fetch16(); write8(addr, a); used = 13; break; }
      case 0x3A: { const auto addr = fetch16(); a = read8(addr); used = 13; break; }
      case 0x07: { const bool carry = (a & 0x80) != 0; a = static_cast<std::uint8_t>((a << 1) | (carry ? 1 : 0)); f = static_cast<std::uint8_t>((f & (S|Z|PV)) | (a & 0x28) | (carry ? C : 0)); used = 4; break; } // RLCA
      case 0x0F: { const bool carry = (a & 1) != 0; a = static_cast<std::uint8_t>((a >> 1) | (carry ? 0x80 : 0)); f = static_cast<std::uint8_t>((f & (S|Z|PV)) | (a & 0x28) | (carry ? C : 0)); used = 4; break; }
      case 0x09: case 0x19: case 0x29: case 0x39: { const auto lhs = getPair(2), rhs = getPair((op >> 4) & 3); const std::uint32_t sum = static_cast<std::uint32_t>(lhs) + rhs; setPair(2, static_cast<std::uint16_t>(sum)); f = static_cast<std::uint8_t>((f & (S|Z|PV)) | (((lhs ^ rhs ^ sum) & 0x1000) ? H : 0) | (sum > 0xFFFF ? C : 0)); used = 11; break; }
      case 0x10: { const auto displacement = static_cast<std::int8_t>(fetch8()); --b; if (b != 0) { pc = static_cast<std::uint16_t>(pc + displacement); used = 13; } else used = 8; break; } // DJNZ
      case 0x18: { const auto displacement = static_cast<std::int8_t>(fetch8()); pc = static_cast<std::uint16_t>(pc + displacement); used = 12; break; }
      case 0x20: case 0x28: case 0x30: case 0x38: { const auto displacement = static_cast<std::int8_t>(fetch8()); const unsigned cc = (op == 0x20 ? 0 : op == 0x28 ? 1 : op == 0x30 ? 2 : 3); if (condition(cc)) { pc = static_cast<std::uint16_t>(pc + displacement); used = 12; } else used = 7; break; }
      case 0x17: { // RLA
        const bool oldCarry = (f & C) != 0;
        const bool carry = (a & 0x80) != 0;
        a = static_cast<std::uint8_t>((a << 1) | (oldCarry ? 1 : 0));
        f = static_cast<std::uint8_t>((f & (S|Z|PV)) | (a & 0x28) | (carry ? C : 0));
        used = 4;
        break;
      }
      case 0x1F: { // RRA
        const bool oldCarry = (f & C) != 0;
        const bool carry = (a & 1) != 0;
        a = static_cast<std::uint8_t>((a >> 1) | (oldCarry ? 0x80 : 0));
        f = static_cast<std::uint8_t>((f & (S|Z|PV)) | (a & 0x28) | (carry ? C : 0));
        used = 4;
        break;
      }
      case 0x2F: { // CPL
        a = static_cast<std::uint8_t>(~a);
        f = static_cast<std::uint8_t>((f & (S|Z|PV|C)) | H | N | (a & 0x28));
        used = 4;
        break;
      }
      case 0x3F: { // CCF
        const bool oldCarry = (f & C) != 0;
        f = static_cast<std::uint8_t>((f & (S|Z|PV)) | (a & 0x28) |
            (oldCarry ? H : 0) | (oldCarry ? 0 : C));
        used = 4;
        break;
      }
      case 0x37: { // SCF: preserve S/Z/PV, copy undocumented bits from A, set carry
        f = static_cast<std::uint8_t>((f & (S|Z|PV)) | (a & 0x28) | C);
        used = 4;
        break;
      }
      case 0x27: { // DAA
        std::uint8_t correction = 0; bool carry = (f & C) != 0;
        if ((f & H) || (!(f & N) && (a & 0x0F) > 9)) correction |= 0x06;
        if (carry || (!(f & N) && a > 0x99)) { correction |= 0x60; carry = true; }
        const auto before = a; a = static_cast<std::uint8_t>((f & N) ? a - correction : a + correction);
        f = static_cast<std::uint8_t>((f & N) | (carry ? C : 0) | (((before ^ a) & 0x10) ? H : 0) | szp(a)); used = 4; break;
      }
      case 0xAF: a = 0; f = Z | PV; used = 4; break; // XOR A
      case 0xA7: f = static_cast<std::uint8_t>(szp(a) | H); used = 4; break; // AND A
      case 0xB7: f = szp(a); used = 4; break; // OR A
      case 0xCB: { // CB-prefixed rotate/shift, BIT, RES and SET
        const std::uint8_t ext = fetch8();
        const unsigned group = ext >> 6;
        const unsigned bit = (ext >> 3) & 7;
        const unsigned r = ext & 7;
        std::uint8_t value = getR(r);
        if (group == 0) {
          bool carry = false;
          std::uint8_t result = value;
          switch (bit) {
            case 0: carry = (value & 0x80) != 0; result = static_cast<std::uint8_t>((value << 1) | (carry ? 1 : 0)); break; // RLC
            case 1: carry = (value & 0x01) != 0; result = static_cast<std::uint8_t>((value >> 1) | (carry ? 0x80 : 0)); break; // RRC
            case 2: { const bool old = (f & C) != 0; carry = (value & 0x80) != 0; result = static_cast<std::uint8_t>((value << 1) | (old ? 1 : 0)); break; } // RL
            case 3: { const bool old = (f & C) != 0; carry = (value & 0x01) != 0; result = static_cast<std::uint8_t>((value >> 1) | (old ? 0x80 : 0)); break; } // RR
            case 4: carry = (value & 0x80) != 0; result = static_cast<std::uint8_t>(value << 1); break; // SLA
            case 5: carry = (value & 0x01) != 0; result = static_cast<std::uint8_t>((value >> 1) | (value & 0x80)); break; // SRA
            case 6: carry = (value & 0x80) != 0; result = static_cast<std::uint8_t>((value << 1) | 1); break; // SLL (undocumented)
            default: carry = (value & 0x01) != 0; result = static_cast<std::uint8_t>(value >> 1); break; // SRL
          }
          setR(r, result);
          f = static_cast<std::uint8_t>(szp(result) | (carry ? C : 0));
        } else if (group == 1) { // BIT b,r
          const bool set = (value & (1u << bit)) != 0;
          // On BIT (HL), undocumented flag bits 5/3 are copied from the
          // high byte of the effective address, not from the memory value.
          const std::uint8_t undocumented = r == 6 ? h : value;
          f = static_cast<std::uint8_t>((f & C) | H | (set ? 0 : Z | PV) |
              (bit == 7 && set ? S : 0) | (undocumented & 0x28));
        } else if (group == 2) { // RES b,r
          setR(r, static_cast<std::uint8_t>(value & ~(1u << bit)));
        } else { // SET b,r
          setR(r, static_cast<std::uint8_t>(value | (1u << bit)));
        }
        used = r == 6 ? (group == 1 ? 12 : 15) : 8;
        break;
      }
      case 0xED: { // Extended Z80 opcodes
        const std::uint8_t ext = fetch8();
        if ((ext & 0xC7) == 0x40) { // IN r,(C), including undocumented IN (C)
          const unsigned r = (ext >> 3) & 7;
          const std::uint16_t port = getPair(0);
          const std::uint8_t value = portRead_ ? portRead_(port) : 0xFF;
          if (r != 6) setR(r, value);
          f = static_cast<std::uint8_t>((f & C) | szp(value) | (value & 0x28));
          used = 12;
        } else if ((ext & 0xC7) == 0x41) { // OUT (C),r; OUT (C),0 for r=6
          const unsigned r = (ext >> 3) & 7;
          const std::uint8_t value = r == 6 ? 0 : getR(r);
          if (portWrite_) portWrite_(getPair(0), value);
          used = 12;
        } else if (ext == 0xA2 || ext == 0xAA || ext == 0xB2 || ext == 0xBA ||
                   ext == 0xA3 || ext == 0xAB || ext == 0xB3 || ext == 0xBB) {
          const bool input = (ext == 0xA2 || ext == 0xAA || ext == 0xB2 || ext == 0xBA);
          const bool decrement = (ext == 0xAA || ext == 0xBA || ext == 0xAB || ext == 0xBB);
          const bool repeat = (ext == 0xB2 || ext == 0xBA || ext == 0xB3 || ext == 0xBB);
          const std::uint16_t hl = getPair(2);
          const std::uint16_t port = getPair(0);
          std::uint8_t value = 0;
          if (input) {
            value = portRead_ ? portRead_(port) : 0xFF;
            write8(hl, value);
          } else {
            value = read8(hl);
            if (portWrite_) portWrite_(port, value);
          }
          const std::uint8_t nextB = static_cast<std::uint8_t>(b - 1);
          const std::uint8_t nextL = static_cast<std::uint8_t>(l + (decrement ? -1 : 1));
          b = nextB;
          setPair(2, static_cast<std::uint16_t>((h << 8) | nextL));
          const unsigned adjustment = input
              ? static_cast<unsigned>(static_cast<std::uint8_t>(c + (decrement ? -1 : 1)))
              : static_cast<unsigned>(nextL);
          const unsigned sum = static_cast<unsigned>(value) + adjustment;
          const bool halfCarry = sum > 0xFFu;
          const bool carry = halfCarry;
          const std::uint8_t parityInput = static_cast<std::uint8_t>((sum & 7u) ^ nextB);
          f = static_cast<std::uint8_t>((nextB & (S | Y | X)) |
              (nextB == 0 ? Z : 0) | (halfCarry ? H : 0) |
              (parityEven(parityInput) ? PV : 0) | ((value & 0x80) ? N : 0) |
              (carry ? C : 0));
          if (repeat && nextB != 0) {
            pc = static_cast<std::uint16_t>(pc - 2);
            used = 21;
          } else used = 16;
        } else if (ext == 0xA0 || ext == 0xA8 || ext == 0xB0 || ext == 0xB8) {
          const bool decrement = (ext == 0xA8 || ext == 0xB8);
          const bool repeat = (ext == 0xB0 || ext == 0xB8);
          const std::uint16_t hl = getPair(2), de = getPair(1);
          const std::uint8_t value = read8(hl);
          write8(de, value);
          setPair(2, static_cast<std::uint16_t>(hl + (decrement ? -1 : 1)));
          setPair(1, static_cast<std::uint16_t>(de + (decrement ? -1 : 1)));
          const std::uint16_t bc = static_cast<std::uint16_t>(getPair(0) - 1);
          setPair(0, bc);
          const std::uint8_t sum = static_cast<std::uint8_t>(a + value);
          f = static_cast<std::uint8_t>((f & (S|Z|C)) | (bc != 0 ? PV : 0) |
              (sum & 0x08) | ((sum & 0x02) << 4));
          if (repeat && bc != 0) {
            pc = static_cast<std::uint16_t>(pc - 2);
            used = 21;
          } else used = 16;
        } else if ((ext & 0xCF) == 0x4A || (ext & 0xCF) == 0x42) {
          const unsigned pairIndex = (ext >> 4) & 3;
          const std::uint16_t lhs = getPair(2);
          const std::uint16_t rhs = getPair(pairIndex);
          const unsigned carryIn = (f & C) ? 1u : 0u;
          const bool subtract = (ext & 0x08) == 0;
          const std::uint32_t wide = subtract
              ? static_cast<std::uint32_t>(lhs) - static_cast<std::uint32_t>(rhs) - carryIn
              : static_cast<std::uint32_t>(lhs) + static_cast<std::uint32_t>(rhs) + carryIn;
          const std::uint16_t result = static_cast<std::uint16_t>(wide);
          const bool halfCarry = ((lhs ^ rhs ^ result) & 0x1000) != 0;
          const bool overflow = subtract
              ? (((lhs ^ rhs) & (lhs ^ result) & 0x8000) != 0)
              : (((~(lhs ^ rhs)) & (lhs ^ result) & 0x8000) != 0);
          const bool carry = subtract
              ? static_cast<std::uint32_t>(lhs) < static_cast<std::uint32_t>(rhs) + carryIn
              : wide > 0xFFFFu;
          f = static_cast<std::uint8_t>((subtract ? N : 0) |
              ((result & 0x8000) ? S : 0) | (result == 0 ? Z : 0) |
              (halfCarry ? H : 0) | (overflow ? PV : 0) | (carry ? C : 0));
          setPair(2, result);
          used = 15;
        } else {
          unsupportedOpcode = ext;
          halted = true;
          used = 0;
        }
        break;
      }
      case 0xE9: pc = getPair(2); used = 4; break; // JP (HL)
      case 0xEB: { // EX DE,HL
        const std::uint16_t de = getPair(1), hl = getPair(2);
        setPair(1, hl); setPair(2, de); used = 4; break;
      }
      case 0xE3: { // EX (SP),HL
        const std::uint8_t lo = read8(sp);
        const std::uint8_t hi = read8(static_cast<std::uint16_t>(sp + 1));
        write8(sp, l);
        write8(static_cast<std::uint16_t>(sp + 1), h);
        l = lo; h = hi; used = 19; break;
      }
      case 0xF9: sp = getPair(2); used = 6; break; // LD SP,HL
      case 0xC3: pc = fetch16(); used = 10; break;
      case 0xC9: pc = pop(); used = 10; break; // RET
      case 0xC7: case 0xCF: case 0xD7: case 0xDF:
      case 0xE7: case 0xEF: case 0xF7: case 0xFF: { // RST p
        push(pc);
        pc = static_cast<std::uint16_t>(op & 0x38);
        used = 11;
        break;
      }
      case 0xCD: { const auto addr = fetch16(); push(pc); pc = addr; used = 17; break; } // CALL
      case 0xC0: case 0xC8: case 0xD0: case 0xD8:
      case 0xE0: case 0xE8: case 0xF0: case 0xF8: { // RET cc
        const unsigned cc = (op >> 3) & 7;
        if (condition(cc)) { pc = pop(); used = 11; }
        else used = 5;
        break;
      }
      case 0xC2: case 0xCA: case 0xD2: case 0xDA:
      case 0xE2: case 0xEA: case 0xF2: case 0xFA: { // JP cc,nn
        const auto address = fetch16();
        if (condition((op >> 3) & 7)) pc = address;
        used = 10;
        break;
      }
      case 0xC4: case 0xCC: case 0xD4: case 0xDC:
      case 0xE4: case 0xEC: case 0xF4: case 0xFC: { // CALL cc,nn
        const auto address = fetch16();
        if (condition((op >> 3) & 7)) { push(pc); pc = address; used = 17; }
        else used = 10;
        break;
      }
      case 0xC6: case 0xCE: case 0xD6: case 0xDE: case 0xE6: case 0xEE: case 0xF6: case 0xFE: {
        const std::uint8_t value = fetch8();
        if (op == 0xC6 || op == 0xCE) { const unsigned carry = (op == 0xCE && (f & C)) ? 1 : 0; const unsigned sum = a + value + carry; const auto result = static_cast<std::uint8_t>(sum); f = static_cast<std::uint8_t>((result & S) | (result == 0 ? Z : 0) | (((a ^ value ^ result) & 0x10) ? H : 0) | ((~(a ^ value) & (a ^ result) & 0x80) ? PV : 0) | (sum > 255 ? C : 0)); a = result; }
        else if (op == 0xD6 || op == 0xDE || op == 0xFE) { const unsigned carry = (op == 0xDE && (f & C)) ? 1 : 0; const auto result = static_cast<std::uint8_t>(a - value - carry); f = static_cast<std::uint8_t>(N | (result & S) | (result == 0 ? Z : 0) | (((a ^ value ^ result) & 0x10) ? H : 0) | (((a ^ value) & (a ^ result) & 0x80) ? PV : 0) | (static_cast<unsigned>(a) < static_cast<unsigned>(value) + carry ? C : 0)); if (op != 0xFE) a = result; }
        else if (op == 0xE6) { a &= value; f = static_cast<std::uint8_t>(szp(a) | H); }
        else if (op == 0xEE) { a ^= value; f = szp(a); }
        else { a |= value; f = szp(a); }
        used = 7; break;
      }
      case 0xD3: { // OUT (n),A; high port byte is A before the write
        const std::uint8_t low = fetch8();
        const std::uint16_t port = static_cast<std::uint16_t>((a << 8) | low);
        if (portWrite_) portWrite_(port, a);
        used = 11;
        break;
      }
      case 0xDB: { // IN A,(n); high port byte is old A
        const std::uint8_t low = fetch8();
        const std::uint16_t port = static_cast<std::uint16_t>((a << 8) | low);
        if (portRead_) a = portRead_(port);
        used = 11;
        break;
      }
      case 0xF3: used = 4; break; // DI (interrupt model not wired yet)
      case 0xFB: used = 4; break; // EI (interrupt model not wired yet)
      case 0x76: halted = true; used = 4; break;
      default: unsupportedOpcode = op; halted = true; used = 0; break;
    }
  }
  cycles += used;
  return used;
}
}  // namespace enterprise128::cpu
