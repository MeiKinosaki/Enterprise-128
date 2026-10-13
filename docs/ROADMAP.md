# Enterprise128-360 roadmap

## v0.1b — project scaffold
- [x] Establish a portable C++17 project structure.
- [x] Add a cloud CI host build and smoke test.

## v0.2b — first Z80 execution slice (in progress)
- [x] Add a standalone Z80 state model with 64 KiB test memory.
- [x] Implement a first instruction subset: NOP, LD A,n, LD B,n, LD C,n, LD (nn),A, LD A,(nn), INC A, DEC A, XOR A, JP nn, CP n and HALT.
- [x] Add basic regression assertions and connect them to CTest/CI.
- [ ] Verify the cloud workflow actually passes; the commit alone does not prove a successful run.
- [ ] Expand flag behavior and implement the remaining base instruction set.
- [ ] Add register-pair operations, stack operations, calls/returns, conditional branches and interrupts.
- [ ] Verify CPU core licensing/provenance before choosing any upstream implementation.

## Later milestones
- Enterprise memory map, ROM and paging.
- NICK video and DAVE audio emulation.
- Keyboard and controller input.
- Supported tape/disk image loading.
- Xbox 360/Xenon toolchain, platform input/video/audio integration and default.xex packaging.

## Status notes
The current Z80 core is an early instruction-subset prototype for host-side tests. It is not cycle-complete or hardware-accurate, and it does not yet emulate a complete Enterprise 128. The host executable is not an Xbox 360 XEX. Passing host CI would not demonstrate Xbox 360 compatibility.
