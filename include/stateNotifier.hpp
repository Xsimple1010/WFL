#ifndef STATE_NOTIFIER_H
#define STATE_NOTIFIER_H

struct wfl_status {
    bool running;
    bool playing;
    bool pause;
};

typedef void on_status_change_t(wfl_status status); 


class StateNotifierClass {
    private:
        wfl_status status = { 0 };
    public:
        void init(on_status_change_t* onChange);
        wfl_status getStates();
        void setRunning(bool isRunning);
        void setPlaying(bool isPlaying);
        void setPaused(bool isPaused);
};

#endif