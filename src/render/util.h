#ifndef RENDER_UTIL_H
#define RENDER_UTIL_H

#include "types.h"

char *VE_Util_ReadFile(const char *path, uint32_t *size);
void VE_Render_CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer *pBuffer, VkDeviceMemory *pBufferMemory);
void VE_Render_CopyBuffer(VkBuffer dst, VkBuffer src, VkDeviceSize size);

#endif //RENDER_UTIL_H
