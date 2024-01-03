#include "threads/threadIoEvents.hpp"

static void eventLoop(StateNotifierClass* state, GamePadClass* gamePadClass) {

    gamePadClass->init();

    while (state->getStates().running) {
        while (SDL_PollEvent(&state->event))
        {
            switch (state->event.type) {
                case SDL_WINDOWEVENT:
				{
					switch (state->event.window.event) {

						case SDL_WINDOWEVENT_CLOSE: 
						{
							state->setPlaying(false);
							state->setPaused(true);
							break;
						}
					}
				}

                case SDL_CONTROLLERDEVICEADDED: {
                    gamePadClass->onConnect(state->event.cdevice.which);
                    break;
                }

                case SDL_CONTROLLERDEVICEREMOVED: {
                    gamePadClass->onDisconnect(state->event.cdevice.which);
                    break;
                }

                case SDL_CONTROLLERBUTTONDOWN: {
                    auto st = state->event.cbutton.button;
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