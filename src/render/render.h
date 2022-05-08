#ifndef RENDER_RENDER_H
#define RENDER_RENDER_H

#include "types.h"

void VE_Render_Init(SDL_Window *window);
void VE_Render_Destroy();

void VE_Render_BeginFrame();
void VE_Render_EndFrame();
void VE_Render_Draw(VE_MeshObject_T *pMeshObject);

void VE_Render_Resize();

#endif // RENDER_RENDER_H
