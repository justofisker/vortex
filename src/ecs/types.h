#ifndef ECS_TYPES_H
#define ECS_TYPES_H

#include <stdint.h>

typedef struct VE_EntityT {
	void* components;
	uint32_t componentCount;
} VE_EntityT;

typedef struct VE_SceneT {
	VE_EntityT *entities;
	uint32_t entityCount;
} VE_SceneT;

typedef (*ComponentSystem)(VE_EntityT *, void *);

#endif //ECS_TYPES_H