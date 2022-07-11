#define SDL_MAIN_HANDLED
#include <SDL.h>
#include "render/render.h"
#include "render/shader.h"
#include "render/texture.h"
#include "render/mesh.h"
#include "ecs/ecs.h"
#include "ecs/builtin.h"
#include "audio/audio.h"
#include "input/input.h"

void VE_Init();

void VE_Run();

void VE_Destroy();