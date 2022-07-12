#include "globals.h"

ComponentUpdateSystem VE_G_ComponentUpdateSystems[VE_ECS_COMPONENT_MAX] = { NULL };
ComponentDestroySystem VE_G_ComponentDestroySystems[VE_ECS_COMPONENT_MAX] = { NULL };
uint32_t VE_G_ComponentSizes[VE_ECS_COMPONENT_MAX] = { 0 };
const char *VE_G_pComponentNames[VE_ECS_COMPONENT_MAX] = { NULL };
uint32_t VE_G_ComponentCount = 0;
VE_SceneT VE_G_CurrentScene = { NULL, 0 };
uint32_t VE_G_LastTick = 0;
uint32_t VE_G_DeltaMilliseconds = 0;
float VE_G_DeltaSeconds = 0.0f;