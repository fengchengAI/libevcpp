//
// Created by feng on 2020/12/2.
//

#ifndef LIBEVCPP_EV_EPOLL_H
#define LIBEVCPP_EV_EPOLL_H
#include "ev_loop.h"
// TODO?
// Multiplexing是一个基类，如果要实现select，poll等方法，就继承此类
class Multiplexing{
public:
    Multiplexing();
    int backend_fd;
    double backend_mintime; /* assumed typical timer resolution */
    virtual void backend_init(ev_loop *loop,int flag) = 0;
    virtual void backend_modify(ev_loop * loop, int fd, int oev, int nev) = 0;
    virtual void backend_poll(ev_loop * loop, double timeout) = 0;
    virtual void fork(ev_loop * loop) = 0;

    virtual void destroy() = 0;
};

class ev_epoll : public Multiplexing{
public:
    ev_epoll(){};

    int epoll_epoll_create();
    //void init(ev_loop *loop, int flags);
    struct epoll_event * epoll_events;
    int epoll_eventmax;
    int * epoll_eperms;
    int epoll_epermcnt;
    int epoll_epermmax;
    void backend_init(ev_loop *loop,int flag) override;
    void backend_modify(ev_loop * loop, int fd, int oev, int nev) override;
    void backend_poll(ev_loop * loop, double timeout) override;
    void destroy() override;
    void fork(ev_loop * loop) override;
};



Multiplexing * selectMultiplexing(int condition );

#endif //LIBEVCPP_EV_EPOLL_H
