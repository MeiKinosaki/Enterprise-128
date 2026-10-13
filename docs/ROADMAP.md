# Enterprise128-360 roadmap

## v0.1b — project scaffold
- [x] Establish a portable C++17 project structure.
- [x] Add a cloud CI host build and smoke test.
- [ ] Verify the repository and license of the upstream EP128emu source before integrating code.
- [ ] Select and validate an Xbox 360/Xenon toolchain and XEX packaging flow.

## v0.2b — Z80 core
- [ ] Integrate or implement a Z80 CPU core with documented provenance and compatible licensing.
- [ ] Add instruction and interrupt tests.

## Later milestones
- Memory map and Enterprise ROM handling.
- NICK video and DAVE audio emulation.
- Keyboard and controller input.
- Supported tape/disk image loading.
- Xbox 360 video/audio integration and default.xex packaging.

## Status notes
The current executable is a host-side scaffold only. It is not an emulator and is not an Xbox 360 XEX. A successful host CI build does not demonstrate Xbox 360 compatibility.
