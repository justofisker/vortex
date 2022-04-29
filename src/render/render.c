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

    VkPhysicalDeviceProperties physicalDeviceProperties;
    vkGetPhysicalDeviceProperties(VE_G_PhysicalDevice, &physicalDeviceProperties);
    printf("%s (Vulkan %d.%d.%d)\n", physicalDeviceProperties.deviceName, VK_VERSION_MAJOR(physicalDeviceProperties.apiVersion),
           VK_VERSION_MINOR(physicalDeviceProperties.apiVersion),
           VK_VERSION_PATCH(physicalDeviceProperties.apiVersion));
}

void VE_Render_Destroy() {
    vkDeviceWaitIdle(VE_G_Device);

    VE_Render_DestroyAllPrograms(1);
    for (uint32_t i = 0; i < VE_RENDER_MAX_FRAMES_IN_FLIGHT; ++i) {
        vkDestroySemaphore(VE_G_Device, VE_G_pImageAvailableSemaphores[i], NULL);
        vkDestroySemaphore(VE_G_Device, VE_G_pRenderFinishedSemaphores[i], NULL);
        vkDestroyFence(VE_G_Device, VE_G_pInFlightFences[i], NULL);
    }
    vkDestroyCommandPool(VE_G_Device, VE_G_TransferCommandPool, NULL);
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
    vkWaitForFences(VE_G_Device, 1, &VE_G_pInFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
    vkResetFences(VE_G_Device, 1, &VE_G_pInFlightFences[currentFrame]);

    VkResult result = vkAcquireNextImageKHR(VE_G_Device, VE_G_Swapchain, UINT64_MAX, VE_G_pImageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        VE_Render_Resize();
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        perror("Failed to acquire swapchain image!\n");
        exit(-1);
    }
}

void VE_Render_EndFrame() {
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

    VkPresentInfoKHR presentInfo = { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &VE_G_Swapchain;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = NULL;

    vkQueuePresentKHR(VE_G_PresentQueue, &presentInfo);

    currentFrame = (currentFrame + 1) % VE_RENDER_MAX_FRAMES_IN_FLIGHT;
}

void VE_Render_Draw(VE_ProgramT *pProgram, VE_BufferT *pVertexBuffer, VE_BufferT *pIndexBuffer) {
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

    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(VE_G_pCommandBuffers[currentFrame], 0, 1, &pVertexBuffer->buffer, offsets);
    vkCmdBindIndexBuffer(VE_G_pCommandBuffers[currentFrame], pIndexBuffer->buffer, 0, VK_INDEX_TYPE_UINT16);

    vkCmdDrawIndexed(VE_G_pCommandBuffers[currentFrame], pIndexBuffer->instanceCount, 1, 0, 0, 0);
    vkCmdEndRenderPass(VE_G_pCommandBuffers[currentFrame]);

    vkEndCommandBuffer(VE_G_pCommandBuffers[currentFrame]);
}

static uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(VE_G_PhysicalDevice, &memProperties);
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    perror("failed to find memory type :(.");
    exit(-1);
}

static void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer *pBuffer, VkDeviceMemory *pBufferMemory) {
    VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    vkCreateBuffer(VE_G_Device, &bufferInfo, NULL, pBuffer);

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(VE_G_Device, *pBuffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    vkAllocateMemory(VE_G_Device, &allocInfo, NULL, pBufferMemory);

    vkBindBufferMemory(VE_G_Device, *pBuffer, *pBufferMemory, 0);
}

static void copyBuffer(VkBuffer dst, VkBuffer src, VkDeviceSize size) {
    VkCommandBufferAllocateInfo allocInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = VE_G_TransferCommandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(VE_G_Device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    VkBufferCopy copyRegion = { 0 };
    copyRegion.srcOffset = 0; // Optional
    copyRegion.dstOffset = 0; // Optional
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, src, dst, 1, &copyRegion);

    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(VE_G_GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(VE_G_GraphicsQueue);

    vkFreeCommandBuffers(VE_G_Device, VE_G_TransferCommandPool, 1, &commandBuffer);
}

VE_BufferT *VE_Render_CreateVertexBuffer(VE_VertexT *vertices, uint32_t count) {
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    VkDeviceSize bufferSize = sizeof(vertices[0]) * count;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);

    void* data;
    vkMapMemory(VE_G_Device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices, (size_t) bufferSize);
    vkUnmapMemory(VE_G_Device, stagingBufferMemory);

    VE_BufferT *pVertexBuffer = malloc(sizeof(VE_BufferT));

    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &pVertexBuffer->buffer, &pVertexBuffer->deviceMemory);

    copyBuffer(pVertexBuffer->buffer, stagingBuffer, bufferSize);

    vkDestroyBuffer(VE_G_Device, stagingBuffer, NULL);
    vkFreeMemory(VE_G_Device, stagingBufferMemory, NULL);

    pVertexBuffer->instanceCount = count;

    return pVertexBuffer;
}

VE_BufferT *VE_Render_CreateIndexBuffer(uint16_t *indices, uint32_t count) {
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    VkDeviceSize bufferSize = sizeof(indices[0]) * count;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);

    void* data;
    vkMapMemory(VE_G_Device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, indices, (size_t) bufferSize);
    vkUnmapMemory(VE_G_Device, stagingBufferMemory);

    VE_BufferT *pIndexBuffer = malloc(sizeof(VE_BufferT));

    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &pIndexBuffer->buffer, &pIndexBuffer->deviceMemory);

    copyBuffer(pIndexBuffer->buffer, stagingBuffer, bufferSize);

    vkDestroyBuffer(VE_G_Device, stagingBuffer, NULL);
    vkFreeMemory(VE_G_Device, stagingBufferMemory, NULL);

    pIndexBuffer->instanceCount = count;

    return pIndexBuffer;
}

void VE_Render_DestroyBuffer(VE_BufferT *pBuffer) {
    vkDeviceWaitIdle(VE_G_Device);

    vkDestroyBuffer(VE_G_Device, pBuffer->buffer, NULL);
    vkFreeMemory(VE_G_Device, pBuffer->deviceMemory, NULL);

    free(pBuffer);
}