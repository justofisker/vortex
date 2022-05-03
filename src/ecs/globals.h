#ifndef ECS_GLOBALS_H
#define ECS_GLOBALS_H

#ifndef VE_ECS_COMPONENT_MAX
#define VE_ECS_COMPONENT_MAX 128
#endif //COMPONENT_MAX

#include "types.h"

extern ComponentSystem VE_G_ComponentSystems[VE_ECS_COMPONENT_MAX];
extern uint32_t VE_G_ComponentSizes[VE_ECS_COMPONENT_MAX];
extern uint32_t VE_G_ComponentCount;

#endif //ECS_GLOBALS_H