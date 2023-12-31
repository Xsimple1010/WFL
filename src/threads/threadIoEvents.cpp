#include "threads/threadIoEvents.hpp"

static SDL_Event event;

static void initSDL() {
    if (SDL_Init(SDL_INIT_EVENTS | SDL_INIT_GAMECONTROLLER) < 0) {
		die("SDL could not initialize! SDL_Error: ", SDL_GetError());
    }
}

static void eventLoop(StateNotifierClass* state, GamePadClass* gamePadClass) {

    initSDL();

    while (state->getStates().running) {
        
        while (SDL_PollEvent(&event))
        {
            switch (event.type) {
                case SDL_QUIT: {
                    if(state->getStates().running) initSDL();
                    
                    state->setPlaying(false);
                    break;
                }

                case SDL_CONTROLLERDEVICEADDED: {
                    gamePadClass->onConnect(event.cdevice.which);
                    break;
                }

                case SDL_CONTROLLERDEVICEREMOVED: {
                    gamePadClass->onDisconnect(event.cdevice.which);
                    break;
                }
            }
        }
    }
}

void initThreadIoEvents(StateNotifierClass* state, GamePadClass* gamePadClass) {
    std::thread eventLoopThread(eventLoop, state, gamePadClass);

    eventLoopThread.detach();
}