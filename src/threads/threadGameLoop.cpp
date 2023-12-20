#include "threads/threadGameLoop.hpp"
#include <thread>



void gameThread(game_loop_params params, thread_game_deinit_handle handleDeinit,  bool* fullDeinit, bool* running, ControllerClass *controller) {
	gameLoop(params);

	// params.video->deinit();
	// params.audio->audioDeinit();
	// params.libretro->deinit();
	// *params.externalCoreData = { 0 };

	handleDeinit(params, fullDeinit, running, controller);
}

void initThreadGame(game_loop_params params, thread_game_deinit_handle handleDeinit,  bool* fullDeinit, bool* running ,ControllerClass *controller) {
	std::thread gThread(gameThread, params, handleDeinit, fullDeinit, running, controller);

	gThread.detach();
}
