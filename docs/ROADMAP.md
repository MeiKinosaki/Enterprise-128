# Enterprise128-360 roadmap

## v0.1b — project scaffold
- [x] Establish a portable C++17 project structure.
- [x] Add cloud CI build and test entry points.

## v0.2b — CPU and memory prototype (in progress)
- [x] Add standalone Z80 state and 64 KiB test address space.
- [x] Add an initial instruction subset including register loads, memory loads/stores, arithmetic/logical operations, branches, stack, CALL/RET and HALT.
- [x] Add regression tests for a small CPU program, stack/call flow and a DJNZ loop.
- [x] Add an eight-bank 16 KiB RAM abstraction (128 KiB total) with four CPU-visible windows.
- [x] Add basic memory mapping tests and wire them into CTest/CI.
- [x] Connect host smoke executable to CPU and memory prototypes.
- [ ] Confirm latest GitHub Actions run passes; source commits alone do not confirm compilation.
- [ ] Complete the documented base Z80 instruction set and all flags/timing semantics.
- [ ] Implement interrupt handling and I/O port abstraction.
- [ ] Replace the generic bank-window model with verified Enterprise-specific memory/ROM mapping.

## v0.3b — machine subsystems (planned)
- [ ] ROM/EXOS boot flow and system initialization.
- [ ] NICK video timing and rendering.
- [ ] DAVE sound and interrupt/timer behavior.
- [ ] Keyboard/joystick input abstraction.
- [ ] Tape/disk image loading and error reporting.

## Xbox 360 delivery (planned)
- [ ] Select and verify a compatible Xenon toolchain and SDK environment.
- [ ] Implement Xbox 360 graphics, audio, controller and file access backends.
- [ ] Package and validate default.xex on a suitable test environment.

## Accuracy and status
The Z80 implementation is a growing prototype, not a complete or cycle-accurate CPU. The 128 KiB bank-window memory class is a development abstraction, not verified Enterprise hardware mapping. NICK, DAVE, ROM/EXOS, media loading, and Xbox 360 integration are not implemented yet. No successful cloud build or Xbox 360 XEX has been confirmed by these commits.
