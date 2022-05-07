#include <volk.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "init.h"
#include "globals.h"
#include "SDL_vulkan.h"
#include "util.h"
#include <string.h>
#include "texture.h"

#ifndef NDEBUG
int VE_Render_CheckValidationSupport() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, NULL);
    VkLayerProperties *pAvailableLayers = malloc(sizeof(VkLayerProperties) * layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, pAvailableLayers);
    for (int i = 0; i < layerCount; ++i) {
        if (strcmp("VK_LAYER_KHRONOS_validation", pAvailableLayers[i].layerName) == 0) {
            free(pAvailableLayers);
            return 1;
        }
    }
    free(pAvailableLayers);
    return 0;
}

static VkBool32 VKAPI_CALL VE_Render_DebugReportCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData) {
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

void VE_Render_CreateInstance() {
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
    const char **ppInstanceExtensions = malloc(sizeof(char*) * sdlInstanceCount);
    createInfo.enabledExtensionCount = sdlInstanceCount;
#else
    const char* ppDebugLayers[] = {"VK_LAYER_KHRONOS_validation" };
    createInfo.enabledLayerCount = sizeof(ppDebugLayers[0]) / sizeof(ppDebugLayers);
    createInfo.ppEnabledLayerNames = ppDebugLayers;
    const char **ppInstanceExtensions = malloc(sizeof(char*) * (sdlInstanceCount + 1));
    ppInstanceExtensions[sdlInstanceCount] = VK_EXT_DEBUG_REPORT_EXTENSION_NAME;
    createInfo.enabledExtensionCount = sdlInstanceCount + 1;
#endif // NDEBUG
    createInfo.ppEnabledExtensionNames = ppInstanceExtensions;
    SDL_Vulkan_GetInstanceExtensions(VE_G_Window, &sdlInstanceCount, ppInstanceExtensions );
    vkCreateInstance(&createInfo, NULL, &VE_G_Instance);

    volkLoadInstanceOnly(VE_G_Instance);
    free(ppInstanceExtensions);
}

void VE_Render_PickPhysicalDeviceAndQueues() {
    uint32_t count;
    vkEnumeratePhysicalDevices(VE_G_Instance, &count, NULL);
    VkPhysicalDevice *devices = malloc(sizeof(VkPhysicalDevice) * count);
    vkEnumeratePhysicalDevices(VE_G_Instance, &count, devices);
    uint32_t deviceScore = UINT32_MAX;
    for (uint32_t i = 0; i < count; ++i) {
        // TODO: Implement physical device picking logic

        uint32_t currentDeviceScore = 0;

        VkPhysicalDeviceProperties physicalDeviceProperties;
        vkGetPhysicalDeviceProperties(devices[i], &physicalDeviceProperties);

        if (physicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            currentDeviceScore += 100;

        // Require device to have a graphics capable queue
        uint32_t queueGraphicsIndex = UINT32_MAX, queuePresentIndex = UINT32_MAX;
        uint32_t queueFamilyCount;
        vkGetPhysicalDeviceQueueFamilyProperties(devices[i], &queueFamilyCount, NULL);
        VkQueueFamilyProperties *pQueueFamilyProperties = malloc(sizeof(VkQueueFamilyProperties) * queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(devices[i], &queueFamilyCount, pQueueFamilyProperties);
        for (uint32_t j = 0; j < queueFamilyCount; ++j) {
            if (pQueueFamilyProperties[j].queueFlags & VK_QUEUE_GRAPHICS_BIT)
                queueGraphicsIndex = j;

            VkBool32 presentSupport;
            vkGetPhysicalDeviceSurfaceSupportKHR(devices[i], j, VE_G_Surface, &presentSupport);
            if (presentSupport)
                queuePresentIndex = j;

            if (queueGraphicsIndex != UINT32_MAX && queuePresentIndex != UINT32_MAX)
                break;
        }
        free(pQueueFamilyProperties);

        if (queueGraphicsIndex == UINT32_MAX || queuePresentIndex == UINT32_MAX)
            continue;

        // Require device to have certain extensions supported
        const char *ppRequiredExtensions[] = {
                VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };

        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(devices[i], NULL, &extensionCount, NULL);
        VkExtensionProperties *pAvailableExtensions = malloc(sizeof(VkExtensionProperties) * extensionCount);
        vkEnumerateDeviceExtensionProperties(devices[i], NULL, &extensionCount, pAvailableExtensions);
        for (uint32_t j = 0; j < sizeof(ppRequiredExtensions[0]) / sizeof(ppRequiredExtensions); ++j) {
            char foundExtension = 0;
            for (uint32_t k = 0; k < extensionCount; ++k) {
                if (strcmp(ppRequiredExtensions[j], pAvailableExtensions[k].extensionName) == 0) {
                    foundExtension = 1;
                    break;
                }
            }
            if (!foundExtension) {
                free(pAvailableExtensions);
                continue;
            }
        }
        free(pAvailableExtensions);

        if (currentDeviceScore > deviceScore || deviceScore == UINT32_MAX) {
            VE_G_PhysicalDevice = devices[i];
            VE_G_GraphicsQueueIndex = queueGraphicsIndex;
            VE_G_PresentQueueIndex = queuePresentIndex;
            deviceScore = currentDeviceScore;
        }
    }
    if (!VE_G_PhysicalDevice) {
        fprintf(stderr, "Failed to find suitable graphics card!\n");
        exit(1);
    }
    free(devices);
}

void VE_Render_CreateDevice() {
    VkPhysicalDeviceFeatures enabledFeatures = { 0 };

    float priority = 1.0f;
    VkDeviceQueueCreateInfo pQueueCreateInfo[2] = {0 };
    pQueueCreateInfo[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    pQueueCreateInfo[0].queueFamilyIndex = VE_G_GraphicsQueueIndex;
    pQueueCreateInfo[0].queueCount = 1;
    pQueueCreateInfo[0].pQueuePriorities = &priority;
    pQueueCreateInfo[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    pQueueCreateInfo[1].queueFamilyIndex = VE_G_PresentQueueIndex;
    pQueueCreateInfo[1].queueCount = 1;
    pQueueCreateInfo[1].pQueuePriorities = &priority;

    VkDeviceCreateInfo createInfo = {VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
    createInfo.queueCreateInfoCount = 1 + (VE_G_GraphicsQueueIndex != VE_G_PresentQueueIndex);
    createInfo.pQueueCreateInfos = pQueueCreateInfo;
    createInfo.pEnabledFeatures = &enabledFeatures;

#ifndef NDEBUG
    const char *ppDebugLayers[] = {"VK_LAYER_KHRONOS_validation"};
    createInfo.ppEnabledLayerNames = ppDebugLayers;
    createInfo.enabledLayerCount = sizeof(ppDebugLayers[0]) / sizeof(ppDebugLayers);
#endif
    const char *ppDeviceExtensions[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
    createInfo.ppEnabledExtensionNames = ppDeviceExtensions;
    createInfo.enabledExtensionCount = sizeof(ppDeviceExtensions[0]) / sizeof(ppDeviceExtensions);

    vkCreateDevice(VE_G_PhysicalDevice, &createInfo, NULL, &VE_G_Device);

    volkLoadDevice(VE_G_Device);

    vkGetDeviceQueue(VE_G_Device, VE_G_GraphicsQueueIndex, 0, &VE_G_GraphicsQueue);
    vkGetDeviceQueue(VE_G_Device, VE_G_PresentQueueIndex, 0, &VE_G_PresentQueue);
}

void VE_Render_CreateSurface()
{
    if(!SDL_Vulkan_CreateSurface(VE_G_Window, VE_G_Instance, &VE_G_Surface)) {
        fprintf(stderr, "Failed to create surface!");
        exit(1);
    }
}

static VkFormat VE_Render_FindSupportedFormat(VkFormat *pFormats, uint32_t formatCount, VkImageTiling tiling, VkFormatFeatureFlags features) {
    for (uint32_t i = 0; i < formatCount; ++i) {
        VkFormatProperties properties;
        vkGetPhysicalDeviceFormatProperties(VE_G_PhysicalDevice, pFormats[i], &properties);

        if (tiling == VK_IMAGE_TILING_LINEAR && (properties.linearTilingFeatures & features) == features) {
            return pFormats[i];
        } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & features) == features) {
            return pFormats[i];
        }
    }

    fprintf(stderr, "failed to find format!\n");
    exit(-1);
}

static VkFormat VE_Render_FindDepthFormat() {
    VkFormat pFormats[] = {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT};
    return VE_Render_FindSupportedFormat(pFormats, sizeof(pFormats) / sizeof(pFormats[0]), VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

static char VE_Render_HasStencilComponent(VkFormat format) {
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

void VE_Render_CreateSwapchain() {
    uint32_t availableFormatsCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(VE_G_PhysicalDevice, VE_G_Surface, &availableFormatsCount, NULL);
    VkSurfaceFormatKHR *pAvailableFormats = malloc(sizeof(VkSurfaceFormatKHR) * availableFormatsCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(VE_G_PhysicalDevice, VE_G_Surface, &availableFormatsCount, pAvailableFormats);
    VkSurfaceFormatKHR surfaceFormat = pAvailableFormats[0];
    for (uint32_t i = 0; i < availableFormatsCount; ++i) {
        if (pAvailableFormats[i].format == VK_FORMAT_B8G8R8A8_SRGB &&
            pAvailableFormats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            surfaceFormat = pAvailableFormats[i];
            break;
        }
    }
    free(pAvailableFormats);
    VE_G_SwapchainFormat = surfaceFormat.format;

    uint32_t availablePresentModesCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(VE_G_PhysicalDevice, VE_G_Surface, &availablePresentModesCount, NULL);
    VkPresentModeKHR *pAvailablePresentModes = malloc(sizeof(VkPresentModeKHR) * availablePresentModesCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(VE_G_PhysicalDevice, VE_G_Surface, &availablePresentModesCount,
                                              pAvailablePresentModes);
    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
    for (uint32_t i = 0; i < availablePresentModesCount; ++i) {
        if (pAvailablePresentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
            presentMode = pAvailablePresentModes[i];
            break;
        }
    }
    free(pAvailablePresentModes);

    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(VE_G_PhysicalDevice, VE_G_Surface, &surfaceCapabilities);
    int w, h;
    SDL_Vulkan_GetDrawableSize(VE_G_Window, &w, &h);
    VE_G_SwapchainExtent = (VkExtent2D) {
            min(max(w, surfaceCapabilities.minImageExtent.width), surfaceCapabilities.maxImageExtent.width),
            min(max(h, surfaceCapabilities.minImageExtent.height), surfaceCapabilities.maxImageExtent.height),
    };

    uint32_t imageCount = surfaceCapabilities.minImageCount + 1;
    if (surfaceCapabilities.maxImageCount)
        imageCount = min(imageCount, surfaceCapabilities.maxImageCount);

    VkSwapchainCreateInfoKHR createInfo = {VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
    createInfo.surface = VE_G_Surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = VE_G_SwapchainExtent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    uint32_t pQueueFamilyIndices[] = {VE_G_GraphicsQueueIndex, VE_G_PresentQueueIndex};
    if (VE_G_GraphicsQueueIndex != VE_G_PresentQueueIndex) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = pQueueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = NULL;
    }

    createInfo.preTransform = surfaceCapabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    vkCreateSwapchainKHR(VE_G_Device, &createInfo, NULL, &VE_G_Swapchain);

    vkGetSwapchainImagesKHR(VE_G_Device, VE_G_Swapchain, &VE_G_SwapchainImageCount, NULL);
    VE_G_pSwapchainImages = malloc(sizeof(VkImage) * VE_G_SwapchainImageCount);
    vkGetSwapchainImagesKHR(VE_G_Device, VE_G_Swapchain, &VE_G_SwapchainImageCount, VE_G_pSwapchainImages);

    VE_G_pSwapchainImageViews = malloc(sizeof(VkImageView) * VE_G_SwapchainImageCount);
    for (uint32_t i = 0; i < VE_G_SwapchainImageCount; ++i) {
        VE_G_pSwapchainImageViews[i] = VE_Render_CreateImageView(VE_G_pSwapchainImages[i], VE_G_SwapchainFormat, VK_IMAGE_ASPECT_COLOR_BIT);
    }

    VE_G_DepthImageFormat = VE_Render_FindDepthFormat();

    VE_Render_CreateImage(VE_G_SwapchainExtent.width, VE_G_SwapchainExtent.height, VE_G_DepthImageFormat, VK_IMAGE_TILING_OPTIMAL,
                          VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                          &VE_G_DepthImage, &VE_G_DepthImageMemory);

    VE_G_DepthImageView = VE_Render_CreateImageView(VE_G_DepthImage, VE_G_DepthImageFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

    VE_Render_TransitionImageLayout(VE_G_DepthImage, VE_G_DepthImageFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}

void VE_Render_CreateCommandPool() {
    VkCommandPoolCreateInfo poolInfo = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT ;
    poolInfo.queueFamilyIndex = VE_G_GraphicsQueueIndex;

    vkCreateCommandPool(VE_G_Device, &poolInfo, NULL, &VE_G_CommandPool);

    poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT ;
    vkCreateCommandPool(VE_G_Device, &poolInfo, NULL, &VE_G_TransferCommandPool);

    VkCommandBufferAllocateInfo allocInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
    allocInfo.commandPool = VE_G_CommandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = VE_RENDER_MAX_FRAMES_IN_FLIGHT;

    vkAllocateCommandBuffers(VE_G_Device, &allocInfo, VE_G_pCommandBuffers);
}

void VE_Render_CreateSyncObjects() {
    VkSemaphoreCreateInfo semaphoreInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
    VkFenceCreateInfo fenceInfo = { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (uint32_t i = 0; i < VE_RENDER_MAX_FRAMES_IN_FLIGHT; ++i) {
        vkCreateSemaphore(VE_G_Device, &semaphoreInfo, NULL, &VE_G_pImageAvailableSemaphores[i]);
        vkCreateSemaphore(VE_G_Device, &semaphoreInfo, NULL, &VE_G_pRenderFinishedSemaphores[i]);
        vkCreateFence(VE_G_Device, &fenceInfo, NULL, &VE_G_pInFlightFences[i]);
    }
}

void VE_Render_CreateRenderpass() {
    VkAttachmentDescription colorAttachment = {0};
    colorAttachment.format = VE_G_SwapchainFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentDescription depthAttachment = {0};
    depthAttachment.format = VE_G_DepthImageFormat;
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorAttachmentRef = {0};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef = {0};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {0};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    VkSubpassDependency dependency = {0};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask =
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask =
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    VkAttachmentDescription attachments[] = {colorAttachment, depthAttachment};

    VkRenderPassCreateInfo renderPassInfo = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
    renderPassInfo.attachmentCount = sizeof(attachments) / sizeof(attachments[0]);
    renderPassInfo.pAttachments = attachments;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    vkCreateRenderPass(VE_G_Device, &renderPassInfo, NULL, &VE_G_RenderPass);
}

void VE_Render_CreateFramebuffers() {
    VkImageView framebufferAttachments[] = { VK_NULL_HANDLE, VE_G_DepthImageView };

    VkFramebufferCreateInfo framebufferInfo = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
    framebufferInfo.renderPass = VE_G_RenderPass;
    framebufferInfo.attachmentCount = sizeof(framebufferAttachments) / sizeof(framebufferAttachments[0]);
    framebufferInfo.pAttachments = framebufferAttachments;
    framebufferInfo.width = VE_G_SwapchainExtent.width;
    framebufferInfo.height = VE_G_SwapchainExtent.height;
    framebufferInfo.layers = 1;

    VE_G_pFramebuffers = malloc(sizeof(VkFramebuffer) * VE_G_SwapchainImageCount);
    for (uint32_t i = 0; i < VE_G_SwapchainImageCount; ++i) {
        framebufferAttachments[0] = VE_G_pSwapchainImageViews[i];
        vkCreateFramebuffer(VE_G_Device, &framebufferInfo, NULL, &VE_G_pFramebuffers[i]);
    }
}
