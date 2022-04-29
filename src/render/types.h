#ifndef RENDER_TYPES_H
#define RENDER_TYPES_H

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>
#include <cglm/types.h>

typedef struct SDL_Window SDL_Window;

typedef struct VE_ProgramT {
    VkPipeline pipeline;
    VkRenderPass renderPass;
    VkPipelineLayout layout;
    VkFramebuffer *pFramebuffers;
    const char *pVertexPath;
    const char *pFragmentPath;
} VE_ProgramT;

typedef struct VE_VertexT {
    vec2 pos;
    vec3 color;
} VE_VertexT;

typedef struct VE_BufferT {
    VkBuffer buffer;
    VkDeviceMemory deviceMemory;
    uint32_t instanceCount;
} VE_BufferT;

#endif //RENDER_TYPES_H
