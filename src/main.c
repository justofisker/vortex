#define SDL_MAIN_HANDLED
#include <SDL.h>
#include "render/render.h"
#include "render/shader.h"
#include "render/texture.h"
#include "ecs/ecs.h"
#include "ecs/builtin.h"
#include "audio/audio.h"

int main(int argc, char *argv[]) {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);

    SDL_Window *window = SDL_CreateWindow("vortex engine - Built at " __DATE__ " " __TIME__, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
    VE_Render_Init(window);
    VE_ProgramT *pTriangleProgram = VE_Render_CreateProgram("shaders/triangle.vert.spv", "shaders/triangle.frag.spv");
    VE_TextureT *pTexture = VE_Render_LoadTexture("texture.jpg", NULL);
    VE_Render_SetProgramSampler(pTriangleProgram, pTexture);

    VE_VertexT vertices[] = {
            {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
            {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
            {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
            {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},

            {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
            {{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
            {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
            {{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}
    };

    uint16_t indices[] = {
            0, 1, 2, 2, 3, 0,
            4, 5, 6, 6, 7, 4
    };

    VE_BufferT *pVertexBuffer = VE_Render_CreateVertexBuffer(vertices, sizeof(vertices) / sizeof(vertices[0]));
    VE_BufferT *pIndexBuffer = VE_Render_CreateIndexBuffer(indices, sizeof(indices) / sizeof(indices[0]));

    VE_Audio_Init();

    VE_SetupBuiltinComponents();

    VE_EntityHandleT entHandle = VE_ECS_CreateEntity();

    VE_TestComponentSpawner testComponentSpawner = VE_NewTestComponentSpawner();
    VE_ECS_InsertComponent(entHandle, &testComponentSpawner);
    VE_Transform transform = VE_NewTransform((vec3) { 0.0f, 1.0f, 0.0f }, (vec3) { 0.0f, 0.0f, 0.0f }, (vec3) { 1.0f, 1.0f, 1.0f });
    VE_ECS_InsertComponent(entHandle, &transform); 

    char running = 1;
    char minimized = 0;
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
                            VE_Render_SetProgramSampler(pTriangleProgram, pTexture);
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
        }
        if (minimized) continue;
        // Tick
        VE_ECS_UpdateScene();

        // Render
        VE_Render_BeginFrame();
        VE_Render_UpdateUniformBuffer(pTriangleProgram);
        VE_Render_Draw(pTriangleProgram, pVertexBuffer, pIndexBuffer);
        VE_Render_EndFrame();
    }

    VE_ECS_DestroyScene();

    VE_Audio_Destroy();

    VE_Render_DestroyTexture(pTexture);
    VE_Render_DestroyBuffer(pVertexBuffer);
    VE_Render_DestroyBuffer(pIndexBuffer);
    VE_Render_DestroyProgram(pTriangleProgram);
    VE_Render_Destroy();
    return 0;
}
