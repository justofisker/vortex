#ifndef AUDIO_AUDIO_H
#define AUDIO_AUDIO_H

#include <cglm/types.h>
#include <AL/al.h>
#include <vortex/types.h>

int VE_Audio_Init();
void VE_Audio_Destroy();

ALuint VE_Audio_CreateSource(ALuint sound);
void VE_Audio_SetSourcePosition(ALuint source, vec3 position);
void VE_Audio_SetSourceVelocity(ALuint source, vec3 velocity);
void VE_Audio_SetSourceRelative(ALuint source, int relative);
void VE_Audio_DestroySource(ALuint source);

void VE_Audio_Play(ALuint sound);

void VE_Audio_SetListenerPosition(vec3 position);
void VE_Audio_SetListenerOrientation(vec3 forward, vec3 up);

#endif //AUDIO_AUDIO_H