#ifndef RENDER_RENDER_H
#define RENDER_RENDER_H

typedef struct SDL_Window SDL_Window;

void VE_Render_Init(SDL_Window *window);
void VE_Render_Destroy();
void VE_Render_CreateShader();

#endif // RENDER_RENDER_H