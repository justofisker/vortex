#include "ecs.h"
#include "globals.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <SDL.h>

uint32_t VE_ECS_RegisterComponent(const char *pComponentName, uint32_t componentSize, ComponentUpdateSystem componentUpdateSystem, ComponentDestroySystem componentDestroySystem) {
	if (VE_G_ComponentCount >= VE_ECS_COMPONENT_MAX) {
		return VE_ECS_COMPONENT_MAX;
	}
	VE_G_ComponentUpdateSystems[VE_G_ComponentCount] = componentUpdateSystem;
	VE_G_ComponentDestroySystems[VE_G_ComponentCount] = componentDestroySystem;
	VE_G_ComponentSizes[VE_G_ComponentCount] = componentSize;
	VE_G_pComponentNames[VE_G_ComponentCount] = pComponentName;
	return VE_G_ComponentCount++;
}

uint32_t VE_ECS_GetComponentIdFromName(const char *pComponentName) {
	for (uint32_t i = 0; i < VE_G_ComponentCount; i++) {
		if (strcmp(pComponentName, VE_G_pComponentNames[i]) == 0) {
			return i;
		}
	}
	return VE_ECS_COMPONENT_MAX;
}

uint32_t VE_ECS_GetEntitySize(VE_EntityHandleT entityHandle) {
	uint32_t size = 0;
	for (uint32_t i = 0; i < VE_G_CurrentScene.pEntities[entityHandle].componentCount; i++) {
		void *pComponent = (char *)VE_G_CurrentScene.pEntities[entityHandle].pComponents + size;
		uint32_t id = *(uint32_t *)pComponent;
		size += VE_G_ComponentSizes[id];
	}
	return size;
}

// Copy component data into entity. Frees component data afterwards.
// DO NOT attempt to insert the same component to two entities.
// DO NOT attempt to use the component pointer after adding it. Get the component off of the entity;
void VE_ECS_InsertComponent(VE_EntityHandleT entityHandle, void *pComponent) {
	uint32_t id = *(uint32_t *)pComponent;
	uint32_t componentSize = VE_G_ComponentSizes[id];
	uint32_t prevSize = VE_ECS_GetEntitySize(entityHandle);

	void *pComponents = realloc(VE_G_CurrentScene.pEntities[entityHandle].pComponents, prevSize + componentSize);
	if (!pComponents) {
		printf("Failed to insert component.");
		return;
	}
	memcpy((char *)pComponents + prevSize, pComponent, componentSize);
	free(pComponent);
	VE_G_CurrentScene.pEntities[entityHandle].pComponents = pComponents;
	VE_G_CurrentScene.pEntities[entityHandle].componentCount++;
}

void *VE_ECS_GetComponent(VE_EntityHandleT entityHandle, uint32_t id) {
	uint32_t offset = 0;
	for (uint32_t i = 0; i < VE_G_CurrentScene.pEntities[entityHandle].componentCount; i++) {
		void *pComponent = (char *)VE_G_CurrentScene.pEntities[entityHandle].pComponents + offset;
		uint32_t c_id = *(uint32_t *)pComponent;
		if (id == c_id) {
			return pComponent;
		}
		offset += VE_G_ComponentSizes[c_id];
	}
	return NULL;
}

VE_EntityT *VE_ECS_GetEntity(uint32_t index) {
	if (index >= VE_G_CurrentScene.entityCount) {
		return NULL;
	}
	else {
		return &VE_G_CurrentScene.pEntities[index];
	}
}

void VE_ECS_DestroyEntity(VE_EntityHandleT entityHandle) {
	uint32_t offset = 0;
	for (uint32_t i = 0; i < VE_G_CurrentScene.pEntities[entityHandle].componentCount; i++) {
		void *pComponent = (char *)VE_G_CurrentScene.pEntities[entityHandle].pComponents + offset;
		uint32_t id = *(uint32_t *)pComponent;
		ComponentDestroySystem destroySystem = VE_G_ComponentDestroySystems[id];
		if (destroySystem) {
			destroySystem(pComponent);
		}
        offset += VE_G_ComponentSizes[id];
	}
	free(VE_G_CurrentScene.pEntities[entityHandle].pComponents);
	VE_G_CurrentScene.pEntities[entityHandle].pComponents = NULL;
	VE_G_CurrentScene.pEntities[entityHandle].componentCount = 0;
}

VE_EntityHandleT VE_ECS_CreateEntity() {
	for (uint32_t i = 0; i < VE_G_CurrentScene.entityCount; i++) {
		if (VE_G_CurrentScene.pEntities[i].pComponents == NULL) {
			return i;
		}
	}
	VE_EntityT *pEntities = realloc(VE_G_CurrentScene.pEntities, (VE_G_CurrentScene.entityCount + 1) * sizeof(VE_EntityT));
	if (!pEntities) {
		printf("Failed to create new entity.");
		return UINT32_MAX;
	}
	VE_EntityT entity = { NULL, 0 };
	pEntities[VE_G_CurrentScene.entityCount] = entity;
	VE_G_CurrentScene.pEntities = pEntities;
	return VE_G_CurrentScene.entityCount++;
}

void VE_ECS_UpdateScene() {
	uint32_t now = SDL_GetTicks();
	VE_G_DeltaMilliseconds = now - VE_G_LastTick;
	VE_G_LastTick = now;
	VE_G_DeltaSeconds = VE_G_DeltaMilliseconds / 1000.0f;
	for (uint32_t i = 0; i < VE_G_CurrentScene.entityCount; i++) {
		uint32_t offset = 0;
		for (uint32_t j = 0; j < VE_G_CurrentScene.pEntities[i].componentCount; j++) {
			// Break if the entity has been deleted or is empty. This is here in case a system deletes the entity.
			if (VE_G_CurrentScene.pEntities[i].pComponents == NULL) {
				break;
			}
			void *pComponent = (char *)VE_G_CurrentScene.pEntities[i].pComponents + offset;
			uint32_t id = *(uint32_t *)pComponent;
			ComponentUpdateSystem system = VE_G_ComponentUpdateSystems[id];
			if (system) {
				system(i, pComponent);
			}
			offset += VE_G_ComponentSizes[id];
		}
	}
}

void VE_ECS_DestroyScene() {
	for (uint32_t i = 0; i < VE_G_CurrentScene.entityCount; i++) {
		VE_ECS_DestroyEntity(i);
	}
	free(VE_G_CurrentScene.pEntities);
	VE_G_CurrentScene.pEntities = NULL;
	VE_G_CurrentScene.entityCount = 0;
}
