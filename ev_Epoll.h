//
// Created by feng on 2020/12/2.
//

#ifndef LIBEVCPP_EV_EPOLL_H
#define LIBEVCPP_EV_EPOLL_H
#include "ev_loop.h"


class ev_Epoll {
public:
    ev_Epoll(){
    };
    static ev_Epoll * GetThis();
    int epoll_epoll_create();


    void backend_init() ;
    void backend_modify(int fd, int oev, int nev) ;
    void backend_poll(double timeout) ;
    void destroy() ;
    int getfd(){
        return backend_fd;
    }
    double getmintime(){
        return backend_mintime;
    }
private:
    struct epoll_event * epoll_events;
    int epoll_eventmax;
    int * epoll_eperms;
    int backend_fd;
    double backend_mintime; /* assumed typical timer resolution */

};

#endif //LIBEVCPP_EV_EPOLL_H
