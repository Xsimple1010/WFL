#include "threads/threadGameLoop.hpp"
#include <thread>

void initThreadGame(game_loop_params params) {
	std::thread gThread(gameLoop, params);

	gThread.detach();
}
