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

typedef void (*ComponentUpdateSystem)(VE_EntityT *, void *);
typedef void (*ComponentDestroySystem)(void *);

#endif //ECS_TYPES_H