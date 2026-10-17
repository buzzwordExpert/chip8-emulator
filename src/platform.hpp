#pragma once

#include <SDL3/SDL.h>
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

    private:
        SDL_Window* window{};
        SDL_Renderer* renderer{};
        SDL_Texture* texture{};

        void Cleanup();

        // The destructor doesn't run when a constructor throws, so clean up here
        [[noreturn]] void Fail(char const* what);

        // Map a physical key to the hex keypad
        static void SetKey(uint8_t* keys, SDL_Scancode scancode, uint8_t state);
};
