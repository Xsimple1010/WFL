#include <SDL.h>
#include <iostream>

#include "Controller.hpp"
#include "LibretroClass.hpp"
#include "debug.hpp"
#include "CpuFeatures.hpp"
#include "Audio.hpp"
#include "Video.hpp"
#include "threads/threadIoEvents.hpp"
#include "threads/threadGameLoop.hpp"
#include "WFL.h"
#include "gameLoop.hpp"
#include <thread>

static bool running = true;
static SDL_Event event;
static bool singleThread = false;

static controller_internal_events controllerInternalEvents;
static controller_events controllerEvents;
static libretro_external_data externalCoreData;
static core_event_functions eventFunction; 
static video_info videoInfo;

static Libretro libretro = Libretro(&eventFunction, &externalCoreData, &videoInfo);
static ControllerClass controller = ControllerClass(&controllerEvents, &controllerInternalEvents);
static VideoClass video;
static AudioClass audio;

//audio events
static void audioSample(int16_t left, int16_t right) {
	int16_t buffer[2] = { left, right };

	audio.audioWrite(buffer, 1);
}

static size_t audioSampleBatch(const int16_t* data, size_t frames) {
	return audio.audioWrite(data, frames);
}

//controller events
static void inputPoll() {
	controller.inputPoll();
}

static int16_t inputState(unsigned port, unsigned device, unsigned index, unsigned id) {
	return controller.inputState(port, device, index, id);
}

static void onDeviceAppend(controller_device device) {
	if(libretro.coreIsLoaded) {
		libretro.setControllerPortDevice(device.port, device.type);
	}
}

//video events
static bool setPixelFormat(unsigned format) {
	return video.setPixelFormat(format);
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

	externalCoreData.runLoopFrameTimeLast = 0;

	videoInfo.window = NULL;
	videoInfo.gVideo = {0};
	videoInfo.gScale = 2;
	videoInfo.gVideo.hw.version_major = 4;
	videoInfo.gVideo.hw.version_minor = 5;
	videoInfo.gVideo.hw.context_type = RETRO_HW_CONTEXT_OPENGL_CORE;
	videoInfo.gVideo.hw.context_reset = noop;
	videoInfo.gVideo.hw.context_destroy = noop;
}
//===========================================


//WFLAPI
void wflInit(bool isSingleThread, controller_events events, wfl_paths paths) {
	controllerEvents.onConnect = events.onConnect;
	controllerEvents.onDisconnect = events.onDisconnect;

	controllerInternalEvents.onAppend = onDeviceAppend;

	externalCoreData.paths.save = paths.save;
	externalCoreData.paths.system = paths.system;

	running = true;
	singleThread = isSingleThread;

	initThreadIoEvents(&running, &controller);
}

void wflLoadCore(const char* path) {
	if(libretro.coreIsLoaded) return;
	initializeVariables();
	libretro.coreLoad(path);
}

void wflStop() {
	running = false;
	deinitThreadIoEvents();
}

void wflDeinit() {
	running = false;

	video.deinit();
	audio.audioDeinit();
	controller.deinit();
	libretro.deinit();

	externalCoreData = { 0 };
    eventFunction = { 0 };
    SDL_Quit();
}

void wflLoadGame(const char* path) {
	if(libretro.gameIsLoaded) return;

	game_loop_params gameParams;

	gameParams.gamePath = path;
	gameParams.running = &running;
	gameParams.audio = &audio;
	gameParams.videoInfo = &videoInfo;
	gameParams.video = &video;
	gameParams.libretro = &libretro;
	gameParams.externalCoreData = &externalCoreData;


	if(singleThread) {
		gameLoop(gameParams);
		wflDeinit();
	} else {
		initThreadGame(gameParams);
	}
}

void wflSetController(controller_device device) {
	controller.append(device);
}

vector<wfl_joystick> wflGetConnectedJoysticks() {
	return controller.getConnectedJoysticks();
}