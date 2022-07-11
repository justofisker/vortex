#ifndef RENDER_UTIL_H
#define RENDER_UTIL_H

#include "types.h"

#ifndef min
#define min(x, y) (((x) < (y)) ? (x) : (y))
#endif // min
#ifndef max
#define max(x, y) (((x) > (y)) ? (x) : (y))
#endif //max

char *VE_Util_ReadFile(const char *path, uint32_t *size);
void VE_Render_CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer *pBuffer, VkDeviceMemory *pBufferMemory);
void VE_Render_CopyBuffer(VkBuffer dst, VkBuffer src, VkDeviceSize size);
uint32_t VE_Render_FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
VkCommandBuffer VE_Render_BeginSingleUseCommand();
void VE_Render_EndSingleUseCommand(VkCommandBuffer commandBuffer);

#endif //RENDER_UTIL_H
