#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <iostream>
#include "chip8.hpp"
#include <string>

chip8 emulator;

int main(int argc, char *argv[]) {
    const int SCREEN_WIDTH = 640;
    const int SCREEN_HEIGHT = 320;
    int FPS = 500;
    int frameDelay = 1000 / FPS;
    Uint32 frameStart;
    int frameTime;

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        std::cout << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        std::cout << "SDL_mixer could not initialize! SDL_mixer Error: " << Mix_GetError() << std::endl;
        return 1;
    }
    Mix_Chunk *beep = Mix_LoadWAV("./beep.wav");
    if (beep == NULL) {
        std::cout << "Failed to load beep sound effect! SDL_mixer Error: " << Mix_GetError() << std::endl;
        return 1;
    }
    Mix_Volume(-1, MIX_MAX_VOLUME / 4);

    SDL_Window *window = SDL_CreateWindow(("Chip8: FPS: " + std::to_string(FPS)).c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        std::cout << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return 1;
    }
    SDL_SetWindowTitle(window, ("Chip8: FPS: " + std::to_string(FPS)).c_str());

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        std::cout << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    emulator.loadROM(argv[1]);

    while (true) {
        frameStart = SDL_GetTicks();

        emulator.cycle();

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        if (emulator.drawFlag) {
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_Rect rect;
            rect.w = 10;
            rect.h = 10;
            rect.x = 0;
            rect.y = 0;
            for (int i=0; i<64; ++i) {
                for (int j=0; j<32; ++j) {
                    if (emulator.display[i][j] == 1) {
                        rect.x = i*10;
                        rect.y = j*10;
                        SDL_RenderFillRect(renderer, &rect);
                    }
                }
            }
            SDL_RenderPresent(renderer);
            emulator.drawFlag = false;
        }

        if (emulator.soundTimer > 0) {
            Mix_PlayChannel(-1, beep, 0);
            --emulator.soundTimer;
        }

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    return 0;
                case SDL_KEYDOWN:
                    switch (event.key.keysym.sym) {
                        case SDLK_1:
                            emulator.keypad[0x1] = 1;
                            break;
                        case SDLK_2:
                            emulator.keypad[0x2] = 1;
                            break;
                        case SDLK_3:
                            emulator.keypad[0x3] = 1;
                            break;
                        case SDLK_4:
                            emulator.keypad[0xC] = 1;
                            break;
                        case SDLK_q:
                            emulator.keypad[0x4] = 1;
                            break;
                        case SDLK_w:
                            emulator.keypad[0x5] = 1;
                            break;
                        case SDLK_e:
                            emulator.keypad[0x6] = 1;
                            break;
                        case SDLK_r:
                            emulator.keypad[0xD] = 1;
                            break;
                        case SDLK_a:
                            emulator.keypad[0x7] = 1;
                            break;
                        case SDLK_s:
                            emulator.keypad[0x8] = 1;
                            break;
                        case SDLK_d:
                            emulator.keypad[0x9] = 1;
                            break;
                        case SDLK_f:
                            emulator.keypad[0xE] = 1;
                            break;
                        case SDLK_z:
                            emulator.keypad[0xA] = 1;
                            break;
                        case SDLK_x:
                            emulator.keypad[0x0] = 1;
                            break;
                        case SDLK_c:
                            emulator.keypad[0xB] = 1;
                            break;
                        case SDLK_v:
                            emulator.keypad[0xF] = 1;
                            break;
                        case SDLK_UP:
                            ++FPS;
                            frameDelay = 1000 / FPS;
                            SDL_SetWindowTitle(window, ("Chip8: FPS: " + std::to_string(FPS)).c_str());
                            break;
                        case SDLK_DOWN:
                            --FPS;
                            frameDelay = 1000 / FPS;
                            SDL_SetWindowTitle(window, ("Chip8: FPS: " + std::to_string(FPS)).c_str());
                            break;
                    }
                    break;
                case SDL_KEYUP:
                    switch (event.key.keysym.sym) {
                        case SDLK_1:
                            emulator.keypad[0x1] = 0;
                            break;
                        case SDLK_2:
                            emulator.keypad[0x2] = 0;
                            break;
                        case SDLK_3:
                            emulator.keypad[0x3] = 0;
                            break;
                        case SDLK_4:
                            emulator.keypad[0xC] = 0;
                            break;
                        case SDLK_q:
                            emulator.keypad[0x4] = 0;
                            break;
                        case SDLK_w:
                            emulator.keypad[0x5] = 0;
                            break;
                        case SDLK_e:
                            emulator.keypad[0x6] = 0;
                            break;
                        case SDLK_r:
                            emulator.keypad[0xD] = 0;
                            break;
                        case SDLK_a: 
                            emulator.keypad[0x7] = 0;
                            break;
                        case SDLK_s:
                            emulator.keypad[0x8] = 0;
                            break;
                        case SDLK_d:
                            emulator.keypad[0x9] = 0;
                            break;
                        case SDLK_f:
                            emulator.keypad[0xE] = 0;
                            break;
                        case SDLK_z:
                            emulator.keypad[0xA] = 0;
                            break;
                        case SDLK_x:
                            emulator.keypad[0x0] = 0;
                            break;
                        case SDLK_c:
                            emulator.keypad[0xB] = 0;
                            break;
                        case SDLK_v:
                            emulator.keypad[0xF] = 0;
                            break;
                    }
                    break;
            }
        }

        frameTime = SDL_GetTicks() - frameStart;
        if (frameDelay > frameTime) {
            SDL_Delay(frameDelay - frameTime);
        }
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}