#include "video/nick.h"
#include "audio/dave.h"
#include "input/controller.h"
#include "media/tape.h"
#include <cassert>
#include <cstdint>
#include <iostream>
int main() {
  enterprise128::video::Nick nick;
  nick.reset(); nick.setPixel(12, 8, 7);
  assert(nick.pixel(12, 8) == 7 && nick.pixel(400, 8) == 0);
  enterprise128::audio::Dave dave;
  dave.reset(); dave.writeRegister(3, 0x55);
  assert(dave.readRegister(3) == 0x55 && dave.nextSample() == 0);
  enterprise128::input::Controller pad;
  pad.reset(); pad.set(enterprise128::input::Controller::Fire1, true);
  assert(pad.pressed(enterprise128::input::Controller::Fire1));
  const std::uint8_t raw[] = {0x01, 0x02, 0x03};
  enterprise128::media::TapeImage tape;
  assert(tape.loadBytes(raw, sizeof(raw)) && tape.bytes().size() == 3);
  tape.clear(); assert(tape.empty());
  std::cout << "Subsystem prototype tests passed\n";
}
