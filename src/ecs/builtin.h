#ifndef ECS_BUILTIN_H
#define ECS_BUILTIN_H

#include "types.h"
#include "cglm/cglm.h"
#include "cglm/mat4.h"
#include "cglm/euler.h"

typedef struct VE_TestComponent {
	uint32_t _id;
	int counter;
} VE_TestComponent;
extern uint32_t VE_TestComponentID;
VE_TestComponent VE_NewTestComponent(int counter);

typedef struct VE_Transform {
	uint32_t _id;
	vec3 position;
	vec3 rotation;
	vec3 scale;
	bool _update;
	mat4 _matrix;
} VE_Transform;
extern uint32_t VE_TransformID;
VE_Transform VE_NewTransform(vec3 position, vec3 rotation, vec3 scale);

void VE_SetupBuiltinComponents();

#endif //ECS_BUILTIN_H