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
    ev_timer();

    void init(std::function<void(ev_loop &loop, ev_watcher *w, int)> cb, double at_, double repeat_);
    void start (ev_loop &loop) noexcept ;
    void set_at(double );
    double get_at();
    void stop() ;
    void clear_pending();
    void set_repeat(double );
    double get_repeat();

    std::function<void(ev_loop *loop, ev_timer *w, int)>cb;
    double at;
    double repeat;
};

/*
//  ev_watcher_time是用来做堆的。
struct ev_watcher_time{
    int active;
    int pending;
    int priority;
    void *data;
    std::function<void(ev_loop &loop, ev_watcher_time *w, int)>cb;
    double at;
};
*/
auto cmp = [](ev_timer * a1, ev_timer * a2) { return a1->get_at() > a2->get_at(); };

class Timer{
public:
    Timer(ev_loop *loop);
    std::priority_queue<ev_timer *, std::vector<ev_timer *>, cmp > timer_queue;
    void timers_reify ();


    void push(ev_timer *);
    ev_timer * top();
    void pop();
    size_t size();



private: ev_loop *loop;
};
#endif //LIBEVCPP_EV_TIMER_H
