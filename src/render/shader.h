#ifndef RENDER_SHADER_H
#define RENDER_SHADER_H

#include <stdint.h>

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>
typedef struct VE_Shader {
    VkPipeline pipeline;
    VkRenderPass renderPass;
    VkPipelineLayout layout;
    VkFramebuffer *pFramebuffers;
} VE_Shader;

VE_Shader *VE_Render_CreateShader(const char *pVertexPath, const char *pFragmentPath);
void VE_Render_DestroyShader(VE_Shader *pShader);
void VE_Render_DestroyShaders(VE_Shader **ppShaders, uint32_t count);

#endif //RENDER_SHADER_H
