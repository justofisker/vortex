#ifndef ECS_ECS_H
#define ECS_ECS_H

#include "types.h"

// Returns new component type's ID. If it returns COMPONENT_MAX, then we are out of component slots.
uint32_t VE_RegisterComponent(ComponentSystem componentSystem, uint32_t componentSize);

uint32_t VE_GetEntitySize(VE_EntityT *entity);
void VE_InsertComponent(VE_EntityT *entity, void *component);
VE_EntityT *VE_GetEntity(VE_SceneT *scene, uint32_t index);
// Returns the index of the newly created entity. If it returns UINT32_MAX, there has been an error.
uint32_t VE_CreateEntity(VE_SceneT *scene);

void VE_UpdateScene(VE_SceneT *scene);

#endif //ECS_ECS_H