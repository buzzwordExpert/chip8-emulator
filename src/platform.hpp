#pragma once

#include <SDL3/SDL.h>
#include <atomic>
#include <cstdint>

class Platform {
    public:
        Platform(char const* title, int windowWidth, int windowHeight, int textureWidth, int textureHeight);
        ~Platform();

        Platform(Platform const&) = delete;
        Platform& operator=(Platform const&) = delete;
        void Update(void const* buffer, int pitch);
        
        // Poll events and update the 16-key hex keypad in place.
        // Returns true when the emulator should quit (window closed or Escape).
        bool ProcessInput(uint8_t* keys);

        // Turn the beep on or off (call with true while the sound timer is running).
        // If no audio device could be opened this does nothing.
        void SetSound(bool on);

    private:
        SDL_Window* window{};
        SDL_Renderer* renderer{};
        SDL_Texture* texture{};
        SDL_AudioStream* audioStream{};

        // Written by the main thread, read by SDL's audio thread
        std::atomic<bool> beeping{false};

        // Only touched by the audio thread: position in the square wave and the
        // current volume, which ramps up and down to avoid clicks
        float phase{};
        float gain{};

        // Called by SDL whenever the audio device needs more samples
        static void SDLCALL AudioCallback(void* userdata, SDL_AudioStream* stream, int additionalAmount, int totalAmount);

        void Cleanup();

        // The destructor doesn't run when a constructor throws, so clean up here
        [[noreturn]] void Fail(char const* what);

        // Map a physical key to the hex keypad
        static void SetKey(uint8_t* keys, SDL_Scancode scancode, uint8_t state);
};
