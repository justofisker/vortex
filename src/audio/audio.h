#ifndef AUDIO_AUDIO_H
#define AUDIO_AUDIO_H
#include "types.h"

int VE_Audio_Init(int channels);
void VE_Audio_Destroy();

VE_MusicT VE_Audio_LoadMusic(const char *pFileName, int loops);
void VE_Audio_DestroyMusic(VE_MusicT music);

void VE_Audio_PlayMusic(VE_MusicT music);

#endif //AUDIO_AUDIO_H