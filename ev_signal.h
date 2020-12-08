//
// Created by feng on 2020/12/8.
//

#ifndef LIBEVCPP_EV_SIGNAL_H
#define LIBEVCPP_EV_SIGNAL_H

#include "watcher.h"
#include <csignal>

/* associate signal watchers to a signal */
typedef struct
{
    sig_atomic_t volatile pending;
    struct ev_loop *loop;
    ev_watcher_list head;
} ANSIG;

static ANSIG signals [NSIG - 1];

class ev_signal : public ev_watcher
{
    void stop();
    void start(ev_loop &loop);
    void set_signum(int);
    int get_signum();
    void init(std::function<void(ev_loop &loop, ev_watcher *w, int)> cb, int sig);
    std::function<void(ev_loop *loop, ev_signal *w, int)>cb;
    ev_watcher_list list;
    int signum;
} ;

#endif //LIBEVCPP_EV_SIGNAL_H
