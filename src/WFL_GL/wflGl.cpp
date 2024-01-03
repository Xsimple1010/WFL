#include "WFL_GL/wflGl.hpp"

static SDL_GLContext GlCtx;
static g_shader_t gShader = { 0 };

static const char* g_vshader_src =
"#version 150\n"
"in vec2 i_pos;\n"
"in vec2 i_coord;\n"
"out vec2 o_coord;\n"
"uniform mat4 u_mvp;\n"
"void main() {\n"
"o_coord = i_coord;\n"
"gl_Position = vec4(i_pos, 0.0, 1.0) * u_mvp;\n"
"}";

static const char* g_fshader_src =
"#version 150\n"
"in vec2 o_coord;\n"
"uniform sampler2D u_tex;\n"
"void main() {\n"
"gl_FragColor = texture2D(u_tex, o_coord);\n"
"}";


static void noop() {}

WFLGlClass::WFLGlClass() {
	setToDefaultValues();
}


WFLGlClass::~WFLGlClass() {
	videoInfo = {0};
}


void WFLGlClass::setToDefaultValues() {
	videoInfo.window = NULL;
	videoInfo.gVideo = {0};
	videoInfo.gScale = 2;
	videoInfo.gVideo.hw.version_major = 4;
	videoInfo.gVideo.hw.version_minor = 5;
	videoInfo.gVideo.hw.context_type = RETRO_HW_CONTEXT_OPENGL_CORE;
	videoInfo.gVideo.hw.context_reset = noop;
	videoInfo.gVideo.hw.context_destroy = noop;
}

//olhar depois
void WFLGlClass::initFrameBuffer(int width, int height)
{
	glGenFramebuffers(1, &videoInfo.gVideo.fbo_id);
	glBindFramebuffer(GL_FRAMEBUFFER, videoInfo.gVideo.fbo_id);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, videoInfo.gVideo.texeture_id, 0);

	if (videoInfo.gVideo.hw.depth && videoInfo.gVideo.hw.stencil) {
		glGenRenderbuffers(1, &videoInfo.gVideo.rbo_id);
		glBindRenderbuffer(GL_RENDERBUFFER, videoInfo.gVideo.rbo_id);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);

		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, videoInfo.gVideo.rbo_id);
	}
	else if (videoInfo.gVideo.hw.depth) {
		glGenRenderbuffers(1, &videoInfo.gVideo.rbo_id);
		glBindRenderbuffer(GL_RENDERBUFFER, videoInfo.gVideo.rbo_id);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);

		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, videoInfo.gVideo.rbo_id);
	}

	if (videoInfo.gVideo.hw.depth || videoInfo.gVideo.hw.stencil)
		glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	SDL_assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void WFLGlClass::init(retro_game_geometry* geometry) {

	int nwidth, nheight;
	
	resizeToAspect(
		geometry->aspect_ratio,
		geometry->base_width * 1, 
		geometry->base_height * 1, 
		&nwidth, 
		&nheight
	);

	nwidth *= videoInfo.gScale;
	nheight *= videoInfo.gScale;
	

	if (!videoInfo.window) {
		createWindow(nwidth, nheight);
	}

	if (videoInfo.gVideo.texeture_id) {
		glDeleteTextures(1, &videoInfo.gVideo.texeture_id);
	}

	if (!videoInfo.gVideo.pixelfmt) {
		videoInfo.gVideo.pixelfmt = GL_UNSIGNED_SHORT_5_5_5_1;
	}

	glGenTextures(1, &videoInfo.gVideo.texeture_id);

	if (!videoInfo.gVideo.texeture_id) {
		die("Failed to create the video texture");
	}

	glBindTexture(GL_TEXTURE_2D, videoInfo.gVideo.texeture_id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, geometry->max_width, geometry->max_height, 0,
		videoInfo.gVideo.pixeltype, videoInfo.gVideo.pixelfmt, NULL);

	glBindTexture(GL_TEXTURE_2D, 0);


	//olhar depois
	initFrameBuffer(geometry->max_width, geometry->max_height);

	videoInfo.gVideo.texture_w = geometry->max_width;
	videoInfo.gVideo.texture_h = geometry->max_height;
	videoInfo.gVideo.clip_w = geometry->base_width;
	videoInfo.gVideo.clip_h = geometry->base_height;

	SDL_SetWindowSize(videoInfo.window, nwidth, nheight);

	videoInfo.gVideo.hw.context_reset();
}

void WFLGlClass::deinit() {
	SDL_HideWindow(videoInfo.window);

	glDeleteTextures(1, &videoInfo.gVideo.texeture_id);
	glDeleteFramebuffers(1, &videoInfo.gVideo.fbo_id);
	glDeleteRenderbuffers(1, &videoInfo.gVideo.rbo_id);
	glDeleteBuffers(1, &gShader.vbo);
	glDeleteVertexArrays(1, &gShader.vao);

	SDL_GL_DeleteContext(GlCtx);
	SDL_DestroyWindow(videoInfo.window);

	SDL_QuitSubSystem(SDL_INIT_AUDIO);

	gShader = {0};
	videoInfo = {0};
}

void WFLGlClass::createWindow(int width, int height) {
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);

	if (videoInfo.gVideo.hw.context_type == RETRO_HW_CONTEXT_OPENGL_CORE || videoInfo.gVideo.hw.version_major >= 3) {
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, videoInfo.gVideo.hw.version_major);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, videoInfo.gVideo.hw.version_minor);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
	}

	switch (videoInfo.gVideo.hw.context_type)
	{
	case RETRO_HW_CONTEXT_OPENGL_CORE: {
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
		break;
	}
	case RETRO_HW_CONTEXT_OPENGLES2: {
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
		break;
	}
	case RETRO_HW_CONTEXT_OPENGL: {
		if (videoInfo.gVideo.glmajor >= 3) {
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
		}
		break;
	}
	default:
		die(
			"Unsupported hw context %i. (only OPENGL, OPENGL_CORE and OPENGLES2 supported)", 
			videoInfo.gVideo.hw.context_type
		);
		break;
	}

	videoInfo.window = SDL_CreateWindow(
		"WFL",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		width,
		height,
		SDL_WINDOW_OPENGL
	);

	if (!videoInfo.window) {
		die("Failed to create window: %s", SDL_GetError());
	}

	GlCtx = SDL_GL_CreateContext(videoInfo.window);
	SDL_GL_MakeCurrent(videoInfo.window, GlCtx);

	if (!GlCtx) {
		die("Failed to create OpenGL context: %s", SDL_GetError());
	}


	if(SDL_WasInit(SDL_INIT_VIDEO) != 0) {
		if (SDL_Init(SDL_INIT_VIDEO) < 0) {
			die("SDL could not initialize! SDL_Error: ", SDL_GetError());
    	}
	} 
	

	if (videoInfo.gVideo.hw.context_type == RETRO_HW_CONTEXT_OPENGLES2) {
		//if (!gladLoadGLES2Loader((GLADloadproc)SDL_GL_GetProcAddress))
			die("Failed to initialize glad.");
	}
	else {
		if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
			die("Failed to initialize glad.");
	}

	initShader();

	SDL_GL_SetSwapInterval(1);
	SDL_GL_SwapWindow(videoInfo.window);

	fprintf(stderr, "GL_SHADING_LANGUAGE_VERSION: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
	fprintf(stderr, "GL_VERSION: %s\n", glGetString(GL_VERSION));

	glViewport(0, 0, width, height);
}

static void ortho2d(float m[4][4], float left, float right, float bottom, float top) {
	m[0][0] = 1; m[0][1] = 0; m[0][2] = 0; m[0][3] = 0;
	m[1][0] = 0; m[1][1] = 1; m[1][2] = 0; m[1][3] = 0;
	m[2][0] = 0; m[2][1] = 0; m[2][2] = 1; m[2][3] = 0;
	m[3][0] = 0; m[3][1] = 0; m[3][2] = 0; m[3][3] = 1;

	m[0][0] = 2.0f / (right - left);
	m[1][1] = 2.0f / (top - bottom);
	m[2][2] = -1.0f;
	m[3][0] = -(right + left) / (right - left);
	m[3][1] = -(top + bottom) / (top - bottom);
}

void WFLGlClass::initShader() {
	GLuint vShader = compileShader(GL_VERTEX_SHADER, 1, &g_vshader_src);
	GLuint fShader = compileShader(GL_FRAGMENT_SHADER, 1, &g_fshader_src);
	GLuint program = glCreateProgram();

	SDL_assert(program);

	glAttachShader(program, vShader);
	glAttachShader(program, fShader);
	glLinkProgram(program);

	glDeleteShader(vShader);
	glDeleteShader(fShader);

	glValidateProgram(program);

	GLint status;
	glGetProgramiv(program, GL_LINK_STATUS, &status);

	if (status == GL_FALSE) {
		char buffer[4096];
		glGetProgramInfoLog(program, sizeof(buffer), NULL, buffer);
		die("Failed to link shader program: %s", buffer);
	}

	gShader.program = program;
	gShader.i_pos = glGetAttribLocation(program, "i_pos");
	gShader.i_coord = glGetAttribLocation(program, "i_coord");
	gShader.u_tex = glGetUniformLocation(program, "u_tex");
	gShader.u_mvp = glGetUniformLocation(program, "u_mvp");

	glGenVertexArrays(1, &gShader.vao);
	glGenBuffers(1, &gShader.vbo);

	glUseProgram(gShader.program);
	glUniform1i(gShader.u_tex, 0);

	float m[4][4];
	if (videoInfo.gVideo.hw.bottom_left_origin)
		ortho2d(m, -1, 1, 1, -1);
	else
		ortho2d(m, -1, 1, -1, 1);

	glUniformMatrix4fv(gShader.u_mvp, 1, GL_FALSE, (float*)m);
	
	glUseProgram(0);
};

GLuint WFLGlClass::compileShader(unsigned type, unsigned count, const char** strings) {
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, count, strings, NULL);
	glCompileShader(shader);

	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

	if (status == GL_FALSE) {
		char buffer[4096];
		glGetShaderInfoLog(shader, sizeof(buffer), NULL, buffer);
		die("Failed to compile %s shader: %s", type == GL_VERTEX_SHADER ? "vertex" : "fragment", buffer);
	}

	return shader;
}

bool WFLGlClass::setPixelFormat(unsigned format) {
	switch (format) {
	case RETRO_PIXEL_FORMAT_0RGB1555:
		videoInfo.gVideo.pixelfmt = GL_UNSIGNED_SHORT_5_5_5_1;
		videoInfo.gVideo.pixeltype = GL_BGRA;
		videoInfo.gVideo.bpp = sizeof(uint16_t);
		break;
	case RETRO_PIXEL_FORMAT_XRGB8888:
		videoInfo.gVideo.pixelfmt = GL_UNSIGNED_INT_8_8_8_8_REV;
		videoInfo.gVideo.pixeltype = GL_BGRA;
		videoInfo.gVideo.bpp = sizeof(uint32_t);
		break;
	case RETRO_PIXEL_FORMAT_RGB565:
		videoInfo.gVideo.pixelfmt = GL_UNSIGNED_SHORT_5_6_5;
		videoInfo.gVideo.pixeltype = GL_RGB;
		videoInfo.gVideo.bpp = sizeof(uint16_t);
		break;
	default:
		die("Unknown pixel type %u", format);
	}
	return true;
}

void WFLGlClass::refreshVertexData() {
	SDL_assert(videoInfo.gVideo.texture_w);
	SDL_assert(videoInfo.gVideo.texture_h);
	SDL_assert(videoInfo.gVideo.clip_w);
	SDL_assert(videoInfo.gVideo.clip_h);

	float bottom = (float)videoInfo.gVideo.clip_h / videoInfo.gVideo.texture_h;
	float right = (float)videoInfo.gVideo.clip_w / videoInfo.gVideo.texture_w;

	float vertex_data[] = {
		// pos, coord
		-1.0f, -1.0f, 0.0f,  bottom, // left-bottom
		-1.0f,  1.0f, 0.0f,  0.0f,   // left-top
		 1.0f, -1.0f, right,  bottom,// right-bottom
		 1.0f,  1.0f, right,  0.0f,  // right-top
	};

	glBindVertexArray(gShader.vao);

	glBindBuffer(GL_ARRAY_BUFFER, gShader.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data), vertex_data, GL_STREAM_DRAW);

	glEnableVertexAttribArray(gShader.i_pos);
	glEnableVertexAttribArray(gShader.i_coord);
	glVertexAttribPointer(gShader.i_pos, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, 0);
	glVertexAttribPointer(gShader.i_coord, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void*)(2 * sizeof(float)));

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void WFLGlClass::resizeToAspect(double ratio, int sw, int sh, int* dw, int* dh) {
	*dw = sw;
	*dh = sh;

	if (ratio <= 0)
		ratio = (double)sw / sh;

	if ((float)sw / sh < 1)
		*dw = *dh * ratio;
	else
		*dh = *dw / ratio;
}

void WFLGlClass:: videoRefresh(const void* data, unsigned width, unsigned height, unsigned pitch) {
	if (videoInfo.gVideo.clip_w != width || videoInfo.gVideo.clip_h != height)
	{
		videoInfo.gVideo.clip_h = height;
		videoInfo.gVideo.clip_w = width;

		refreshVertexData();
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, videoInfo.gVideo.texeture_id);

		videoInfo.gVideo.pitch = pitch;

	if (data && data != RETRO_HW_FRAME_BUFFER_VALID) {
		glPixelStorei(GL_UNPACK_ROW_LENGTH, videoInfo.gVideo.pitch / videoInfo.gVideo.bpp);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height,
			videoInfo.gVideo.pixeltype, videoInfo.gVideo.pixelfmt, data);
	}

	int w = 0, h = 0;
	SDL_GetWindowSize(videoInfo.window, &w, &h);
	glViewport(0, 0, w, h);

	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(gShader.program);

	

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, videoInfo.gVideo.texeture_id);


	glBindVertexArray(gShader.vao);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);

	glUseProgram(0);

	SDL_GL_SwapWindow(videoInfo.window);
}
