#include "gameLoop.hpp"
static SDL_Event event;


void gameLoop(game_loop_params params) {
	auto video = params.video;
	auto videoInfo = params.videoInfo;
	auto libretro = params.libretro;
	auto path = params.gamePath;
	auto audio = params.audio;
	auto playing = params.playing;
	auto pause = params.pause;
	auto externalCoreData = params.externalCoreData;

	video->setInfo(videoInfo);
	retro_system_av_info avInfo = libretro->loadGame(path);

	video->init(&avInfo.geometry);
	if (SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
		die("SDL could not initialize! SDL_Error: ", SDL_GetError());
    }
	audio->init(avInfo.timing.sample_rate);

	while (*playing) {
		if(*pause) return;

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


		while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT: {
					std::cout << "quit" << std::endl;
					*playing = false;
					*pause = true;
					break;
				}
			
				case SDL_WINDOWEVENT:
				{
					switch (event.window.event) {

						case SDL_WINDOWEVENT_CLOSE: 
						{
							*playing = false;
							*pause = true;
							break;
						}

						//case SDL_WINDOWEVENT_RESIZED:
							//resize_cb(ev.window.data1, ev.window.data2);
							//break;
					}

				}
            }
		}        

		libretro->run();
	}
}