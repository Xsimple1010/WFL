#ifndef VIDEO_VARIABLES_H
#define VIDEO_VARIABLES_H

#include "SDL2/SDL.h"
#include "glad/glad.h"
#include "libretro.h"

typedef void refresh_vertex_data_t();
typedef bool video_set_pixel_format_t(unsigned format);
typedef float g_scale_t;


typedef struct  {
	GLuint texeture_id;
	GLuint fbo_id;
	GLuint rbo_id;

	int glmajor;
	int glminor;

	GLuint pitch;
	GLint texture_w, texture_h;
	GLuint clip_w, clip_h;

	GLuint pixelfmt;
	GLuint pixeltype;
	GLuint bpp;

	struct retro_hw_render_callback hw;
} g_video_t;

typedef struct {
	GLuint vao;
	GLuint vbo;
	GLuint program;

	GLint i_pos;
	GLint i_coord;
	GLint u_tex;
	GLint u_mvp;

} g_shader_t;


struct video_info {
	g_video_t gVideo;
	g_scale_t gScale;
	SDL_Window* window = NULL;
};

#endif // VIDEO_VARIABLES_H
