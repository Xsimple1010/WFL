#ifndef GAME_H
#define GAME_H

#include "gameLoop.hpp"
struct thread_game_extra_data_deinit {
    bool* fullDeinit;
    GamePadClass *gamePadClass;
};

void initThreadGame(game_loop_params params,  thread_game_extra_data_deinit dataDeinit);

#endif