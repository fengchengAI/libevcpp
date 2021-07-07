//
// Created by feng on 2020/12/3.
//


#include "ev_io.h"
#include "ev_fdmanager.h"

ev_io::ev_io() : ev_watcher(),fd(0),events(0)
{
}
ev_io::~ev_io()
{
}

void ev_io::init(std::function<void(ev_loop *loop, ev_io *w, int)> cb_, int fd_, int events_)
{
    cb = std::move(cb_);
    fd = fd_;
    events = events_;
}

void ev_io::call_back(ev_loop *loop, ev_watcher *w, int event){
    cb(loop, dynamic_cast<ev_io *>(w), event);
}

void ev_io::stop() {

    ev_watcher::stop();

    clear_pending();
    if(!get_active())
        return;
    assert(("libev: ev_io_stop called with illegal fd(must stay constant after start!)", fd >= 0 ));

    FdManaher::GetThis()->remove(fd, this);

    FdManaher::GetThis()->fd_change(fd, 0);

}
void ev_io::start()
{
    if(get_active()){
        std::cerr<<"已经运行了"<<std::endl;
        return;
    }

    assert(("libev: ev_io_start called with negative fd", fd >= 0));
    assert(("libev: ev_io_start called with illegal event mask", !(events & ~(EV_READ | EV_WRITE))));

    ev_start();
    FdManaher::GetThis()->push_front(fd, this);
    FdManaher::GetThis()->fd_change(fd, events);

}

int ev_io::get_fd() const {
    return fd;
}

void ev_io::set_fd(int fd_) {
    fd = fd_;
}

void ev_io::set_event(int event_) {
    events = event_;
}

int ev_io::get_event() const {
    return events;
}
