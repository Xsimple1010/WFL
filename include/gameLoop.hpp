#ifndef GAME_LOOP
#define GAME_LOOP

#include "Controller.hpp"
#include "LibretroClass.hpp"
#include "Video.hpp"
#include "Audio.hpp"
#include "debug.hpp"
#include "CpuFeatures.hpp"

struct game_loop_params {
    bool* running; 
    Libretro* libretro;
    libretro_external_data* externalCoreData;
    VideoClass* video;
    const char* gamePath;
    video_info* videoInfo;
    AudioClass* audio;
};

void gameLoop(game_loop_params params);

#endif 