#ifndef RENDER_GLOBALS_H
#define RENDER_GLOBALS_H

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>
typedef struct SDL_Window SDL_Window;

extern SDL_Window *VE_G_Window;
extern VkInstance VE_G_Instance;
extern VkPhysicalDevice VE_G_PhysicalDevice;
extern VkDevice VE_G_Device;
extern VkSurfaceKHR VE_G_Surface;

#endif // RENDER_GLOBALS_H