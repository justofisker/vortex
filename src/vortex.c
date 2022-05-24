#include "vortex.h"

void VE_Init() {
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window *window = SDL_CreateWindow("vortex engine - Built at " __DATE__ " " __TIME__, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
    VE_Render_Init(window);

    VE_Input_Init(window);

    VE_Input_SetMouseMode(VE_MOUSEMODE_RELATIVE);

    VE_Audio_Init();

    VE_SetupBuiltinComponents();
}

void VE_Run() {
    char running = 1;
    char minimized = 0;
    uint64_t last = SDL_GetTicks64();
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

        if (minimized) continue;

        // Render
        VE_Render_RenderScene();

        while (SDL_GetTicks64() < last + 2) {

        }

        VE_Input_EndFrame();

        last = SDL_GetTicks64();
    }
}

void VE_Destroy() {
    VE_ECS_DestroyScene();

    VE_Audio_Destroy();

    VE_Render_Destroy();
}
