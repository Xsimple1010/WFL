#ifndef WFL_KEYBOARD_DEFS_H
#define WFL_KEYBOARD_DEFS_H 

#include "SDL2/SDL_gamecontroller.h"
#include "libretro.h"

#define WFL_DEVICE_KEYBOARD       1
#define WFL_DEVICE_JOYSTICK       2

enum wfl_joystick_native_buttons {
	WFL_JOYSTICK_NATIVE_BT_INVALID          = SDL_CONTROLLER_BUTTON_INVALID,
    WFL_JOYSTICK_NATIVE_BT_A                = SDL_CONTROLLER_BUTTON_A,
    WFL_JOYSTICK_NATIVE_BT_B                = SDL_CONTROLLER_BUTTON_B,
    WFL_JOYSTICK_NATIVE_BT_X                = SDL_CONTROLLER_BUTTON_X,
    WFL_JOYSTICK_NATIVE_BT_Y                = SDL_CONTROLLER_BUTTON_Y,
    WFL_JOYSTICK_NATIVE_BT_BACK             = SDL_CONTROLLER_BUTTON_BACK,
    WFL_JOYSTICK_NATIVE_BT_GUIDE            = SDL_CONTROLLER_BUTTON_GUIDE,
    WFL_JOYSTICK_NATIVE_BT_START            = SDL_CONTROLLER_BUTTON_START,
    WFL_JOYSTICK_NATIVE_BT_L2               = SDL_CONTROLLER_BUTTON_LEFTSTICK,
    WFL_JOYSTICK_NATIVE_BT_R2       		= SDL_CONTROLLER_BUTTON_RIGHTSTICK,
    WFL_JOYSTICK_NATIVE_BT_L     			= SDL_CONTROLLER_BUTTON_LEFTSHOULDER,
    WFL_JOYSTICK_NATIVE_BT_R    			= SDL_CONTROLLER_BUTTON_RIGHTSHOULDER,
    WFL_JOYSTICK_NATIVE_BT_DPAD_UP          = SDL_CONTROLLER_BUTTON_DPAD_UP,
    WFL_JOYSTICK_NATIVE_BT_DPAD_DOWN        = SDL_CONTROLLER_BUTTON_DPAD_DOWN,
    WFL_JOYSTICK_NATIVE_BT_DPAD_LEFT        = SDL_CONTROLLER_BUTTON_DPAD_LEFT,
    WFL_JOYSTICK_NATIVE_BT_DPAD_RIGHT       = SDL_CONTROLLER_BUTTON_DPAD_RIGHT,

	 /* Xbox Series X share button, PS5 microphone button, Nintendo Switch Pro capture button, Amazon Luna microphone button */
    WFL_JOYSTICK_NATIVE_BT_MISC1            = SDL_CONTROLLER_BUTTON_MISC1,
	/* Xbox Elite paddle P1 (upper left, facing the back) */   
    WFL_JOYSTICK_NATIVE_BT_PADDLE1          = SDL_CONTROLLER_BUTTON_PADDLE1,  
	/* Xbox Elite paddle P3 (upper right, facing the back) */
    WFL_JOYSTICK_NATIVE_BT_PADDLE2          = SDL_CONTROLLER_BUTTON_PADDLE2,  
	/* Xbox Elite paddle P2 (lower left, facing the back) */
    WFL_JOYSTICK_NATIVE_BT_PADDLE3          = SDL_CONTROLLER_BUTTON_PADDLE3,  
	/* Xbox Elite paddle P4 (lower right, facing the back) */
    WFL_JOYSTICK_NATIVE_BT_PADDLE4          = SDL_CONTROLLER_BUTTON_PADDLE4,  
	/* PS4/PS5 touchpad button */
    WFL_JOYSTICK_NATIVE_BT_TOUCHPAD         = SDL_CONTROLLER_BUTTON_TOUCHPAD, 

    WFL_JOYSTICK_NATIVE_BT_MAX              = SDL_CONTROLLER_BUTTON_MAX
};

enum wfl_joystick_retro_buttons {
	WFL_JOYSTICK_RETRO_BT_INVALID          = -1,
    WFL_JOYSTICK_RETRO_BT_A                = RETRO_DEVICE_ID_JOYPAD_A,
    WFL_JOYSTICK_RETRO_BT_B                = RETRO_DEVICE_ID_JOYPAD_B,
    WFL_JOYSTICK_RETRO_BT_X                = RETRO_DEVICE_ID_JOYPAD_Y,
    WFL_JOYSTICK_RETRO_BT_Y                = RETRO_DEVICE_ID_JOYPAD_X,
    WFL_JOYSTICK_RETRO_BT_BACK             = RETRO_DEVICE_ID_JOYPAD_SELECT,
    // WFL_JOYSTICK_RETRO_BT_GUIDE           = RETRO_DEVICE_ID_JOYPAD_SELECT,
    WFL_JOYSTICK_RETRO_BT_START            = RETRO_DEVICE_ID_JOYPAD_START,
    WFL_JOYSTICK_RETRO_BT_L2			   = RETRO_DEVICE_ID_JOYPAD_L2,
    WFL_JOYSTICK_RETRO_BT_R2		       = RETRO_DEVICE_ID_JOYPAD_R2,
    WFL_JOYSTICK_RETRO_BT_L     		   = RETRO_DEVICE_ID_JOYPAD_L,
    WFL_JOYSTICK_RETRO_BT_R				   = RETRO_DEVICE_ID_JOYPAD_R,
    WFL_JOYSTICK_RETRO_BT_DPAD_UP          = RETRO_DEVICE_ID_JOYPAD_UP,
    WFL_JOYSTICK_RETRO_BT_DPAD_DOWN        = RETRO_DEVICE_ID_JOYPAD_DOWN,
    WFL_JOYSTICK_RETRO_BT_DPAD_LEFT        = RETRO_DEVICE_ID_JOYPAD_LEFT,
    WFL_JOYSTICK_RETRO_BT_DPAD_RIGHT       = RETRO_DEVICE_ID_JOYPAD_RIGHT,
};

//keyboard end other inputs
struct Keyboard_keymap {
	unsigned native;
	unsigned retro;
};

struct joystick_keymap {
	wfl_joystick_native_buttons native;
	wfl_joystick_retro_buttons retro;
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
	struct Keyboard_keymap keyboardKeyBinds[16];
	struct joystick_keymap joystickKeyBinds[16];
	bool operator==(const controller_device& other) const {
		return this->id == other.id;
	}
};

#endif