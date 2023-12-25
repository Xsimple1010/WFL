#ifndef WFL_GL_h
#define WFL_GL_h

#include "SDL2/SDL.h"
#include "glad/glad.h"
#include "debug.hpp"
#include "videoDefs.hpp"

class WFLGlClass {
	public:
		WFLGlClass();
		~WFLGlClass();
		video_info videoInfo;
		void setInfo(video_info* info);
		void init(retro_game_geometry* geometry);
		bool setPixelFormat(unsigned format);
		void refreshVertexData();
		void resizeToAspect(double ratio, int sw, int sh, int* dw, int* dh);
		void videoRefresh(const void* data, unsigned width, unsigned height, unsigned pitch);
		void deinit();
		void setToDefaultValues();
	private:
		void initFrameBuffer(int width, int height);
		void createWindow(int width, int height);
		void initShader();
		GLuint compileShader(unsigned type, unsigned count, const char** strings);
};

#endif