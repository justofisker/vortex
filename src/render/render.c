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
    vkDestroySemaphore(VE_G_Device, VE_G_ImageAvailableSemaphore, NULL);
    vkDestroySemaphore(VE_G_Device, VE_G_RenderFinishedSemaphore, NULL);
    vkDestroyFence(VE_G_Device, VE_G_InFlightFence, NULL);
    free(VE_G_pCommandBuffers);
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

static int32_t imageIndex = 0;

void VE_Render_BeginFrame() {
    vkAcquireNextImageKHR(VE_G_Device, VE_G_Swapchain, UINT64_MAX, VE_G_ImageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
}

void VE_Render_EndFrame() {
    vkWaitForFences(VE_G_Device, 1, &VE_G_InFlightFence, VK_TRUE, UINT64_MAX);
    vkResetFences(VE_G_Device, 1, &VE_G_InFlightFence);

    VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };

    VkSemaphore waitSemaphores[] = { VE_G_ImageAvailableSemaphore };
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &VE_G_pCommandBuffers[imageIndex];

    VkSemaphore signalSemaphores[] = { VE_G_RenderFinishedSemaphore };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    vkQueueSubmit(VE_G_GraphicsQueue, 1, &submitInfo, VE_G_InFlightFence);

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
}

void VE_Render_Draw(VE_Shader *pShader) {
    VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    beginInfo.pInheritanceInfo = NULL;

    vkBeginCommandBuffer(VE_G_pCommandBuffers[imageIndex], &beginInfo);

    VkRenderPassBeginInfo renderPassInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
    renderPassInfo.renderPass = pShader->renderPass;
    renderPassInfo.framebuffer = pShader->pFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset = (VkOffset2D){0, 0};
    renderPassInfo.renderArea.extent = VE_G_SwapchainExtent;
    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;
    vkCmdBeginRenderPass(VE_G_pCommandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(VE_G_pCommandBuffers[imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, pShader->pipeline);
    vkCmdDraw(VE_G_pCommandBuffers[imageIndex], 3, 1, 0, 0);
    vkCmdEndRenderPass(VE_G_pCommandBuffers[imageIndex]);

    vkEndCommandBuffer(VE_G_pCommandBuffers[imageIndex]);
}
