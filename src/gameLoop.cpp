#include "gameLoop.hpp"


void gameLoop(game_loop_params params) {
	auto video = params.video;
	auto libretro = params.libretro;
	auto path = params.gamePath;
	auto audio = params.audio;
	auto status = params.status;
	auto externalCoreData = params.externalCoreData;

	retro_system_av_info avInfo = libretro->loadGame(path);
	SDL_Event event;

	video->init(&avInfo.geometry);
	libretro->updateVideoInfo(video->videoInfo);
	audio->init(avInfo.timing.sample_rate);

	while (status->getStates().playing) {
		
		//in systems like window it is necessary for 
		//the program to listen for events so that the window remains open
		while (SDL_PollEvent(&event)) { } 

		status->pausedMtx.lock();

		if(!status->getStates().pause) {
			if (externalCoreData->runLoopFrameTime.callback) {
				retro_time_t current = cpuFeaturesGetTimeUsec();
				retro_time_t delta = current - externalCoreData->runLoopFrameTimeLast;

				if (!externalCoreData->runLoopFrameTimeLast) {
					delta = externalCoreData->runLoopFrameTime.reference;
				}

				externalCoreData->runLoopFrameTimeLast = current;
				externalCoreData->runLoopFrameTime.callback(delta);
			}

			if (externalCoreData->audioCallback.callback) {
				externalCoreData->audioCallback.callback();
			}

			libretro->run();
		};

		status->pausedMtx.unlock();
	}

	video->deinit();
	audio->deinit();
	libretro->deinit();
}