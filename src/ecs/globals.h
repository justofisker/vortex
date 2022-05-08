#ifndef ECS_GLOBALS_H
#define ECS_GLOBALS_H

#ifndef VE_ECS_COMPONENT_MAX
#define VE_ECS_COMPONENT_MAX 128
#endif //COMPONENT_MAX

#include "types.h"

extern ComponentUpdateSystem VE_G_ComponentUpdateSystems[VE_ECS_COMPONENT_MAX];
extern ComponentDestroySystem VE_G_ComponentDestroySystems[VE_ECS_COMPONENT_MAX];
extern uint32_t VE_G_ComponentSizes[VE_ECS_COMPONENT_MAX];
extern const char *VE_G_pComponentNames[VE_ECS_COMPONENT_MAX];
extern uint32_t VE_G_ComponentCount;
extern VE_SceneT VE_G_CurrentScene;

#endif //ECS_GLOBALS_H