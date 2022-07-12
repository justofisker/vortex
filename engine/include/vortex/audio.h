#ifndef VORTEX_AUDIO_H
#define VORTEX_AUDIO_H
#include <vortex/types.h>

VE_Audio VE_Audio_LoadSound(const char *pFileName);
void VE_Audio_DestroySound(VE_Audio sound);

#endif // VORTEX_AUDIO_H
