#include "builtin.h"
#include "ecs.h"
#include "../render/mesh.h"
#include "../render/render.h"
#include <cglm/cglm.h>

uint32_t VE_TestComponentID = 0;
uint32_t VE_TestComponentSpawnerID = 0;
uint32_t VE_TransformID = 0;
uint32_t VE_SoundPlayerID = 0;
uint32_t VE_AudioListenerID = 0;
uint32_t VE_MeshID = 0;
uint32_t VE_CameraID = 0;

void VE_TestComponent_UpdateSystem(VE_EntityHandleT entityHandle, void *pData) {
	VE_TestComponent* component = (VE_TestComponent *)pData;
	//printf("Updating component with value %i\n", component->counter);
	component->counter++;
	if (component->counter >= component->maxVal) {
		VE_ECS_DestroyEntity(entityHandle);
	}
	VE_Transform *pTransform = VE_ECS_GetComponent(entityHandle, VE_TransformID);
	if (pTransform) {
		pTransform->position[1] = 2.0 - component->counter * 0.1f;
	}
}

VE_TestComponent *VE_NewTestComponent(int counter, int maxVal) {
	VE_TestComponent *pComponent = malloc(sizeof(VE_TestComponent));
	*pComponent = (VE_TestComponent){VE_TestComponentID, counter, maxVal};
	return pComponent;
}

void VE_TestComponentSpawner_UpdateSystem(VE_EntityHandleT entityHandle, void *pData) {
#define RAND_FLOAT (((float)rand()/(float)(RAND_MAX)) * 2.0 - 1.0)
	VE_TestComponentSpawner *pSpawner = (VE_TestComponentSpawner *)pData;
	VE_EntityHandleT newEntity = VE_ECS_CreateEntity();
	VE_ECS_InsertComponent(newEntity, VE_NewTestComponent(0, 100));
	
	VE_ECS_InsertComponent(newEntity, VE_NewTransform((vec3) { RAND_FLOAT * 0.2f, 2.0f, RAND_FLOAT * 0.2f }, GLM_VEC3_ZERO, GLM_VEC3_ONE));
	VE_ECS_InsertComponent(newEntity, VE_NewMesh(VE_Render_CreateCubeMesh(0.1, 0.1, 0.1, pSpawner->pProgram)));
	VE_Render_SetMeshObjectTexture(((VE_Mesh *)VE_ECS_GetComponent(newEntity, VE_MeshID))->pMeshObject, pSpawner->pTexture);
}

VE_TestComponentSpawner *VE_NewTestComponentSpawner(VE_ProgramT *pProgram, VE_TextureT *pTexture) {
	VE_TestComponentSpawner *pComponent = malloc(sizeof(VE_TestComponentSpawner));
	*pComponent = (VE_TestComponentSpawner){ VE_TestComponentSpawnerID, pProgram, pTexture };
	return pComponent;
}

void VE_Transform_UpdateSystem(VE_EntityHandleT entityHandle, void *pData) {
	VE_Transform *transform = (VE_Transform *)pData;
	if (transform->_update) {
        mat4 transform_mat = GLM_MAT4_IDENTITY_INIT;
		glm_translate(transform_mat, transform->position);
		glm_rotate(transform_mat, transform->rotation[2], GLM_ZUP);
		glm_rotate(transform_mat, transform->rotation[1], GLM_YUP);
		glm_rotate(transform_mat, transform->rotation[0], GLM_XUP);
		glm_scale(transform_mat, transform->scale);
        glm_mat4_copy(transform_mat, transform->_matrix); // TODO figure out why this causes SIGSEGV
		transform->_update = 0;
	}
}

VE_Transform *VE_NewTransform(vec3 position, vec3 rotation, vec3 scale) {
	VE_Transform *pComponent = malloc(sizeof(VE_Transform));
	*pComponent = (VE_Transform){
		VE_TransformID,
		{position[0], position[1], position[2]},
		{rotation[0], rotation[1], rotation[2]},
		{scale[0], scale[1], scale[2]},
		1,
		GLM_MAT4_IDENTITY_INIT
	};
	return pComponent;
}

void VE_Camera_UpdateSystem(VE_EntityHandleT entityHandle, void *pData) {
	VE_Camera *pCamera = pData;
	mat4 projectionMatrix = GLM_MAT4_IDENTITY_INIT;
	glm_perspective(pCamera->fov, VE_Render_GetAspectRatio(), pCamera->nearPlane, pCamera->farPlane, projectionMatrix);
	projectionMatrix[1][1] *= -1;
	VE_Render_SetProjectionMatrix(projectionMatrix);
	VE_Transform *transform = VE_ECS_GetComponent(entityHandle, VE_TransformID);
	if (transform) {
		mat4 viewMatrix = GLM_MAT4_IDENTITY_INIT;
		glm_mat4_inv(transform->_matrix, viewMatrix);
		VE_Render_SetViewMatrix(viewMatrix);
	}
}

VE_Camera *VE_NewCamera(float fov, float nearPlane, float farPlane) {
	VE_Camera *pComponent = malloc(sizeof(VE_Camera));
	*pComponent = (VE_Camera){ VE_CameraID, fov, nearPlane, farPlane };
	return pComponent;
}

void VE_Mesh_DeleteSystem(void *pData) {
    VE_Mesh *pMesh = pData;
    VE_Render_UnregisterEntity(pMesh->pMeshObject);
    VE_Render_DestroyMeshObject(pMesh->pMeshObject);
}

void VE_Mesh_UpdateSystem(VE_EntityHandleT entityHandle, void *pData) {
    VE_Mesh *pMesh = pData;
    VE_Transform *transform = (VE_Transform*) VE_ECS_GetComponent(entityHandle, VE_TransformID);
    if (transform) {
        VE_Render_UpdateMeshUniformBuffer(pMesh->pMeshObject, transform->_matrix);
    }
}

VE_Mesh *VE_NewMesh(VE_MeshObject_T *pMeshObject) {
    VE_Mesh *pComponent = malloc(sizeof(VE_Mesh));
    *pComponent = (VE_Mesh){ VE_MeshID, pMeshObject };
    VE_Render_RegisterEntity(pMeshObject);
    return pComponent;
}

void VE_SoundPlayer_UpdateSystem(VE_EntityHandleT entityHandle, void *pData) {
	VE_SoundPlayer *soundPlayer = (VE_SoundPlayer *)pData;
	VE_Transform *transform = (VE_Transform *)VE_ECS_GetComponent(entityHandle, VE_TransformID);
	if (transform) {
		VE_Audio_SetSourceRelative(soundPlayer->source, 0);
		VE_Audio_SetSourcePosition(soundPlayer->source, transform->position);
	}
	else {
		VE_Audio_SetSourceRelative(soundPlayer->source, 1);
	}
}

void VE_SoundPlayer_DestroySystem(void *pData) {
	VE_SoundPlayer *soundPlayer = pData;
	VE_Audio_DestroySource(soundPlayer->source);
}

VE_SoundPlayer *VE_NewSoundPlayer(ALuint sound, float volume, float pitch, char looping) {
	ALuint source = VE_Audio_CreateSource(sound);
	VE_SoundPlayer *pComponent = malloc(sizeof(VE_SoundPlayer));
	*pComponent = (VE_SoundPlayer){VE_SoundPlayerID, source, volume, pitch, looping};
	return pComponent;
}

void VE_PlaySoundPlayer(VE_SoundPlayer *soundPlayer) {
	alSourcef(soundPlayer->source, AL_PITCH, soundPlayer->pitch);
	alSourcef(soundPlayer->source, AL_GAIN, soundPlayer->volume);
	alSourcei(soundPlayer->source, AL_LOOPING, soundPlayer->looping);
	VE_Audio_Play(soundPlayer->source);
}

void VE_AudioListener_UpdateSystem(VE_EntityHandleT entityHandle, void *pData) {
	VE_Transform *transform = VE_ECS_GetComponent(entityHandle, VE_TransformID);
	if (transform) {
		VE_Audio_SetListenerPosition(transform->position);

		vec3 forwardVec = { 0.0, 0.0, 0.0 };
		glm_vec3_copy(transform->_matrix[2], forwardVec);
		glm_vec3_negate(forwardVec);
		vec3 upVec = { 0.0, 0.0, 0.0 };
		glm_vec3_copy(transform->_matrix[1], upVec);
		
		VE_Audio_SetListenerOrientation(forwardVec, upVec);
	}
}

VE_AudioListener *VE_NewAudioListener() {
	VE_AudioListener *pComponent = malloc(sizeof(VE_AudioListener));
	*pComponent = (VE_AudioListener){ VE_AudioListenerID };
	return pComponent;
}

void VE_SetupBuiltinComponents() {
	VE_TestComponentID = VE_ECS_RegisterComponent("TestComponent", sizeof(VE_TestComponent), VE_TestComponent_UpdateSystem, NULL);
	VE_TestComponentSpawnerID = VE_ECS_RegisterComponent("TestComponentSpawner", sizeof(VE_TestComponentSpawner), VE_TestComponentSpawner_UpdateSystem, NULL);
	VE_TransformID = VE_ECS_RegisterComponent("Transform", sizeof(VE_Transform), VE_Transform_UpdateSystem, NULL);
	VE_CameraID = VE_ECS_RegisterComponent("Camera", sizeof(VE_Camera), VE_Camera_UpdateSystem, NULL);
	VE_MeshID = VE_ECS_RegisterComponent("Mesh", sizeof(VE_Mesh), VE_Mesh_UpdateSystem, VE_Mesh_DeleteSystem);
	VE_SoundPlayerID = VE_ECS_RegisterComponent("SoundPlayer", sizeof(VE_SoundPlayer), VE_SoundPlayer_UpdateSystem, VE_SoundPlayer_DestroySystem);
	VE_AudioListenerID = VE_ECS_RegisterComponent("AudioListener", sizeof(VE_AudioListener), VE_AudioListener_UpdateSystem, NULL);
}
