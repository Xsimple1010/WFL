#ifndef IO_EVENTS_H
#define IO_EVENTS_H

#include "SDL.h"
#include "Controller.hpp"
#include "debug.hpp"
#include <thread>

void initThreadIoEvents(bool* running, bool* playing, ControllerClass* controller);

static void eventLoop(bool* running, bool* playing, ControllerClass* controller);

#endif