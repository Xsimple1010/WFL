#ifndef GAME_PAD_H
#define GAME_PAD_H

#include <SDL.h>
#include <iostream>
#include <vector>
#include <xmemory>
#include <chrono>
#include "gamePadDefs.hpp"
#include "debug.hpp"
#include "stateNotifier.hpp"

using std::vector;
static size_t const deviceMaxSize = 6;

typedef void on_device_append_t(wfl_game_pad device);

static vector<wfl_game_pad> gamePads;
static unsigned GJoy[RETRO_DEVICE_ID_JOYPAD_R3] = { 0 };


//libretro events
static void inputPoll()
{	
	for (wfl_game_pad gamePad : gamePads) 
	{	
		if(gamePad.nativeInfo.type == WFL_DEVICE_JOYSTICK){
			for (game_pad_keymap keymap : gamePad.gamePadKeyBinds)
			{
				GJoy[keymap.retro] = SDL_GameControllerGetButton(gamePad.nativeInfo.controllerToken, (SDL_GameControllerButton)keymap.native);
			}
		} else if(gamePad.nativeInfo.type == WFL_DEVICE_KEYBOARD) {
			auto g_kbd = SDL_GetKeyboardState(NULL);

			for (int i = 0; gamePad.keyboardKeyBinds[i].retro || gamePad.keyboardKeyBinds[i].native; ++i)
			{
				GJoy[gamePad.keyboardKeyBinds[i].retro] = g_kbd[gamePad.keyboardKeyBinds[i].native];
			}
		}
	}
	
}

static int16_t inputState(unsigned port, unsigned deviceType, unsigned index, unsigned id) {
	
	if(port > gamePads.max_size()){
		return 0;
	}

	for (const wfl_game_pad device : gamePads)
	{
		if(device.type == deviceType) {
			
			return GJoy[id];
		}
	}
	
	return 0;
}


class GamePadClass {
	public:
		on_device_disconnect_t* onDisconnectCb;
		on_device_connect_t* onConnectCb;
		on_device_append_t* onDeviceAppendCb;
		StateNotifierClass* state;

		GamePadClass(StateNotifierClass* state);
		
		//events
		void onDisconnect(SDL_JoystickID id );
		void onConnect(SDL_JoystickID id);

		//libretro callbacks
		retro_input_state_t inputStateCb;
		retro_input_poll_t inputPollCb;
		
		vector<wfl_game_pad> getConnected();
		vector<wfl_device> getAll();
		
		int getKeyDown();
		void append(struct wfl_game_pad device);
		void deinit();
};

#endif // GAME_PAD_H
