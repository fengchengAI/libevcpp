//
// Created by feng on 2020/12/2.
//

#include "watcher.h"

ev_watcher::ev_watcher():
            active(0),pending(0),priority(0),data(nullptr),cb(nullptr){}
void ev_watcher::init(std::function<void(ev_loop *loop, ev_watcher *w, int)> cb_)
{
    active  = pending = priority =  0;
    data = nullptr;
    cb = cb_;
}
void ev_watcher::set_cb(std::function<void(ev_loop *loop, ev_watcher *w, int)> cb_)
{
    cb = cb_;
}
void ev_watcher::set_priority(int pri_)
{
    priority = pri_;
}
void ev_watcher::set_active(int act_)
{
    active = act_;
}
void ev_watcher::set_pending(int pen_)
{
    pending = pen_;
}
int ev_watcher::get_pending()
{
    return pending;
}
int ev_watcher::get_priority()
{
    return priority;
}
int ev_watcher::get_active()
{
    return active;
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
void ev_watcher::stop(){
    active = 0;
    loop->activecnt--;
}
ev_loop * ev_watcher::get_loop(){
    return loop;
}