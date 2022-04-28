#ifndef RENDER_SHADER_H
#define RENDER_SHADER_H

#include <stdint.h>

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>
typedef struct VE_ProgramT {
    VkPipeline pipeline;
    VkRenderPass renderPass;
    VkPipelineLayout layout;
    VkFramebuffer *pFramebuffers;
    const char *pVertexPath;
    const char *pFragmentPath;
} VE_ProgramT;

VE_ProgramT *VE_Render_CreateProgram(const char *pVertexPath, const char *pFragmentPath);
void VE_Render_DestroyProgram(VE_ProgramT *pProgram);
void VE_Render_DestroyAllPrograms(char freeMemory);
void VE_Render_RecreateAllPrograms();

#endif //RENDER_SHADER_H
