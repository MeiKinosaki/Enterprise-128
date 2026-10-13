# Enterprise128-360 roadmap

## v0.1b — project scaffold
- [x] Portable C++17 project and CI entry point.

## v0.2b — CPU and memory prototypes (in progress)
- [x] Initial Z80 state and instruction subset, including register loads, arithmetic/logical operations, branches, stack, CALL/RET and HALT.
- [x] CPU regression tests for instruction flow and basic stack/subroutine behavior.
- [x] 128 KiB RAM prototype split into eight 16 KiB banks, exposed through four 16 KiB windows.
- [x] Memory tests and host smoke executable integration.
- [ ] Complete base instruction set, exact flags/timing, interrupts and I/O semantics.
- [ ] Verify Enterprise-specific memory/ROM mapping against trusted technical references.

## v0.3b — subsystem prototypes (interfaces added; emulation incomplete)
- [x] NICK framebuffer prototype (320x256 indexed pixels, bounds checks).
- [x] DAVE register-storage prototype.
- [x] Controller button-state abstraction.
- [x] In-memory raw-byte tape container.
- [x] Smoke tests for subsystem interfaces added to CTest/CI.
- [ ] Implement NICK display modes, palette, timing and interrupts.
- [ ] Implement DAVE channels, timers, noise and audio sample generation.
- [ ] Implement the real Enterprise keyboard matrix and joystick port mapping.
- [ ] Parse actual supported tape/disk formats and add file I/O.
- [ ] Implement ROM/EXOS boot flow and connect CPU to mapped memory and I/O.

## Xbox 360 delivery (not yet implemented)
- [ ] Select and validate a Xenon-compatible toolchain and SDK environment.
- [ ] Implement Xbox 360 graphics, audio, controller and file access backends.
- [ ] Package and validate default.xex on a suitable test environment.

## Accuracy and status
The CPU supports only a subset of Z80 opcodes and is not cycle-accurate. The memory model is generic and not a verified Enterprise memory controller. NICK, DAVE, input and tape are only early software prototypes, not hardware-complete emulation. The CI workflow has been configured, but its latest run has not been verified in this work. No Xbox 360 XEX has been produced or tested.
