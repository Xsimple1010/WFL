#include "stateNotifier.hpp"

static on_status_change_t* notifier;

void StateNotifierClass::init (on_status_change_t* onChangeCB) {
    notifier = onChangeCB;
}

wfl_status StateNotifierClass::getStates() {
    return status;
}

void StateNotifierClass::setRunning(bool isRunning) {
    runningMtx.lock();
    status.running = isRunning;
    notifier(status);
    runningMtx.unlock();
};

void StateNotifierClass::setPlaying(bool isPlaying) {
    playingMtx.lock();
    status.playing = isPlaying;
    notifier(status);
    playingMtx.unlock();
};

void StateNotifierClass::setPaused(bool isPaused) {
    pausedMtx.lock();
    status.pause = isPaused;
    notifier(status);
    pausedMtx.unlock();
};