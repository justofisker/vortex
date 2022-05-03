#include "ecs.h"
#include "globals.h"
#include <stdlib.h>

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

uint32_t VE_ECS_GetEntitySize(VE_EntityT *pEntity) {
	uint32_t size = 0;
	for (uint32_t i = 0; i < pEntity->componentCount; i++) {
		void *pComponent = (char *)pEntity->pComponents + size;
		uint32_t id = *(uint32_t *)pComponent;
		size += VE_G_ComponentSizes[id];
	}
	return size;
}

void VE_ECS_InsertComponent(VE_EntityT *pEntity, void *pComponent) {
	uint32_t id = *(uint32_t *)pComponent;
	uint32_t componentSize = VE_G_ComponentSizes[id];
	uint32_t prevSize = VE_ECS_GetEntitySize(pEntity);

	void *pComponents = realloc(pEntity->pComponents, prevSize + componentSize);
	if (!pComponents) {
		printf("Failed to insert component.");
		return;
	}
	memcpy((char *)pComponents + prevSize, pComponent, componentSize);
	pEntity->pComponents = pComponents;
	pEntity->componentCount++;
}

void *VE_ECS_GetComponent(VE_EntityT *pEntity, uint32_t id) {
	uint32_t offset = 0;
	for (uint32_t i = 0; i < pEntity->componentCount; i++) {
		void *pComponent = (char *)pEntity->pComponents + offset;
		uint32_t c_id = *(uint32_t *)pComponent;
		if (id == c_id) {
			return pComponent;
		}
		offset += VE_G_ComponentSizes[c_id];
	}
	return NULL;
}

VE_EntityT *VE_ECS_GetEntity(VE_SceneT *pScene, uint32_t index) {
	if (index >= pScene->entityCount) {
		return NULL;
	}
	else {
		return &pScene->pEntities[index];
	}
}

void VE_ECS_DestroyEntity(VE_EntityT *pEntity) {
	uint32_t offset = 0;
	for (uint32_t i = 0; i < pEntity->componentCount; i++) {
		void *pComponent = (char *)pEntity->pComponents + offset;
		uint32_t id = *(uint32_t *)pComponent;
		ComponentDestroySystem destroySystem = VE_G_ComponentDestroySystems[id];
		if (destroySystem) {
			destroySystem(pComponent);
		}
	}
	free(pEntity->pComponents);
}

uint32_t VE_ECS_CreateEntity(VE_SceneT *pScene) {
	VE_EntityT *pEntities = realloc(pScene->pEntities, (pScene->entityCount + 1) * sizeof(VE_EntityT));
	if (!pEntities) {
		printf("Failed to create new entity.");
		return UINT32_MAX;
	}
	VE_EntityT entity = { NULL, 0 };
	pEntities[pScene->entityCount] = entity;
	pScene->pEntities = pEntities;
	return pScene->entityCount++;
}

void VE_ECS_UpdateScene(VE_SceneT *pScene) {
	for (uint32_t i = 0; i < pScene->entityCount; i++) {
		VE_EntityT *entity = &pScene->pEntities[i];
		uint32_t offset = 0;
		for (uint32_t j = 0; j < entity->componentCount; j++) {
			void *pComponent = (char *)entity->pComponents + offset;
			uint32_t id = *(uint32_t *)pComponent;
			ComponentUpdateSystem system = VE_G_ComponentUpdateSystems[id];
			if (system) {
				system(entity, pComponent);
			}
			offset += VE_G_ComponentSizes[id];
		}
	}
}

void VE_ECS_DestroyScene(VE_SceneT *pScene) {
	for (uint32_t i = 0; i < pScene->entityCount; i++) {
		VE_ECS_DestroyEntity(&pScene->pEntities[i]);
	}
	free(pScene->pEntities);
}
