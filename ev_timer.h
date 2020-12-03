//
// Created by feng on 2020/12/8.
//

#ifndef LIBEVCPP_EV_TIMER_H
#define LIBEVCPP_EV_TIMER_H

#include <queue>
#include "watcher.h"

class ev_timer : public ev_watcher
{
public:
    ev_timer(){};

    void init(std::function<void(ev_loop &loop, ev_watcher *w, int)> cb, double at_, double repeat_){
        ev_watcher::init(cb);
        at = at_;
        repeat = repeat_;
    }
    void start (ev_loop &loop) noexcept ;
    void set_at(double );
    double get_at();

    void set_repeat(double );
    double get_repeat();

    std::function<void(ev_loop *loop, ev_timer *w, int)>cb;
    double at;
    double repeat;
};


//  ev_watcher_time是用来做堆的。
struct ev_watcher_time{
    int active;
    int pending;
    int priority;
    void *data;
    std::function<void(ev_loop &loop, ev_watcher_time *w, int)>cb;
    double at;
};

auto cmp = [](std::pair<ev_watcher *, double> a1, std::pair<ev_watcher *, double> a2) { return (a1.second > a2.second); };

class Timer{
public:
    Timer(ev_loop *loop);
    std::vector<std::pair<ev_watcher *, double> > anhe;
    std::priority_queue<int, decltype(anhe), decltype(cmp) > timer_queue;
    void timers_reify ();


    private: ev_loop *loop;
};
#endif //LIBEVCPP_EV_TIMER_H
