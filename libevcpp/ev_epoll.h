//
// Created by feng on 2020/12/2.
//

#ifndef LIBEVCPP_EV_EPOLL_H
#define LIBEVCPP_EV_EPOLL_H
#include "ev_loop.h"
class ev_loop;
class Multiplexing{
public:
    int backend_fd;
    double backend_mintime; /* assumed typical timer resolution */
    virtual void backend_init(ev_loop *loop,int flag) = 0;
    virtual void backend_modify(ev_loop * loop, int fd, int oev, int nev) = 0;
    virtual void backend_poll(ev_loop * loop, double timeout) = 0;



};

class ev_epoll : public Multiplexing{
public:
    ev_epoll(){}
    void epoll_destroy (ev_loop * loop);
    void epoll_fork (ev_loop * loop);
    int epoll_epoll_create ();
    //void init(ev_loop *loop, int flags);
    struct epoll_event * epoll_events;
    int epoll_eventmax;
    int * epoll_eperms;
    int epoll_epermcnt;
    int epoll_epermmax;
    void backend_init(ev_loop *loop,int flag) override;

    void backend_modify(ev_loop * loop, int fd, int oev, int nev) override;
    void backend_poll(ev_loop * loop, double timeout) override;
};



Multiplexing * selectMultiplexing( int condition ){
    switch (condition) {
        case  0x00000004U:
            return new ev_epoll();
    }
}

#endif //LIBEVCPP_EV_EPOLL_H
