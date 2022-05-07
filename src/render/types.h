#ifndef RENDER_TYPES_H
#define RENDER_TYPES_H

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>
#include <cglm/types.h>

#define VE_RENDER_MAX_FRAMES_IN_FLIGHT 2
#define VE_RENDER_MAX_PROGRAMS 128

typedef struct SDL_Window SDL_Window;

typedef struct VE_ProgramT {
    VkPipeline pipeline;
    VkRenderPass renderPass;
    VkPipelineLayout layout;
    VkFramebuffer *pFramebuffers;
    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorPool descriptorPool;
    VkDescriptorSet pDescriptorSets[VE_RENDER_MAX_FRAMES_IN_FLIGHT];
    VkBuffer pUniformBuffer[VE_RENDER_MAX_FRAMES_IN_FLIGHT];
    VkDeviceMemory pUniformBufferMemory[VE_RENDER_MAX_FRAMES_IN_FLIGHT];
    const char *pVertexPath;
    const char *pFragmentPath;
} VE_ProgramT;

typedef struct VE_VertexT {
    vec3 pos;
    vec3 color;
    vec2 texCoord;
} VE_VertexT;

typedef struct VE_UniformBufferObjectT {
    mat4 model;
    mat4 view;
    mat4 projection;
} VE_UniformBufferObjectT;

typedef struct VE_BufferT {
    VkBuffer buffer;
    VkDeviceMemory deviceMemory;
    uint32_t instanceCount;
} VE_BufferT;

typedef struct VE_TextureT {
    VkImage image;
    VkDeviceMemory memory;
    VkImageView imageView;
    VkSampler sampler;
} VE_TextureT;

typedef struct VE_LoadTextureOptionsT {
    uint32_t e;
} VE_LoadTextureOptionsT;

typedef struct VE_ProgramSourceT {
    uint32_t *pVertexSource;
    uint32_t vertexLength;
    uint32_t *pFragmentSource;
    uint32_t fragmentLength;
} VE_ProgramSourceT;

#endif //RENDER_TYPES_H
