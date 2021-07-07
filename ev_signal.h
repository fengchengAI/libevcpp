//
// Created by feng on 2020/12/8.
//

#ifndef LIBEVCPP_EV_SIGNAL_H
#define LIBEVCPP_EV_SIGNAL_H

#include "watcher.h"
#include "ev_io.h"
#include <csignal>
#include <cstdint>
#include <sys/signalfd.h>

/* associate signal watchers to a signal */

class ev_signal : public ev_watcher
{
public:
    ev_signal();
    void stop() override;
    void start();
    void set_signum(int);
    int get_signum();
    ~ev_signal();
    void call_back(ev_loop *loop, ev_watcher *w, int) override;

    void init(std::function<void(ev_loop *loop, ev_signal *w, int)> cb_, int sig);
private:
    int signum;
    std::function<void(ev_loop *loop, ev_signal *w, int)> cb;
};



struct ANSIG
{
    sig_atomic_t volatile pending;
    ev_loop *loop;
    std::forward_list<ev_signal*> head;
};

void ev_feed_signal_event(ev_loop* loop, int signum);

static ANSIG signals[NSIG - 1];

void childcb(ev_loop* loop, ev_signal * w, int revents);

class Signal{
public:
    static Signal * GetThis();
    explicit Signal();
    int get_fd();
    void sigaddset(int sig);
    sigset_t* fd_set_ptr();
    ~Signal();
private:
    int sigfd;
    ev_io* sigfd_w;
    sigset_t sigfd_set;
};

#endif //LIBEVCPP_EV_SIGNAL_H
