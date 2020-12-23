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
class Signal;

class ev_signal : public ev_watcher
{
public:
    ev_signal();
    void stop() override;
    //void ev_feed_signal_event(int signum);
    void start(ev_loop *loop);
    void set_signum(int);
    int get_signum();
    ~ev_signal();
    void call_back(ev_loop *loop, void *w, int) override;

    //void child_reap(int chain, int pid, int status);
    void init(std::function<void(ev_loop *loop, ev_signal *w, int)> cb_, int sig);
    //std::forward_list<ev_signal*> list;
private:
    int signum;
    std::function<void(ev_loop *loop, ev_signal *w, int)> cb;
    Signal *sigs;
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
    explicit Signal(ev_loop *loop_);
    int get_fd();
    void sigaddset(int sig);
    sigset_t* fd_set_ptr();
    ~Signal();
private:
    ev_loop *loop;
    int sigfd;
    ev_io* sigfd_w;
    sigset_t sigfd_set;
};

#endif //LIBEVCPP_EV_SIGNAL_H
