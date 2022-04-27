#ifndef RENDER_GLOBALS_H
#define RENDER_GLOBALS_H

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>
typedef struct SDL_Window SDL_Window;

extern SDL_Window *VE_G_Window;
extern VkInstance VE_G_Instance;
extern VkPhysicalDevice VE_G_PhysicalDevice;
extern VkDevice VE_G_Device;
extern uint32_t VE_G_GraphicsQueueIndex;
extern uint32_t VE_G_PresentQueueIndex;
extern VkQueue VE_G_GraphicsQueue;
extern VkQueue VE_G_PresentQueue;
extern VkSurfaceKHR VE_G_Surface;
extern VkSwapchainKHR VE_G_Swapchain;
extern VkFormat VE_G_SwapchainFormat;
extern VkExtent2D VE_G_SwapchainExtent;
extern uint32_t VE_G_SwapchainImageCount;
extern VkImage *VE_G_pSwapchainImages;
extern VkImageView *VE_G_pSwapchainImageViews;
extern VkCommandPool VE_G_CommandPool;
extern VkCommandBuffer *VE_G_pCommandBuffers;
extern VkSemaphore VE_G_ImageAvailableSemaphore;
extern VkSemaphore VE_G_RenderFinishedSemaphore;
extern VkFence VE_G_InFlightFence;
#ifndef NDEBUG
extern VkDebugReportCallbackEXT VE_G_DebugCallback;
#endif // NDEBUG

#endif // RENDER_GLOBALS_H
