#ifndef AUDIO_TYPES_H
#define AUDIO_TYPES_H
#include "SDL_mixer.h"

typedef struct VE_MusicT {
	Mix_Music *music;
	int loops;
} VE_MusicT;

#endif //AUDIO_TYPES_H