//
// Created by feng on 2020/12/15.
//

#ifndef LIBEVCPP_EV_OTHER_WATCHER_H
#define LIBEVCPP_EV_OTHER_WATCHER_H

#include "watcher.h"
#include "ev_loop.h"

class ec_child : public ev_watcher
{
    void start(ev_loop *);
    void stop();
    void init(std::function<void(ev_loop *loop, ec_child *w, int)> , int ,int );
    std::function<void(ev_loop *, ec_child *, int)> cb;

    std::forward_list<ev_watcher*> list;
    int flags;   /* private */
    int pid;     /* ro */
    int rpid;    /* rw, holds the received pid */
    int rstatus; /* rw, holds the exit status, use the macros from sys/wait.h */

};

class ev_async : public ev_watcher
{
public:
    sig_atomic_t get_sent();
    void set_sent(sig_atomic_t sent_);
private:
    sig_atomic_t volatile sent; /* private */
} ;
#endif //LIBEVCPP_EV_OTHER_WATCHER_H
