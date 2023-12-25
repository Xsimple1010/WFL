#ifndef VIDEO_H
#define VIDEO_H

#include "SDL2/SDL.h"
#include "glad/glad.h"
#include "debug.hpp"
#include "videoDefs.hpp"
#include "WFL_GL/wflGl.hpp"

class VideoClass {
	public:
		video_info* videoInfo;

		void init(retro_game_geometry* geometry);
		bool setPixelFormat(unsigned format);
		void refreshVertexData();
		void resizeToAspect(double ratio, int sw, int sh, int* dw, int* dh);
		void videoRefresh(const void* data, unsigned width, unsigned height, unsigned pitch);
		void deinit();
		void setToDefaultValues();
};

#endif