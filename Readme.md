## Chip8-emulator implementation using C++ and SDL3

- currently in progress

### Core

- [x] Project setup and README
- [x] Chip8 class (memory, registers, PC, index, stack, timers, video buffer)
- [x] Load ROM into memory
- [x] Load fontset into memory
- [x] Random number generator

### Opcodes

- [x] NULL (do nothing)
- [x] 00E0 - CLS
- [x] 00EE - RET
- [x] 1nnn - JP addr
- [x] 2nnn - CALL addr
- [x] 3xkk - SE Vx, byte
- [x] 4xkk - SNE Vx, byte
- [x] 5xy0 - SE Vx, Vy
- [x] 6xkk - LD Vx, byte
- [x] 7xkk - ADD Vx, byte
- [x] 8xy0 - LD Vx, Vy
- [x] 8xy1 - OR Vx, Vy
- [x] 8xy2 - AND Vx, Vy
- [x] 8xy3 - XOR Vx, Vy
- [x] 8xy4 - ADD Vx, Vy
- [x] 8xy5 - SUB Vx, Vy
- [x] 8xy6 - SHR Vx
- [x] 8xy7 - SUBN Vx, Vy
- [x] 8xyE - SHL Vx
- [x] 9xy0 - SNE Vx, Vy
- [x] Annn - LD I, addr
- [x] Bnnn - JP V0, addr
- [x] Cxkk - RND Vx, byte
- [x] Dxyn - DRW Vx, Vy, nibble
- [x] Ex9E - SKP Vx
- [x] ExA1 - SKNP Vx
- [x] Fx07 - LD Vx, DT
- [x] Fx0A - LD Vx, K
- [x] Fx15 - LD DT, Vx
- [x] Fx18 - LD ST, Vx
- [x] Fx1E - ADD I, Vx
- [x] Fx29 - LD F, Vx
- [x] Fx33 - LD B, Vx
- [x] Fx55 - LD [I], Vx
- [x] Fx65 - LD Vx, [I]

### CPU

- [x] Opcode function pointer tables (main table plus sub-tables for 0, 8, E and F) filled in the constructor
- [x] Sub-table dispatch functions (`Table0`, `Table8`, `TableE`, `TableF`)
- [x] Fetch (read the opcode at PC and advance PC)
- [x] Decode / execute (call the handler from the table inside `Cycle`)
- [x] Delay timer and sound timer decrement at 60 Hz (`UpdateTimers`, called from the main loop independently of the cycle delay)

### SDL3 platform layer

- [x] Create window, renderer and texture
- [x] Draw the 64x32 video buffer to the screen (with scaling)
- [x] Keyboard input mapping for the 16-key hex keypad
- [ ] Sound (beep while the sound timer is non-zero)
- [x] Main loop with cycle delay (configurable emulation speed)
- [x] Command line arguments (scale, cycle delay, ROM path)

### Finishing up

- [ ] Build system (Makefile / CMake)
- [ ] Test with test ROMs (e.g. IBM logo, Corax+ / BestCoder test suite)
- [ ] Run real games (Pong, Tetris, Space Invaders, ...)
- [ ] Usage instructions and screenshots in the README
