//
// Created by feng on 2020/12/2.
//

#ifndef LIBEVCPP_ANFD_H
#define LIBEVCPP_ANFD_H

#include <vector>
#include <map>
#include "ev_loop.h"
#include "ev_epoll.h"
#include "ev.h"
#include "ev_io.h"
#include "watcher.h"



#define EV_ANFD_REIFY 1

class ev_io;
struct ANFD
{
    std::forward_list<ev_io*> list;
    unsigned char events; /* the events watched for */
    unsigned char reify;  /* flag set when this ANFD needs reification (EV_ANFD_REIFY, EV__IOFDSET) */
    //reify会被设置为w->events & EV__IOFDSET | EV_ANFD_REIFY

    unsigned char emask;  /* some backends store the actual kernel mask in here */
    unsigned char eflags; /* flags field for use by backends */
#if EV_USE_EPOLL
    unsigned int egen;    /* generation counter to counter epoll bugs */
#endif
};
class FdWatcher {
public:

    FdWatcher(ev_loop * loop_);
    ANFD &get_anfd(int index);
    void fd_event (int fd, int revents);
    void fd_event_nocheck (int fd, int revents);
    void fd_kill (int fd);

    void fd_change (int fd, int flags);
    void fd_rearm_all ();
    void fd_reify ();
    void remove(int fd, ev_io* w);
    void push_front(int fd, ev_io* w);
    size_t size();
private:
    ev_loop * loop;
    std::map<int,ANFD> anfd;
    std::vector<int> fdchanges;
    std::vector<int> fdkills;

};

#endif //LIBEVCPP_ANFD_H
