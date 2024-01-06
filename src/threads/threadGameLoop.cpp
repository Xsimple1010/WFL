#include "threads/threadGameLoop.hpp"
#include <thread>

static void gameThreadStop(game_loop_params params,  thread_game_extra_data_deinit dataDeinit) {
	params.status->setPlaying(false);
	params.status->setPaused(true);
}

static void gameThreadDeinit(game_loop_params params,  thread_game_extra_data_deinit dataDeinit) {
	gameThreadStop(params, dataDeinit);

	dataDeinit.gamePadClass->deinit();
	params.status->setRunning(false);

    SDL_Quit();
}

static void gameThread(game_loop_params params,  thread_game_extra_data_deinit dataDeinit) {
	gameLoop(params);

	if(*dataDeinit.fullDeinit) {
		gameThreadDeinit(params, dataDeinit);
	} else {
		gameThreadStop(params, dataDeinit);
	}
}

void initThreadGame(game_loop_params params,  thread_game_extra_data_deinit dataDeinit) {
	std::thread gThread(gameThread, params, dataDeinit);

	gThread.detach();
}
