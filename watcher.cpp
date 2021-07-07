//
// Created by feng on 2020/12/2.
//

#include "watcher.h"

#include "ev_loop.h"
ev_watcher::ev_watcher():
            active(false), pending(0), priority(0), data(nullptr){}

void ev_watcher::set_data(void *d_){
  data= d_;
}

void ev_watcher::init(std::function<void(ev_loop *loop, ev_watcher *w, int)> cb_)
{
    active = false;
    pending = priority =  0;
    data = nullptr;
    cb = cb_;
}

void ev_watcher::clear_pending(){
    if(pending)
    {
        ev_loop::GetThis()->pendings[priority-EV_MINPRI][pending - 1].w = nullptr;
        pending = 0;
    }
}

void ev_watcher::set_priority(int pri_)
{
    priority = pri_;
}

void ev_watcher::set_pending(int pen_)
{
    pending = pen_;
}

int ev_watcher::get_pending() const
{
    return pending;
}

int ev_watcher::get_priority() const
{
    return priority;
}

bool ev_watcher::get_active() const
{
    return active;
}

void ev_watcher::pri_adjust()
{
    int pri = priority;
    pri = pri < EV_MINPRI ? EV_MINPRI : pri;
    pri = pri > EV_MAXPRI ? EV_MAXPRI : pri;
    priority = pri;
}

void ev_watcher::ev_start()
{
    pri_adjust();
    active = true;
    ++ev_loop::GetThis()->activecnt;
}

ev_watcher::~ev_watcher(){

}
void ev_watcher::stop(){
    active = false;
    ev_loop::GetThis()->activecnt--;
}


void * ev_watcher::get_data() {
    return data;
}
