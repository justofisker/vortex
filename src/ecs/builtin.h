#ifndef ECS_BUILTIN_H
#define ECS_BUILTIN_H

#include "types.h"
#include "cglm/cglm.h"
#include "cglm/mat4.h"
#include "cglm/euler.h"

typedef struct VE_TestComponent {
	uint32_t _id;
	int counter;
	int maxVal;
} VE_TestComponent;
extern uint32_t VE_TestComponentID;
VE_TestComponent VE_NewTestComponent(int counter, int maxVal);

typedef struct VE_TestComponentSpawner {
	uint32_t id;
} VE_TestComponentSpawner;
extern uint32_t VE_TestComponentSpawnerID;
VE_TestComponentSpawner VE_NewTestComponentSpawner();

typedef struct VE_Transform {
	uint32_t _id;
	vec3 position;
	vec3 rotation;
	vec3 scale;
	char _update;
	mat4 _matrix;
} VE_Transform;
extern uint32_t VE_TransformID;
VE_Transform VE_NewTransform(vec3 position, vec3 rotation, vec3 scale);

void VE_SetupBuiltinComponents();

#endif //ECS_BUILTIN_H