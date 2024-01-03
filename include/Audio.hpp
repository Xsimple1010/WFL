#ifndef AUDIO_H
#define AUDIO_H

#include <SDL.h>
#include "libretro.h"
#include "SDL2/SDL_audio.h"
#include "debug.hpp"



class AudioClass {
    public:
        bool init(int frequency);
        void deinit();
        size_t write(const int16_t* buffer, size_t frames);
};

#endif // !AUDIO_H
