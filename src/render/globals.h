#ifndef RENDER_GLOBALS_H
#define RENDER_GLOBALS_H

#include "types.h"

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
extern VkCommandPool VE_G_TransferCommandPool;
extern VkCommandBuffer VE_G_pCommandBuffers[VE_RENDER_MAX_FRAMES_IN_FLIGHT];
extern VkSemaphore VE_G_pImageAvailableSemaphores[VE_RENDER_MAX_FRAMES_IN_FLIGHT];
extern VkSemaphore VE_G_pRenderFinishedSemaphores[VE_RENDER_MAX_FRAMES_IN_FLIGHT];
extern VkFence VE_G_pInFlightFences[VE_RENDER_MAX_FRAMES_IN_FLIGHT];
extern VE_ProgramT *VE_G_ppPrograms[VE_RENDER_MAX_PROGRAMS];
extern uint32_t VE_G_ProgramCount;
extern uint32_t VE_G_CurrentFrame;
extern VkImage VE_G_DepthImage;
extern VkDeviceMemory VE_G_DepthImageMemory;
extern VkImageView VE_G_DepthImageView;
extern VkFormat VE_G_DepthImageFormat;
extern VkRenderPass VE_G_RenderPass;
extern VkFramebuffer *VE_G_pFramebuffers;
#ifndef NDEBUG
extern VkDebugReportCallbackEXT VE_G_DebugCallback;
#endif // NDEBUG

#endif // RENDER_GLOBALS_H
