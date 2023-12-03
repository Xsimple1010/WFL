#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <SDL2/SDL.h>
#include <iostream>
#include <list>
#include "controllerDefs.hpp"

typedef void on_device_disconnect_t(SDL_JoystickID id, int port);
typedef void on_device_connect_t(SDL_GameController* gmController);
struct controller_events {
	on_device_disconnect_t* onDisconnect;
	on_device_connect_t* onConnect;
};

using std::list;

class ControllerClass {
	list<controller_device> devices;
	controller_events* callbacksEvents;

	public:
		ControllerClass(controller_events* events);
		void deinit();
		list<Joystick> getConnectedJoysticks();
		void append(controller_device device);
		void inputPoll();
		int16_t inputState(unsigned port, unsigned device, unsigned index, unsigned id); 
		void onConnect(SDL_JoystickID id);
		void onDisconnect(SDL_JoystickID id );
		void identify();
		void checkerChanges();
};

#endif // CONTROLLER_H
