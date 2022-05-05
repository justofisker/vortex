#include "audio.h"

int VE_Audio_Init() {
	VE_G_pAudioDevice = alcOpenDevice(0);
	if (!VE_G_pAudioDevice) {
		printf("Failed to open audio device. Error: %s\n", alGetError());
	}

	VE_G_pAudioContext = alcCreateContext(VE_G_pAudioDevice, 0);
	if (!VE_G_pAudioContext) {
		printf("Failed to open audio device. Error: %s\n", alGetError());
	}

	alcMakeContextCurrent(VE_G_pAudioContext);

	return 0;
}

void VE_Audio_Destroy() {
	alcDestroyContext(VE_G_pAudioContext);

	alcCloseDevice(VE_G_pAudioDevice);
}
