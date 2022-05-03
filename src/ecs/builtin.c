#include "builtin.h"
#include "ecs.h"

uint32_t VE_TestComponentID = 0;
uint32_t VE_TransformID = 0;

void VE_TestComponent_System(VE_EntityT *pEntity, void *pData) {
	VE_TestComponent* component = (VE_TestComponent *)pData;
	printf("Updating component with value %i\n", component->counter);
	component->counter++;
	VE_Transform *transform = (VE_Transform *)VE_ECS_GetComponent(pEntity, VE_TransformID);
	if (transform) {
		printf("Found transform on entity, position is %.3f %.3f %.3f\n", transform->position[0], transform->position[1], transform->position[2]);
	}
}

VE_TestComponent VE_NewTestComponent(int counter) {
	VE_TestComponent testComponent = { VE_TestComponentID, counter };
	return testComponent;
}

void VE_Transform_System(VE_EntityT *pEntity, void *pData) {
	printf("Updating transform...\n");
	VE_Transform *transform = (VE_Transform *)pData;
	if (transform->_update) {
		mat4 transform_mat = GLM_MAT4_IDENTITY_INIT;
		glm_translate(transform_mat, transform->position);
		glm_rotate(transform_mat, glm_rad(transform->rotation[0]), GLM_XUP);
		glm_rotate(transform_mat, glm_rad(transform->rotation[1]), GLM_YUP);
		glm_rotate(transform_mat, glm_rad(transform->rotation[2]), GLM_ZUP);
		glm_scale(transform_mat, transform->scale);
		transform->_update = false;
	}
}

VE_Transform VE_NewTransform(vec3 position, vec3 rotation, vec3 scale) {
	VE_Transform transform = {
		VE_TransformID,
		{position[0], position[1], position[2]},
		{rotation[0], rotation[1], rotation[2]},
		{scale[0], scale[1], scale[2]},
		true,
		GLM_MAT4_IDENTITY_INIT
	};
	return transform;
}

void VE_SetupBuiltinComponents() {
	VE_TestComponentID = VE_ECS_RegisterComponent("TestComponent", sizeof(VE_TestComponent), VE_TestComponent_System, NULL);
	VE_TransformID = VE_ECS_RegisterComponent("Transform", sizeof(VE_Transform), VE_Transform_System, NULL);
}
