#include <SDL.h>
#include <iostream>

#include "Controller.hpp"
#include "LibretroClass.hpp"
#include "debug.hpp"
#include "CpuFeatures.hpp"
#include "Audio.hpp"
#include "Video.hpp"

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
static void setController(){
	vector<Joystick> joysticks = controller.getConnectedJoysticks();

	controller_device deviceGamePad = {
		.index = 0,
		.port = 0,
		.type = RETRO_DEVICE_JOYPAD,
		.nativeInfo = {
			.type = WFL_DEVICE_JOYSTICK,
		},
		.keyboardKeyBinds = {0},
		.joystickKeyBinds = {
			{ SDL_CONTROLLER_BUTTON_A, RETRO_DEVICE_ID_JOYPAD_A },
			{ SDL_CONTROLLER_BUTTON_B, RETRO_DEVICE_ID_JOYPAD_B },
			{ SDL_CONTROLLER_BUTTON_X, RETRO_DEVICE_ID_JOYPAD_Y },
			{ SDL_CONTROLLER_BUTTON_Y, RETRO_DEVICE_ID_JOYPAD_X },
			{ SDL_CONTROLLER_BUTTON_DPAD_UP, RETRO_DEVICE_ID_JOYPAD_UP },
			{ SDL_CONTROLLER_BUTTON_DPAD_DOWN, RETRO_DEVICE_ID_JOYPAD_DOWN },
			{ SDL_CONTROLLER_BUTTON_DPAD_LEFT, RETRO_DEVICE_ID_JOYPAD_LEFT },
			{ SDL_CONTROLLER_BUTTON_DPAD_RIGHT, RETRO_DEVICE_ID_JOYPAD_RIGHT },
			{ SDL_CONTROLLER_BUTTON_START, RETRO_DEVICE_ID_JOYPAD_START },
			{ SDL_CONTROLLER_BUTTON_BACK, RETRO_DEVICE_ID_JOYPAD_SELECT },
			{ SDL_CONTROLLER_BUTTON_LEFTSHOULDER, RETRO_DEVICE_ID_JOYPAD_L },
			{ SDL_CONTROLLER_BUTTON_RIGHTSHOULDER, RETRO_DEVICE_ID_JOYPAD_R },
		}
	};

	for (Joystick joy : joysticks)
	{

		SDL_GameController* gmController = SDL_GameControllerOpen(joy.index);

		if (SDL_GameControllerGetType(gmController) == SDL_CONTROLLER_TYPE_PS5) {
			deviceGamePad.nativeInfo.controllerToken = gmController;
			deviceGamePad.index = joy.index;
			deviceGamePad.id = joy.id;

			controller.append(deviceGamePad);
		} else {
			SDL_GameControllerClose(gmController);
		}
	}

	libretro.setControllerPortDevice(deviceGamePad.port, deviceGamePad.type);
}

static void onDisconnect(SDL_JoystickID id, int port){
	
}

static void onConnect(SDL_GameController* gmController){
	setController();
}

static void inputPoll() {
	controller.inputPoll();
}

static int16_t inputState(unsigned port, unsigned device, unsigned index, unsigned id) {
	return controller.inputState(port, device, index, id);
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
	externalCoreData.gScale = 2;
	externalCoreData.audioCallback;
	externalCoreData.runLoopFrameTime;
	externalCoreData.runLoopFrameTimeLast = 0;

	externalCoreData.gVideo.hw.version_major = 4;
	externalCoreData.gVideo.hw.version_minor = 5;
	externalCoreData.gVideo.hw.context_type = RETRO_HW_CONTEXT_OPENGL_CORE;
	externalCoreData.gVideo.hw.context_reset = noop;
	externalCoreData.gVideo.hw.context_destroy = noop;

	controllerEvents.onConnect = onConnect;
	controllerEvents.onDisconnect = onDisconnect;
}
//===========================================

int main(int argc, char* argv[]) {
    initializeVariables();
	
	libretro.coreLoad("C:/RetroArch-Win64/cores/bsnes_libretro.dll");

	retro_system_av_info avInfo = libretro.loadGame("C:\\RetroArch-Win64\\roms\\Mega Man X (USA).sfc");

	videoInit(&avInfo.geometry);
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS | SDL_INIT_GAMECONTROLLER) < 0) {
        std::cout << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return 1;
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


	controller.deinit();
	audioDeinit();
	video.deinit();
    SDL_Quit();

    return 0;
}
