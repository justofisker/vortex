#include "globals.h"

SDL_Window *VE_G_Window = NULL;
VkInstance VE_G_Instance = VK_NULL_HANDLE;
VkPhysicalDevice VE_G_PhysicalDevice = VK_NULL_HANDLE;
VkDevice VE_G_Device = VK_NULL_HANDLE;
uint32_t VE_G_GraphicsQueueIndex = UINT32_MAX;
uint32_t VE_G_PresentQueueIndex = UINT32_MAX;
VkQueue VE_G_GraphicsQueue = VK_NULL_HANDLE;
VkQueue VE_G_PresentQueue = VK_NULL_HANDLE;
VkSurfaceKHR VE_G_Surface = VK_NULL_HANDLE;
VkSwapchainKHR VE_G_Swapchain = VK_NULL_HANDLE;
VkFormat VE_G_SwapchainFormat = VK_FORMAT_MAX_ENUM;
VkExtent2D VE_G_SwapchainExtent = { 0 };
uint32_t VE_G_SwapchainImageCount = 0;
VkImage *VE_G_pSwapchainImages = NULL;
VkImageView *VE_G_pSwapchainImageViews = NULL;
VkCommandPool VE_G_CommandPool = VK_NULL_HANDLE;
VkCommandBuffer VE_G_pCommandBuffers[VE_RENDER_MAX_FRAMES_IN_FLIGHT] = {VK_NULL_HANDLE };
VkSemaphore VE_G_pImageAvailableSemaphores[VE_RENDER_MAX_FRAMES_IN_FLIGHT] = {VK_NULL_HANDLE };
VkSemaphore VE_G_pRenderFinishedSemaphores[VE_RENDER_MAX_FRAMES_IN_FLIGHT] = {VK_NULL_HANDLE };
VkFence VE_G_pInFlightFences[VE_RENDER_MAX_FRAMES_IN_FLIGHT] = {VK_NULL_HANDLE };
VE_ProgramT *VE_G_ppPrograms[VE_RENDER_MAX_PROGRAMS] = { NULL };
uint32_t VE_G_ProgramCount = 0;
#ifndef NDEBUG
VkDebugReportCallbackEXT VE_G_DebugCallback = VK_NULL_HANDLE;
#endif // NDEBUG
