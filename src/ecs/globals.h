#ifndef ECS_GLOBALS_H
#define ECS_GLOBALS_H

#ifndef COMPONENT_MAX
#define COMPONENT_MAX 128
#endif //COMPONENT_MAX

#include "types.h"

extern ComponentSystem VE_G_ComponentSystems[COMPONENT_MAX];
extern uint32_t VE_G_ComponentSizes[COMPONENT_MAX];
extern uint32_t VE_G_ComponentCount;

#endif //ECS_GLOBALS_H