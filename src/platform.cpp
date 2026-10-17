#include "platform.hpp"
#include <stdexcept>
#include <string>

Platform::Platform(char const* title, int windowWidth, int windowHeight, int textureWidth, int textureHeight) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        throw std::runtime_error(std::string("SDL_Init failed: ") + SDL_GetError());
    }

    window = SDL_CreateWindow(title, windowWidth, windowHeight, 0);
    if (!window) {
        Fail("SDL_CreateWindow failed");
    }

    renderer = SDL_CreateRenderer(window, nullptr);
    if (!renderer) {
        Fail("SDL_CreateRenderer failed");
    }

    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, textureWidth, textureHeight);
    if (!texture) {
        Fail("SDL_CreateTexture failed");
    }

    // Keep the pixels sharp when scaling the 64x32 buffer up to the window
    SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);
}

Platform::~Platform() {
    Cleanup();
}

void Platform::Cleanup() {
    if (texture) {
        SDL_DestroyTexture(texture);
        texture = nullptr;
    }
    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }
    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }
    SDL_Quit();
}

void Platform::Fail(char const* what) {
    std::string message = std::string(what) + ": " + SDL_GetError();
    Cleanup();
    throw std::runtime_error(message);
}

void Platform::Update(void const* buffer, int pitch) {
    SDL_UpdateTexture(texture, nullptr, buffer, pitch);
    SDL_RenderClear(renderer);
    SDL_RenderTexture(renderer, texture, nullptr, nullptr);
    SDL_RenderPresent(renderer);
}

bool Platform::ProcessInput(uint8_t* keys) {
    bool quit = false;
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_EVENT_QUIT:
                quit = true;
                break;

            case SDL_EVENT_KEY_DOWN:
                if (event.key.scancode == SDL_SCANCODE_ESCAPE) {
                    quit = true;
                } else {
                    SetKey(keys, event.key.scancode, 1);
                }
                break;

            case SDL_EVENT_KEY_UP:
                SetKey(keys, event.key.scancode, 0);
                break;
        }
    }

    return quit;
}

// Keypad layout, mapped by physical key position so it works on any layout:
//   1 2 3 4        1 2 3 C
//   Q W E R   ->   4 5 6 D
//   A S D F        7 8 9 E
//   Z X C V        A 0 B F
void Platform::SetKey(uint8_t* keys, SDL_Scancode scancode, uint8_t state) {
    switch (scancode) {
        case SDL_SCANCODE_X: keys[0x0] = state; break;
        case SDL_SCANCODE_1: keys[0x1] = state; break;
        case SDL_SCANCODE_2: keys[0x2] = state; break;
        case SDL_SCANCODE_3: keys[0x3] = state; break;
        case SDL_SCANCODE_Q: keys[0x4] = state; break;
        case SDL_SCANCODE_W: keys[0x5] = state; break;
        case SDL_SCANCODE_E: keys[0x6] = state; break;
        case SDL_SCANCODE_A: keys[0x7] = state; break;
        case SDL_SCANCODE_S: keys[0x8] = state; break;
        case SDL_SCANCODE_D: keys[0x9] = state; break;
        case SDL_SCANCODE_Z: keys[0xA] = state; break;
        case SDL_SCANCODE_C: keys[0xB] = state; break;
        case SDL_SCANCODE_4: keys[0xC] = state; break;
        case SDL_SCANCODE_R: keys[0xD] = state; break;
        case SDL_SCANCODE_F: keys[0xE] = state; break;
        case SDL_SCANCODE_V: keys[0xF] = state; break;
        default: break;
    }
}
