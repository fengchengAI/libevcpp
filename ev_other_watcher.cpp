//
// Created by feng on 2020/12/15.
//

#include "ev_other_watcher.h"
#include <unistd.h>
#include "ev_loop.h"
sig_atomic_t ev_async::get_sent() const {
    return sent;
}

void ev_async::set_sent(sig_atomic_t sent_) {
    sent = sent_;
}

void ev_async::start() {
    if(get_active())
        return;
    sent = 0;
    ev_loop::GetThis()->event_init();
    ev_loop::GetThis()->asyncs.push_back(this);
    ev_start();
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
    if(ev_loop::GetThis()->async_pending)
        return;
    ev_loop::GetThis()->async_pending = 1;
    int old_errno;
    old_errno = errno; /* save errno because write will clobber it */

    uint64_t counter = 1;
    write(ev_loop::GetThis()->event_fd, &counter, sizeof(uint64_t));

    errno = old_errno;
}

void ev_async::call_back(ev_loop *loop, ev_watcher *w, int event) {
    cb(loop, dynamic_cast<ev_async*>(w), event);
}


