#include "render.h"
#include <stdio.h>
#include <volk.h>
#include <SDL_vulkan.h>
#include "util.h"
#include "globals.h"

void VE_Render_Init(SDL_Window *window)
{
	VE_G_Window = window;
	volkInitialize();
	VE_Render_CreateInstance();
#ifndef NDEBUG
    if (VE_Render_CheckValidationSupport()) {
        VE_Render_RegisterDebugReportCallback();
    } else {
        fprintf(stderr, "Failed to register Vulkan debug layers!\n");
    }
#endif // NDEBUG
    VE_Render_PickPhysicalDeviceAndQueues();
    VE_Render_CreateDevice();
    VE_Render_CreateSurface();
}

void VE_Render_Destroy()
{
    vkDestroySurfaceKHR(VE_G_Instance, VE_G_Surface, NULL);
    vkDestroyDevice(VE_G_Device, NULL);
#ifndef NDEBUG
    vkDestroyDebugReportCallbackEXT(VE_G_Instance, VE_G_DebugCallback, NULL);
#endif // NDEBUG
	vkDestroyInstance(VE_G_Instance, NULL);
}
