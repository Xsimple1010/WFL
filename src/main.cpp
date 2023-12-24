#include <SDL.h>
#include <iostream>
#include "WFL.h"
#include <string>

using std::string;

static wfl_events events = {0};

//inputs
static void setController(wfl_joystick joystick){

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

	deviceGamePad.index = joystick.index;
	deviceGamePad.name = joystick.name;
	deviceGamePad.id = joystick.id;

	wflSetController(deviceGamePad);
}

void onDisconnect(wfl_joystick joystick, int port){
	std::cout <<"onDisconnect -> " << joystick.name << std::endl;
}

void onConnect(wfl_joystick joystick){
    std::cout << "onConnect -> " << joystick.name << std::endl;
    setController(joystick);
}

void onGameClose() {
	std::cout << "onGameClose ->" << std::endl;
}

void onGameStart() {
	std::cout << "onGameStart ->" << std::endl;
}

int main(int argc, char* argv[]) {
    events.onConnect 	= &onConnect;
    events.onDisconnect = &onDisconnect;
	events.onGameClose 	= &onGameClose;
	events.onGameStart 	= &onGameStart;

	string rootPath 	= "C:/WFL/";
	string savePath 	= rootPath + "save";
	string systemPath 	= rootPath + "system";
	string corePath 	= rootPath + "cores/bsnes_libretro.dll";
	string romPath 		= rootPath + "roms/Mega Man X (USA).sfc";

	wfl_paths paths;
	paths.save = savePath.c_str();
	paths.system = systemPath.c_str();

    wflInit(true, true, events, paths);
   
	wflLoadCore(corePath.c_str());
    wflLoadGame(romPath.c_str());

    return 0;
}
