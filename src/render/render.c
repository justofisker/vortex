#include "render.h"

#include <volk.h>
#include <SDL_vulkan.h>

#include "util.h"
#include "globals.h"

void VE_RenderInit(SDL_Window *window)
{
	VE_G_Window = window;
	volkInitialize();
	VE_Render_CreateInstance();
    VE_Render_PickPhysicalDevice();
}

void VE_RenderDestroy()
{
	vkDestroyInstance(VE_G_Instance, NULL);
}
