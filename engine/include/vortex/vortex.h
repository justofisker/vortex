#ifndef VORTEX_H
#define VORTEX_H

#include "types.h"

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include "../../src/render/render.h"
#include "../../src/render/shader.h"
#include "../../src/render/texture.h"
#include "../../src/render/mesh.h"
#include "../../src/input/input.h"

#include "ecs.h"
#include "components.h"
#include "audio.h"

void VE_Init(const char *title);
void VE_Run();
void VE_Destroy();

#endif // VORTEX_H
