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
class ev_watcher;
class ev_child : public ev_watcher
{
public:

    int flags;
    int rpid;
    int rstatus;
    int pid;
};
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
    std::forward_list<ev_watcher*> list;
    int signum;
} ;



typedef struct
{
    sig_atomic_t volatile pending;
    ev_loop *loop;
    std::forward_list<ev_watcher*> head;
} ANSIG;
void ev_feed_signal_event (ev_loop* loop, int signum);


static ANSIG signals [NSIG - 1];



void childcb (ev_loop* loop, ev_signal * w, int revents);


#endif //LIBEVCPP_EV_SIGNAL_H
