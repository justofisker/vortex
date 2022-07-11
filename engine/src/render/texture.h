#ifndef RENDER_TEXTURE_H
#define RENDER_TEXTURE_H

#include "types.h"

VE_TextureT *VE_Render_LoadTexture(const char *pTexturePath, VE_LoadTextureOptionsT *pOptions);
void VE_Render_DestroyTexture(VE_TextureT *pTexture);
void VE_Render_CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags memoryProperties, VkImage *pImage, VkDeviceMemory *pDeviceMemory);
VkImageView VE_Render_CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
void VE_Render_TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
void VE_Render_CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

#endif //RENDER_TEXTURE_H
