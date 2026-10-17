#include "platform.hpp"
#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {
    const int SAMPLE_RATE = 44100;
    const float BEEP_FREQUENCY = 440.0f;   // Hz
    const float BEEP_VOLUME = 0.15f;       // 0..1, a square wave is loud so keep it low
    const float FADE_SECONDS = 0.005f;     // ramp volume over 5 ms to avoid clicks
}

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

    // Sound is optional: if there is no audio device, carry on without a beep
    // instead of refusing to start the emulator
    if (!SDL_InitSubSystem(SDL_INIT_AUDIO)) {
        std::cerr << "Warning: audio unavailable, running without sound (" << SDL_GetError() << ")\n";
        return;
    }

    SDL_AudioSpec spec{};
    spec.format = SDL_AUDIO_S16;
    spec.channels = 1;
    spec.freq = SAMPLE_RATE;

    audioStream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, AudioCallback, this);
    if (!audioStream) {
        std::cerr << "Warning: could not open audio device, running without sound (" << SDL_GetError() << ")\n";
        return;
    }

    // Audio devices start paused. The callback outputs silence until SetSound(true).
    SDL_ResumeAudioStreamDevice(audioStream);
}

Platform::~Platform() {
    Cleanup();
}

void Platform::Cleanup() {
    if (audioStream) {
        SDL_DestroyAudioStream(audioStream);
        audioStream = nullptr;
    }
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

void Platform::SetSound(bool on) {
    beeping = on;
}

void SDLCALL Platform::AudioCallback(void* userdata, SDL_AudioStream* stream, int additionalAmount, int /*totalAmount*/) {
    Platform* self = static_cast<Platform*>(userdata);

    float const target = self->beeping ? 1.0f : 0.0f;
    float const fadeStep = 1.0f / (FADE_SECONDS * SAMPLE_RATE);
    float const phaseStep = BEEP_FREQUENCY / SAMPLE_RATE;

    // additionalAmount is in bytes; each sample is a 16 bit integer
    int samplesLeft = additionalAmount / static_cast<int>(sizeof(int16_t));

    // Generate in small chunks so the buffer can live on the stack
    int16_t buffer[512];

    while (samplesLeft > 0) {
        int count = std::min(samplesLeft, 512);

        for (int i = 0; i < count; ++i) {
            // Move the volume toward the target (on or off) a little each sample
            if (self->gain < target) {
                self->gain = std::min(self->gain + fadeStep, target);
            } else if (self->gain > target) {
                self->gain = std::max(self->gain - fadeStep, target);
            }

            float square = (self->phase < 0.5f) ? 1.0f : -1.0f;
            self->phase += phaseStep;
            if (self->phase >= 1.0f) {
                self->phase -= 1.0f;
            }

            buffer[i] = static_cast<int16_t>(square * self->gain * BEEP_VOLUME * 32767.0f);
        }

        SDL_PutAudioStreamData(stream, buffer, count * static_cast<int>(sizeof(int16_t)));
        samplesLeft -= count;
    }
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
