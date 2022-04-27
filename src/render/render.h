#ifndef RENDER_RENDER_H
#define RENDER_RENDER_H

typedef struct SDL_Window SDL_Window;
typedef struct VE_Shader VE_Shader;

void VE_Render_Init(SDL_Window *window);
void VE_Render_Destroy();

void VE_Render_BeginFrame();
void VE_Render_EndFrame();
void VE_Render_Draw(VE_Shader *pShader);

#endif // RENDER_RENDER_H
