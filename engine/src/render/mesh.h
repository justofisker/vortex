#ifndef RENDER_MESH
#define RENDER_MESH

#include "types.h"

VE_MeshObject_T *VE_Render_CreateCylinderMesh(int vertex_count, float radius, float depth, VE_ProgramT *pProgram);
VE_MeshObject_T *VE_Render_CreatePlaneMesh(float width, float height, VE_ProgramT *pProgram);
VE_MeshObject_T *VE_Render_CreateCubeMesh(float width, float height, float depth, VE_ProgramT *pProgram);
VE_MeshObject_T *VE_Render_CreateUVSphereMesh(float radius, uint32_t rings, uint32_t sectors, VE_ProgramT *pProgram);

VE_ImportedModel_T VE_Render_ImportModel(const char *pFilePath, VE_ProgramT *pProgram);
void VE_Render_DestroyImportedModel(VE_ImportedModel_T model);

VE_MeshObject_T *VE_Render_CreateMeshObject(VE_VertexT *vertices, uint32_t vertexCount, uint16_t *indices, uint32_t indexCount, VE_ProgramT *pProgram);
void VE_Render_SetMeshObjectTexture(VE_MeshObject_T *pMeshObject, VE_TextureT *pTexture);
void VE_Render_UpdateMeshUniformBuffer(VE_MeshObject_T *pMeshObject, mat4 modelMatrix);
void VE_Render_DestroyMeshObject(VE_MeshObject_T *pMeshObject);

VE_BufferT *VE_Render_CreateVertexBuffer(VE_VertexT *vertices, uint32_t count);
VE_BufferT *VE_Render_CreateIndexBuffer(uint16_t *indices, uint32_t count);
void VE_Render_DestroyBuffer(VE_BufferT *pBuffer);

#endif // RENDER_MESH