#ifndef RENDER_TEXTURE_H
#define RENDER_TEXTURE_H

#include "types.h"

VE_TextureT *VE_Render_LoadTexture(const char *pTexturePath, VE_LoadTextureOptionsT *pOptions);
void VE_Render_TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
void VE_Render_CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
void VE_Render_DestroyTexture(VE_TextureT *pTexture);

#endif //RENDER_TEXTURE_H
