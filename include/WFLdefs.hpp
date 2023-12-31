#ifndef WFL_DEFS_H
#define WFL_DEFS_H

#include "gamePadDefs.hpp"
#include "stateNotifier.hpp"

typedef void on_game_start();
typedef void on_game_close();

struct wfl_events {
    on_game_close* onGameClose;
	on_game_start* onGameStart;
    on_device_disconnect_t* onDisconnect;
	on_device_connect_t* onConnect;
    on_status_change_t* onStatusChange;
};

struct wfl_paths {
    const char* save;
    const char* system;
};


#endif