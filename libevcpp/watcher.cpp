//
// Created by feng on 2020/12/2.
//

#include "watcher.h"

ev_watcher::ev_watcher(){}
void ev_watcher::init(std::function<void(ev_loop &loop, ev_watcher *w, int)> cb_)
{
    active  = pending = priority =  0;
    data = nullptr;
    cb = cb_;
}
void ev_watcher::set_cb(std::function<void(ev_loop &loop, ev_watcher *w, int)> cb_)
{
    cb = cb_;
}
void ev_watcher::set_priority(int pri_)
{
    priority = pri_;
}
void ev_watcher::pri_adjust ()
{
    int pri = priority;
    pri = pri < EV_MINPRI ? EV_MINPRI : pri;
    pri = pri > EV_MAXPRI ? EV_MAXPRI : pri;
    priority = pri;
}
void ev_watcher::ev_start (int active_)
{
    pri_adjust ();
    active = active_;
    ++loop->activecnt;
}