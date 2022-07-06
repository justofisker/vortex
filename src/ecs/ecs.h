#ifndef ECS_ECS_H
#define ECS_ECS_H

#include "types.h"

#define VE_ECS_REGISTER_COMPONENT(component, updateSystem, destroySystem) VE_ECS_RegisterComponent(#component, sizeof(component), updateSystem, destroySystem)

// Returns new component type's ID. If it returns VE_COMPONENT_MAX, then we are out of component slots.
// The update system runs every frame and has two parameters, a VE_EntityT* and a void* which points to the component's data.
// The destroy system is run when the entity is deleted, and just takes a void* which points to the component's data.
uint32_t VE_ECS_RegisterComponent(const char* pComponentName, uint32_t componentSize, ComponentUpdateSystem componentUpdateSystem, ComponentDestroySystem componentDestroySystem);
// Returns the component ID, or VE_COMPONENT_MAX if it's not found.
uint32_t VE_ECS_GetComponentIdFromName(const char *pComponentName);

// Create a new entity, or grab an empty one. Note that if an entity does not get components inserted, it will be considered empty and be reused by this function.
// Returns the index of the newly created entity. If it returns UINT32_MAX, there has been an error.
// DO NOT attempt to create multiple entities sequentially without adding any components, as you'll just make the same entity over and over again.
VE_EntityHandleT VE_ECS_CreateEntity();
// Byte size of the components block of the specified entity.
uint32_t VE_ECS_GetEntitySize(VE_EntityHandleT entityHandle);
void VE_ECS_InsertComponent(VE_EntityHandleT entityHandle, void *pComponent);
void *VE_ECS_GetComponent(VE_EntityHandleT entityHandle, uint32_t id);
void VE_ECS_DestroyEntity(VE_EntityHandleT entityHandle);

void VE_ECS_UpdateScene();

void VE_ECS_DestroyScene();

#endif //ECS_ECS_H