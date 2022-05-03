#include "ecs.h"
#include "globals.h"
#include <stdlib.h>

uint32_t VE_RegisterComponent(ComponentSystem componentSystem, uint32_t componentSize) {
	if (VE_G_ComponentCount >= COMPONENT_MAX) {
		return COMPONENT_MAX;
	}
	VE_G_ComponentSystems[VE_G_ComponentCount] = componentSystem;
	VE_G_ComponentSizes[VE_G_ComponentCount] = componentSize;
	return VE_G_ComponentCount++;
}

uint32_t VE_GetEntitySize(VE_EntityT *entity) {
	uint32_t size = 0;
	for (uint32_t i = 0; i < entity->componentCount; i++) {
		void *component = (char *)entity->components + size;
		uint32_t id = *(uint32_t *)component;
		size += VE_G_ComponentSizes[id];
	}
	return size;
}

void VE_InsertComponent(VE_EntityT *entity, void *component) {
	uint32_t id = *(uint32_t *)component;
	printf("Adding component with ID: %u", id);
	uint32_t componentSize = VE_G_ComponentSizes[id];
	uint32_t prevSize = VE_GetEntitySize(entity);

	void *components = realloc(entity->components, prevSize + componentSize);
	if (!components) {
		printf("Failed to insert component.");
		return;
	}
	memcpy((char *)components + prevSize, component, componentSize);
	entity->components = components;
	entity->componentCount++;
}

VE_EntityT *VE_GetEntity(VE_SceneT *scene, uint32_t index) {
	return &scene->entities[index];
}

uint32_t VE_CreateEntity(VE_SceneT *scene) {
	VE_EntityT *entities = realloc(scene->entities, (scene->entityCount + 1) * sizeof(VE_EntityT));
	if (!entities) {
		printf("Failed to create new entity.");
		return UINT32_MAX;
	}
	VE_EntityT entity = { NULL, 0 };
	entities[scene->entityCount] = entity;
	scene->entities = entities;
	return scene->entityCount++;
}

void VE_UpdateScene(VE_SceneT *scene) {
	for (uint32_t i = 0; i < scene->entityCount; i++) {
		VE_EntityT *entity = &scene->entities[i];
		uint32_t offset = 0;
		for (uint32_t j = 0; j < entity->componentCount; j++) {
			void *component = (char *)entity->components + offset;
			uint32_t id = *(uint32_t *)component;
			VE_G_ComponentSystems[id](entity, component);
			offset += VE_G_ComponentSizes[id];
		}
	}
}
