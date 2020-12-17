//
// Created by feng on 2020/12/15.
//

#include "ev_other_watcher.h"

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
    if(get_active())
        return;
    sent = 0;
}
