//
// Created by feng on 2020/12/8.
//

#ifndef LIBEVCPP_EV_SIGNAL_H
#define LIBEVCPP_EV_SIGNAL_H

#include "watcher.h"
#include <csignal>
#include <cstdint>
#include <sys/signalfd.h>
/* associate signal watchers to a signal */

class ev_child : public ev_watcher
{
public:

    int flags;
    int rpid;
    int rstatus;
    int pid;
};

std::forward_list<ev_child *> childs [EV_PID_HASHSIZE];

typedef struct
{
    sig_atomic_t volatile pending;
    ev_loop *loop;
    ev_watcher_list head;
} ANSIG;
void ev_feed_signal_event (ev_loop* loop, int signum);


static ANSIG signals [NSIG - 1];

class ev_signal : public ev_watcher
{
public:
    void stop();
    void ev_feed_signal_event (int signum);
    void start(ev_loop *loop);
    void set_signum(int);
    int get_signum();
    void child_reap (int chain, int pid, int status);
    void childcb (int revents);
    void init(std::function<void(ev_loop *loop, ev_signal *w, int)> cb, int sig);
    std::function<void(ev_loop *loop, ev_signal *w, int)>cb;
    ev_watcher_list list;
    int signum;
} ;

void childcb (ev_loop* loop, ev_signal * w, int revents);


#endif //LIBEVCPP_EV_SIGNAL_H
