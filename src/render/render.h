#ifndef RENDER_RENDER_H
#define RENDER_RENDER_H

#include "types.h"

void VE_Render_Init(SDL_Window *window);
void VE_Render_Destroy();

void VE_Render_BeginFrame();
void VE_Render_EndFrame();
void VE_Render_Draw(VE_ProgramT *pProgram, VE_BufferT *pVertexBuffer, VE_BufferT *pIndexBuffer);
VE_BufferT *VE_Render_CreateVertexBuffer(VE_VertexT *vertices, uint32_t count);
VE_BufferT *VE_Render_CreateIndexBuffer(uint16_t *indices, uint32_t count);
void VE_Render_DestroyBuffer(VE_BufferT *pBuffer);

void VE_Render_Resize();

#endif // RENDER_RENDER_H
