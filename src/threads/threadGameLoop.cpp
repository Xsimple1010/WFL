#include "threads/threadGameLoop.hpp"
#include <thread>

static void gameThreadStop(game_loop_params params,  thread_game_extra_data_deinit dataDeinit) {
	*params.playing = false;
	*params.pause = true;

	params.video->deinit();
	params.audio->deinit();
	params.libretro->deinit();

	SDL_Quit();
}

static void gameThreadDeinit(game_loop_params params,  thread_game_extra_data_deinit dataDeinit) {
	gameThreadStop(params, dataDeinit);

	dataDeinit.controller->deinit();
	*dataDeinit.running = false;

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
