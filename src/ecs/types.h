#ifndef ECS_TYPES_H
#define ECS_TYPES_H

#include <stdint.h>

typedef struct VE_EntityT {
	void* pComponents;
	uint32_t componentCount;
} VE_EntityT;

typedef struct VE_SceneT {
	VE_EntityT *pEntities;
	uint32_t entityCount;
} VE_SceneT;

typedef (*ComponentSystem)(VE_EntityT *, void *);

#endif //ECS_TYPES_H