#include <cstdint>


const unsigned int MEMORY_SIZE = 4096;
const unsigned int REGISTER_COUNT = 16;
const unsigned int STACK_LEVELS = 16;
const unsigned int VIDEO_HEIGHT = 32;
const unsigned int VIDEO_LENGTH = 64;


class Chip8 {
    public:
        Chip8();
        void LoadROM(char const* filename);

    private:

        uint8_t memory[MEMORY_SIZE]{};
        uint8_t registers[REGISTER_COUNT]{};
        uint16_t pc{};
        uint16_t index{};
        uint8_t delayTimer{};
        uint8_t soundTimer{};
        uint16_t stack[STACK_LEVELS]{};
        uint8_t sp{};
        uint16_t opcode{};
};