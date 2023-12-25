#ifndef GAME_LOOP
#define GAME_LOOP

#include "Controller.hpp"
#include "LibretroClass.hpp"
#include "Video.hpp"
#include "Audio.hpp"
#include "debug.hpp"
#include "CpuFeatures.hpp"
#include "stateNotifier.hpp"

struct game_loop_params {
    StateNotifierClass* status; 
    Libretro* libretro;
    libretro_external_data* externalCoreData;
    VideoClass* video;
    const char* gamePath;
    AudioClass* audio;
};

void gameLoop(game_loop_params params);

#endif 