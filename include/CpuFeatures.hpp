#ifndef CPU_FEATURES_H
#define CPU_FEATURES_H

#include "libretro.h"
#include "SDL2/SDL_cpuinfo.h"
#include "SDL2/SDL_timer.h"

retro_time_t cpuFeaturesGetTimeUsec(void);

uint64_t getCpuFeatures();

#endif // !CPU_FEATURES_H
