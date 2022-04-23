#ifndef RENDER_RENDER_H
#define RENDER_RENDER_H

typedef struct SDL_Window SDL_Window;

void VE_RenderInit(SDL_Window *window);
void VE_RenderDestroy();

#endif // RENDER_RENDER_H