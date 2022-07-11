#ifndef INPUT_INPUT_H
#define INPUT_INPUT_H

#include <SDL_events.h>
#include <cglm/types.h>

void VE_Input_Init(SDL_Window *window);
void VE_Input_Event(SDL_Event *pEvent);
void VE_Input_EndFrame();

void VE_Input_GetMouseMotion(ivec2 mouseMotion);
void VE_Input_GetMousePosition(ivec2 mousePosition);
void VE_Input_SetMousePosition(int x, int y);
char VE_Input_IsLeftMousePressed();
char VE_Input_IsRightMousePressed();
char VE_Input_IsLeftMouseJustPressed();
char VE_Input_IsRightMouseJustPressed();

char VE_Input_IsKeyPressed(SDL_Scancode scancode);
char VE_Input_IsKeyJustPressed(SDL_Scancode scancode);

typedef enum VE_MouseMode {
	VE_MOUSEMODE_NORMAL,
	VE_MOUSEMODE_LOCKED,
	VE_MOUSEMODE_RELATIVE,
	VE_MOUSEMODE_MAX
} VE_MouseMode;

void VE_Input_SetMouseMode(VE_MouseMode mouseMode);
VE_MouseMode VE_Input_GetMouseMode();

#endif // INPUT_INPUT_H
