//
// Created by feng on 2020/12/8.
//

#ifndef LIBEVCPP_EV_TIME_H
#define LIBEVCPP_EV_TIME_H

#include "watcher.h"
class ev_timer : ev_watcher
{
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


#endif //LIBEVCPP_EV_TIME_H
