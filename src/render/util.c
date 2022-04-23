#include <volk.h>
#include "util.h"

#include "globals.h"

void VE_Render_CreateInstance()
{
	VkApplicationInfo appInfo = { VK_STRUCTURE_TYPE_APPLICATION_INFO };
	appInfo.pNext;
	appInfo.pApplicationName = "vortex testS";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "vortex";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo createInfo = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
	createInfo.pNext;
	createInfo.flags;
	createInfo.pApplicationInfo = &appInfo;
	createInfo.enabledLayerCount = 0;
	createInfo.ppEnabledLayerNames = NULL;
	createInfo.enabledExtensionCount = 0;
	createInfo.ppEnabledExtensionNames = NULL;


	vkCreateInstance(&createInfo, NULL, &VE_G_Instance);
}

void VE_Render_CreateDevice()
{
}

void VE_Render_CreateSurface()
{
}
