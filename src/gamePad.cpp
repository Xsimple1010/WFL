#include "gamePad.hpp"

GamePadClass::GamePadClass() {
	SDL_GameControllerAddMappingsFromFile("./include/gamecontrollerdb.txt");

	inputPollCb = &inputPoll;
	inputStateCb = &inputState;
}

void GamePadClass::deinit() {
	for (wfl_game_pad device :gamePads)
	{
		SDL_GameControllerClose(device.nativeInfo.controllerToken);
	}

	gamePads.clear();
}

int GamePadClass::getKeyDown() {
	if (SDL_Init(SDL_INIT_EVENTS | SDL_INIT_GAMECONTROLLER) < 0) {
		std::cout << SDL_GetError() << std::endl;
    }

	SDL_Event event;

	int bt = SDL_CONTROLLER_BUTTON_INVALID;
	bool running = true;

	while (running) {
        
        while (SDL_PollEvent(&event))
        {
            switch (event.type) {
                case SDL_CONTROLLERBUTTONDOWN: {
					running = false;
					bt = event.cbutton.button;
					break;
                }
            }
        }
    }
	
	return bt;
}


vector<wfl_device> GamePadClass::getAll() {
	vector<wfl_device> devices;
	int jNun = SDL_NumJoysticks();

	for (int i = 0; i < jNun; i++)
	{	

		if(SDL_IsGameController(i) == SDL_TRUE) {
			wfl_device device;

			device.id = SDL_JoystickGetDeviceInstanceID(i);
			device.index = i;
			device.name = SDL_JoystickNameForIndex(i);

			for (auto gamePad : gamePads) {
				if(gamePad.id == device.id) {
					device.connected = true;
				}
			}

			devices.push_back(device);
		}

	}
	
	return devices;
}

vector<wfl_game_pad> GamePadClass::getConnected() {
	return gamePads;
}

void GamePadClass::append(wfl_game_pad newGamePad) {
	if(newGamePad.port > deviceMaxSize) {
		return;
	};

	if(gamePads.empty()) {
		SDL_GameController* gmController = SDL_GameControllerOpen(newGamePad.index);

		newGamePad.nativeInfo.controllerToken = gmController;

		gamePads.push_back(newGamePad);
		onDeviceAppendCb(newGamePad);
		return;
	}


	bool exist = false;
	for (wfl_game_pad& device : gamePads)
	{
		if(device.id == newGamePad.id) {
			device = newGamePad;
			exist = true;
			break;
		}
	}

	if(!exist) {
		onDeviceAppendCb(newGamePad);
	}
}

void GamePadClass::onConnect(SDL_JoystickID id) {
	auto devices = getAll();

	for (wfl_device device : devices) 
	{
		if(device.id == id) {
			onConnectCb(device);
		}
	}
	
}

void GamePadClass::onDisconnect(SDL_JoystickID id) {

	wfl_game_pad rmGamPad;
	
	for (wfl_game_pad gamePad : gamePads) 
	{
		if(gamePad.id == id) {
			SDL_GameControllerClose(gamePad.nativeInfo.controllerToken);
			rmGamPad = gamePad;
		}
	}

	if(!gamePads.empty()) {

		wfl_device gamePad = { 0 };

		gamePad.id = rmGamPad.id;
		gamePad.index = rmGamPad.index;
		gamePad.name = rmGamPad.name;
	
		onDisconnectCb(gamePad, rmGamPad.port);
		gamePads.erase(std::find(gamePads.begin(), gamePads.end(), rmGamPad));
	}
}





