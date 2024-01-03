#include "gameLoop.hpp"
static SDL_Event event;


void gameLoop(game_loop_params params) {
	auto video = params.video;
	auto libretro = params.libretro;
	auto path = params.gamePath;
	auto audio = params.audio;
	auto status = params.status;
	auto externalCoreData = params.externalCoreData;

	retro_system_av_info avInfo = libretro->loadGame(path);

	video->init(&avInfo.geometry);
	audio->init(avInfo.timing.sample_rate);

	while (status->getStates().playing) {
		
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_WINDOWEVENT:
				{
					switch (event.window.event) {

						case SDL_WINDOWEVENT_CLOSE: 
						{
							// *playing = false;
							// *pause = true;

							status->setPlaying(false);
							status->setPaused(true);
							break;
						}
					}
				}
			}
		} 

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

	}
}