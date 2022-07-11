#include "texture.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <stdio.h>
#include <volk.h>
#include "util.h"
#include "globals.h"
#include <string.h>

VE_TextureT *VE_Render_LoadTexture(const char *pTexturePath, VE_LoadTextureOptionsT *pOptions) {
    int texWidth, texHeight, texChannels;
    stbi_uc *pixels = stbi_load(pTexturePath, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;

    if (!pixels) {
        fprintf(stderr, "Failed to load image!\n");
        exit(-1);
    }

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    VE_Render_CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer,
                           &stagingBufferMemory);

    void *data;
    vkMapMemory(VE_G_Device, stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, (size_t) imageSize);
    vkUnmapMemory(VE_G_Device, stagingBufferMemory);

    stbi_image_free(pixels);

    VE_TextureT *pTexture = malloc(sizeof(VE_TextureT));

    VE_Render_CreateImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
                          VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                          VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &pTexture->image, &pTexture->memory);

    VE_Render_TransitionImageLayout(pTexture->image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED,
                                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    VE_Render_CopyBufferToImage(stagingBuffer, pTexture->image, (uint32_t) texWidth, (uint32_t) texHeight);
    VE_Render_TransitionImageLayout(pTexture->image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vkFreeMemory(VE_G_Device, stagingBufferMemory, NULL);
    vkDestroyBuffer(VE_G_Device, stagingBuffer, NULL);

    pTexture->imageView = VE_Render_CreateImageView(pTexture->image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);

    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(VE_G_PhysicalDevice, &properties);

    VkSamplerCreateInfo samplerCreateInfo = {VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
    samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
    samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
    samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.anisotropyEnable = VK_FALSE; // TODO
    samplerCreateInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    samplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
    samplerCreateInfo.compareEnable = VK_FALSE;
    samplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerCreateInfo.mipLodBias = 0.0f;
    samplerCreateInfo.minLod = 0.0f;
    samplerCreateInfo.maxLod = 0.0f;

    vkCreateSampler(VE_G_Device, &samplerCreateInfo, NULL, &pTexture->sampler);

    return pTexture;
}

void VE_Render_DestroyTexture(VE_TextureT *pTexture) {
    vkQueueWaitIdle(VE_G_GraphicsQueue);
    vkDestroySampler(VE_G_Device, pTexture->sampler, NULL);
    vkDestroyImageView(VE_G_Device, pTexture->imageView, NULL);
    vkFreeMemory(VE_G_Device, pTexture->memory, NULL);
    vkDestroyImage(VE_G_Device, pTexture->image, NULL);
}

void VE_Render_CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags memoryProperties, VkImage *pImage, VkDeviceMemory *pDeviceMemory) {
    VkImageCreateInfo imageInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.flags = 0;

    vkCreateImage(VE_G_Device, &imageInfo, NULL, pImage);

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(VE_G_Device, *pImage, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = VE_Render_FindMemoryType(memRequirements.memoryTypeBits,
                                                         memoryProperties);

    vkAllocateMemory(VE_G_Device, &allocInfo, NULL, pDeviceMemory);

    vkBindImageMemory(VE_G_Device, *pImage, *pDeviceMemory, 0);
}

VkImageView VE_Render_CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) {
    VkImageViewCreateInfo viewInfo = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;
    vkCreateImageView(VE_G_Device, &viewInfo, NULL, &imageView);

    return imageView;
}

static char VE_Render_HasStencilComponent(VkFormat format) {
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

void VE_Render_TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
    VkCommandBuffer commandBuffer = VE_Render_BeginSingleUseCommand();

    VkImageMemoryBarrier barrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

        if (VE_Render_HasStencilComponent(format)) {
            barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
    } else {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    switch (oldLayout) {
        case VK_IMAGE_LAYOUT_UNDEFINED:
            barrier.srcAccessMask = 0;
            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            break;
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            break;
        default:
            fprintf(stderr, "unsupported layout transition!");
            exit(-1);
    }

    switch (newLayout) {
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            break;
        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            break;
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            break;
        default:
            fprintf(stderr, "unsupported layout transition!");
            exit(-1);
    }

    vkCmdPipelineBarrier(
            commandBuffer,
            sourceStage, destinationStage,
            0,
            0, NULL,
            0, NULL,
            1, &barrier
    );

    VE_Render_EndSingleUseCommand(commandBuffer);
}

void VE_Render_CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
    VkCommandBuffer commandBuffer = VE_Render_BeginSingleUseCommand();

    VkBufferImageCopy region = { 0 };
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = (VkOffset3D){0, 0, 0};
    region.imageExtent = (VkExtent3D){
            width,
            height,
            1
    };

    vkCmdCopyBufferToImage(
            commandBuffer,
            buffer,
            image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &region
    );

    VE_Render_EndSingleUseCommand(commandBuffer);
}
