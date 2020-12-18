//
// Created by feng on 2020/12/15.
//

#include "ev_other_watcher.h"
#include <unistd.h>
static std::array<std::forward_list<ec_child*>, EV_PID_HASHSIZE>childs ;

void ec_child::start(ev_loop *loop) {
#if EV_MULTIPLICITY
    assert (("libev: child watchers are only supported in the default loop", loop == ev_default_loop_ptr));
#endif
    if(get_active())
        return;
    childs[pid&((EV_PID_HASHSIZE) - 1)].push_front(this);
}

void ec_child::stop() {
    clear_pending();
    if(!get_active())
        return;
    childs[pid&((EV_PID_HASHSIZE) - 1)].remove(this);
    ev_watcher::stop();
}

void ec_child::init(std::function<void(ev_loop *, ec_child *, int)> cb_, int pid_, int flag_) {
    cb = cb_;
    pid = pid_;
    flags = flag_;
}

sig_atomic_t ev_async::get_sent() {
    return sent;
}

void ev_async::set_sent(sig_atomic_t sent_) {
    sent = sent_;

}

void ev_async::start(ev_loop *loop) {
    set_loop(loop);
    loop->base_event.push_back(this);
    if(get_active())
        return;
    sent = 0;
    get_loop()->event_init();
    ev_start( get_loop()->asyncs.size()+1);
    get_loop()->asyncs.push_back(this);
}

void ev_async::stop() {
    clear_pending();
    if(!get_active())
        return;
    ev_watcher::stop();
}

ev_async::ev_async(): ev_watcher() {

}

void ev_async::init(std::function<void(ev_loop *, ev_async *, int)> cb_) {
    cb = cb_;
}

void ev_async::async_send() {
    sent = 1;
    if(get_loop()->async_pending)
        return;
    get_loop()->async_pending = 1;
    int old_errno;
    old_errno = errno; /* save errno because write will clobber it */

#if EV_USE_EVENTFD
    uint64_t counter = 1;
    write (get_loop()->event_fd, &counter, sizeof (uint64_t));
#endif
    errno = old_errno;
}

void ev_async::call_back(ev_loop *loop, void *w, int event) {
    cb(loop, static_cast<ev_async*>(w), event);
}


