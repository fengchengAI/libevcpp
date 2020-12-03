//
// Created by feng on 2020/12/3.
//

#include "ev_io.h"

/* set in reify when reification needed */
#define EV_ANFD_REIFY 1

ev_io::ev_io(std::function<void(ev_loop &loop, ev_watcher *w, int)> cb, int fd_,int events_)
{
    ev_watcher::init(cb);
    fd = fd_;
    events = events_ | EV__IOFDSET;
}
void ev_io::init(std::function<void(ev_loop &loop, ev_watcher *w, int)> cb, int fd_,int events_)
{
    ev_watcher::init(cb);
    fd = fd_;
    events = events_ | EV__IOFDSET;
}
void ev_io::start (ev_loop &loop)
{
    loop.base_event.push_back(this);
    if (get_active())
        return;

    assert (("libev: ev_io_start called with negative fd", fd >= 0));
    assert (("libev: ev_io_start called with illegal event mask", !(events & ~(EV__IOFDSET | EV_READ | EV_WRITE))));

#if EV_VERIFY >= 2
    assert (("libev: ev_io_start called on watcher with invalid fd", fd_valid (fd)));
#endif

    ev_start (1);

    // 给anfds分配fd+1大小的空间，因为anfds是由fd的索引访问的，这样高校，为什么加1,因为文件描述符从0开始

    anfds[loop].anfd.at(fd).list.push_front(this);


    /* common bug, apparently */
    assert (("libev: ev_io_start called with corrupted watcher", !anfds[loop].anfd.empty()));


    // 此时的w->events 为 events_ | EV__IOFDSET，而events_为ev_TYPE_init传入的
    anfds[loop].fd_change ( fd, events & EV__IOFDSET | EV_ANFD_REIFY);
    events &= ~EV__IOFDSET;
}