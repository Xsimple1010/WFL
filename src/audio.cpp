#include "Audio.hpp"

static SDL_AudioDeviceID  audioDeviceID;

bool AudioClass::init(int frequency) {
	SDL_AudioSpec desired;
	SDL_AudioSpec obtained;

	SDL_zero(desired);
	SDL_zero(obtained);

	desired.format = AUDIO_S16;
	desired.freq = frequency;
	desired.channels = 2;
	desired.samples = 4096;

	audioDeviceID = SDL_OpenAudioDevice(NULL, 0, &desired, &obtained, 0);

	if(!audioDeviceID)
	{
		die("Failed to open playback device: %s", SDL_GetError());
	}

	SDL_PauseAudioDevice(audioDeviceID, 0);

	return true;
}

void AudioClass::deinit() {
	SDL_CloseAudioDevice(audioDeviceID);
}

size_t AudioClass::write(const int16_t *buffer, size_t frames) {
	int channels = 2;
	SDL_QueueAudio(audioDeviceID, buffer, sizeof(*buffer) * frames * channels);

	return frames;
}