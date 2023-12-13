#ifndef WFL_KEYBOARD_DEFS_H
#define WFL_KEYBOARD_DEFS_H 

#include <SDL2/SDL_gamecontroller.h>
#include "libretro.h"

#define WFL_DEVICE_KEYBOARD       1
#define WFL_DEVICE_JOYSTICK       2

//keyboard end other inputs
struct Keyboard_keymap {
	unsigned native;
	unsigned retro;
};

struct joystick_keymap {
	SDL_GameControllerButton native;
	unsigned int retro;
};

typedef void on_device_disconnect_t(SDL_JoystickID id, int port);
typedef void on_device_connect_t(SDL_GameController* gmController);
struct controller_events {
	on_device_disconnect_t* onDisconnect;
	on_device_connect_t* onConnect;
};



struct controller_native_info {
	unsigned int type;
	SDL_GameController* controllerToken;
};

struct wfl_joystick {
	SDL_JoystickID id;
	int index;
	const char* name;
};

struct controller_device {
	SDL_JoystickID id;
	int index;
	int port;
	unsigned int type;	
	controller_native_info nativeInfo;
	struct Keyboard_keymap keyboardKeyBinds[RETRO_DEVICE_ID_JOYPAD_R3 + 1];
	struct joystick_keymap joystickKeyBinds[RETRO_DEVICE_ID_JOYPAD_R3 + 1];
	bool operator==(const controller_device& other) const {
		return this->id == other.id;
	}
};

typedef void on_device_append_t(controller_device device);

struct controller_internal_events {
	on_device_append_t* onAppend;
};
#endif