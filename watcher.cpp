//
// Created by feng on 2020/12/2.
//

#include "watcher.h"

ev_watcher::ev_watcher():
            active(0),pending(0),priority(0),data(nullptr){}

void ev_watcher::call_back(ev_loop *loop, void *w, int event) {
    cb(loop, static_cast<ev_watcher*>(w), event);
}

void ev_watcher::set_data(void *d_){
  data= d_;
}

void ev_watcher::init(std::function<void(ev_loop *loop, ev_watcher *w, int)> cb_)
{
    active  = pending = priority =  0;
    data = nullptr;
    cb = cb_;
}

void ev_watcher::clear_pending(){
    if (pending)
    {
        loop->pendings [priority-EV_MINPRI][pending - 1].w = loop->pending_w;
        pending = 0;
    }
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

void ev_watcher::set_loop(ev_loop * loop_){
    loop = loop_;
}

void * ev_watcher::get_data() {
    return data;
}
