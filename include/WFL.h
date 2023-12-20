#ifndef WFL_H
#define WFL_H

#ifdef WFL_EXPORTS
#define WFLAPI __declspec(dllexport)
#else
#define WFLAPI __declspec(dllimport)
#endif

#include "controllerDefs.hpp"
#include "WFLdefs.hpp"
#include <vector>

using std::vector;

void WFLAPI wflInit( 
    bool isSingleThread, 
    bool deinitOnStop, 
    struct controller_events events, 
    wfl_paths paths
);
void WFLAPI wflLoadCore(const char* path);
void WFLAPI wflLoadGame(const char* path);
//void wflPause();
void wflStop();
void WFLAPI wflSetController(struct controller_device device);
vector<wfl_joystick> WFLAPI wflGetConnectedJoysticks();

#endif




