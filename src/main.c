#define SDL_MAIN_HANDLED
#include <SDL.h>
#include "render/render.h"
#include "render/shader.h"
#include "render/texture.h"
#include "render/mesh.h"
#include "ecs/ecs.h"
#include "ecs/builtin.h"
#include "audio/audio.h"

#include <cglm/cglm.h>

int main(int argc, char *argv[]) {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);

    SDL_Window *window = SDL_CreateWindow("vortex engine - Built at " __DATE__ " " __TIME__, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
    VE_Render_Init(window);
    VE_ProgramT *pTriangleProgram = VE_Render_CreateProgram("shaders/triangle.vert.spv", "shaders/triangle.frag.spv");
    VE_TextureT *pTexture = VE_Render_LoadTexture("texture.jpg", NULL);

    VE_MeshObject_T *pPlaneMesh = VE_Render_CreatePlaneMesh(1.0f, 1.0f, pTriangleProgram);
    VE_Render_SetMeshObjectTexture(pPlaneMesh, pTexture);
    VE_MeshObject_T *pCylinderMesh = VE_Render_CreateCylinderMesh(32, 0.5f, 1.0f, pTriangleProgram);
    VE_Render_SetMeshObjectTexture(pCylinderMesh, pTexture);
    pCylinderMesh->transform.position[2] = -1.0f;


    VE_Audio_Init();

    ALuint audio = VE_Audio_LoadSound("music.ogg");

    ALuint source = VE_Audio_CreateSource(audio);

    VE_SetupBuiltinComponents();

    VE_EntityHandleT entHandle = VE_ECS_CreateEntity();

    VE_ECS_InsertComponent(entHandle, VE_NewTestComponentSpawner());
    VE_ECS_InsertComponent(entHandle, VE_NewTransform((vec3) { 1.0f, 0.0f, 0.0f }, (vec3) { 0.0f, 0.0f, 0.0f }, (vec3) { 1.0f, 1.0f, 1.0f }));
    VE_ECS_InsertComponent(entHandle, VE_NewSoundPlayer(audio, 1.0, 1.0, 1));

    VE_PlaySoundPlayer(VE_ECS_GetComponent(entHandle, VE_SoundPlayerID));

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
                            VE_Render_SetMeshObjectTexture(pPlaneMesh, pTexture);
                            VE_Render_SetMeshObjectTexture(pCylinderMesh, pTexture);
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

        pPlaneMesh->transform.position[0] = SDL_sinf(SDL_GetTicks() / 1000.0f);
        VE_Render_UpdateMeshUniformBuffer(pPlaneMesh);
        glm_quatv(pCylinderMesh->transform.rotation, SDL_GetTicks() / 1000.0f, (vec3){0.0f, 0.0f, 1.0f});
        VE_Render_UpdateMeshUniformBuffer(pCylinderMesh);

        // Tick
        VE_ECS_UpdateScene();

        // Render
        VE_Render_BeginFrame();
        VE_Render_Draw(pCylinderMesh);
        VE_Render_Draw(pPlaneMesh);
        VE_Render_EndFrame();
    }

    VE_ECS_DestroyScene();

    VE_Audio_Destroy();

    VE_Render_DestroyTexture(pTexture);
    VE_Render_DestroyMeshObject(pCylinderMesh);
    VE_Render_DestroyMeshObject(pPlaneMesh);
    VE_Render_DestroyProgram(pTriangleProgram);
    VE_Render_Destroy();
    return 0;
}
