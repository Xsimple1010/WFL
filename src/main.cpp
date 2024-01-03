#include <SDL.h>
#include <iostream>
#include "WFL.h"
#include <string>
#include <future>

using std::string;
using std::future;
using std::async;

static wfl_events events = {0};

//inputs
static void setController(wfl_device device){

	wfl_game_pad gamePad = {
		.index = 0,
		.port = 0,
		.type = RETRO_DEVICE_JOYPAD,
		.nativeInfo = {
			.type = WFL_DEVICE_JOYSTICK,
		},
		.keyboardKeyBinds = {0},
		.gamePadKeyBinds = {
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

	gamePad.index = device.index;
	gamePad.name = device.name;
	gamePad.id = device.id;

	wflSetGamePad(gamePad);
}

void onDisconnect(wfl_device device, int port){
	std::cout <<"onDisconnect -> " << device.name << std::endl;
}

void onConnect(wfl_device device){
    std::cout << "onConnect -> " << device.name << std::endl;
    setController(device);
}

void onGameClose() {
	std::cout << "onGameClose ->" << std::endl;
}

void onGameStart() {
	std::cout << "onGameStart ->" << std::endl;
}

void onStatusChange(wfl_status status) {}

int main(int argc, char* argv[]) {
    events.onConnect 		= &onConnect;
    events.onDisconnect 	= &onDisconnect;
	events.onGameClose 		= &onGameClose;
	events.onGameStart 		= &onGameStart;
	events.onStatusChange 	= &onStatusChange;

	string rootPath 		= "C:/WFL/";
	string savePath 		= rootPath + "save";
	string systemPath 		= rootPath + "system";
	string corePath 		= rootPath + "cores/bsnes_libretro.dll";
	string romPath 			= rootPath + "roms/Mega Man X (USA).sfc";

	wfl_paths paths;
	paths.save = savePath.c_str();
	paths.system = systemPath.c_str();

    wflInit(true, true, events, paths);

	wflLoadCore(corePath.c_str());
	auto keyFuture = async(std::launch::async, WFlGetKeyDown);
    wflLoadGame(romPath.c_str());
    
	return 0;
}
