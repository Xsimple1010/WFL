#include "Controller.hpp"

static unsigned GJoy[RETRO_DEVICE_ID_JOYPAD_R3] = { 0 };


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

vector<Joystick> ControllerClass::getConnectedJoysticks() {
	vector<Joystick> joysticks;
	int jNun = SDL_NumJoysticks();

	for (int i = 0; i < jNun; i++)
	{	

		if(SDL_IsGameController(i) == SDL_TRUE) {
			Joystick joy = {
				.id = SDL_JoystickGetDeviceInstanceID(i),
				.index = i,
				.name = SDL_JoystickNameForIndex(i),
			};

			joysticks.push_back(joy);
		}

	}
	
	return joysticks;
}

void ControllerClass::append(controller_device newDevice) {
	if(newDevice.port > deviceMaxSize) return;

	for (controller_device device : devices)
	{
		if(device.id == newDevice.id) {
			device = newDevice;
		}
	}

	if(devices.empty()) {
		devices.push_back(newDevice);
	}
	 
}

void ControllerClass::inputPoll()
{
	for (controller_device device : devices) 
	{	
		if(device.nativeInfo.type == WFL_DEVICE_JOYSTICK){
			for (joystick_keymap keymap : device.joystickKeyBinds)
			{
				GJoy[keymap.retro] = SDL_GameControllerGetButton(device.nativeInfo.controllerToken, keymap.native);
			}
		} else if(device.nativeInfo.type == WFL_DEVICE_KEYBOARD) {
			int i;
			auto g_kbd = SDL_GetKeyboardState(NULL);

			for (i = 0; device.keyboardKeyBinds[i].retro || device.keyboardKeyBinds[i].native; ++i)
			{
				GJoy[device.keyboardKeyBinds[i].retro] = g_kbd[device.keyboardKeyBinds[i].native];
			}
		}
	}
}

int16_t ControllerClass::inputState(unsigned port, unsigned deviceType, unsigned index, unsigned id) {
	if(port > devices.max_size()){
		return 0;
	}

	for (const controller_device device : devices)
	{
		if(device.type == deviceType) {
			return GJoy[id];
		}
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
			SDL_GameControllerClose(device.nativeInfo.controllerToken);
			rmDevice = device;
		}
	}

	if(!devices.empty()) {
		devices.erase(std::find(devices.begin(), devices.end(), rmDevice));
	}

}

void ControllerClass::checkerChanges() {
	
}