#ifndef CHIP8_HPP
#define CHIP8_HPP
#include <cstdint>

class chip8 {
    public:
        chip8();
        void loadROM(const char * filename);
        void cycle();
        uint8_t display[64][32];
        uint8_t keypad[16]; // Keypad
        uint8_t soundTimer; // Sound timer
        bool drawFlag;
    private:
        uint16_t opcode; // 2 bytes
        uint8_t memory[4096]; // 4K memory
        uint8_t V[16]; // 16 8-bit registers
        uint16_t I; // Index register
        uint16_t pc; // Program counter
        uint8_t delayTimer; // Delay timer
        uint16_t stack[16]; // Stack
        uint8_t sp; // Stack pointer
};

#endif