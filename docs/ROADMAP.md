# Enterprise128-360 roadmap

## v0.1b — project scaffold
- [x] Portable C++17 project and CI entry point.

## v0.2b — CPU and memory prototypes (in progress)
- [x] Initial Z80 state and instruction subset.
- [x] Basic CPU regression tests for register, memory, stack and branch behavior.
- [x] Generic 128 KiB RAM prototype with four 16 KiB windows.
- [ ] Finish Z80 instruction set, exact flag/timing semantics, interrupts and I/O.
- [ ] Verify Enterprise-specific memory/ROM mapping.

## v0.3b — subsystem prototypes (in progress)
- [x] NICK framebuffer API.
- [x] DAVE register storage API.
- [x] Controller button state.
- [x] Raw in-memory tape byte container.
- [x] Smoke tests for subsystem APIs.
- [ ] Implement real NICK modes, timing, palette and interrupts.
- [ ] Implement DAVE channels, timers and audio generation.
- [ ] Implement Enterprise keyboard matrix and joystick port mapping.
- [ ] Parse real tape/disk formats and add safe file loading.

## v0.4b — machine integration scaffold (in progress)
- [x] Add a system-level Emulator facade containing CPU, memory, video, audio, controller and tape components.
- [x] Add reset, program-load bounds checks and bounded instruction-run helper.
- [x] Add machine-level regression tests and wire them into CTest.
- [x] Update host executable to exercise the integrated facade.
- [x] Add configurable Z80 read/write bus callbacks and route instruction fetches, operands, stack, and data accesses through the machine memory bus.
- [x] Load test programs into mapped machine RAM.
- [ ] Verify CPU bus integration in a successful CI run and add exhaustive boundary/mapping tests.
- [ ] Add memory-mapped or port-mapped I/O only after verifying Enterprise hardware behavior.
- [ ] Add deterministic frame/audio stepping and machine-level integration tests.

## Xbox 360 delivery (not yet implemented)
- [ ] Select and verify a Xenon-compatible toolchain/SDK environment.
- [ ] Add Xbox 360 graphics, audio, controller and storage backends.
- [ ] Package and validate default.xex in a suitable test environment.

## Accuracy and status
The CPU remains a partial Z80 prototype, not cycle-accurate. The bank-window memory model is generic, not verified Enterprise hardware mapping. NICK, DAVE, input and tape remain basic prototypes. The Emulator facade groups these components but does not yet connect CPU bus accesses to the machine memory. CI configuration is present; the latest successful build/test run has not been confirmed in this work. No Xbox 360 XEX has been produced or tested.
