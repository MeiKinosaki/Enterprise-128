#include "video/nick.h"
#include "audio/dave.h"
#include "input/controller.h"
#include "input/keyboard_matrix.h"
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
  assert(nick.paletteColor(0).r == 0 && nick.paletteColor(15).r == 255);
  nick.setPaletteColor(3, {12, 34, 56});
  const auto color = nick.paletteColor(3);
  assert(color.r == 12 && color.g == 34 && color.b == 56);
  nick.setPaletteColor(16, {1, 2, 3}); // invalid index ignored
  assert(nick.paletteColor(16).r == 0);
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
  enterprise128::input::KeyboardMatrix keyboard;
  keyboard.reset();
  assert(keyboard.readRow(0) == 0xFF && !keyboard.keyPressed(2, 3));
  keyboard.setKey(2, 3, true);
  assert(keyboard.keyPressed(2, 3) && keyboard.readRow(2) == 0xF7);
  keyboard.setKey(2, 3, false);
  assert(!keyboard.keyPressed(2, 3) && keyboard.readRow(2) == 0xFF);
  keyboard.setKey(8, 0, true);
  keyboard.setKey(0, 8, true);
  assert(keyboard.readRow(8) == 0xFF && !keyboard.keyPressed(8, 0));

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
