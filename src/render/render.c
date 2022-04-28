#include "render.h"
#include <stdio.h>
#include <volk.h>
#include <SDL_vulkan.h>
#include "init.h"
#include "globals.h"
#include "shader.h"

void VE_Render_Init(SDL_Window *window) {
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
    VE_Render_CreateCommandPool();
    VE_Render_CreateSyncObjects();
}

void VE_Render_Destroy() {
    VE_Render_DestroyAllPrograms(1);
    for (uint32_t i = 0; i < VE_RENDER_MAX_FRAMES_IN_FLIGHT; ++i) {
        vkDestroySemaphore(VE_G_Device, VE_G_pImageAvailableSemaphores[i], NULL);
        vkDestroySemaphore(VE_G_Device, VE_G_pRenderFinishedSemaphores[i], NULL);
        vkDestroyFence(VE_G_Device, VE_G_pInFlightFences[i], NULL);
    }
    vkDestroyCommandPool(VE_G_Device, VE_G_CommandPool, NULL);
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

void VE_Render_Resize() {
    vkDeviceWaitIdle(VE_G_Device);

    for (uint32_t i = 0; i < VE_G_SwapchainImageCount; ++i) {
        vkDestroyImageView(VE_G_Device, VE_G_pSwapchainImageViews[i], NULL);
    }
    free(VE_G_pSwapchainImageViews);
    if (VE_G_pSwapchainImages)
        free(VE_G_pSwapchainImages);
    vkDestroySwapchainKHR(VE_G_Device, VE_G_Swapchain, NULL);
    VE_Render_CreateSwapchain();
    VE_Render_RecreateAllPrograms();
}

static uint32_t imageIndex = 0;
static uint32_t currentFrame = 0;

void VE_Render_BeginFrame() {
    vkAcquireNextImageKHR(VE_G_Device, VE_G_Swapchain, UINT64_MAX, VE_G_pImageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
}

void VE_Render_EndFrame() {
    vkWaitForFences(VE_G_Device, 1, &VE_G_pInFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
    vkResetFences(VE_G_Device, 1, &VE_G_pInFlightFences[currentFrame]);

    VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };

    VkSemaphore waitSemaphores[] = { VE_G_pImageAvailableSemaphores[currentFrame] };
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &VE_G_pCommandBuffers[currentFrame];

    VkSemaphore signalSemaphores[] = { VE_G_pRenderFinishedSemaphores[currentFrame] };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    vkQueueSubmit(VE_G_GraphicsQueue, 1, &submitInfo, VE_G_pInFlightFences[currentFrame]);

    VkPresentInfoKHR presentInfo = { 0 };
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &VE_G_Swapchain;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = NULL;

    vkQueuePresentKHR(VE_G_PresentQueue, &presentInfo);

    vkDeviceWaitIdle(VE_G_Device);

    currentFrame = (currentFrame + 1) % VE_RENDER_MAX_FRAMES_IN_FLIGHT;
}

void VE_Render_Draw(VE_ProgramT *pProgram) {
    VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    beginInfo.pInheritanceInfo = NULL;

    vkBeginCommandBuffer(VE_G_pCommandBuffers[currentFrame], &beginInfo);

    VkRenderPassBeginInfo renderPassInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
    renderPassInfo.renderPass = pProgram->renderPass;
    renderPassInfo.framebuffer = pProgram->pFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset = (VkOffset2D){0, 0};
    renderPassInfo.renderArea.extent = VE_G_SwapchainExtent;
    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;
    vkCmdBeginRenderPass(VE_G_pCommandBuffers[currentFrame], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(VE_G_pCommandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, pProgram->pipeline);
    vkCmdDraw(VE_G_pCommandBuffers[currentFrame], 3, 1, 0, 0);
    vkCmdEndRenderPass(VE_G_pCommandBuffers[currentFrame]);

    vkEndCommandBuffer(VE_G_pCommandBuffers[currentFrame]);
}
