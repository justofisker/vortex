#define SDL_MAIN_HANDLED
#include <SDL.h>
#include "render/render.h"
#include "render/shader.h"
#include "render/texture.h"
#include "render/mesh.h"
#include "ecs/ecs.h"
#include "ecs/builtin.h"
#include "audio/audio.h"
#include "input/input.h"

#include <cglm/cglm.h>

int main(int argc, char *argv[]) {
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window *window = SDL_CreateWindow("vortex engine - Built at " __DATE__ " " __TIME__, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
    VE_Render_Init(window);
    VE_ProgramT *pMaterialProgram = VE_Render_CreateProgram("shaders/material.vert.spv", "shaders/material.frag.spv");
    VE_ProgramT* pEnvironmentSphereProgram = VE_Render_CreateProgram("shaders/environment_sphere.vert.spv", "shaders/environment_sphere.frag.spv");
    VE_TextureT *pTexture = VE_Render_LoadTexture("assets/textures/texture.png", NULL);
    VE_TextureT *pEnvironment = VE_Render_LoadTexture("assets/textures/environment.hdr", NULL);

    VE_Input_Init(window);

    VE_Input_SetMouseMode(VE_MOUSEMODE_RELATIVE);

    VE_Audio_Init();

    ALuint audio = VE_Audio_LoadSound("assets/sound/music.ogg");

    VE_SetupBuiltinComponents();

    VE_EntityHandleT entTest = VE_ECS_CreateEntity();
    //VE_ECS_InsertComponent(entTest, VE_NewTestComponentSpawner(pMaterialProgram, pTexture));
    VE_ECS_InsertComponent(entTest, VE_NewTransform((vec3) {1.0f, 0.0f, 0.0f }, (vec3) {0.0f, 0.0f, 0.0f }, (vec3) {1.0f, 1.0f, 1.0f }));

    VE_EntityHandleT entCamera = VE_ECS_CreateEntity();
    VE_ECS_InsertComponent(entCamera, VE_NewFlyCam(4.0, 0.001));
    VE_ECS_InsertComponent(entCamera, VE_NewTransform((vec3) { 2.0, 2.0, 2.0 }, (vec3) { glm_rad(-45.0), glm_rad(45.0), 0.0 }, GLM_VEC3_ONE));
    VE_ECS_InsertComponent(entCamera, VE_NewCamera(glm_rad(60.0), 0.1, 256.0));
    VE_ECS_InsertComponent(entCamera, VE_NewAudioListener());

    VE_EntityHandleT entPlane = VE_ECS_CreateEntity();
    VE_ECS_InsertComponent(entPlane, VE_NewTransform(GLM_VEC3_ZERO, GLM_VEC3_ZERO, GLM_VEC3_ONE));
    VE_ECS_InsertComponent(entPlane, VE_NewMesh(VE_Render_CreatePlaneMesh(1.0f, 1.0f, pMaterialProgram)));
    VE_Render_SetMeshObjectTexture(((VE_Mesh*)VE_ECS_GetComponent(entPlane, VE_MeshID))->pMeshObject, pTexture);
    VE_ECS_InsertComponent(entPlane, VE_NewSoundPlayer(audio, 1.0, 1.0, 1));
    VE_PlaySoundPlayer(VE_ECS_GetComponent(entPlane, VE_SoundPlayerID));

    VE_EntityHandleT entCylinder = VE_ECS_CreateEntity();
    VE_ECS_InsertComponent(entCylinder, VE_NewTransform((vec3){0.0f, -2.0f, 0.0f}, GLM_VEC3_ZERO, GLM_VEC3_ONE));
    VE_ECS_InsertComponent(entCylinder, VE_NewMesh(VE_Render_CreateCylinderMesh(32, 0.5f, 1.0f, pMaterialProgram)));
    VE_Render_SetMeshObjectTexture(((VE_Mesh*)VE_ECS_GetComponent(entCylinder, VE_MeshID))->pMeshObject, pTexture);

    VE_EntityHandleT entSphere = VE_ECS_CreateEntity();
    VE_ECS_InsertComponent(entSphere, VE_NewTransform((vec3) { 0.0f, 0.0f, 0.0f }, GLM_VEC3_ZERO, GLM_VEC3_ONE));
    VE_ECS_InsertComponent(entSphere, VE_NewMesh(VE_Render_CreateUVSphereMesh(-128.0, 32, 64, pEnvironmentSphereProgram)));
    VE_Render_SetMeshObjectTexture(((VE_Mesh *)VE_ECS_GetComponent(entSphere, VE_MeshID))->pMeshObject, pEnvironment);


    VE_ImportedModel_T importedModel = VE_Render_ImportModel("assets/models/testModel.obj", pMaterialProgram);
    VE_EntityHandleT entTestModel = VE_ECS_CreateEntity();
    VE_ECS_InsertComponent(entTestModel, VE_NewTransform((vec3) { 0.0f, -8.0f, 0.0f }, GLM_VEC3_ZERO, GLM_VEC3_ONE));
    for (int i = 0; i < importedModel.numMeshes; i++) {
        VE_ECS_InsertComponent(entTestModel, VE_NewMesh(importedModel.meshes[i]));
    }

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
                            VE_Render_SetMeshObjectTexture(((VE_Mesh*)VE_ECS_GetComponent(entPlane, VE_MeshID))->pMeshObject, pTexture);
                            VE_Render_SetMeshObjectTexture(((VE_Mesh*)VE_ECS_GetComponent(entCylinder, VE_MeshID))->pMeshObject, pTexture);
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

        if (VE_Input_IsKeyJustPressed(SDL_SCANCODE_ESCAPE)) {
            if (VE_Input_GetMouseMode() == VE_MOUSEMODE_RELATIVE) {
                VE_Input_SetMouseMode(VE_MOUSEMODE_NORMAL);
                ivec2 gameSize;
                VE_Render_GetGameSize(gameSize);
                VE_Input_SetMousePosition(gameSize[0] / 2, gameSize[1] / 2);
            } else
                VE_Input_SetMouseMode(VE_MOUSEMODE_RELATIVE);
        }

        ((VE_Transform*) VE_ECS_GetComponent(entPlane, VE_TransformID))->position[0] = SDL_sinf(SDL_GetTicks() / 1000.0f);
        ((VE_Transform*) VE_ECS_GetComponent(entPlane, VE_TransformID))->_update = 1;
        ((VE_Transform*) VE_ECS_GetComponent(entCylinder, VE_TransformID))->rotation[2] = SDL_GetTicks() / 1000.0f;
        ((VE_Transform*) VE_ECS_GetComponent(entCylinder, VE_TransformID))->_update = 1;

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

    VE_ECS_DestroyScene();

    VE_Render_DestroyImportedModel(importedModel);

    VE_Audio_DestroySound(audio);

    VE_Audio_Destroy();

    VE_Render_DestroyTexture(pEnvironment);
    VE_Render_DestroyTexture(pTexture);
    VE_Render_DestroyProgram(pEnvironmentSphereProgram);
    VE_Render_DestroyProgram(pMaterialProgram);
    VE_Render_Destroy();
    return 0;
}
