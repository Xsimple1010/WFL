#ifndef GAME_H
#define GAME_H

#include "Controller.hpp"
#include "LibretroClass.hpp"
#include "Video.hpp"
#include "Audio.hpp"
#include "gameLoop.hpp"

typedef void thread_game_deinit_handle(game_loop_params params, bool *fullDeinit, bool* running, ControllerClass *controller);

void initThreadGame(game_loop_params params, thread_game_deinit_handle handleDeinit,  bool *fullDeinit, bool* running, ControllerClass *controller);

#endif