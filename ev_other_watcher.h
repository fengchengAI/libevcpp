//
// Created by feng on 2020/12/15.
//

#ifndef LIBEVCPP_EV_OTHER_WATCHER_H
#define LIBEVCPP_EV_OTHER_WATCHER_H

#include "watcher.h"
#include "ev_loop.h"
#include <map>
class ev_child : public ev_watcher
{
public:
    ev_child();
    void start(ev_loop *);
    void stop() override;
    void init(std::function<void(ev_loop *loop, ev_child *w, int)> , int ,int );
    void call_back(ev_loop *loop, void *w, int) override;
    void set_pid(int pid_);
    void set_rpid(int rpid_);
    void set_rstatus(int rstatus_);
    void set_flags(int flags_);
    int get_pid() const;
    int get_rpid() const;
    int get_rstatus() const;
    int get_flags();


private:
    int flags;   /* private */
    int pid;     /* ro */
    int rpid;    /* rw, holds the received pid */
    int rstatus; /* rw, holds the exit status, use the macros from sys/wait.h */
    std::function<void(ev_loop *, ev_child *, int)> cb;
};

extern std::map<int,std::forward_list<ev_child*>> childs;

class ev_async : public ev_watcher
{
public:
    ev_async();
    void init(std::function<void(ev_loop *, ev_async *, int)> cb_);
    void start(ev_loop *loop);
    void stop() override;
    void async_send();
    void call_back(ev_loop *loop, void *w, int) override;

    sig_atomic_t get_sent() const;
    void set_sent(sig_atomic_t sent_);
private:
    std::function<void(ev_loop *, ev_async *, int)> cb;
    sig_atomic_t volatile sent; /* private */
} ;
#endif //LIBEVCPP_EV_OTHER_WATCHER_H
