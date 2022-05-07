#ifndef RENDER_SHADER_H
#define RENDER_SHADER_H

#include "types.h"

VE_ProgramT *VE_Render_CreateProgram(const char *pVertexPath, const char *pFragmentPath);
void VE_Render_DestroyProgram(VE_ProgramT *pProgram);
void VE_Render_DestroyAllPrograms(char freeMemory);
void VE_Render_RecreateAllPrograms();
void VE_Render_UpdateUniformBuffer(VE_ProgramT *pProgram);
void VE_Render_SetProgramSampler(VE_ProgramT *pProgram, VE_TextureT *pTexture);
void VE_Render_CreateProgramLayout(VE_ProgramT *Program, VE_ProgramSourceT *pProgramSource);
void VE_Render_CreateGraphicsPipeline(VE_ProgramT *pProgram, VE_ProgramSourceT *pProgramSource);
VkRenderPass VE_Render_CreateRenderpass();

#endif //RENDER_SHADER_H
