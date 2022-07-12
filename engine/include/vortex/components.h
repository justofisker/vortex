#ifndef VORTEX_COMPONENTS_H
#define VORTEX_COMPONENTS_H

#include "types.h"

typedef struct VE_TestComponent {
	uint32_t _id;
	int counter;
	int maxVal;
} VE_TestComponent;
extern uint32_t VE_TestComponentID;
VE_TestComponent *VE_NewTestComponent(int counter, int maxVal);

typedef struct VE_TestComponentSpawner {
	uint32_t id;
	VE_ProgramT *pProgram;
	VE_TextureT *pTexture;
} VE_TestComponentSpawner;
extern uint32_t VE_TestComponentSpawnerID;
VE_TestComponentSpawner *VE_NewTestComponentSpawner(VE_ProgramT *pProgram, VE_TextureT *pTexture);

typedef struct VE_Transform {
	uint32_t _id;
	vec3 position;
	vec3 rotation;
	vec3 scale;
	char _update;
	mat4 _matrix;
} VE_Transform;
extern uint32_t VE_TransformID;
VE_Transform *VE_NewTransform(vec3 position, vec3 rotation, vec3 scale);

typedef struct VE_Mesh {
	uint32_t _id;
	VE_MeshObject_T *pMeshObject;
} VE_Mesh;
extern uint32_t VE_MeshID;
VE_Mesh *VE_NewMesh(VE_MeshObject_T *pMeshObject);

typedef struct VE_Camera {
	uint32_t id;
	float fov;
	float nearPlane;
	float farPlane;
} VE_Camera;
extern uint32_t VE_CameraID;
VE_Camera *VE_NewCamera(float fov, float nearPlane, float farPlane);

typedef struct VE_FlyCam {
	uint32_t id;
	float moveSpeed;
	float mouseSensitivity;
} VE_FlyCam;
extern uint32_t VE_FlyCamID;
VE_FlyCam *VE_NewFlyCam(float moveSpeed, float mouseSensitivity);

typedef struct VE_SoundPlayer {
	uint32_t _id;
	VE_Audio source;
	float volume;
	float pitch;
	char looping;
} VE_SoundPlayer;
extern uint32_t VE_SoundPlayerID;
VE_SoundPlayer *VE_NewSoundPlayer(VE_Audio sound, float volume, float pitch, char looping);
void VE_PlaySoundPlayer(VE_SoundPlayer *soundPlayer);

typedef struct VE_AudioListener {
	uint32_t _id;
} VE_AudioListener;
extern uint32_t VE_AudioListenerID;
VE_AudioListener *VE_NewAudioListener();

#endif // VORTEX_COMPONENTS_H
