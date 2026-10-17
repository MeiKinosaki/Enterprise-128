#include "machine/memory.h"
#include <cassert>
#include <iostream>
int main() {
  enterprise128::machine::Memory m;
  m.write(0x0123, 0xA5);
  assert(m.read(0x0123) == 0xA5);
  m.mapWindow(0, 4);
  assert(m.read(0x0123) == 0);
  m.write(0x0123, 0x5A);
  assert(m.read(0x0123) == 0x5A);
  m.mapWindow(0, 0);
  assert(m.read(0x0123) == 0xA5);
  // Invalid mappings are ignored and leave the selected bank unchanged.
  m.mapWindow(0, 255);
  m.mapWindow(4, 1);
  assert(m.mappedBank(0) == 0 && m.mappedBank(4) == 0xFF);
  assert(m.read(0x0123) == 0xA5);
  m.reset();
  assert(m.mappedBank(0) == 0 && m.mappedBank(1) == 1);
  assert(m.read(0x0123) == 0);
  std::cout << "Memory tests passed\n";
}
