#include "input.h"
#include <SDL.h>
#include <cglm/cglm.h>
#include <string.h>
#include <assert.h>

static ivec2 mouseMotion = {0, 0};
static char isMousePressed = 0;
static uint32_t mouseWheelScroll = 0;

static uint8_t mouseButtons = 0;
static uint8_t justPressedMouseButtons = 0;

static uint64_t justPressedKeys[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
static uint64_t keys[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

void VE_Input_Init() {

}

void VE_Input_Event(SDL_Event *pEvent) {
    switch (pEvent->type) {
        case SDL_MOUSEMOTION:
            mouseMotion[0] += pEvent->motion.xrel;
            mouseMotion[1] += pEvent->motion.yrel;
            break;
        case SDL_MOUSEBUTTONDOWN:
            mouseButtons |= SDL_BUTTON(pEvent->button.button);
            justPressedMouseButtons |= SDL_BUTTON(pEvent->button.button);
            break;
        case SDL_MOUSEBUTTONUP:
            mouseButtons &= ~SDL_BUTTON(pEvent->button.button);
            break;
        case SDL_MOUSEWHEEL:
            mouseWheelScroll += pEvent->wheel.direction;
            break;
        case SDL_KEYDOWN:
            if (pEvent->key.repeat) break;
            keys[pEvent->key.keysym.scancode / 64] |= (1ull << (pEvent->key.keysym.scancode % 64));
            justPressedKeys[pEvent->key.keysym.scancode / 64] |= (1ull << (pEvent->key.keysym.scancode % 64));
            break;
        case SDL_KEYUP:
            keys[pEvent->key.keysym.scancode / 64] &= ~(1ull << (pEvent->key.keysym.scancode % 64));
            break;
    }
}

void VE_Input_EndFrame() {
    memset(justPressedKeys, 0, sizeof(justPressedKeys));
    justPressedMouseButtons = 0;
    mouseMotion[0] = 0;
    mouseMotion[1] = 0;
}

void VE_Input_GetMouseMotion(ivec2 _mouseMotion) {
    _mouseMotion[0] = mouseMotion[0];
    _mouseMotion[1] = mouseMotion[1];
}

char VE_Input_IsLeftMousePressed() {
    return !!(mouseButtons & SDL_BUTTON_LMASK);
    
}

char VE_Input_IsRightMousePressed() {
    return !!(mouseButtons & SDL_BUTTON_RMASK);
}

char VE_Input_IsLeftMouseJustPressed() {
    return !!(justPressedMouseButtons & SDL_BUTTON_LMASK);
}

char VE_Input_IsRightMouseJustPressed() {
    return !!(justPressedMouseButtons & SDL_BUTTON_RMASK);
}

char VE_Input_IsKeyPressed(SDL_Scancode scancode) {
    assert(scancode < SDL_NUM_SCANCODES);
    return !!(keys[scancode / 64] & (1ull << (scancode % 64)));
}

char VE_Input_IsKeyJustPressed(SDL_Scancode scancode) {
    assert(scancode < SDL_NUM_SCANCODES);
    return !!(justPressedKeys[scancode / 64] & (1ull << (scancode % 64)));
}
