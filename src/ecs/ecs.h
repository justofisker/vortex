#ifndef ECS_ECS_H
#define ECS_ECS_H

#include "types.h"

// Returns new component type's ID. If it returns VE_COMPONENT_MAX, then we are out of component slots.
// The update system runs every frame and has two parameters, a VE_EntityT* and a void* which points to the component's data.
// The destroy system is run when the entity is deleted, and just takes a void* which points to the component's data.
uint32_t VE_ECS_RegisterComponent(const char* pComponentName, uint32_t componentSize, ComponentUpdateSystem componentUpdateSystem, ComponentDestroySystem componentDestroySystem);
// Returns the component ID, or VE_COMPONENT_MAX if it's not found.
uint32_t VE_ECS_GetComponentIdFromName(const char *pComponentName);

// Create a new entity, or grab an empty one. Note that if an entity does not get components inserted, it will be considered empty and be reused by this function.
// Returns the index of the newly created entity. If it returns UINT32_MAX, there has been an error.
// DO NOT attempt to create multiple entities sequentially without adding any components, as you'll just make the same entity over and over again.
uint32_t VE_ECS_CreateEntity(VE_SceneT *pScene);
// Byte size of the components block of the specified entity.
uint32_t VE_ECS_GetEntitySize(VE_EntityT *pEntity);
void VE_ECS_InsertComponent(VE_EntityT *pEntity, void *pComponent);
void *VE_ECS_GetComponent(VE_EntityT *pEntity, uint32_t id);
VE_EntityT *VE_ECS_GetEntity(VE_SceneT *pScene, uint32_t index);
void VE_ECS_DestroyEntity(VE_EntityT *pEntity);

void VE_ECS_UpdateScene(VE_SceneT *pScene);

void VE_ECS_DestroyScene(VE_SceneT *pScene);

#endif //ECS_ECS_H