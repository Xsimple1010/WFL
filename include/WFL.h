#ifndef WFL_H
#define WFL_H

#ifdef WFL_EXPORTS
#define WFLAPI __declspec(dllexport)
#else
#define WFLAPI __declspec(dllimport)
#endif

#include "WFLdefs.hpp"
#include <vector>

using std::vector;

void WFLAPI wflInit( 
    bool isSingleThread, 
    bool fullDeinit, 
    wfl_events events,
    wfl_paths paths
);


//initialize
void WFLAPI wflLoadCore(const char* path);
void WFLAPI wflLoadGame(const char* path);
void WFLAPI wflDeinit();

//game status
void WFLAPI wflStop();
void WFLAPI wflPause();
void WFLAPI wflResume();
void WFLAPI wflReset();

//save
void WFLAPI wflSave();
void WFLAPI wflLoadSave();

//gamePads
int WFLAPI WFlGetKeyDown();
void WFLAPI wflSetGamePad(struct wfl_game_pad device);
vector<wfl_device> WFLAPI wflGetAllGamePads();
vector<wfl_game_pad> WFLAPI wflGetGamePad();

#endif




