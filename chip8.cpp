#include <fstream>
#include <cstdint>
#include <iostream>
#include "chip8.hpp"
#include <bitset>

uint8_t fontset[80] = { // each character is 5 bytes 5x16=80
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80 // F
};

chip8::chip8() {
    pc = 0x200; // Start at address 0x200
    opcode = 0; 
    I = 0; 
    sp = 0; 
    soundTimer = 0;
    delayTimer = 0;
    std::fill(std::begin(memory), std::end(memory), 0);
    std::fill(std::begin(V), std::end(V), 0);
    std::fill(std::begin(stack), std::end(stack), 0);
    memset(display, 0, sizeof(display));
    std::fill(std::begin(keypad), std::end(keypad), 0);

    for (int i=0; i<80; ++i)
        memory[0x50+i] = fontset[i];

    std::cout << "chip8 initialized" << std::endl;
}

void chip8::loadROM(const char* filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (file.is_open()) {
        std::streamsize size = file.tellg();
        char* buffer = new char[size];

        file.seekg(0, std::ios::beg);
        file.read(buffer, size);
        file.close();

        for (long i=0; i<size; ++i)
            memory[0x200 + i] = buffer[i];

        std::cout << "ROM loaded" << std::endl;
        
        delete[] buffer;
    } else {
        std::cout << "Unable to open file" << std::endl;
    }
}

void chip8::cycle() {
    // Fetch opcode
    opcode = memory[pc] << 8 | memory[pc + 1];
    //std::cout << std::hex << pc << std::endl;
    // Decode and execute opcode
    switch(opcode & 0xF000) { 
        case 0x0000: 
            switch (opcode & 0x000F) { //get last nibble
                case 0x0000: // 0x00E0: Clears the screen
                    memset(display, 0, sizeof(display));
                    pc += 2;
                    break;
                case 0x000E: // 0x00EE: Returns from a subroutine
                    --sp;
                    pc = stack[sp];
                    pc += 2;
                    break;
            }
            break;
        case 0x1000: // 0x1NNN: Jumps to address NNN
            pc = opcode & 0x0FFF;
            break;
        case 0x2000: // 0x2NNN: Calls subroutine at NNN
            stack[sp] = pc;
            ++sp;
            pc = opcode & 0x0FFF;
            break;
        case 0x3000: // 0x3XNN: Skips the next instruction if VX equals NN
            if (V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF))
                pc += 4;
            else
                pc += 2;
            break;
        case 0x4000: // 0x4XNN: Skips the next instruction if VX doesn't equal NN
            if (V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF))
                pc += 4;
            else
                pc += 2;
            break;
        case 0x5000: // 0x5XY0: Skips the next instruction if VX equals VY
            if (V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 4])
                pc += 4;
            else
                pc += 2;
            break;
        case 0x6000: // 0x6XNN: Sets VX to NN
            V[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
            pc += 2;
            break;
        case 0x7000: // 0x7XNN: Adds NN to VX
            V[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
            pc += 2;
            break;
        case 0x8000:
            switch (opcode & 0x000F) {
                case 0x0000: // 0x8XY0: Sets VX to the value of VY
                    V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;
                case 0x0001: // 0x8XY1: Sets VX to VX or VY
                    V[(opcode & 0x0F00) >> 8] |= V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;
                case 0x0002: // 0x8XY2: Sets VX to VX and VY
                    V[(opcode & 0x0F00) >> 8] &= V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;
                case 0x0003: // 0x8XY3: Sets VX to VX xor VY
                    V[(opcode & 0x0F00) >> 8] ^= V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;
                case 0x0004: // 0x8XY4: Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there isn't
                    V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];
                    if (V[(opcode & 0x0F00) >> 8] > 255)
                        V[0xF] = 1;
                    else
                        V[0xF] = 0;
                    pc += 2;
                    break;
                case 0x0005: // 0x8XY5: VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there isn't
                    if (V[(opcode & 0x0F00) >> 8] > V[(opcode & 0x00F0) >> 4])
                        V[0xF] = 1;
                    else
                        V[0xF] = 0;
                    V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;
                case 0x0006: // 0x8XY6: Stores the least significant bit of VX in VF and then shifts VX to the right by 1
                    V[0xF] = V[(opcode & 0x0F00) >> 8] & 0x1;
                    V[(opcode & 0x0F00) >> 8] >>= 1;
                    pc += 2;
                    break;
                case 0x0007: // 0x8XY7: Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there isn't
                    if (V[(opcode & 0x00F0) >> 4] > V[(opcode & 0x0F00) >> 8])
                        V[0xF] = 1;
                    else
                        V[0xF] = 0;
                    V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8];
                    pc += 2;
                    break;
                case 0x000E: // 0x8XYE: Stores the most significant bit of VX in VF and then shifts VX to the left by 1
                    V[0xF] = V[(opcode & 0x0F00) >> 8] >> 7;
                    V[(opcode & 0x0F00) >> 8] <<= 1;
                    pc += 2;
                    break;
            }
            break;
        case 0x9000: // 0x9XY0: Skips the next instruction if VX doesn't equal VY
            if (V[(opcode & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4])
                pc += 4;
            else
                pc += 2;
            break;
        case 0xA000: // 0xANNN: Sets I to the address NNN
            I = opcode & 0x0FFF;
            pc += 2;
            break;
        case 0xB000: // 0xBNNN: Jumps to the address NNN plus V0
            pc = (opcode & 0x0FFF) + V[0];
            break;
        case 0xC000: // 0xCXNN: Sets VX to the result of a bitwise and operation on a random number and NN
            V[(opcode & 0x0F00) >> 8] = (rand() % 256) & (opcode & 0x00FF);
            pc += 2;
            break;
        case 0xD000: {// 0xDXYN: Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N pixels
            uint8_t x = V[(opcode & 0x0F00) >> 8] % 64;
            uint8_t y = V[(opcode & 0x00F0) >> 4] % 32;
            uint8_t height = opcode & 0x000F;
            V[0xF] = 0; // set VF to 0

            for (int row=0; row<height; ++row) {
                uint8_t spriteRow = memory[I+row];
                for (int col=0; col<8; ++col) {
                    if ((spriteRow & (0x80 >> col)) != 0) {
                        if (display[x+col][y+row] == 1)
                            V[0xF] = 1;
                        display[x+col][y+row] ^= 1;
                    }
                }
            }
            drawFlag = true;
            pc += 2;
            break;
        }
        case 0xE000:
            switch (opcode & 0x000F) {
                case 0x000E: // 0xEX9E: Skips the next instruction if the key stored in VX is presed 
                    if (keypad[V[(opcode & 0x0F00) >> 8]])
                        pc += 4;
                    else
                        pc += 2;
                    break;
                case 0x0001: // 0xEXA1: Skips the next instruction if the key stored in VX isn't pressed
                    if (!keypad[V[(opcode & 0x0F00) >> 8]])
                        pc += 4;
                    else
                        pc += 2;
                    break;
            }
            break;
        case 0xF000:
            switch(opcode & 0x00FF) {
                case 0x0007: // 0xFX07: Sets VX to the value of the delay timer
                    V[(opcode & 0x0F00) >> 8] = delayTimer;
                    pc += 2;
                    break;
                case 0x000A: // 0xFX0A: A key press is awaited, and then stored in VX
                    for (int i=0; i<16; ++i) {
                        if (keypad[i]) {
                            V[(opcode & 0x0F00) >> 8] = i;
                            pc += 2;
                        }
                    }
                    break;
                case 0x0015: // 0xFX15: Sets the delay timer to VX
                    delayTimer = V[(opcode & 0x0F00) >> 8];
                    pc += 2;
                    break;
                case 0x0018: // 0xFX18: Sets the sound timer to VX
                    soundTimer = V[(opcode & 0x0F00) >> 8];
                    pc += 2;
                    break;
                case 0x001E: // 0xFX1E: Adds VX to I
                    I += V[(opcode & 0x0F00) >> 8];
                    pc += 2;
                    break;
                case 0x0029: // 0xFX29: Sets I to the location of the sprite for the character in VX
                    I = 0x50 + V[(opcode & 0x0F00) >> 8] * 5;
                    pc += 2;
                    break;
                case 0x0033: // 0xFX33: Stores the binary-coded decimal representation of VX
                    memory[I] = V[(opcode & 0x0F00) >> 8] / 100;
                    memory[I+1] = (V[(opcode & 0x0F00) >> 8] / 10) % 10;
                    memory[I+2] = (V[(opcode & 0x0F00) >> 8] % 100) % 10;
                    pc += 2;
                    break;
                case 0x0055: // 0xFX55: Stores V0 to VX in memory starting at address I
                    for (int i=0; i<=((opcode & 0x0F00) >> 8); ++i)
                        memory[I+i] = V[i];
                    pc += 2;
                    break;
                case 0x0065: // 0xFX65: Fills V0 to VX with values from memory starting at address I
                    for (int i=0; i<=((opcode & 0x0F00) >> 8); ++i)
                        V[i] = memory[I+i];
                    pc += 2;
                    break;
            }
            break;
    }

    if (delayTimer > 0)
        --delayTimer;
    if (soundTimer > 0) {
        --soundTimer;
    }
}