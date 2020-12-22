//
// Created by feng on 2020/12/3.
//

#ifndef LIBEVCPP_EV_IO_H
#define LIBEVCPP_EV_IO_H

#include <map>
#include "watcher.h"

class ev_watcher;

class ev_io : public ev_watcher
{
public:
    ev_io();
    void init(std::function<void(ev_loop *, ev_io *, int)> cb, int fd_,int events_);
    void call_back(ev_loop *loop, void *w, int) override;
    void start(ev_loop *loop);
    void stop() override;
    int get_fd();
    void set_fd(int fd_);
    int get_event();
    void set_event(int event_);
private:
    std::function<void(ev_loop *, ev_io *, int)> cb;
    int fd;
    int events;
};


#endif //LIBEVCPP_EV_IO_H
