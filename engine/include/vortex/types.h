#ifndef VORTEX_TYPES_H
#define VORTEX_TYPES_H

#include <stdint.h>
#include <cglm/types.h>

// General

typedef unsigned char VE_Bool;
#define VE_TRUE 1;
#define VE_FALSE 0;

// Audio

typedef unsigned int VE_Audio;

// ECS

typedef struct VE_EntityT {
	void *pComponents;
	uint32_t componentCount;
} VE_EntityT;

typedef uint32_t VE_EntityHandleT;

typedef struct VE_SceneT {
	VE_EntityT *pEntities;
	uint32_t entityCount;
} VE_SceneT;

typedef void (*ComponentUpdateSystem)(VE_EntityHandleT, void *);
typedef void (*ComponentDestroySystem)(void *);

// Render

typedef struct VE_TextureT VE_TextureT;
typedef struct VE_ProgramT VE_ProgramT;
typedef struct VE_MeshObject_T VE_MeshObject_T;

#endif // VORTEX_TYPES_H
