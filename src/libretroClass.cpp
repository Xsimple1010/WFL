#include "LibretroClass.hpp"

static libretro_external_data* externalData;
static core_event_functions* eventFunctions;

Libretro::Libretro(core_event_functions* eventFuncs, libretro_external_data* exterData) {
    externalData = exterData;
    eventFunctions = eventFuncs;
}

void Libretro::run() {
    retroFunctions.retro_run();
}

void Libretro::setControllerPortDevice(unsigned port, unsigned device) {
    retroFunctions.retro_set_controller_port_device(port, device);
}

//https://github.com/heuripedes/sdlarch/blob/c7760c81df688bfa146c7f0d2409656ca3eb35d2/sdlarch.c#L863
void Libretro::coreLoad(const char* coreFile) {
    if(retroFunctions.initialized) return;

    void (*set_environment)(retro_environment_t) = NULL;
	void (*set_video_refresh)(retro_video_refresh_t) = NULL;
	void (*set_input_poll)(retro_input_poll_t) = NULL;
	void (*set_input_state)(retro_input_state_t) = NULL;
	void (*set_audio_sample)(retro_audio_sample_t) = NULL;
	void (*set_audio_sample_batch)(retro_audio_sample_batch_t) = NULL;
	
	memset(&retroFunctions, 0, sizeof(retroFunctions));

    retroFunctions.handle = SDL_LoadObject(coreFile);

    load_retro_sym(retro_init);
	load_retro_sym(retro_deinit);
	load_retro_sym(retro_api_version);
	load_retro_sym(retro_get_system_info);
	load_retro_sym(retro_get_system_av_info);
	load_retro_sym(retro_set_controller_port_device);
	load_retro_sym(retro_reset);
	load_retro_sym(retro_run);
	load_retro_sym(retro_load_game);
	load_retro_sym(retro_unload_game);

	load_sym(set_environment, retro_set_environment);
	load_sym(set_video_refresh, retro_set_video_refresh);
	load_sym(set_input_poll, retro_set_input_poll);
	load_sym(set_input_state, retro_set_input_state);
	load_sym(set_audio_sample, retro_set_audio_sample);
	load_sym(set_audio_sample_batch, retro_set_audio_sample_batch);

    set_environment(environment);
	set_video_refresh(eventFunctions->videoRefresh);
	set_input_poll(eventFunctions->inputPoll);
	set_input_state(eventFunctions->inputState);
	set_audio_sample(eventFunctions->audioSample);
	set_audio_sample_batch(eventFunctions->audioSampleBatch);
	
	
	if (!retroFunctions.handle) {
		die("Failed to load core: %s", SDL_GetError());
	}

    retroFunctions.retro_init();
    retroFunctions.initialized = true;
}

void Libretro::deinit() {
    if(!retroFunctions.initialized) return;
    
    retroFunctions.retro_unload_game();
    
    if (retroFunctions.initialized) {
        retroFunctions.retro_deinit();
        retroFunctions.initialized = false;
    }

    if (retroFunctions.handle) {
        SDL_UnloadObject(retroFunctions.handle);
    }
    externalData = NULL;
    eventFunctions = NULL;
}

void Libretro::unloadGame() {
    retroFunctions.retro_unload_game();
}

retro_system_av_info Libretro::loadGame(const char* fileName) {
	struct retro_system_av_info SysAvInfo = { 0 };
	struct retro_system_info sysInfo = { 0 };
	struct retro_game_info gameInfo = { fileName, 0 };

	gameInfo.path = fileName;
	gameInfo.meta = "";
	gameInfo.data = NULL;
	gameInfo.size = 0;


    if (fileName) {
        retroFunctions.retro_get_system_info(&sysInfo);

        if (!sysInfo.need_fullpath) {
            SDL_RWops* file = SDL_RWFromFile(fileName, "rb");
            Sint64 size;

            if (!file)
                die("Failed to load %s: %s", fileName, SDL_GetError());

            size = SDL_RWsize(file);

            if (size < 0)
                die("Failed to query game file size: %s", SDL_GetError());

            gameInfo.size = size;
            gameInfo.data = SDL_malloc(gameInfo.size);

            if (!gameInfo.data)
                die("Failed to allocate memory for the content");

            if (!SDL_RWread(file, (void*)gameInfo.data, gameInfo.size, 1))
                die("Failed to read file data: %s", SDL_GetError());

            SDL_RWclose(file);
        }
    }

	if (!retroFunctions.retro_load_game(&gameInfo)) {
		die("The core failed to load the content.");
	}

	retroFunctions.retro_get_system_av_info(&SysAvInfo);

	if (gameInfo.data) {
		SDL_free((void*)gameInfo.data);
	}

	return SysAvInfo;
}


retro_system_info Libretro::getSystemInfo() {
	retro_system_info systemInfo = {0};

	retroFunctions.retro_get_system_info(&systemInfo);
    return systemInfo;
}

static void core_log(enum retro_log_level level, const char* fmt, ...) {
    char buffer[4096] = { 0 };
    static const char* levelstr[] = { "dbg", "inf", "wrn", "err" };
    va_list va;

    va_start(va, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, va);
    va_end(va);

    if (level == 0)
        return;

    fprintf(stderr, "[%s] %s", levelstr[level], buffer);
    fflush(stderr);

    if (level == RETRO_LOG_ERROR)
        exit(0);
}

static uintptr_t core_get_current_framebuffer() {
    return externalData->gVideo.fbo_id;
}

/**
 * A simple counter. Usually nanoseconds, but can also be CPU cycles.
 *
 * @see retro_perf_get_counter_t
 * @return retro_perf_tick_t The current value of the high resolution counter.
 */
static retro_perf_tick_t core_get_perf_counter() {
    return (retro_perf_tick_t)SDL_GetPerformanceCounter();
}

/**
 * Register a performance counter.
 *
 * @see retro_perf_register_t
 */
static void core_perf_register(struct retro_perf_counter* counter) {
    retroFunctions.perf_counter_last = counter;
    counter->registered = true;
}

/**
 * Starts a registered counter.
 *
 * @see retro_perf_start_t
 */
static void core_perf_start(struct retro_perf_counter* counter) {
    if (counter->registered) {
        counter->start = core_get_perf_counter();
    }
}

/**
 * Stops a registered counter.
 *
 * @see retro_perf_stop_t
 */
static void core_perf_stop(struct retro_perf_counter* counter) {
    counter->total = core_get_perf_counter() - counter->start;
}

/**
 * Log and display the state of performance counters.
 *
 * @see retro_perf_log_t
 */
static void core_perf_log() {
    // TODO: Use a linked list of counters, and loop through them all.
    core_log(
        RETRO_LOG_INFO, 
        "[timer] %s: %i - %i", 
        retroFunctions.perf_counter_last->ident, 
        retroFunctions.perf_counter_last->start, 
        retroFunctions.perf_counter_last->total
    );
}

static bool environment(unsigned cmd, void* data) {
    switch (cmd) {
    case RETRO_ENVIRONMENT_SET_VARIABLES: {
     
        return false;
    }
    case RETRO_ENVIRONMENT_GET_VARIABLE: {
        struct retro_variable* var = (struct retro_variable*)data;

        if (!g_vars)
            return false;

        for (const struct retro_variable* v = g_vars; v->key; ++v) {
            if (strcmp(var->key, v->key) == 0) {
                var->value = v->value;
                break;
            }
        }

        return false;
    }
    case RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE: {
        bool* bval = (bool*)data;
        *bval = false;
        return false;
    }
    case RETRO_ENVIRONMENT_GET_LOG_INTERFACE: {
        struct retro_log_callback* cb = (struct retro_log_callback*)data;
        cb->log = core_log;
        return false;
    }
    case RETRO_ENVIRONMENT_GET_PERF_INTERFACE: {
        struct retro_perf_callback* perf = (struct retro_perf_callback*)data;
        perf->get_time_usec = cpuFeaturesGetTimeUsec;
        perf->get_cpu_features = getCpuFeatures;
        perf->get_perf_counter = core_get_perf_counter;
        perf->perf_register = core_perf_register;
        perf->perf_start = core_perf_start;
        perf->perf_stop = core_perf_stop;
        perf->perf_log = core_perf_log;
        return true;
    }
    case RETRO_ENVIRONMENT_GET_CAN_DUPE: {
        bool* bval = (bool*)data;
        *bval = true;
        return true;
    }
    case RETRO_ENVIRONMENT_SET_PIXEL_FORMAT: {
        const enum retro_pixel_format* fmt = (enum retro_pixel_format*)data;
        std::cout << "env -> " << *fmt << std::endl;

        if (*fmt > RETRO_PIXEL_FORMAT_RGB565)
            return false;

        return eventFunctions->setPixelFormat(*fmt);
    }
    case RETRO_ENVIRONMENT_SET_HW_RENDER: {
        struct retro_hw_render_callback* hw = (struct retro_hw_render_callback*)data;
        hw->get_current_framebuffer = core_get_current_framebuffer;
        hw->get_proc_address = (retro_hw_get_proc_address_t)SDL_GL_GetProcAddress;
        externalData->gVideo.hw = *hw;
        return true;
    }
    case RETRO_ENVIRONMENT_SET_FRAME_TIME_CALLBACK: {
        const struct retro_frame_time_callback* frame_time =
            (const struct retro_frame_time_callback*)data;
        externalData->runLoopFrameTime = *frame_time;
        return true;
    }
    case RETRO_ENVIRONMENT_SET_AUDIO_CALLBACK: {
        struct retro_audio_callback* audio_cb = (struct retro_audio_callback*)data;
        std::cout << audio_cb << std::endl;
        externalData->audioCallback = *audio_cb;
        return true;
    }
    case RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY:
    case RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY: {
        const char** dir = (const char**)data;
        *dir = ".";
        return true;
    }
    case RETRO_ENVIRONMENT_SET_GEOMETRY: {
        const struct retro_game_geometry* geom = (const struct retro_game_geometry*)data;
        externalData->gVideo.clip_w = geom->base_width;
        externalData->gVideo.clip_h = geom->base_height;

        // some cores call this before we even have a window
        if (externalData->window) {
            eventFunctions->refreshVertexData();

            int ow = 0, oh = 0;
            eventFunctions->resizeToAspect(geom->aspect_ratio, geom->base_width, geom->base_height, &ow, &oh);

            ow *= externalData->gScale;
            oh *= externalData->gScale;

            SDL_SetWindowSize(externalData->window, ow, oh);
        }
        return true;
    }
    case RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME: {
        //g_retro.supports_no_game = *(bool*)data;
        return true;
    }
    case RETRO_ENVIRONMENT_GET_AUDIO_VIDEO_ENABLE: {
        int* value = (int*)data;
        *value = 1 << 0 | 1 << 1;
        return true;
    }
    default:
        core_log(RETRO_LOG_DEBUG, "Unhandled env #%u", cmd);
        return false;
    }

    return false;
}


