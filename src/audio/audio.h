#ifndef AUDIO_AUDIO_H
#define AUDIO_AUDIO_H
#include "globals.h"

int VE_Audio_Init();
void VE_Audio_Destroy();

ALuint VE_Audio_LoadSound(const char *pFileName);
void VE_Audio_DestroySound(ALuint sound);

ALuint VE_Audio_CreateSource(ALuint sound);

void VE_Audio_Play(ALuint sound);

#endif //AUDIO_AUDIO_H