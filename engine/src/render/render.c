#include "render.h"
#include <stdio.h>
#include <volk.h>
#include <SDL_vulkan.h>
#include "init.h"
#include "globals.h"
#include "shader.h"
#include "util.h"
#include <stdlib.h>
#include <string.h>
#include "../ecs/builtin.h"
#include <cglm/cglm.h>

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
    VE_Render_CreateCommandPool();
    VE_Render_CreateSwapchain();
    VE_Render_CreateSyncObjects();
    VE_Render_CreateRenderpass();
    VE_Render_CreateFramebuffers();

    VkPhysicalDeviceProperties physicalDeviceProperties;
    vkGetPhysicalDeviceProperties(VE_G_PhysicalDevice, &physicalDeviceProperties);
    printf("%s (Vulkan %d.%d.%d)\n", physicalDeviceProperties.deviceName, VK_API_VERSION_MAJOR(physicalDeviceProperties.apiVersion),
           VK_API_VERSION_MINOR(physicalDeviceProperties.apiVersion),
           VK_API_VERSION_PATCH(physicalDeviceProperties.apiVersion));
}

void VE_Render_Destroy() {
    vkDeviceWaitIdle(VE_G_Device);

    VE_Render_DestroyAllPrograms(1);

    for (uint32_t i = 0; i < VE_G_SwapchainImageCount; ++i)
        vkDestroyFramebuffer(VE_G_Device, VE_G_pFramebuffers[i], NULL);
    free(VE_G_pFramebuffers);
    vkDestroyRenderPass(VE_G_Device, VE_G_RenderPass, NULL);
    for (uint32_t i = 0; i < VE_RENDER_MAX_FRAMES_IN_FLIGHT; ++i) {
        vkDestroySemaphore(VE_G_Device, VE_G_pImageAvailableSemaphores[i], NULL);
        vkDestroySemaphore(VE_G_Device, VE_G_pRenderFinishedSemaphores[i], NULL);
        vkDestroyFence(VE_G_Device, VE_G_pInFlightFences[i], NULL);
    }
    vkDestroyCommandPool(VE_G_Device, VE_G_TransferCommandPool, NULL);
    vkDestroyCommandPool(VE_G_Device, VE_G_CommandPool, NULL);
    vkDestroyImage(VE_G_Device, VE_G_DepthImage, NULL);
    vkFreeMemory(VE_G_Device, VE_G_DepthImageMemory, NULL);
    vkDestroyImageView(VE_G_Device, VE_G_DepthImageView, NULL);
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

    vkDestroyImage(VE_G_Device, VE_G_DepthImage, NULL);
    vkFreeMemory(VE_G_Device, VE_G_DepthImageMemory, NULL);
    vkDestroyImageView(VE_G_Device, VE_G_DepthImageView, NULL);
    for (uint32_t i = 0; i < VE_G_SwapchainImageCount; ++i) {
        vkDestroyImageView(VE_G_Device, VE_G_pSwapchainImageViews[i], NULL);
        vkDestroyFramebuffer(VE_G_Device, VE_G_pFramebuffers[i], NULL);
    }
    free(VE_G_pSwapchainImageViews);
    if (VE_G_pSwapchainImages)
        free(VE_G_pSwapchainImages);
    vkDestroySwapchainKHR(VE_G_Device, VE_G_Swapchain, NULL);
    VE_Render_CreateSwapchain();
    VE_Render_RecreateAllPrograms();
    VE_Render_CreateFramebuffers();
}

float VE_Render_GetAspectRatio() {
    return VE_G_SwapchainExtent.width / (float)VE_G_SwapchainExtent.height;
}

static uint32_t imageIndex = 0;

void VE_Render_BeginFrame() {
    vkWaitForFences(VE_G_Device, 1, &VE_G_pInFlightFences[VE_G_CurrentFrame], VK_TRUE, UINT64_MAX);
    vkResetFences(VE_G_Device, 1, &VE_G_pInFlightFences[VE_G_CurrentFrame]);

    VkResult result = vkAcquireNextImageKHR(VE_G_Device, VE_G_Swapchain, UINT64_MAX, VE_G_pImageAvailableSemaphores[VE_G_CurrentFrame], VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        VE_Render_Resize();
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        perror("Failed to acquire swapchain image!\n");
        exit(-1);
    }

    VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    beginInfo.pInheritanceInfo = NULL;

    vkBeginCommandBuffer(VE_G_pCommandBuffers[VE_G_CurrentFrame], &beginInfo);

    VkRenderPassBeginInfo renderPassInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
    renderPassInfo.renderPass = VE_G_RenderPass;
    renderPassInfo.framebuffer = VE_G_pFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset = (VkOffset2D){0, 0};
    renderPassInfo.renderArea.extent = VE_G_SwapchainExtent;
    VkClearValue clearColor[2] = { {{{0.0f, 0.0f, 0.0f, 1.0f}}}, {1.0f, 0}};
    renderPassInfo.clearValueCount = 2;
    renderPassInfo.pClearValues = clearColor;
    vkCmdBeginRenderPass(VE_G_pCommandBuffers[VE_G_CurrentFrame], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void VE_Render_EndFrame() {
    vkCmdEndRenderPass(VE_G_pCommandBuffers[VE_G_CurrentFrame]);
    vkEndCommandBuffer(VE_G_pCommandBuffers[VE_G_CurrentFrame]);

    VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };

    VkSemaphore waitSemaphores[] = { VE_G_pImageAvailableSemaphores[VE_G_CurrentFrame] };
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &VE_G_pCommandBuffers[VE_G_CurrentFrame];

    VkSemaphore signalSemaphores[] = { VE_G_pRenderFinishedSemaphores[VE_G_CurrentFrame] };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    vkQueueSubmit(VE_G_GraphicsQueue, 1, &submitInfo, VE_G_pInFlightFences[VE_G_CurrentFrame]);

    VkPresentInfoKHR presentInfo = { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &VE_G_Swapchain;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = NULL;

    vkQueuePresentKHR(VE_G_PresentQueue, &presentInfo);

    VE_G_CurrentFrame = (VE_G_CurrentFrame + 1) % VE_RENDER_MAX_FRAMES_IN_FLIGHT;
}

void VE_Render_Draw(VE_MeshObject_T *pMeshObject) {
    VkCommandBuffer cmd = VE_G_pCommandBuffers[VE_G_CurrentFrame];

    // TODO: This should only be called once per frame
    vkCmdPushConstants(cmd, pMeshObject->pProgram->layout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(VE_FragmentShaderPushConstants_t), &VE_G_ShaderPushConstants);
    
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pMeshObject->pProgram->pipeline);

    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(cmd, 0, 1, &pMeshObject->pVertexBuffer->buffer, offsets);
    vkCmdBindIndexBuffer(cmd, pMeshObject->pIndexBuffer->buffer, 0, VK_INDEX_TYPE_UINT16);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pMeshObject->pProgram->layout, 0, 1, &pMeshObject->pDescriptorSets[VE_G_CurrentFrame], 0, NULL);

    vkCmdDrawIndexed(cmd, pMeshObject->pIndexBuffer->instanceCount, 1, 0, 0, 0);
}

void VE_Render_RenderScene() {
    VE_Render_BeginFrame();
    for (uint32_t i = 0; i < VE_G_MeshEntityCount; ++i) {
        VE_Render_Draw(VE_G_pMeshEntities[i]);
    }
    VE_Render_EndFrame();
}

void VE_Render_RegisterEntity(VE_MeshObject_T *pMeshObject) {
    VE_G_pMeshEntities[VE_G_MeshEntityCount++] = pMeshObject;
}

void VE_Render_UnregisterEntity(VE_MeshObject_T *pMeshObject) {
    for (uint32_t i = 0; i < VE_G_MeshEntityCount; ++i) {
        if (VE_G_pMeshEntities[i] == pMeshObject) {
            if (--VE_G_MeshEntityCount && i != VE_G_MeshEntityCount) {
                VE_G_pMeshEntities[i] = VE_G_pMeshEntities[VE_G_MeshEntityCount];
            }
        }
    }
}

void VE_Render_SetProjectionMatrix(mat4 projectionMatrix) {
    glm_mat4_copy(projectionMatrix, VE_G_ProjectionMatrix);
}

void VE_Render_SetViewMatrix(mat4 viewMatrix) {
    glm_mat4_copy(viewMatrix, VE_G_ViewMatrix);
}

void VE_Render_GetGameSize(ivec2 gameSize)
{
    gameSize[0] = VE_G_SwapchainExtent.width;
    gameSize[1] = VE_G_SwapchainExtent.height;
}
