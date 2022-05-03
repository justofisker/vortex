#ifndef ECS_BUILTIN_H
#define ECS_BUILTIN_H

#include "types.h"

typedef struct VE_TestComponent {
	uint32_t id;
	int counter;
} VE_TestComponent;
extern uint32_t VE_TestComponentID;
VE_TestComponent VE_NewTestComponent(int counter);

void VE_SetupBuiltinComponents();

#endif //ECS_BUILTIN_H