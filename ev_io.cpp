//
// Created by feng on 2020/12/3.
//

#include "ev_io.h"

#include "anfd.h"

/* set in reify when reification needed */
#define EV_ANFD_REIFY 1

ev_io::ev_io(std::function<void(ev_loop *loop, ev_io *w, int)> cb_, int fd_,int events_)
{
    cb = cb_;
    fd = fd_;
    events = events_ | EV__IOFDSET;
}


void ev_io::init(std::function<void(ev_loop *loop, ev_io *w, int)> cb_, int fd_,int events_)
{
    cb = cb_;
    fd = fd_;
    events = events_ | EV__IOFDSET;
}

void ev_io::call_back(ev_loop *loop, void *w, int event){
    cb(loop, static_cast< ev_io *>(w), event);
}

ev_io::ev_io(){}

void ev_io::stop() {

    clear_pending();
    if (!get_active())
        return;

    assert (("libev: ev_io_stop called with illegal fd (must stay constant after start!)", fd >= 0 && fd < get_loop()->fdwtcher->size()));

#if EV_VERIFY >= 2
    assert (("libev: ev_io_stop called on watcher with invalid fd", fd_valid (w->fd)));
#endif
    get_loop()->fdwtcher->remove(fd,this);

    --get_loop()->activecnt;
    set_active(0);
    get_loop()->fdwtcher->fd_change(fd, EV_ANFD_REIFY);

}
void ev_io::start (ev_loop *loop)
{
    set_loop(loop);
    loop->base_event.push_back(this);
    if (get_active())
        return;

    assert (("libev: ev_io_start called with negative fd", fd >= 0));
    assert (("libev: ev_io_start called with illegal event mask", !(events & ~(EV__IOFDSET | EV_READ | EV_WRITE))));

#if EV_VERIFY >= 2
    assert (("libev: ev_io_start called on watcher with invalid fd", fd_valid (fd)));
#endif

    ev_start (1);

    // 给anfds分配fd+1大小的空间，因为anfds是由fd的索引访问的，这样高校，为什么加1,因为文件描述符从0开始
    loop->fdwtcher->push_front(fd,this);

    /* common bug, apparently */
    assert (("libev: ev_io_start called with corrupted watcher", loop->fdwtcher->size()!=0));


    // 此时的w->events 为 events_ | EV__IOFDSET，而events_为ev_TYPE_init传入的
    loop->fdwtcher->fd_change ( fd, events & EV__IOFDSET | EV_ANFD_REIFY);
    events &= ~EV__IOFDSET;

}