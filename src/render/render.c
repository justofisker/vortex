#include "render.h"

#include <volk.h>
#include <SDL_vulkan.h>

#include "util.h"
#include "globals.h"

static SDL_Window *window;

void VE_RenderInit(SDL_Window *_window)
{
	window = _window;
	volkInitialize();
	VE_Render_CreateInstance();
}

void VE_RenderDestroy()
{
	vkDestroyInstance(VE_G_Instance, NULL);
}
