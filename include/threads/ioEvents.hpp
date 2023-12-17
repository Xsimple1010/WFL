#ifndef IO_EVENTS_H
#define IO_EVENTS_H

#include <SDL.h>
#include "Controller.hpp"

void initThreadIoEvents(bool* running, ControllerClass* controller);

void deinitThreadIoEvents();

static void eventLoop(bool* running, ControllerClass* controller);

#endif