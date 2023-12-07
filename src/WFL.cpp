#include "SDL2/SDL.h"
#include <iostream>

#include "Controller.hpp"
#include "LibretroClass.hpp"
#include "debug.hpp"
#include "CpuFeatures.hpp"
#include "Audio.hpp"
#include "Video.hpp"
#include "WFL.h"

bool running = true;
SDL_Event event;

static controller_events controllerEvents;
static libretro_external_data externalCoreData;
static core_event_functions eventFunction; 

static Libretro libretro = Libretro(&eventFunction, &externalCoreData);
static ControllerClass controller = ControllerClass(&controllerEvents);
static VideoClass video;

//audio
static void audioSample(int16_t left, int16_t right) {
	int16_t buffer[2] = { left, right };

	audioWrite(buffer, 1);
}

static size_t audioSampleBatch(const int16_t* data, size_t frames) {
	return audioWrite(data, frames);
	return frames;
}

//inputs
void setController(){
	
}

static void inputPoll() {
	controller.inputPoll();
}

static int16_t inputState(unsigned port, unsigned device, unsigned index, unsigned id) {
	return controller.inputState(port, device, index, id);
	return 1;
}

//video
static void videoInit(retro_game_geometry *geometry) {
	std::cout << geometry->max_height << std::endl;

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


//initialization variables
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
	externalCoreData.gScale = 1;
	externalCoreData.audioCallback;
	externalCoreData.runLoopFrameTime;
	externalCoreData.runLoopFrameTimeLast = 0;

	externalCoreData.gVideo.hw.version_major = 4;
	externalCoreData.gVideo.hw.version_minor = 5;
	externalCoreData.gVideo.hw.context_type = RETRO_HW_CONTEXT_OPENGL_CORE;
	externalCoreData.gVideo.hw.context_reset = noop;
	externalCoreData.gVideo.hw.context_destroy = noop;

	//controllerEvents.onConnect = onConnect;
	//controllerEvents.onDisconnect = onDisconnect;
}
//===========================================


//WFLAPI
void WFLAPI wflSetCallbacks(controller_events events) {
	controllerEvents.onConnect = events.onConnect;
	controllerEvents.onDisconnect = events.onDisconnect;
}

void WFLAPI wflInit() {
	initializeVariables();
}

void WFLAPI wflLoadCore(const char* path) {
	libretro.coreLoad(path);
}

void WFLAPI wflLoadGame(const char* path) {
	retro_system_av_info avInfo = libretro.loadGame(path);

	videoInit(&avInfo.geometry);
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS | SDL_INIT_GAMECONTROLLER) < 0) {
		die("SDL could not initialize! SDL_Error: ", SDL_GetError());
    }
	audioInit(avInfo.timing.sample_rate);

	while (running) {

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

        while (SDL_PollEvent(&event)) {
            switch (event.type) {
               case SDL_QUIT: {
					running = false; break;
					break;
				}
			
				case SDL_WINDOWEVENT:
				{
					switch (event.window.event) {

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

				case SDL_CONTROLLERDEVICEADDED: {
					controller.onConnect(event.cdevice.which);
					break;
				}

				case SDL_CONTROLLERDEVICEREMOVED: {
					controller.onDisconnect(event.cdevice.which);
					break;
				}
            }
        }

		libretro.run();
	}
}

void WFLAPI wflUnloadGame() {
	// libretro.

	audioDeinit();
	video.deinit();
}

void WFLAPI wflSetController(controller_device device) {
	controller.append(device);
}

void WFLAPI wflDeinit() {
	controller.deinit();
    SDL_Quit();
}

int main(int argc, char* argv[]) {
    return 0;
}
