#include <SDL.h>
#include <iostream>
#include "WFL.h"
#include <string>

using std::string;

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
			{ WFL_JOYSTICK_NATIVE_BT_A, WFL_JOYSTICK_RETRO_BT_A },
			{ WFL_JOYSTICK_NATIVE_BT_B, WFL_JOYSTICK_RETRO_BT_B },
			{ WFL_JOYSTICK_NATIVE_BT_X, WFL_JOYSTICK_RETRO_BT_X },
			{ WFL_JOYSTICK_NATIVE_BT_Y, WFL_JOYSTICK_RETRO_BT_Y },
			{ WFL_JOYSTICK_NATIVE_BT_DPAD_UP, WFL_JOYSTICK_RETRO_BT_DPAD_UP },
			{ WFL_JOYSTICK_NATIVE_BT_DPAD_DOWN, WFL_JOYSTICK_RETRO_BT_DPAD_DOWN },
			{ WFL_JOYSTICK_NATIVE_BT_DPAD_LEFT, WFL_JOYSTICK_RETRO_BT_DPAD_LEFT },
			{ WFL_JOYSTICK_NATIVE_BT_DPAD_RIGHT, WFL_JOYSTICK_RETRO_BT_DPAD_RIGHT },
			{ WFL_JOYSTICK_NATIVE_BT_START, WFL_JOYSTICK_RETRO_BT_START },
			{ WFL_JOYSTICK_NATIVE_BT_BACK, WFL_JOYSTICK_RETRO_BT_BACK },
			{ WFL_JOYSTICK_NATIVE_BT_L, WFL_JOYSTICK_RETRO_BT_L },
			{ WFL_JOYSTICK_NATIVE_BT_R, WFL_JOYSTICK_RETRO_BT_R },
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

	string rootPath = "C:/WFL/";
	string savePath = rootPath + "save";
	string systemPath = rootPath + "system";
	string corePath = rootPath + "cores/bsnes_libretro.dll";
	string romPath = rootPath + "roms/Mega Man X (USA).sfc";

	wfl_paths paths;
	paths.save = savePath.c_str();
	paths.system = systemPath.c_str();

    wflInit(true, true, events, paths);
   
	wflLoadCore(corePath.c_str());
    wflLoadGame(romPath.c_str());

    return 0;
}
