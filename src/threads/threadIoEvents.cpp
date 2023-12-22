#include "threads/threadIoEvents.hpp"

static SDL_Event event;

static void initSDL() {
    if (SDL_Init(SDL_INIT_EVENTS | SDL_INIT_GAMECONTROLLER) < 0) {
		die("SDL could not initialize! SDL_Error: ", SDL_GetError());
    }
}

static void eventLoop(bool* running, bool* playing, ControllerClass* controller) {

    initSDL();

    while (*running) {
        
        while (SDL_PollEvent(&event))
        {
            switch (event.type) {
                case SDL_QUIT: {
                    if(*running) initSDL();
                    
                    *playing = false;
                    break;
                }

                case SDL_CONTROLLERDEVICEADDED: {
                    controller->onConnect(event.cdevice.which);
                    break;
                }

                case SDL_CONTROLLERDEVICEREMOVED: {
                    controller->onDisconnect(event.cdevice.which);
                    break;
                }
            }
        }
    }
}

void initThreadIoEvents(bool* running, bool* playing, ControllerClass* controller) {
    std::thread eventLoopThread(eventLoop, running, playing, controller);

    eventLoopThread.detach();
}