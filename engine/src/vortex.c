#include "vortex.h"

#include <stdio.h>

static SDL_Window *window;

const char *TITLE = "vortex engine";

void VE_Init(const char* title) {
    SDL_Init(SDL_INIT_VIDEO);

    TITLE = title;

    window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
    VE_Render_Init(window);

    VE_Input_Init(window);

    VE_Audio_Init();

    VE_SetupBuiltinComponents();
}

void VE_Run() {
    char running = 1;
    char minimized = 0;
    uint64_t last = SDL_GetPerformanceCounter();
    {
        char buf[256] = { 0 };
        snprintf(buf, sizeof(buf) - 1, "%s | fps: %.1f", TITLE, 0.0);
        SDL_SetWindowTitle(window, buf);
    }
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                running = 0;
                break;
            case SDL_WINDOWEVENT:
                switch (event.window.event) {
                case SDL_WINDOWEVENT_RESIZED:
                    VE_Render_Resize();
                    break;
                case SDL_WINDOWEVENT_MINIMIZED:
                    minimized = 1;
                    break;
                case SDL_WINDOWEVENT_RESTORED:
                    minimized = 0;
                    break;
                }
                break;
            }
            VE_Input_Event(&event);
        }

        // Tick
        VE_ECS_UpdateScene();
        VE_Input_EndFrame();

        if (minimized) SDL_Delay(5);
        else {

            // Render
            VE_Render_RenderScene();

            {
                int fps = 1.0 / ((SDL_GetPerformanceCounter() - last) / (double)SDL_GetPerformanceFrequency());
                char buf[256] = { 0 };
                snprintf(buf, sizeof(buf) - 1, "%s | fps: %d", TITLE, fps);
                SDL_SetWindowTitle(window, buf);
            }
        }

        last = SDL_GetPerformanceCounter();
    }
}

void VE_Destroy() {
    VE_ECS_DestroyScene();

    VE_Audio_Destroy();

    VE_Render_Destroy();
}
