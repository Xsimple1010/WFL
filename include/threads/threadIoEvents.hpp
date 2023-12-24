#ifndef IO_EVENTS_H
#define IO_EVENTS_H

#include "SDL.h"
#include "Controller.hpp"
#include "debug.hpp"
#include <thread>
#include "stateNotifier.hpp"

void initThreadIoEvents(StateNotifierClass* state, ControllerClass* controller);

static void eventLoop(StateNotifierClass* state, ControllerClass* controller);

#endif