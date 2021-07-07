//
// Created by feng on 2020/12/2.
//

#include "ev_fdmanager.h"

FdManaher * t_fdwatcher = nullptr;

FdManaher * FdManaher::GetThis(){
    if (!t_fdwatcher){
        t_fdwatcher = new FdManaher();
    }
    return t_fdwatcher;
}
FdManaher::FdManaher(){
}

ANFD & FdManaher::get_anfd(int index){
    return anfd[index];
}

void FdManaher::fd_event_nocheck(int fd, int revents)
{
    for(auto i : anfd[fd].list){
        int ev = i->get_event() & revents;
        if(ev)
            ev_loop::GetThis()->ev_feed_event(dynamic_cast<ev_watcher*>(i), ev);
    }
}

void FdManaher::push_front(int fd, ev_io* w){
    anfd[fd].list.push_front(w);
}
void FdManaher::remove(int fd, ev_io* w){
    anfd[fd].list.remove(w);
}

size_t FdManaher::size(){
    return anfd.size();
}

void FdManaher::fd_kill(int fd)
{
    for(auto i : anfd[fd].list){
        i->stop();
        ev_loop::GetThis()->ev_feed_event(dynamic_cast<ev_watcher*>(i), EV_ERROR | EV_READ | EV_WRITE);
    }
}

void FdManaher::fd_change(int fd, int event)
{
    anfd[fd].newevent |= event;
    if((anfd[fd].events ^ anfd[fd].newevent)& anfd[fd].newevent)
    {
        fdchanges.insert(fd);
    }
}

void FdManaher::fd_reify()
{
    for (int i : fdchanges) {
        ev_Epoll::GetThis()->backend_modify(i, anfd[i].events, anfd[i].newevent);
    }
    fdchanges.clear();
}
