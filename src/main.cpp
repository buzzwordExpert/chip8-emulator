#include "chip8.hpp"
#include "platform.hpp"
#include <chrono>
#include <iostream>
#include <cstdlib>
#include <string>


int main(int argc, char** argv) {

    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <Scale> <Delay> <ROM>\n";
        std::exit(EXIT_FAILURE);
    }

    // Problem: std::stoi throws an exception on non-numeric input (e.g. "abc"),
    // and nothing catches it, so the program aborts with a cryptic crash message.
    // It also happily accepts nonsense like a scale of 0 or -5, which would create
    // an invalid window. Catch the exception and validate the ranges instead.
    int videoScale;
    int cycleDelay;
    try {
        videoScale = std::stoi(argv[1]);
        cycleDelay = std::stoi(argv[2]);
    } catch (std::exception const&) {
        std::cerr << "Error: <Scale> and <Delay> must be whole numbers\n";
        return EXIT_FAILURE;
    }

    if (videoScale < 1 || cycleDelay < 0) {
        std::cerr << "Error: <Scale> must be at least 1 and <Delay> cannot be negative\n";
        return EXIT_FAILURE;
    }

    char const* romFilename = argv[3];

    // Load the ROM before opening the window, so a bad path doesn't flash a window first
    Chip8 chip8;
    if (!chip8.LoadROM(romFilename)) {
        std::cerr << "Error: could not load ROM '" << romFilename
                  << "' (missing, unreadable, empty, or larger than 3584 bytes)\n";
        return EXIT_FAILURE;
    }

    Platform platform("CHIP-8 Emulator", VIDEO_WIDTH * videoScale, VIDEO_HEIGHT * videoScale, VIDEO_WIDTH, VIDEO_HEIGHT);

    int videoPitch = sizeof(chip8.video[0]) * VIDEO_WIDTH;

    // The CHIP-8 delay and sound timers always count down at 60 Hz (once every ~16.67 ms),
    // no matter how fast the CPU runs.
    auto const timerInterval = std::chrono::microseconds(16667);

    auto lastCycleTime = std::chrono::high_resolution_clock::now();
    auto lastTimerTime = lastCycleTime;
    bool quit = false;

    while (!quit) {
        quit = platform.ProcessInput(chip8.keypad);

        auto currentTime = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration<float, std::chrono::milliseconds::period>(currentTime - lastCycleTime).count();

        // Problem: the timers used to be decremented inside Cycle(), so they ticked once
        // per CPU instruction. With e.g. a 3 ms delay that is ~333 ticks per second instead
        // of 60, which makes games run their timing 5x too fast. They now tick here on
        // their own 60 Hz stopwatch, independent of the cycle delay.
        if (currentTime - lastTimerTime >= timerInterval) {
            chip8.UpdateTimers();
            lastTimerTime += timerInterval;

            // Problem: the screen used to be redrawn after every instruction. CHIP-8 moves a
            // sprite by drawing it again at the old position (which erases it, since drawing
            // XORs) and then at the new one, so the moment in between, with the sprite gone,
            // was shown hundreds of times a second: that is the flicker. Games pace themselves
            // with the 60 Hz delay timer and finish erase + redraw within one tick, so showing
            // the screen once per tick, like the original display refresh, only shows
            // finished frames. It also does ~15x less drawing work.
            platform.Update(chip8.video, videoPitch);
        }

        // Beep for as long as the sound timer is non-zero
        platform.SetSound(chip8.SoundActive());

        if (dt > cycleDelay) {
            lastCycleTime = currentTime;
            chip8.Cycle();
        }

        // Problem: this loop never waits, it just re-checks the clock as fast as the CPU
        // allows, so it pins one core at 100% even when there is nothing to do. Sleeping
        // for 1 ms per pass costs almost no accuracy but drops the CPU usage to near zero.
        SDL_Delay(1);
    }

    return 0;
}