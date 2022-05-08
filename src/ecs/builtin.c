#include "builtin.h"
#include "ecs.h"

uint32_t VE_TestComponentID = 0;
uint32_t VE_TestComponentSpawnerID = 0;
uint32_t VE_TransformID = 0;
uint32_t VE_SoundPlayerID = 0;

void VE_TestComponent_UpdateSystem(VE_EntityHandleT entityHandle, void *pData) {
	VE_TestComponent* component = (VE_TestComponent *)pData;
	//printf("Updating component with value %i\n", component->counter);
	component->counter++;
	VE_Transform *transform = (VE_Transform *)VE_ECS_GetComponent(entityHandle, VE_TransformID);
	if (transform) {
		//printf("Found transform on entity, position is %.3f %.3f %.3f\n", transform->position[0], transform->position[1], transform->position[2]);
	}
	if (component->counter >= component->maxVal) {
		printf("Deleting test component.\n");
		VE_ECS_DestroyEntity(entityHandle);
	}
}

VE_TestComponent *VE_NewTestComponent(int counter, int maxVal) {
	VE_TestComponent *pComponent = malloc(sizeof(VE_TestComponent));
	*pComponent = (VE_TestComponent){VE_TestComponentID, counter, maxVal};
	return pComponent;
}

void VE_TestComponentSpawner_UpdateSystem(VE_EntityHandleT entityHandle, void *pData) {
	VE_TestComponentSpawner *pSpawner = (VE_TestComponentSpawner *)pData;
	VE_EntityHandleT newEntity = VE_ECS_CreateEntity();
	VE_ECS_InsertComponent(newEntity, VE_NewTestComponent(0, 100000));
}

VE_TestComponentSpawner *VE_NewTestComponentSpawner() {
	VE_TestComponentSpawner *pComponent = malloc(sizeof(VE_TestComponentSpawner));
	*pComponent = (VE_TestComponentSpawner){VE_TestComponentSpawnerID};
	return pComponent;
}

void VE_Transform_UpdateSystem(VE_EntityHandleT entityHandle, void *pData) {
	VE_Transform *transform = (VE_Transform *)pData;
	if (transform->_update) {
		mat4 transform_mat = GLM_MAT4_IDENTITY_INIT;
		glm_translate(transform_mat, transform->position);
		glm_rotate(transform_mat, glm_rad(transform->rotation[0]), GLM_XUP);
		glm_rotate(transform_mat, glm_rad(transform->rotation[1]), GLM_YUP);
		glm_rotate(transform_mat, glm_rad(transform->rotation[2]), GLM_ZUP);
		glm_scale(transform_mat, transform->scale);
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

void VE_SoundPlayer_UpdateSystem(VE_EntityHandleT entityHandle, void *pData) {
	VE_SoundPlayer *soundPlayer = (VE_SoundPlayer *)pData;
	VE_Transform *transform = (VE_Transform *)VE_ECS_GetComponent(entityHandle, VE_TransformID);
	if (transform) {
		alSourcei(soundPlayer->source, AL_SOURCE_RELATIVE, AL_FALSE);
		alSource3f(soundPlayer->source, AL_POSITION, transform->position[0], transform->position[1], transform->position[2]);
	}
	else {
		alSourcei(soundPlayer->source, AL_SOURCE_RELATIVE, AL_TRUE);
	}
	
}

void VE_SoundPlayer_DestroySystem(void *pData) {
	VE_SoundPlayer *soundPlayer = pData;
	VE_Audio_DestroySource(&soundPlayer->source);
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

void VE_SetupBuiltinComponents() {
	VE_TestComponentID = VE_ECS_RegisterComponent("TestComponent", sizeof(VE_TestComponent), VE_TestComponent_UpdateSystem, NULL);
	VE_TestComponentSpawnerID = VE_ECS_RegisterComponent("TestComponentSpawner", sizeof(VE_TestComponentSpawner), VE_TestComponentSpawner_UpdateSystem, NULL);
	VE_TransformID = VE_ECS_RegisterComponent("Transform", sizeof(VE_Transform), VE_Transform_UpdateSystem, NULL);
	VE_SoundPlayerID = VE_ECS_RegisterComponent("SoundPlayer", sizeof(VE_SoundPlayer), VE_SoundPlayer_UpdateSystem, VE_SoundPlayer_DestroySystem);
}
