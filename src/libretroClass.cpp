#include "LibretroClass.hpp"

static libretro_external_data* externalData = {0};
static core_event_functions* eventFunctions = {0};
static video_info* videoInfo = {0};
static game_events* gameEvents = {0};

Libretro::Libretro(
    core_event_functions* eventFuncs,
    libretro_external_data* externData,
    game_events* gEvents
) {
    externalData = externData;
    eventFunctions = eventFuncs;
    gameEvents = gEvents;
}

void Libretro::updateVideoInfo(video_info* vinfo) {
    videoInfo = vinfo;
}

void Libretro::run() {
    retroFunctions.retro_run();
}

void Libretro::reset() {
    retroFunctions.retro_reset();
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
    coreSelected = coreFile;

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
	load_retro_sym(retro_serialize);
	load_retro_sym(retro_unserialize);
	load_retro_sym(retro_serialize_size);

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
    coreIsLoaded = true;
    retroFunctions.initialized = true;
}

void Libretro::deinit() {
    if(!retroFunctions.initialized) return;
    
    unloadGame();
    
    retroFunctions.retro_deinit();
    if (retroFunctions.handle) {
        SDL_UnloadObject(retroFunctions.handle);
    }
    
    coreIsLoaded = false;
    gameIsLoaded = false;
    coreSelected = "";
    romSelected = "";
    retroFunctions.initialized = false;
    retroFunctions = { 0 };
}

void Libretro::unloadGame() {
    retroFunctions.retro_unload_game();
    gameEvents->onGameClose();
}

std::filesystem::path Libretro::getCurrentSaveFile() {
    std::filesystem::path saveDir = string(externalData->paths.save);
    saveDir.append(getSystemInfo().library_name);
    saveDir.append(romSelected.stem().string());
    std::filesystem::create_directories(saveDir);

    saveDir.append("save.state");

    return saveDir;
}

bool Libretro::save() {
    if(!coreIsLoaded && !gameIsLoaded) {
        return false;
    }
    
    size_t size = retroFunctions.retro_serialize_size();
    void* data = SDL_malloc(size);

    if(!retroFunctions.retro_serialize(data, size)) {
        SDL_free(data);
        return false;
    }

    bool susses = wflWriterData(
        data, 
        getCurrentSaveFile().string().c_str(), 
        size
    );

    SDL_free(data);

    return susses;

}

bool Libretro::loadSave() {
    if(!coreIsLoaded && !gameIsLoaded) {
        return false;
    }
    
    bool complete = true;
    auto data = wflLoadFileFromSpecificSize(
        getCurrentSaveFile().string().c_str(),
        retroFunctions.retro_serialize_size()
    );

    if(!data) {
        return false;
    }

    try {
        if (!retroFunctions.retro_unserialize(data, retroFunctions.retro_serialize_size())) {
            printf("Failed to load saveState, core returned error");
        }
    } catch(const std::exception& e) {
        std::cerr << e.what() << std::endl;
        complete = false;
    }

    SDL_free(data);

    return complete;
}

retro_system_av_info Libretro::loadGame(const char* fileName) {
	struct retro_system_av_info sysAvInfo = { 0 };
	struct retro_system_info sysInfo = { 0 };
	struct retro_game_info gameInfo = { fileName, 0 };

    if(!coreIsLoaded) {
        return sysAvInfo;
    }

    gameInfo.path = fileName;
	gameInfo.meta = "";
	gameInfo.data = NULL;
	gameInfo.size = 0;


    retroFunctions.retro_get_system_info(&sysInfo);

    if (!sysInfo.need_fullpath) {
        gameInfo.data = wflLoadFile(fileName, &gameInfo.size);
    }

	if (!retroFunctions.retro_load_game(&gameInfo)) {
		die("The core failed to load the content.");
	}

	retroFunctions.retro_get_system_av_info(&sysAvInfo);

	if (gameInfo.data) {
		SDL_free((void*)gameInfo.data);
	}


    gameIsLoaded = true;
    gameEvents->onGameStart();

    romSelected = fileName;
	return sysAvInfo;
}

retro_system_info Libretro::getSystemInfo() {
	retro_system_info systemInfo = {0};

    if(!coreIsLoaded) {
        return systemInfo;
    }

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
    return videoInfo->gVideo.fbo_id;
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
            if (*fmt > RETRO_PIXEL_FORMAT_RGB565)
                return false;

            return eventFunctions->setPixelFormat(*fmt);
        }
        case RETRO_ENVIRONMENT_SET_HW_RENDER: {
            struct retro_hw_render_callback* hw = (struct retro_hw_render_callback*)data;
            hw->get_current_framebuffer = core_get_current_framebuffer;
            hw->get_proc_address = (retro_hw_get_proc_address_t)SDL_GL_GetProcAddress;
            videoInfo->gVideo.hw = *hw;
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
        case RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY: {
            const char** dir = (const char**)data;
            *dir = externalData->paths.save;
            return true;
        }
        case RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY: {
            const char** dir = (const char**)data;
            *dir = externalData->paths.system;
            return true;
        }
        case RETRO_ENVIRONMENT_SET_GEOMETRY: {
            const struct retro_game_geometry* geom = (const struct retro_game_geometry*)data;
            videoInfo->gVideo.clip_w = geom->base_width;
            videoInfo->gVideo.clip_h = geom->base_height;

            // some cores call this before we even have a window
            if (videoInfo->window) {
                eventFunctions->refreshVertexData();

                int ow = 0, oh = 0;
                eventFunctions->resizeToAspect(geom->aspect_ratio, geom->base_width, geom->base_height, &ow, &oh);

                ow *= videoInfo->gScale;
                oh *= videoInfo->gScale;

                SDL_SetWindowSize(videoInfo->window, ow, oh);
            }
            return true;
        }
        case RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME: {
            retroFunctions.supports_no_game = *(bool*)data;
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


