#ifndef ECS_ECS_H
#define ECS_ECS_H

#include "types.h"

// Returns new component type's ID. If it returns VE_COMPONENT_MAX, then we are out of component slots.
uint32_t VE_ECS_RegisterComponent(ComponentSystem componentSystem, uint32_t componentSize);

// Byte size of the components block of the specified entity.
uint32_t VE_ECS_GetEntitySize(VE_EntityT *pEntity);
void VE_ECS_InsertComponent(VE_EntityT *pEntity, void *pComponent);
void *VE_ECS_GetComponent(VE_EntityT *pEntity, uint32_t id);
VE_EntityT *VE_ECS_GetEntity(VE_SceneT *pScene, uint32_t index);
// Returns the index of the newly created entity. If it returns UINT32_MAX, there has been an error.
uint32_t VE_ECS_CreateEntity(VE_SceneT *pScene);

void VE_ECS_UpdateScene(VE_SceneT *pScene);

#endif //ECS_ECS_H