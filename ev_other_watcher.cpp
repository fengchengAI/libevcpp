//
// Created by feng on 2020/12/15.
//

#include "ev_other_watcher.h"
#include <unistd.h>
#include <map>

// TODO 这个结构体正确吗
std::map<int,std::forward_list<ev_child*>> childs;

void ev_child::start(ev_loop *loop) {
    set_loop(loop);

#if EV_MULTIPLICITY
    assert(("libev: child watchers are only supported in the default loop", loop == ev_default_loop_ptr));
#endif
    if(get_active())
        return;
    ev_start(1);
    childs[pid].push_front(this);

}

void ev_child::stop() {
    clear_pending();
    if(!get_active())
        return;
    childs[pid].remove(this);
    ev_watcher::stop();
}

void ev_child::init(std::function<void(ev_loop *, ev_child *, int)> cb_, int pid_, int flag_) {
    cb = cb_;
    pid = pid_;
    flags = flag_;
}

ev_child::ev_child() : ev_watcher(){

}

void ev_child::call_back(ev_loop *loop, void *w, int event) {
    cb(loop,static_cast<ev_child*>(w), event);
}

void ev_child::set_pid(int pid_) {
    pid = pid_;
}

void ev_child::set_rpid(int rpid_) {
    rpid = rpid_;
}

void ev_child::set_rstatus(int rstatus_) {
    rstatus = rstatus_;
}

void ev_child::set_flags(int flags_) {
    flags = flags_;
}

int ev_child::get_pid() {
    return pid;
}

int ev_child::get_rpid() {
    return rpid;
}

int ev_child::get_rstatus() {
    return rstatus;
}

int ev_child::get_flags() {
    return flags;
}

sig_atomic_t ev_async::get_sent() {
    return sent;
}

void ev_async::set_sent(sig_atomic_t sent_) {
    sent = sent_;
}

void ev_async::start(ev_loop *loop) {
    set_loop(loop);
    if(get_active())
        return;
    sent = 0;
    get_loop()->event_init();
    get_loop()->asyncs.push_back(this);
    ev_start(get_loop()->asyncs.size());
}

void ev_async::stop() {
    clear_pending();
    if(!get_active())
        return;
    ev_watcher::stop();
}

ev_async::ev_async():ev_watcher(),sent(0)
{

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
    write(get_loop()->event_fd, &counter, sizeof(uint64_t));
#endif
    errno = old_errno;
}

void ev_async::call_back(ev_loop *loop, void *w, int event) {
    cb(loop, static_cast<ev_async*>(w), event);
}


