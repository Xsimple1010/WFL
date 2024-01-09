#include "SDL.h"
#include "gamePad.hpp"
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

static game_events gameEvents;
static libretro_external_data externalCoreData;
static core_event_functions coreEvents; 

static VideoClass videoClass;
static AudioClass audioClass;
static StateNotifierClass stateClass;
static GamePadClass gamePadClass = GamePadClass(&stateClass);
static Libretro libretro = Libretro(&coreEvents, &externalCoreData, &gameEvents);

//audio events
static void audioSample(int16_t left, int16_t right) {
	int16_t buffer[2] = { left, right };

	audioClass.write(buffer, 1);
}

static size_t audioSampleBatch(const int16_t* data, size_t frames) {
	return audioClass.write(data, frames);
}

//gamePad events
static void onDeviceAppend(wfl_game_pad device) {
	if(libretro.coreIsLoaded) {
		libretro.setControllerPortDevice(device.port, device.type);
	}
}

//video events
static bool setPixelFormat(unsigned format) {
	return videoClass.setPixelFormat(format);
}

static void refreshVertexData() {
	videoClass.refreshVertexData();
}

static void resizeToAspect(double ratio, int sw, int sh, int* dw, int* dh) {
	videoClass.resizeToAspect(ratio, sw, sh, dw, dh);
}

static void videoRefresh(const void* data, unsigned width, unsigned height, size_t pitch) {
	videoClass.videoRefresh(data, width, height, pitch);
}


//initialization variables
static void initializeVariables() {
	coreEvents.setPixelFormat = setPixelFormat;
	coreEvents.refreshVertexData = refreshVertexData;
	coreEvents.resizeToAspect = resizeToAspect;
	coreEvents.videoRefresh = videoRefresh;
	coreEvents.audioSample = audioSample;
	coreEvents.audioSampleBatch = audioSampleBatch;
	coreEvents.inputPoll = gamePadClass.inputPollCb;
	coreEvents.inputState = gamePadClass.inputStateCb;

	externalCoreData.runLoopFrameTimeLast = 0;

	videoClass.setToDefaultValues();
}
//===========================================


//WFLAPI
void wflInit(bool isSingleThread, bool fullDeinit, wfl_events events, wfl_paths paths) {
	if (SDL_Init(SDL_INIT_EVENTS | SDL_INIT_GAMECONTROLLER | SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
		die("SDL could not initialize! SDL_Error: ", SDL_GetError());
    }

	gamePadClass.onConnectCb 		= events.onConnect;
	gamePadClass.onDisconnectCb 	= events.onDisconnect;
	gamePadClass.onDeviceAppendCb 	= onDeviceAppend;

	gameEvents.onGameClose 			= events.onGameClose;
	gameEvents.onGameStart 			= events.onGameStart;

	externalCoreData.paths 			= paths;

	singleThread = isSingleThread;
	enableFullDeinit = fullDeinit;

	stateClass.init(events.onStatusChange);
	stateClass.setRunning(true);
	
	initThreadIoEvents(&stateClass, &gamePadClass);
}

void wflLoadCore(const char* path) {
	if(libretro.coreIsLoaded) return;
	initializeVariables();
	libretro.coreLoad(path);
}

void wflStop() {
	stateClass.setPlaying(false);
	stateClass.setPaused(true);

	externalCoreData = { 0 };
    coreEvents = { 0 };
}

void wflDeinit() {
	wflStop();

	gamePadClass.deinit();
	stateClass.setRunning(false);

    SDL_Quit();
}

void wflResume() {
	stateClass.setPaused(false);
}

void wflPause() {
	stateClass.setPaused(true);
}

void wflReset() {
	libretro.reset();
}

void wflLoadGame(const char* path) {
	if(libretro.gameIsLoaded) return;
	
	stateClass.setPlaying(true);
	stateClass.setPaused(false);

	game_loop_params gameParams;

	gameParams.gamePath 		= path;
	gameParams.video 			= &videoClass;
	gameParams.audio			= &audioClass;
	gameParams.status 			= &stateClass;
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

		extraDataDeinit.gamePadClass 	= &gamePadClass;
		extraDataDeinit.fullDeinit 		= &enableFullDeinit;

		initThreadGame(gameParams, extraDataDeinit);
	}
}

void wflSetGamePad(wfl_game_pad device) {
	gamePadClass.append(device);
}

vector<wfl_game_pad> wflGetGamePad() {
	return gamePadClass.getConnected();
}

vector<wfl_device> wflGetAllGamePads() {
	return gamePadClass.getAll();
}

int WFlGetKeyDown() {
	return gamePadClass.getKeyDown(); 
}

bool wflSave() {
	return libretro.save();
}

bool wflLoadSave() {
	return libretro.loadSave();
}