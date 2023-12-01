#include <SDL2/SDL.h>
#include <glad/glad.h>
#include <iostream>

//WFL includes
#include "LibretroClass.hpp"
#include "debug.hpp"
#include "CpuFeatures.hpp"
#include "Audio.hpp"
#include "Video.hpp"


bool running = true;

static VideoClass video;
static core_event_functions eventFunction;
static libretro_external_data externalCoreData;

//audio
static void audioSample(int16_t left, int16_t right) {
	std::cout << "audioSample" << std::endl;
	int16_t buffer[2] = { left, right };

	audioWrite(buffer, 1);
}

static size_t audioSampleBatch(const int16_t* data, size_t frames) {
	std::cout << "audioSampleBatch" << std::endl;
	return audioWrite(data, frames);
	return frames;
}

//keyboard end other inputs
static void inputPoll() {
	//keyboard.inputPoll();
}

static int16_t inputState(unsigned port, unsigned device, unsigned index, unsigned id) {
	//return keyboard.inputState(port, device, index, id);
	return 0;
}

//video
static void videoInit(retro_game_geometry *geometry) {
	video.init(&externalCoreData, geometry);
};

static bool setPixelFormat(unsigned format) {
	return video.setPixelFormat(format, &externalCoreData);
	return true;
}

static void refreshVertexData() {
	video.refreshVertexData();
}

static void resizeToAspect(double ratio, int sw, int sh, int* dw, int* dh) {
	video.resizeToAspect(ratio, sw, sh, dw, dh);
}

static void videoRefresh(const void* data, unsigned width, unsigned height, size_t pitch) {
	video.videoRefresh(data, width, height, pitch);
}

static void noop() {}

static void initializeVariables() {
	eventFunction.setPixelFormat = setPixelFormat;
	eventFunction.refreshVertexData = refreshVertexData;
	eventFunction.resizeToAspect = resizeToAspect;
	eventFunction.videoRefresh = videoRefresh;
	eventFunction.audioSample = audioSample;
	eventFunction.audioSampleBatch = audioSampleBatch;
	eventFunction.inputPoll = inputPoll;
	eventFunction.inputState = inputState;

	externalCoreData.window = NULL;
	externalCoreData.gVideo = {0};
	externalCoreData.gScale = 2;
	externalCoreData.audioCallback;
	externalCoreData.runLoopFrameTime;
	externalCoreData.runLoopFrameTimeLast = 0;

	externalCoreData.gVideo.hw.version_major = 4;
	externalCoreData.gVideo.hw.version_minor = 5;
	externalCoreData.gVideo.hw.context_type = RETRO_HW_CONTEXT_OPENGL_CORE;
	externalCoreData.gVideo.hw.context_reset = noop;
	externalCoreData.gVideo.hw.context_destroy = noop;
}

void initSDL() {
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS) < 0) {
		die("This core requires a game in order to run");
	}
}

int main(int argc, char* argv[]) {

	initializeVariables();
	Libretro libretro = Libretro(&eventFunction, &externalCoreData);

	

	libretro.coreLoad("C:/RetroArch-Win64/cores/bsnes_libretro.dll");

	//if (!retroFunctions.supports_no_game)
		//die("This core requires a game in order to run");

	retro_system_av_info avInfo = libretro.loadGame("C:\\RetroArch-Win64\\roms\\Mega Man X (USA).sfc");
	
	//try initialize audio end video
	videoInit(&avInfo.geometry);
	initSDL();
	audioInit(avInfo.timing.sample_rate);
	//===

	if (externalCoreData.audioCallback.set_state) {
		externalCoreData.audioCallback.set_state(true);
	}


	SDL_Event sdlEvent;
	
	//Super Nintendo e ps1
	libretro.setControllerPortDevice(0, RETRO_DEVICE_JOYPAD);

	//game loop
	while (running)
	{
		if (externalCoreData.runLoopFrameTime.callback) {
			retro_time_t current = cpuFeaturesGetTimeUsec();
			retro_time_t delta = current - externalCoreData.runLoopFrameTimeLast;

			if (!externalCoreData.runLoopFrameTimeLast) {
				delta = externalCoreData.runLoopFrameTime.reference;
			}

			externalCoreData.runLoopFrameTimeLast = current;
			externalCoreData.runLoopFrameTime.callback(delta);
		}


		if (externalCoreData.audioCallback.callback) {
			externalCoreData.audioCallback.callback();
		}

		//add sdl events
		while (SDL_PollEvent(&sdlEvent)) {
			switch (sdlEvent.type) {
				case SDL_QUIT: {
					running = false; break;
				}
			
				case SDL_WINDOWEVENT:
				{
					switch (sdlEvent.window.event) {

						case SDL_WINDOWEVENT_CLOSE: 
						{
							running = false;
							break;
						}

						//case SDL_WINDOWEVENT_RESIZED:
							//resize_cb(ev.window.data1, ev.window.data2);
							//break;
					}
				}
			}
		}
		//===
//		glBindFramebuffers(GL_FRAMEBUFFER, 0);
		libretro.run();
	}


	//deinit components
//	audioDeinit();
//	video.deinit();
	SDL_Quit();

	return 55;
}