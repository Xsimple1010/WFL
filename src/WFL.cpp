#include "SDL.h"
#include "Controller.hpp"
#include "LibretroClass.hpp"
#include "Audio.hpp"
#include "Video.hpp"
#include "threads/threadIoEvents.hpp"
#include "threads/threadGameLoop.hpp"
#include "WFL.h"
#include "gameLoop.hpp"
#include "stateNotifier.hpp"

static bool enableFullDeinit = true;
static bool singleThread = false;

static controller_internal_events controllerInternalEvents;
static controller_events controllerEvents;
static game_events gameEvents;
static libretro_external_data externalCoreData;
static core_event_functions coreEvents; 

static VideoClass video;
static AudioClass audio;
static StateNotifierClass statusClass;
static ControllerClass controller = ControllerClass(&controllerEvents, &controllerInternalEvents);
static Libretro libretro = Libretro(&coreEvents, &externalCoreData, &gameEvents, video.videoInfo);

//audio events
static void audioSample(int16_t left, int16_t right) {
	int16_t buffer[2] = { left, right };

	audio.write(buffer, 1);
}

static size_t audioSampleBatch(const int16_t* data, size_t frames) {
	return audio.write(data, frames);
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

static void initializeVariables() {
	coreEvents.setPixelFormat = setPixelFormat;
	coreEvents.refreshVertexData = refreshVertexData;
	coreEvents.resizeToAspect = resizeToAspect;
	coreEvents.videoRefresh = videoRefresh;
	coreEvents.audioSample = audioSample;
	coreEvents.audioSampleBatch = audioSampleBatch;
	coreEvents.inputPoll = inputPoll;
	coreEvents.inputState = inputState;

	externalCoreData.runLoopFrameTimeLast = 0;

	video.setToDefaultValues();
}
//===========================================

int WFlGetKeyDown() {
	return controller.getKeyDown(); 
}

//WFLAPI
void wflInit(bool isSingleThread, bool fullDeinit, wfl_events events, wfl_paths paths) {
	controllerEvents.onConnect = events.onConnect;
	controllerEvents.onDisconnect = events.onDisconnect;
	gameEvents.onGameClose = events.onGameClose;
	gameEvents.onGameStart = events.onGameStart;

	externalCoreData.paths = paths;

	controllerInternalEvents.onAppend = onDeviceAppend;

	statusClass.init(events.onStatusChange);
	statusClass.setRunning(true);
	singleThread = isSingleThread;
	enableFullDeinit = fullDeinit;

	initThreadIoEvents(&statusClass, &controller);
}

void wflLoadCore(const char* path) {
	if(libretro.coreIsLoaded) return;
	initializeVariables();
	libretro.coreLoad(path);
}

void wflStop() {
	statusClass.setPlaying(false);
	statusClass.setPaused(true);

	video.deinit();
	audio.deinit();
	libretro.deinit();

	externalCoreData = { 0 };
    coreEvents = { 0 };

	SDL_Quit();
}

void wflDeinit() {
	wflStop();

	controller.deinit();
	statusClass.setRunning(false);

    SDL_Quit();
}

void wflResume() {
	statusClass.setPaused(false);
}

void wflPause() {
	statusClass.setPaused(true);
}

void wflLoadGame(const char* path) {
	if(libretro.gameIsLoaded) return;
	
	statusClass.setPlaying(true);
	statusClass.setPaused(false);

	game_loop_params gameParams;

	gameParams.gamePath 		= path;
	gameParams.video 			= &video;
	gameParams.audio			= &audio;
	gameParams.status 			= &statusClass;
	gameParams.libretro 		= &libretro;
	gameParams.externalCoreData = &externalCoreData;

	if(singleThread) {
		gameLoop(gameParams);

		if(enableFullDeinit) {
			wflDeinit();
		} else {
			wflStop();
		}

	} else {
		thread_game_extra_data_deinit extraDataDeinit;

		extraDataDeinit.controller 	= &controller;
		extraDataDeinit.fullDeinit 	= &enableFullDeinit;

		initThreadGame(gameParams, extraDataDeinit);
	}
}

void wflSetController(controller_device device) {
	controller.append(device);
}

vector<wfl_joystick> wflGetConnectedJoysticks() {
	return controller.getConnectedJoysticks();
}