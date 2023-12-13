#ifndef WFL_H
#define WFL_H

#include "controllerDefs.hpp"
#include <vector>

using std::vector;

void wflInit(struct controller_events events);
void wflLoadCore(const char* path);
void wflLoadGame(const char* path);
//void wflPause();
void wflSetController(struct controller_device device);
vector<wfl_joystick> wflGetConnectedJoysticks();

#endif




