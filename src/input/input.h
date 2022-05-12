#ifndef INPUT_INPUT_H
#define INPUT_INPUT_H

#include <SDL_events.h>
#include <cglm/types.h>

void VE_Input_Init();
void VE_Input_Event(SDL_Event *pEvent);
void VE_Input_EndFrame();

void VE_Input_GetMouseMotion(ivec2 mouseMotion);
char VE_Input_IsLeftMousePressed();
char VE_Input_IsRightMousePressed();
char VE_Input_IsLeftMouseJustPressed();
char VE_Input_IsRightMouseJustPressed();

char VE_Input_IsKeyPressed(SDL_Scancode scancode);
char VE_Input_IsKeyJustPressed(SDL_Scancode scancode);

#endif // INPUT_INPUT_H
