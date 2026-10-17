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
  nick.setPixel(0, 0, 0x1F); // framebuffer currently stores a 4-bit prototype index
  assert(nick.pixel(0, 0) == 0x0F);
  nick.setPixel(320, 0, 3);
  nick.setPixel(0, 256, 3);
  assert(nick.pixel(319, 255) == 0);
  enterprise128::audio::Dave dave;
  dave.reset(); dave.writeRegister(3, 0x55);
  assert(dave.readRegister(3) == 0x55 && dave.nextSample() == 0);
  dave.writeRegister(63, 0xA5);
  assert(dave.readRegister(63) == 0xA5 && dave.readRegister(64) == 0xFF);
  dave.reset();
  assert(dave.readRegister(3) == 0 && dave.readRegister(63) == 0);
  enterprise128::input::Controller pad;
  pad.reset(); pad.set(enterprise128::input::Controller::Fire1, true);
  assert(pad.pressed(enterprise128::input::Controller::Fire1));
  pad.set(enterprise128::input::Controller::ButtonCount, true); // invalid enum is ignored
  assert(!pad.pressed(enterprise128::input::Controller::ButtonCount));
  pad.reset();
  assert(!pad.pressed(enterprise128::input::Controller::Fire1));
  const std::uint8_t raw[] = {0x01, 0x02, 0x03};
  enterprise128::media::TapeImage tape;
  assert(tape.loadBytes(raw, sizeof(raw)) && tape.bytes().size() == 3);
  assert(tape.bytes()[0] == 0x01 && tape.bytes()[2] == 0x03);
  assert(!tape.loadBytes(nullptr, 2)); // invalid input must not crash
  assert(tape.bytes().size() == 3);   // failed load preserves prior image
  assert(tape.loadBytes(nullptr, 0) && tape.empty());
  assert(tape.loadBytes(raw, sizeof(raw)) && !tape.empty());
  tape.clear(); assert(tape.empty());
  std::cout << "Subsystem prototype tests passed\n";
}
