#include "threads/threadGameLoop.hpp"
#include <thread>

static void gameThreadStop(game_loop_params params,  thread_game_extra_data_deinit dataDeinit) {
	*params.playing = false;
	*params.pause = true;

	params.video->deinit();
	params.audio->audioDeinit();
	params.libretro->deinit();

	std::cout << "gameThreadStop" << std::endl;
}

static void gameThreadDeinit(game_loop_params params,  thread_game_extra_data_deinit dataDeinit) {
	gameThreadStop(params, dataDeinit);

	dataDeinit.controller->deinit();

    SDL_Quit();
	
	std::cout << "full deinit" << std::endl;

	*dataDeinit.running = false;
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
