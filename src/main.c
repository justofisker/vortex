#include <stdio.h>
#include <SDL.h>
#include "render/render.h"

int main(int argc, char *argv[]) {
    SDL_Window *window = SDL_CreateWindow("vortex engine - Built at " __DATE__ " " __TIME__, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_VULKAN);
    VE_RenderInit(window);

    char running = 1;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                running = 0;
        }
    }

    SDL_DestroyWindow(window);
    return 0;
}