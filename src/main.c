#define SDL_MAIN_HANDLED
#include <SDL.h>
#include "render/render.h"
#include "render/shader.h"
#include "ecs/ecs.h"
#include "ecs/builtin.h"

int main(int argc, char *argv[]) {
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window *window = SDL_CreateWindow("vortex engine - Built at " __DATE__ " " __TIME__, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
    VE_Render_Init(window);
    VE_ProgramT *pTriangleProgram = VE_Render_CreateProgram("shaders/triangle.vert.spv", "shaders/triangle.frag.spv");

    VE_VertexT vertices[] = {
            {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
            {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
            {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
            {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
    };

    uint16_t indices[] = {
            0, 1, 2, 2, 3, 0
    };

    VE_BufferT *pVertexBuffer = VE_Render_CreateVertexBuffer(vertices, sizeof(vertices) / sizeof(vertices[0]));
    VE_BufferT *pIndexBuffer = VE_Render_CreateIndexBuffer(indices, sizeof(indices) / sizeof(indices[0]));

    VE_SetupBuiltinComponents();

    VE_SceneT scene = { NULL, 0 };

    uint32_t ent_handle = VE_ECS_CreateEntity(&scene);

    VE_EntityT *entity = VE_ECS_GetEntity(&scene, ent_handle);
    VE_TestComponent testComponent = VE_NewTestComponent(0);
    VE_ECS_InsertComponent(entity, &testComponent);
    VE_Transform transform = VE_NewTransform((vec3) { 0.0f, 1.0f, 0.0f }, (vec3) { 0.0f, 0.0f, 0.0f }, (vec3) { 1.0f, 1.0f, 1.0f });
    VE_ECS_InsertComponent(entity, &transform);

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
        VE_ECS_UpdateScene(&scene);

        // Render
        VE_Render_BeginFrame();
        VE_Render_UpdateUniformBuffer(pTriangleProgram);
        VE_Render_Draw(pTriangleProgram, pVertexBuffer, pIndexBuffer);
        VE_Render_EndFrame();
    }

    VE_Render_DestroyBuffer(pVertexBuffer);
    VE_Render_DestroyBuffer(pIndexBuffer);
    VE_Render_DestroyProgram(pTriangleProgram);
    VE_Render_Destroy();
    return 0;
}
