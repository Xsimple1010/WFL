#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <SDL2/SDL.h>
#include <iostream>
#include <vector>
#include <xmemory>
#include "controllerDefs.hpp"

using std::vector;
// using std::array;

static size_t const deviceMaxSize = 6;

class ControllerClass {
	private:
		vector<controller_device> devices;
		controller_events* callbacksEvents;

	public:
		ControllerClass(controller_events* events);
		void deinit();
		vector<Joystick> getConnectedJoysticks();
		void append(controller_device device);
		void inputPoll();
		int16_t inputState(unsigned port, unsigned device, unsigned index, unsigned id); 
		void onConnect(SDL_JoystickID id);
		void onDisconnect(SDL_JoystickID id );
		void identify();
		void checkerChanges();
};

#endif // CONTROLLER_H
