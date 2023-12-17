#include "Controller.hpp"

static unsigned GJoy[RETRO_DEVICE_ID_JOYPAD_R3] = { 0 };


ControllerClass::ControllerClass(controller_events* events, controller_internal_events* inEvents) {
	callbacks = events;
	internalCallbacks = inEvents;
	SDL_GameControllerAddMappingsFromFile("./include/gamecontrollerdb.txt");
}

void ControllerClass::deinit() {
	mtxDevice.lock();
	for (controller_device device :devices)
	{
		SDL_GameControllerClose(device.nativeInfo.controllerToken);
	}

	devices.clear();

	mtxDevice.unlock();
}

vector<wfl_joystick> ControllerClass::getConnectedJoysticks() {
	vector<wfl_joystick> joysticks;
	int jNun = SDL_NumJoysticks();

	for (int i = 0; i < jNun; i++)
	{	

		if(SDL_IsGameController(i) == SDL_TRUE) {
			wfl_joystick joy;

			joy.id = SDL_JoystickGetDeviceInstanceID(i),
			joy.index = i,
			joy.name = SDL_JoystickNameForIndex(i),

			joysticks.push_back(joy);
		}

	}
	
	return joysticks;
}


void ControllerClass::append(controller_device newDevice) {
	mtxDevice.lock();
	if(newDevice.port > deviceMaxSize) {
		mtxDevice.unlock();
		return;
	};

	if(devices.empty()) {
		mtxDevice.unlock();
		devices.push_back(newDevice);
		internalCallbacks->onAppend(newDevice);
		return;
	}


	bool exist = false;
	for (controller_device& device : devices)
	{
		if(device.id == newDevice.id) {
			device = newDevice;
			exist = true;
			break;
		}
	}

	if(!exist) {
		internalCallbacks->onAppend(newDevice);
	}

	mtxDevice.unlock();
}

void ControllerClass::inputPoll()
{	
	mtxDevice.lock();
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
	mtxDevice.unlock();
}

int16_t ControllerClass::inputState(unsigned port, unsigned deviceType, unsigned index, unsigned id) {
	mtxDevice.lock();
	
	if(port > devices.max_size()){
		return 0;
	}

	for (const controller_device device : devices)
	{
		if(device.type == deviceType) {
			mtxDevice.unlock();
			return GJoy[id];
		}
	}
	
	mtxDevice.unlock();

	return 0;
}

void ControllerClass::identify() {

}


void ControllerClass::onConnect(SDL_JoystickID id) {
	SDL_GameController* gmController = SDL_GameControllerFromInstanceID(id);
	callbacks->onConnect(gmController);
}


void ControllerClass::onDisconnect(SDL_JoystickID id) {
	mtxDevice.lock();

	controller_device rmDevice;
	
	for (controller_device device : devices) 
	{
		if(device.id == id) {
			SDL_GameControllerClose(device.nativeInfo.controllerToken);
			rmDevice = device;
		}
	}

	if(!devices.empty()) {
	
		callbacks->onDisconnect(rmDevice.id, rmDevice.port);
		devices.erase(std::find(devices.begin(), devices.end(), rmDevice));
	}

	mtxDevice.unlock();
}
