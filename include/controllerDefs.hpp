#ifndef WFL_KEYBOARD_DEFS_H
#define WFL_KEYBOARD_DEFS_H 

#include <SDL2/SDL.h>
#include "libretro.h"
#include <map>


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

struct controller_native_info {
	unsigned int type;
	SDL_GameController* controllerToken;
};

struct Joystick {
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
	Keyboard_keymap keyboardKeyBinds[RETRO_DEVICE_ID_JOYPAD_R3 + 1];
	joystick_keymap joystickKeyBinds[RETRO_DEVICE_ID_JOYPAD_R3 + 1];
	bool operator==(const controller_device &p)
    {
        return (this->id == p.id);
    }
	bool operator!=(const controller_device &p)
    {
        return (this->id != p.id);
    }
};
#endif