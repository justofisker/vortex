#include "audio.h"
#include "vorbis/vorbisfile.h"

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

ALuint VE_Load_Ogg(const char *pFileName) {
	ALuint buffer;

	//FILE *fp = fopen(&pFileName, "rb");

	alGenBuffers(1, &buffer);
	OggVorbis_File vf;

	int error = 0;

	if ((error = ov_fopen(pFileName, &vf)) != 0) {
		printf("Failed to open OGG file. Error: %i\n", error);
	}

	/*if (ov_open_callbacks(fp, &vf, NULL, 0, OV_CALLBACKS_NOCLOSE) < 0) {
		printf("Attempted to load a non-vorbis OGG file.\n");
		alDeleteBuffers(1, &buffer);
		return 0;
	}*/

	vorbis_info *vi = ov_info(&vf, -1);

	ALenum format = vi->channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;

	size_t dataLen = ov_pcm_total(&vf, -1) * vi->channels * 2;
	short *pcmout = malloc(dataLen);
	if (!pcmout) {
		printf("Failed to allocate space for ogg buffer.\n");
		alDeleteBuffers(1, &buffer);
		return 0;
	}

	for (
		size_t size = 0, offset = 0, sel = 0;
		(size = ov_read(&vf, (char *)pcmout + offset, 4096, 0, 2, 1, (int *)&sel)) > 0;
		offset += size) {
		if (size < 0) {
			printf("Faulty ogg file.\n");
			alDeleteBuffers(1, &buffer);
			return 0;
		}
	}

	alBufferData(buffer, format, pcmout, dataLen, vi->rate);

	free(pcmout);
	ov_clear(&vf);
	//fclose(fp);

	return buffer;
}

ALuint VE_Audio_LoadSound(const char *pFileName) {
	size_t len = strlen(pFileName);
	const char *pFileExtension = pFileName + (len - 3);

	if (strcmp(pFileExtension, "ogg") == 0) {
		return VE_Load_Ogg(pFileName);
	}

	return 0;
}

void VE_Audio_DestroySound(ALuint sound) {
	alDeleteBuffers(1, &sound);
}

ALuint VE_Audio_CreateSource(ALuint sound) {
	ALuint source = 0;
	alGenSources(1, &source);
	alSourcei(source, AL_BUFFER, sound);
}

void VE_Audio_Play(ALuint source) {
}
