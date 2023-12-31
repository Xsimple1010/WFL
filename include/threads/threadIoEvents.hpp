#ifndef IO_EVENTS_H
#define IO_EVENTS_H

#include "SDL.h"
#include "gamePad.hpp"
#include "debug.hpp"
#include <thread>
#include "stateNotifier.hpp"

void initThreadIoEvents(StateNotifierClass* state, GamePadClass* gamePad);

static void eventLoop(StateNotifierClass* state, GamePadClass* gamePad);

#endif