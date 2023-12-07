#ifndef WFL_H
#define WFL_H

#ifdef WFL_EXPORTS
#define WFLAPI __declspec(dllexport)
#else
#define WFLAPI __declspec(dllimport)
#endif

#include "controllerDefs.hpp"

extern "C" {
    void WFLAPI wflSetCallbacks(struct controller_events events);
    void WFLAPI wflInit();
    void WFLAPI wflLoadCore(const char* path);
    void WFLAPI wflLoadGame(const char* path);
    void WFLAPI wflUnloadGame();
    void WFLAPI wflRun();
    //void WFLAPI wflPause();
    void WFLAPI wflSetController(struct controller_device device);
    void WFLAPI wflDeinit();
}

#endif




