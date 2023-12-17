#include "threads/ioEvents.hpp"
#include "debug.hpp"
#include <thread>

static SDL_Event event;

void initThreadIoEvents(bool* running, ControllerClass* controller) {
    if (SDL_Init(SDL_INIT_EVENTS | SDL_INIT_GAMECONTROLLER) < 0) {
		die("SDL could not initialize! SDL_Error: ", SDL_GetError());
    }

    std::thread eventLoopThread(eventLoop, running, controller);

    eventLoopThread.detach();
}

void deinitThreadIoEvents() {
    SDL_Quit();
}

static void eventLoop(bool* running, ControllerClass* controller) {

    while (*running) {
        
        while (SDL_PollEvent(&event))
        {
            switch (event.type) {
                case SDL_QUIT: {
                    *running = false;
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