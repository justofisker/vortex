#include "globals.h"

SDL_Window *VE_G_Window = NULL;
VkInstance VE_G_Instance = VK_NULL_HANDLE;
VkPhysicalDevice VE_G_PhysicalDevice = VK_NULL_HANDLE;
VkDevice VE_G_Device = VK_NULL_HANDLE;
VkQueue VE_G_GraphicsQueue = VK_NULL_HANDLE;
VkSurfaceKHR VE_G_Surface = VK_NULL_HANDLE;
#ifndef NDEBUG
VkDebugReportCallbackEXT VE_G_DebugCallback = VK_NULL_HANDLE;
#endif // NDEBUG