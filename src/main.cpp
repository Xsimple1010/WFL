#include <SDL.h>
#include <iostream>
#include "WFL.h"

static controller_events events = {0};


//inputs
static void setController(){
	vector<wfl_joystick> joysticks = wflGetConnectedJoysticks();

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

	for (wfl_joystick joy : joysticks)
	{

		SDL_GameController* gmController = SDL_GameControllerOpen(joy.index);

		deviceGamePad.nativeInfo.controllerToken = gmController;
        deviceGamePad.index = joy.index;
        deviceGamePad.id = joy.id;

        wflSetController(deviceGamePad);
	}

}

void onDisconnect(SDL_JoystickID id, int port){
	std::cout <<"onDisconnect -> " << id << std::endl;
}

void onConnect(SDL_GameController* gmController){
    std::cout << "onConnect ->" << gmController << std::endl;
    setController();
}

int main(int argc, char* argv[]) {

    events.onConnect = onConnect;
    events.onDisconnect =  onDisconnect;

    wflInit(events);
    wflLoadCore("C:/RetroArch-Win64/cores/bsnes_libretro.dll");
    wflLoadGame("C:/RetroArch-Win64/roms/Mega Man X (USA).sfc");
    // wflStop();
    return 0;
}
