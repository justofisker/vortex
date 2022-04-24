#include <volk.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "util.h"
#include "globals.h"
#include "SDL_vulkan.h"

#ifndef NDEBUG

static VkDebugReportCallbackEXT VE_S_DebugCallback = VK_NULL_HANDLE;

static void VE_Render_CheckValidationSupport() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, NULL);
    VkLayerProperties *avaiableLayers = malloc(sizeof(VkLayerProperties) * layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, avaiableLayers);

    free(avaiableLayers);
}

static VkBool32 VKAPI_CALL VE_Render_DebugReportCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData)
{
    if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
        return VK_FALSE;

    const char* type =
            (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
            ? "ERROR"
            : (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
              ? "WARNING"
              : "INFO";

    char message[4096];
    snprintf(message, sizeof(message) / sizeof(message[0]), "%s: %s\n", type, pMessage);

    printf("%s", message);

    return VK_FALSE;
}

static void VE_Render_RegisterDebugReportCallback() {
    VkDebugReportCallbackCreateInfoEXT createInfo = { VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT };
    createInfo.flags = VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT | VK_DEBUG_REPORT_ERROR_BIT_EXT;
    createInfo.pfnCallback = VE_Render_DebugReportCallback;

    vkCreateDebugReportCallbackEXT(VE_G_Instance, &createInfo, NULL, &VE_S_DebugCallback);
}
#endif

void VE_Render_CreateInstance()
{
	VkApplicationInfo appInfo = { VK_STRUCTURE_TYPE_APPLICATION_INFO };
	appInfo.pNext;
	appInfo.pApplicationName = "vortex test";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "vortex";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo createInfo = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
	createInfo.pNext;
	createInfo.flags;
	createInfo.pApplicationInfo = &appInfo;
#ifdef NDEBUG
	createInfo.enabledLayerCount = 0;
	createInfo.ppEnabledLayerNames = NULL;
    const char *instanceExtensions[] = { NULL, NULL };
#else
    const char* debugLayers[] = {"VK_LAYER_KHRONOS_validation" };
    createInfo.enabledLayerCount = sizeof(debugLayers[0]) / sizeof(debugLayers);
    createInfo.ppEnabledLayerNames = debugLayers;
    const char *instanceExtensions[] = { NULL, NULL, VK_EXT_DEBUG_REPORT_EXTENSION_NAME };
#endif // NDEBUG
    createInfo.enabledExtensionCount = sizeof(instanceExtensions[0]) / sizeof(instanceExtensions);
    createInfo.ppEnabledExtensionNames = instanceExtensions;
    unsigned int sdlInstanceCount;
    SDL_Vulkan_GetInstanceExtensions(VE_G_Window, &sdlInstanceCount, NULL);
    assert(sdlInstanceCount == 2);
    SDL_Vulkan_GetInstanceExtensions(VE_G_Window, &sdlInstanceCount, instanceExtensions );
	vkCreateInstance(&createInfo, NULL, &VE_G_Instance);

    volkLoadInstanceOnly(VE_G_Instance);
}

void VE_Render_PickPhysicalDevice() {
    uint32_t count;
    vkEnumeratePhysicalDevices(VE_G_Instance, &count, NULL);
    VkPhysicalDevice *devices = malloc(sizeof(VkPhysicalDevice) * count);
    vkEnumeratePhysicalDevices(VE_G_Instance, &count, devices);
    for (uint32_t i = 0; i < count; ++i) {
        // TODO: Implement physical device picking logic
        VE_G_PhysicalDevice = devices[i];
        break;
    }
    free(devices);
}

void VE_Render_CreateDevice()
{
}

void VE_Render_CreateSurface()
{
}
