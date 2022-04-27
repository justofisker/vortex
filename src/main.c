#define SDL_MAIN_HANDLED
#include <SDL.h>
#include "render/render.h"
#include "render/shader.h"

int main(int argc, char *argv[]) {
    SDL_Window *window = SDL_CreateWindow("vortex engine - Built at " __DATE__ " " __TIME__, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_VULKAN);
    VE_Render_Init(window);
    VE_Shader *pTriangleShader = VE_Render_CreateShader("shaders/triangle.vert.spv", "shaders/triangle.frag.spv");

    char running = 1;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                running = 0;
        }
    }

    VE_Render_DestroyShader(pTriangleShader);
    VE_Render_Destroy();
    SDL_DestroyWindow(window);
    return 0;
}
