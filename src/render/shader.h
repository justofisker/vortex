#ifndef RENDER_SHADER_H
#define RENDER_SHADER_H

#include "types.h"

VE_ProgramT *VE_Render_CreateProgram(const char *pVertexPath, const char *pFragmentPath);
void VE_Render_DestroyProgram(VE_ProgramT *pProgram);
void VE_Render_DestroyAllPrograms(char freeMemory);
void VE_Render_RecreateAllPrograms();
void VE_Render_CreateProgramLayout(VE_ProgramT *Program, VE_ProgramSourceT *pProgramSource);
void VE_Render_CreateGraphicsPipeline(VE_ProgramT *pProgram, VE_ProgramSourceT *pProgramSource);

// Temporary to be removed
VE_MeshObject_T *VE_Render_CreateMeshObject(VE_VertexT *vertices, uint32_t vertexCount, uint16_t *indices, uint32_t indexCount, VE_ProgramT *pProgram);
void VE_Render_SetMeshObjectTexture(VE_MeshObject_T *pMeshObject, VE_TextureT *pTexture);
void VE_Render_UpdateMeshUniformBuffer(VE_MeshObject_T *pMeshObject);
void VE_Render_DestroyMeshObject(VE_MeshObject_T *pMeshObject);

#endif //RENDER_SHADER_H
