#include "audio.h"

int VE_Audio_Init(int channels) {
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, channels, 2048) < 0) {
		printf("Unable to initialize SDL_mixer. Error: %s\n", Mix_GetError());
		return -1;
	}
	int initFlags = MIX_INIT_OGG;
	if (Mix_Init(initFlags) != initFlags) {
		printf("Unable to initialize SDL_mixer. Error: %s\n", Mix_GetError());
	}
}

void VE_Audio_Destroy() {
	Mix_Quit();
}

VE_MusicT VE_Audio_LoadMusic(const char *pFileName, int loops) {
	Mix_Music *music = Mix_LoadMUS(pFileName);
	if (!music) {
		printf("Failed to load music file %s. Error: %s.\n", pFileName, Mix_GetError());
	}
	return (VE_MusicT) { music, loops };
}

void VE_Audio_DestroyMusic(VE_MusicT music) {
	Mix_FreeMusic(music.music);
}

void VE_Audio_PlayMusic(VE_MusicT music) {
	if (Mix_PlayingMusic()) {
		Mix_HaltMusic();
	}
	if (Mix_PlayMusic(music.music, music.loops) != 0) {
		printf("Failed to play music. Error: %s\n", Mix_GetError());
	}
}
