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
  std::cout << "Memory tests passed\n";
}
