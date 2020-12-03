//
// Created by feng on 2020/12/3.
//

#ifndef LIBEVCPP_EV_IO_H
#define LIBEVCPP_EV_IO_H

#include <map>
#include "watcher.h"


static std::map<ev_loop,FdWatcher> anfds;

class ev_io : public ev_watcher
{
public:
    ev_io(std::function<void(ev_loop &loop, ev_watcher *w, int)> cb, int fd_,int events_);
    void init(std::function<void(ev_loop &loop, ev_watcher *w, int)> cb, int fd_,int events_);
    void start (ev_loop &loop);
    int get_fd(){
        return fd;
    }
    int get_event(){
        return events;
    }
private:
    std::forward_list<ev_watcher*> ev_watcher_list;
    int fd;
    int events;
};


#endif //LIBEVCPP_EV_IO_H
