#include "stateNotifier.hpp"

static on_status_change_t* notifier;

void StateNotifierClass::init (on_status_change_t* onChangeCB) {
    notifier = onChangeCB;
}

wfl_status StateNotifierClass::getStates() {
    return status;
}

void StateNotifierClass::setRunning(bool isRunning) {
    status.running = isRunning;
    notifier(status);
};

void StateNotifierClass::setPlaying(bool isPlaying) {
    status.playing = isPlaying;
    notifier(status);
};

void StateNotifierClass::setPaused(bool isPaused) {
    status.pause = isPaused;
    notifier(status);
};
