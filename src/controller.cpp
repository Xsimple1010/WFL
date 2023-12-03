#include "Controller.hpp"

static unsigned GJoy[RETRO_DEVICE_ID_JOYPAD_R3] = { 0 };
KeyboardClass keyboard;


ControllerClass::ControllerClass(controller_events* events) {
	callbacksEvents = events;
	SDL_GameControllerAddMappingsFromFile("./include/gamecontrollerdb.txt");
}

void ControllerClass::deinit() {
	for (controller_device device :devices)
	{
		SDL_GameControllerClose(device.nativeInfo.controllerToken);
	}

	devices.clear();
}

list<Joystick> ControllerClass::getConnectedJoysticks() {
	list<Joystick> joysticks;
	int jNun = SDL_NumJoysticks();

	for (int i = 0; i < jNun; i++)
	{	

		if(SDL_IsGameController(i) == SDL_TRUE) {
			Joystick joy = {
				.id = SDL_JoystickGetDeviceInstanceID(i),
				.index = i,
				.name = SDL_JoystickNameForIndex(i),
			};

			joysticks.insert(joysticks.begin().operator++(i), joy);
		}

	}
	
	return joysticks;
}

void ControllerClass::append(controller_device device) {
	devices.insert(devices.begin().operator++(device.index), device);
}

void ControllerClass::inputPoll()
{
	for (controller_device device : devices) 
	{
		for (joystick_keymap keymap : device.joystickKeyBinds)
		{
			GJoy[keymap.retro] = SDL_GameControllerGetButton(device.nativeInfo.controllerToken, keymap.native);
		}
		
	}
}

int16_t ControllerClass::inputState(unsigned port, unsigned deviceType, unsigned index, unsigned id) {
	if(port > devices.max_size()){
		return 0;
	}

	controller_device device = devices.begin().operator++(port).operator*();

	if(device.type == deviceType ) {
		return GJoy[id];
	}

	return 0;
}

void ControllerClass::identify() {

}


void ControllerClass::onConnect(SDL_JoystickID id) {
	SDL_GameController* gmController = SDL_GameControllerFromInstanceID(id);
	callbacksEvents->onConnect(gmController);
}


void ControllerClass::onDisconnect(SDL_JoystickID id) {
	controller_device rmDevice;

	for (controller_device device : devices)
	{
		if(device.id == id) {
			rmDevice = device;
			SDL_GameControllerClose(device.nativeInfo.controllerToken);
		}
	}
	

	callbacksEvents->onDisconnect(rmDevice.id, rmDevice.port);	

	devices.remove(rmDevice);
}

void ControllerClass::checkerChanges() {
	
}