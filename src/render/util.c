#include <volk.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "util.h"
#include "globals.h"
#include "SDL_vulkan.h"

#ifndef NDEBUG
int VE_Render_CheckValidationSupport() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, NULL);
    VkLayerProperties *avaiableLayers = malloc(sizeof(VkLayerProperties) * layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, avaiableLayers);
    for (int i = 0; i < layerCount; ++i) {
        if (strcmp("VK_LAYER_KHRONOS_validation", avaiableLayers[i].layerName) == 0) {
            free(avaiableLayers);
            return 1;
        }
    }
    free(avaiableLayers);
    return 0;
}

static VkBool32 VKAPI_CALL VE_Render_DebugReportCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData)
{
//    if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
//        return VK_FALSE;

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

void VE_Render_RegisterDebugReportCallback() {
    VkDebugReportCallbackCreateInfoEXT createInfo = { VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT };
    createInfo.flags = VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT | VK_DEBUG_REPORT_ERROR_BIT_EXT;
    createInfo.pNext = NULL;
    createInfo.pfnCallback = VE_Render_DebugReportCallback;
    createInfo.pUserData = NULL;

    vkCreateDebugReportCallbackEXT(VE_G_Instance, &createInfo, NULL, &VE_G_DebugCallback);
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

    unsigned int sdlInstanceCount;
    SDL_Vulkan_GetInstanceExtensions(VE_G_Window, &sdlInstanceCount, NULL);

	VkInstanceCreateInfo createInfo = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
	createInfo.pNext;
	createInfo.flags;
	createInfo.pApplicationInfo = &appInfo;
#ifdef NDEBUG
	createInfo.enabledLayerCount = 0;
	createInfo.ppEnabledLayerNames = NULL;
    const char **instanceExtensions = malloc(sizeof(char*) * sdlInstanceCount);
    createInfo.enabledExtensionCount = sdlInstanceCount;
#else
    const char* debugLayers[] = {"VK_LAYER_KHRONOS_validation" };
    createInfo.enabledLayerCount = sizeof(debugLayers[0]) / sizeof(debugLayers);
    createInfo.ppEnabledLayerNames = debugLayers;
    char **instanceExtensions = malloc(sizeof(char*) * (sdlInstanceCount + 1));
    instanceExtensions[sdlInstanceCount] = VK_EXT_DEBUG_REPORT_EXTENSION_NAME;
    createInfo.enabledExtensionCount = sdlInstanceCount + 1;
#endif // NDEBUG
    createInfo.ppEnabledExtensionNames = instanceExtensions;
    SDL_Vulkan_GetInstanceExtensions(VE_G_Window, &sdlInstanceCount, instanceExtensions );
    vkCreateInstance(&createInfo, NULL, &VE_G_Instance);

    volkLoadInstanceOnly(VE_G_Instance);
    free(instanceExtensions);
}

void VE_Render_PickPhysicalDeviceAndQueues() {
    uint32_t count;
    vkEnumeratePhysicalDevices(VE_G_Instance, &count, NULL);
    VkPhysicalDevice *devices = malloc(sizeof(VkPhysicalDevice) * count);
    vkEnumeratePhysicalDevices(VE_G_Instance, &count, devices);
    for (uint32_t i = 0; i < count; ++i) {
        // TODO: Implement physical device picking logic
        uint32_t queueGraphicsIndex = UINT32_MAX;
        uint32_t queueFamilyCount;
        vkGetPhysicalDeviceQueueFamilyProperties(devices[i], &queueFamilyCount, NULL);
        VkQueueFamilyProperties *queueFamilyProperties = malloc(sizeof(VkQueueFamilyProperties) * queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(devices[i], &queueFamilyCount, queueFamilyProperties);
        for (uint32_t j = 0; j < queueFamilyCount; ++j) {
            if (queueFamilyProperties[j].queueFlags & VK_QUEUE_GRAPHICS_BIT)
                queueGraphicsIndex = j;
        }
        free(queueFamilyProperties);
        if (queueGraphicsIndex != UINT32_MAX) {
            VE_G_PhysicalDevice = devices[i];
            VE_G_GraphicsQueue = (VkQueue)queueGraphicsIndex; // Not like the nicest thing to do but space optimal
            break;
        }
    }
    if (!VE_G_PhysicalDevice) {
        fprintf(stderr, "Failed to find suitable graphics card!\n");
        exit(1);
    }
    free(devices);
}

void VE_Render_CreateDevice() {
    VkPhysicalDeviceFeatures enabledFeatures = {0};

    float priority = 1.0f;
    VkDeviceQueueCreateInfo queueCreateInfo = {VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
    queueCreateInfo.queueFamilyIndex = VE_G_GraphicsQueue;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &priority;

    VkDeviceCreateInfo createInfo = {VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
    createInfo.queueCreateInfoCount = 1;
    createInfo.pQueueCreateInfos = &queueCreateInfo;
    createInfo.pEnabledFeatures = &enabledFeatures;

#ifndef NDEBUG
    const char *debugLayers[] = {"VK_LAYER_KHRONOS_validation"};
    createInfo.ppEnabledLayerNames = debugLayers;
    createInfo.enabledLayerCount = sizeof(debugLayers[0]) / sizeof(debugLayers);
#endif

    vkCreateDevice(VE_G_PhysicalDevice, &createInfo, NULL, &VE_G_Device);

    volkLoadDevice(VE_G_Device);

    vkGetDeviceQueue(VE_G_Device, queueCreateInfo.queueFamilyIndex, 0, &VE_G_GraphicsQueue);
}

void VE_Render_CreateSurface()
{
}
