#include "LibretroClass.hpp"

static libretro_external_data* externalData = {0};
static core_event_functions* eventFunctions = {0};
static video_info* videoInfo = {0};
static game_events* gameEvents = {0};
retro_core_options_update_display_callback_t core_options_callback;
retro_core_options_v2* options;

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
	load_retro_sym(retro_load_game_special);
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

    std::cout << sysInfo.valid_extensions << std::endl;

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
    std::cout << "core per_log" << std::endl;
    core_log(
        RETRO_LOG_INFO, 
        "[timer] %s: %i - %i", 
        retroFunctions.perf_counter_last->ident, 
        retroFunctions.perf_counter_last->start, 
        retroFunctions.perf_counter_last->total
    );
}

static bool string_is_empty(const char *data)
{
   return !data || (*data == '\0');
}

static void core_option_manager_free_converted(
      struct retro_core_options_v2 *options_v2)
{
   if (!options_v2)
      return;

   if (options_v2->categories)
   {
      free(options_v2->categories);
      options_v2->categories = NULL;
   }

   if (options_v2->definitions)
   {
      free(options_v2->definitions);
      options_v2->definitions = NULL;
   }

   free(options_v2);
}

static bool string_is_equal(const char *a, const char *b)
{
   return (a && b) ? !strcmp(a, b) : false;
}




struct retro_core_options_v2 *core_option_manager_convert_v1_intl(
      const struct retro_core_options_intl *options_v1_intl)
{
   size_t i;
   size_t num_options                                     = 0;
   struct retro_core_option_definition *option_defs_us    = NULL;
   struct retro_core_option_definition *option_defs_local = NULL;
   struct retro_core_options_v2 *options_v2               = NULL;
   struct retro_core_option_v2_definition *option_v2_defs = NULL;

   if (!options_v1_intl)
      return NULL;

   option_defs_us    = options_v1_intl->us;
   option_defs_local = options_v1_intl->local;

   if (!option_defs_us)
      return NULL;

   /* Determine number of options */
   for (;;)
   {
      if (string_is_empty(option_defs_us[num_options].key))
         break;
      num_options++;
   }

   if (num_options < 1)
      return NULL;

   /* Allocate output retro_core_options_v2 struct */
   if (!(options_v2 = (struct retro_core_options_v2 *)
         malloc(sizeof(*options_v2))))
      return NULL;

   /* Note: v1 options have no concept of
    * categories, so this field will be left
    * as NULL */
   options_v2->categories  = NULL;
   options_v2->definitions = NULL;

   /* Allocate output option_v2_defs array
    * > One extra entry required for terminating NULL entry
    * > Note that calloc() sets terminating NULL entry and
    *   correctly 'nullifies' each values array */
   if (!(option_v2_defs = (struct retro_core_option_v2_definition *)
         calloc(num_options + 1, sizeof(*option_v2_defs))))
   {
      core_option_manager_free_converted(options_v2);
      return NULL;
   }

   options_v2->definitions = option_v2_defs;

   /* Loop through options... */
   for (i = 0; i < num_options; i++)
   {
      size_t j;
      size_t num_values                            = 0;
      const char *key                              = option_defs_us[i].key;
      const char *local_desc                       = NULL;
      const char *local_info                       = NULL;
      struct retro_core_option_value *local_values = NULL;

      /* Key is always taken from us english defs */
      option_v2_defs[i].key = key;

      /* Default value is always taken from us english defs */
      option_v2_defs[i].default_value = option_defs_us[i].default_value;

      /* Try to find corresponding entry in local defs array */
      if (option_defs_local)
      {
         size_t index = 0;

         for (;;)
         {
            const char *local_key = option_defs_local[index].key;

            if (string_is_empty(local_key))
               break;

            if (string_is_equal(key, local_key))
            {
               local_desc   = option_defs_local[index].desc;
               local_info   = option_defs_local[index].info;
               local_values = option_defs_local[index].values;
               break;
            }

            index++;
         }
      }

      /* Set desc and info strings */
      option_v2_defs[i].desc = string_is_empty(local_desc) ?
            option_defs_us[i].desc : local_desc;
      option_v2_defs[i].info = string_is_empty(local_info) ?
            option_defs_us[i].info : local_info;

      /* v1 options have no concept of categories
       * (Note: These are already nullified by
       * the preceding calloc(), but we do it
       * explicitly here for code clarity) */
      option_v2_defs[i].desc_categorized = NULL;
      option_v2_defs[i].info_categorized = NULL;
      option_v2_defs[i].category_key     = NULL;

      /* Determine number of values
       * (always taken from us english defs) */
      for (;;)
      {
         if (string_is_empty(option_defs_us[i].values[num_values].value))
            break;
         num_values++;
      }

      /* Copy values */
      for (j = 0; j < num_values; j++)
      {
         const char *value       = option_defs_us[i].values[j].value;
         const char *local_label = NULL;

         /* Value string is always taken from us english defs */
         option_v2_defs[i].values[j].value = value;

         /* Try to find corresponding entry in local defs values array */
         if (local_values)
         {
            size_t value_index = 0;

            for (;;)
            {
               const char *local_value = local_values[value_index].value;

               if (string_is_empty(local_value))
                  break;

               if (string_is_equal(value, local_value))
               {
                  local_label = local_values[value_index].label;
                  break;
               }

               value_index++;
            }
         }

         /* Set value label string */
         option_v2_defs[i].values[j].label = string_is_empty(local_label) ?
               option_defs_us[i].values[j].label : local_label;
      }
   }

   return options_v2;
}


static bool environment(unsigned cmd, void* data) {


    switch (cmd) {

        case RETRO_ENVIRONMENT_GET_CAN_DUPE: { //3
            std::cout << "[Environ]: RETRO_ENVIRONMENT_GET_CAN_DUPE" << std::endl;
            bool* bval = (bool*)data;
            *bval = true;
            return true;
        }
        case RETRO_ENVIRONMENT_SET_PERFORMANCE_LEVEL: {
            std::cout << "[Environ]: RETRO_ENVIRONMENT_SET_PERFORMANCE_LEVEL"<< std::endl;
            return false;
        }
        case RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY: { //9
            std::cout << "[Environ]: RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY" << std::endl;
            const char** dir = (const char**)data;
            *dir = externalData->paths.system;
            return true;
        }
        case RETRO_ENVIRONMENT_SET_PIXEL_FORMAT: { //10
            std::cout << "[Environ]: RETRO_ENVIRONMENT_SET_PIXEL_FORMAT" << std::endl;
            const enum retro_pixel_format* fmt = (enum retro_pixel_format*)data;
            
            if (*fmt > RETRO_PIXEL_FORMAT_RGB565) {
                return false;
            }

            return eventFunctions->setPixelFormat(*fmt);
        }
        case RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS: { //11
            std::cout << "[Environ]: RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS"<< std::endl;
            return true;
        }
        case RETRO_ENVIRONMENT_SET_HW_RENDER: { //14
            std::cout << "[Environ]: RETRO_ENVIRONMENT_SET_HW_RENDER" << std::endl;
            struct retro_hw_render_callback* hw = (struct retro_hw_render_callback*)data;
            hw->get_current_framebuffer = core_get_current_framebuffer;
            hw->get_proc_address = (retro_hw_get_proc_address_t)SDL_GL_GetProcAddress;
            videoInfo->gVideo.hw = *hw;
            return true;
        }
        case RETRO_ENVIRONMENT_GET_VARIABLE: { //15
            std::cout << "[Environ]: RETRO_ENVIRONMENT_GET_VARIABLE" << std::endl;
            struct retro_variable* var = (struct retro_variable*)data;

            g_vars = var;

            if (!g_vars) {
                return false;
            }

            // for (const struct retro_variable* v = g_vars; v->key; ++v) {
            //     if (strcmp(var->key, v->key) == 0) {
            //         // var->value = v->value;
            //         std::cout << "[Environ]: " << v->key<< "-> " << std::endl;
            //         break;
            //     }
            // }
            
            return false;
        }
        case RETRO_ENVIRONMENT_SET_VARIABLES: { //16
            std::cout << "[Environ]: RETRO_ENVIRONMENT_SET_VARIABLES" << std::endl;
        
            return true;
        } 
        case RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE: { //17
            std::cout << "[Environ]: RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE" << std::endl;
            bool* bval = (bool*)data;
            *bval = true;
            return true;
        }
        case RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME: { //18
            std::cout << "[Environ]: RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME" << std::endl;
            retroFunctions.supports_no_game = *(bool*)data;
            return true;
        }
        case RETRO_ENVIRONMENT_GET_LIBRETRO_PATH: { //19
            std::cout << "[Environ]: RETRO_ENVIRONMENT_GET_LIBRETRO_PATH" << std::endl;
            return false;
        }
        case RETRO_ENVIRONMENT_SET_FRAME_TIME_CALLBACK: { //21
            std::cout << "[Environ]: RETRO_ENVIRONMENT_SET_FRAME_TIME_CALLBACK" << std::endl;
            const struct retro_frame_time_callback* frame_time =
                (const struct retro_frame_time_callback*)data;
            externalData->runLoopFrameTime = *frame_time;
            return true;
        }
        case RETRO_ENVIRONMENT_SET_AUDIO_CALLBACK: { //22
            std::cout << "[Environ]: RETRO_ENVIRONMENT_SET_AUDIO_CALLBACK" << std::endl;
            struct retro_audio_callback* audio_cb = (struct retro_audio_callback*)data;
            std::cout << audio_cb << std::endl;
            externalData->audioCallback = *audio_cb;
            return true;
        }

        case RETRO_ENVIRONMENT_GET_LOG_INTERFACE: { //27
            std::cout << "[Environ]: RETRO_ENVIRONMENT_GET_LOG_INTERFACE" << std::endl;
            struct retro_log_callback* cb = (struct retro_log_callback*)data;
            cb->log = core_log;
            return true;
        }
        case RETRO_ENVIRONMENT_GET_PERF_INTERFACE: { //28
            std::cout << "[Environ]: RETRO_ENVIRONMENT_GET_PERF_INTERFACE" << std::endl;
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
        case RETRO_ENVIRONMENT_GET_CORE_ASSETS_DIRECTORY: { //30
            std::cout << "[Environ]: RETRO_ENVIRONMENT_GET_CORE_ASSETS_DIRECTORY" << std::endl;
            const char** dir = (const char**)data;
            *dir = "C:/WFL/assent";
            return true;
        }
        case RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY: { //31
            std::cout << "[Environ]: RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY" << std::endl;
            const char** dir = (const char**)data;
            *dir = externalData->paths.save;
            return true;
        }
        case RETRO_ENVIRONMENT_SET_SUBSYSTEM_INFO: { //34
            std::cout << "[Environ]: RETRO_ENVIRONMENT_SET_SUBSYSTEM_INFO" << std::endl;
        
            return false;
        }
        case RETRO_ENVIRONMENT_SET_CONTROLLER_INFO: { //35
            std::cout << "[Environ]: RETRO_ENVIRONMENT_SET_CONTROLLER_INFO" << std::endl;
        
            return false;
        }
        case RETRO_ENVIRONMENT_SET_GEOMETRY: { //37
            std::cout << "[Environ]: RETRO_ENVIRONMENT_SET_GEOMETRY" << std::endl;
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
        case RETRO_ENVIRONMENT_GET_LANGUAGE: { //39
            std::cout << "[Environ]: RETRO_ENVIRONMENT_GET_LANGUAGE" << std::endl;
            *(unsigned *)data = RETRO_LANGUAGE_ENGLISH;
            std::cout << *(unsigned *)data << std::endl;
            return true;
        }
        case RETRO_ENVIRONMENT_GET_HW_RENDER_INTERFACE: { //41 | RETRO_ENVIRONMENT_EXPERIMENTAL
            std::cout << "[Environ]: RETRO_ENVIRONMENT_GET_HW_RENDER_INTERFACE" << std::endl;
            
            return false;
        }
        case RETRO_ENVIRONMENT_SET_SUPPORT_ACHIEVEMENTS: { //42 | RETRO_ENVIRONMENT_EXPERIMENTAL
            std::cout << "[Environ]: RETRO_ENVIRONMENT_SET_SUPPORT_ACHIEVEMENTS" << std::endl;
            
            return false;
        }
        case RETRO_ENVIRONMENT_SET_HW_RENDER_CONTEXT_NEGOTIATION_INTERFACE: { //43 | RETRO_ENVIRONMENT_EXPERIMENTAL
            std::cout << "[Environ]: RETRO_ENVIRONMENT_SET_HW_RENDER_CONTEXT_NEGOTIATION_INTERFACE" << std::endl;

            return true;
        }
        case RETRO_ENVIRONMENT_SET_HW_SHARED_CONTEXT: { //44 | RETRO_ENVIRONMENT_EXPERIMENTAL
            std::cout << "[Environ]: RETRO_ENVIRONMENT_SET_HW_SHARED_CONTEXT" << std::endl;
            
            return false;
        }
        case RETRO_ENVIRONMENT_GET_VFS_INTERFACE: { //45
            std::cout << "[Environ]: RETRO_ENVIRONMENT_GET_VFS_INTERFACE" << std::endl;
            
            return false;
        }
        case RETRO_ENVIRONMENT_GET_AUDIO_VIDEO_ENABLE: { //47 | RETRO_ENVIRONMENT_EXPERIMENTAL
            std::cout << "[Environ]: RETRO_ENVIRONMENT_GET_AUDIO_VIDEO_ENABLE" << std::endl;
            int* value = (int*)data;
            *value = 1 << 0 | 1 << 1;
            return true;
        }
        case RETRO_ENVIRONMENT_GET_INPUT_BITMASKS: { //51 | RETRO_ENVIRONMENT_EXPERIMENTAL
            std::cout << "[Environ]: RETRO_ENVIRONMENT_GET_INPUT_BITMASKS" << std::endl;
            return true;
        }
        case RETRO_ENVIRONMENT_GET_CORE_OPTIONS_VERSION: { //52
            std::cout << "[Environ]: RETRO_ENVIRONMENT_GET_CORE_OPTIONS_VERSION" << std::endl;
            *(unsigned *)data = 2;
            return true;
        }
        case RETRO_ENVIRONMENT_SET_CORE_OPTIONS_INTL: { //54
            std::cout << "[Environ]: RETRO_ENVIRONMENT_SET_CORE_OPTIONS_INTL." << std::endl;
            struct retro_core_options_v2 *options_v2 =
                  core_option_manager_convert_v1_intl(
                        (const struct retro_core_options_intl*)data);

            // int index = 0;

            // while(options_v2->definitions[index].key != NULL) {
            //     auto definition = options_v2->definitions[index];

            //     if(definition.key != NULL) {
            //         std::cout << definition.key << std::endl;
            //     }
            //     if(definition.values != NULL) {
            //         if(definition.values->label) {
            //             std::cout << definition.values->label << std::endl;
            //         }
            //         std::cout << definition.values->value << std::endl;
            //     }

            //     if(definition.info) {
            //         std::cout << definition.info << std::endl;
            //     }

            //     index++;
            //     std::cout << std::endl;
            // }

            options = options_v2;

            // core_option_manager_free_converted(options_v2);

            return true;
        }
        //aqui as opcoes sao removidas se core_options_display.visible for false
        case RETRO_ENVIRONMENT_SET_CORE_OPTIONS_DISPLAY: { //55
            std::cout << "[Environ]: RETRO_ENVIRONMENT_SET_CORE_OPTIONS_DISPLAY" << std::endl;
            const struct retro_core_option_display *core_options_display =
                  (const struct retro_core_option_display *)data;

            return true;
        }
        case RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2_INTL: { //68
            std::cout << "[Environ]: RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2_INTL." << std::endl;
            
            return true;
        }
        case RETRO_ENVIRONMENT_SET_CORE_OPTIONS_UPDATE_DISPLAY_CALLBACK: { //69
            std::cout << "[Environ]: RETRO_ENVIRONMENT_SET_CORE_OPTIONS_UPDATE_DISPLAY_CALLBACK" << std::endl;
            const struct retro_core_options_update_display_callback *update_display_callback =
                            (const struct retro_core_options_update_display_callback*)data;
            
            if (update_display_callback && update_display_callback->callback) {
                core_options_callback = update_display_callback->callback;
            }
            
            return true;
        }
        default: {
            core_log(RETRO_LOG_DEBUG, "Unhandled env #%u", cmd);
            std::cout << cmd << std::endl;
            return false;
        }
    }

    return false;
}


