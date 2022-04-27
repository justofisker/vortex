#include "render.h"
#include <stdio.h>
#include <volk.h>
#include <SDL_vulkan.h>
#include "init.h"
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
    VE_Render_CreateSurface();
    VE_Render_PickPhysicalDeviceAndQueues();
    VE_Render_CreateDevice();
    VE_Render_CreateSwapchain();
}

void VE_Render_Destroy()
{
    for (uint32_t i = 0; i < VE_G_SwapchainImageCount; ++i) {
        vkDestroyImageView(VE_G_Device, VE_G_pSwapchainImageViews[i], NULL);
    }
    free(VE_G_pSwapchainImageViews);
    if (VE_G_pSwapchainImages)
        free(VE_G_pSwapchainImages);
    vkDestroySwapchainKHR(VE_G_Device, VE_G_Swapchain, NULL);
    vkDestroySurfaceKHR(VE_G_Instance, VE_G_Surface, NULL);
    vkDestroyDevice(VE_G_Device, NULL);
#ifndef NDEBUG
    vkDestroyDebugReportCallbackEXT(VE_G_Instance, VE_G_DebugCallback, NULL);
#endif // NDEBUG
	vkDestroyInstance(VE_G_Instance, NULL);
}
