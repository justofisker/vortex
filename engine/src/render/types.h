#ifndef RENDER_TYPES_H
#define RENDER_TYPES_H

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>
#include <cglm/types.h>

#ifdef _MSC_VER
#define alignas(x) __declspec(align(x))
#else
#include <stdalign.h>
#endif

#define VE_RENDER_MAX_FRAMES_IN_FLIGHT 2
#define VE_RENDER_MAX_PROGRAMS 128

typedef struct SDL_Window SDL_Window;

typedef struct VE_ProgramT {
    VkPipeline pipeline;
    VkPipelineLayout layout;
    uint32_t descriptorSetLayoutCount;
    VkDescriptorSetLayout *pDescriptorSetLayouts;
    const char *pVertexPath;
    const char *pFragmentPath;
} VE_ProgramT;

typedef struct VE_VertexT {
    vec3 position;
    vec3 color;
    vec3 normal;
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

typedef struct VE_MeshObject_T {
    VE_BufferT *pVertexBuffer;
    VE_BufferT *pIndexBuffer;
    VE_ProgramT *pProgram;

    VkDescriptorPool descriptorPool;
    VkDescriptorSet pDescriptorSets[VE_RENDER_MAX_FRAMES_IN_FLIGHT];
    VkBuffer pUniformBuffer[VE_RENDER_MAX_FRAMES_IN_FLIGHT];
    VkDeviceMemory pUniformBufferMemory[VE_RENDER_MAX_FRAMES_IN_FLIGHT];
} VE_MeshObject_T;

typedef struct VE_ImportedModel_T {
    VE_MeshObject_T **meshes;
    uint32_t numMeshes;
    VE_TextureT **textures;
    uint32_t numTextures;
} VE_ImportedModel_T;

typedef struct VE_FragmentShaderPushConstants_t {
    vec3 sunDir;
    alignas(16) vec3 sunColor;
} VE_FragmentShaderPushConstants_t;

#endif //RENDER_TYPES_H
