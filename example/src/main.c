#include <vortex.h>

#include <cglm/cglm.h>

typedef struct VE_TestSceneComponent {
    uint32_t _id;
    VE_EntityHandleT entPlane;
    VE_EntityHandleT entCylinder;
} VE_TestSceneComponent;

void VE_TestSceneComponent_UpdateSystem(VE_EntityHandleT entityHandle, void *pData) {
    VE_TestSceneComponent *testSceneComponent = pData;

    VE_Transform *planeTransform = VE_ECS_GetComponent(testSceneComponent->entPlane, VE_TransformID);
    if (planeTransform) {
        planeTransform->position[0] = SDL_sinf(SDL_GetTicks() / 1000.0f);
        planeTransform->_update = 1;
    }
    VE_Transform *cylinderTransform = VE_ECS_GetComponent(testSceneComponent->entCylinder, VE_TransformID);
    if (cylinderTransform) {
        cylinderTransform->rotation[2] = SDL_GetTicks() / 1000.0f;
        cylinderTransform->_update = 1;
    }
}

int main(int argc, char *argv[]) {
    VE_Init("ExampleVortex");
    VE_Input_SetMouseMode(VE_MOUSEMODE_RELATIVE);
    uint32_t VE_TestSceneComponentID = VE_ECS_REGISTER_COMPONENT(VE_TestSceneComponent, VE_TestSceneComponent_UpdateSystem, NULL);

    VE_ProgramT *pMaterialProgram = VE_Render_CreateProgram("shaders/material.vert.spv", "shaders/material.frag.spv");
    VE_ProgramT *pEnvironmentSphereProgram = VE_Render_CreateProgram("shaders/environment_sphere.vert.spv", "shaders/environment_sphere.frag.spv");
    VE_TextureT *pTexture = VE_Render_LoadTexture("assets/textures/texture.png", NULL);
    VE_TextureT *pEnvironment = VE_Render_LoadTexture("assets/textures/environment.hdr", NULL);

    ALuint audio = VE_Audio_LoadSound("assets/sound/music.ogg");

    VE_EntityHandleT entTest = VE_ECS_CreateEntity();
    //VE_ECS_InsertComponent(entTest, VE_NewTestComponentSpawner(pMaterialProgram, pTexture));
    VE_ECS_InsertComponent(entTest, VE_NewTransform((vec3) {1.0f, 0.0f, 0.0f }, (vec3) {0.0f, 0.0f, 0.0f }, (vec3) {1.0f, 1.0f, 1.0f }));

    VE_EntityHandleT entCamera = VE_ECS_CreateEntity();
    VE_ECS_InsertComponent(entCamera, VE_NewFlyCam(4.0, 0.001));
    VE_ECS_InsertComponent(entCamera, VE_NewTransform((vec3) { 2.0, 2.0, 2.0 }, (vec3) { glm_rad(-45.0), glm_rad(45.0), 0.0 }, GLM_VEC3_ONE));
    VE_ECS_InsertComponent(entCamera, VE_NewCamera(glm_rad(60.0), 0.01, 256.0));
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

    VE_TestSceneComponent *component = malloc(sizeof(VE_TestSceneComponent));
    *component = (VE_TestSceneComponent){VE_TestSceneComponentID, entPlane, entCylinder};
    VE_ECS_InsertComponent(entTest, component);

    VE_Run();

    VE_Render_DestroyImportedModel(importedModel);

    VE_Audio_DestroySound(audio);

    VE_Render_DestroyTexture(pEnvironment);
    VE_Render_DestroyTexture(pTexture);
    VE_Render_DestroyProgram(pEnvironmentSphereProgram);
    VE_Render_DestroyProgram(pMaterialProgram);

    VE_Destroy();
    return 0;
}
