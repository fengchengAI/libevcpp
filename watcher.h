//
// Created by feng on 2020/12/2.
//

#ifndef LIBEVCPP_WATCHER_H
#define LIBEVCPP_WATCHER_H

#include <functional>
#include <forward_list>

#include "ev_loop.h"
#include "utils.h"

class ev_loop;
enum {
    EV_UNDEF = (int)0xFFFFFFFF,
    EV_NONE = 0x00,
    EV_READ = 0x01,
    EV_WRITE = 0x02,
    EV__IOFDSET = 0x80,
    EV_IO = EV_READ,
    EV_TIMER = 0x00000100,

    EV_TIMEOUT = EV_TIMER,

    EV_PERIODIC = 0x00000200,
    EV_SIGNAL = 0x00000400,
    EV_CHILD = 0x00000800,
    EV_STAT = 0x00001000,
    EV_IDLE = 0x00002000,
    EV_PREPARE = 0x00004000,
    EV_CHECK = 0x00008000,
    EV_EMBED = 0x00010000,
    EV_FORK = 0x00020000,
    EV_CLEANUP = 0x00040000,
    EV_ASYNC = 0x00080000,
    EV_CUSTOM = 0x01000000,
    EV_ERROR = (int)0x80000000
};


class ev_watcher : noncopyable
{

public:
    ev_watcher();
    void init(std::function<void(ev_loop *loop, ev_watcher *w, int)> cb_);
    void set_cb(std::function<void(ev_loop *loop, ev_watcher *w, int)> cb_);
    void set_priority(int pri_);
    void set_active(int act_);
    void set_pending(int pen_);
    void set_data(int d_);
    std::function<void(ev_loop *loop, ev_watcher *w, int)>  get_cb();
    int get_priority();
    int get_active();
    int get_pending();
    int get_data();
    void pri_adjust ();
    void ev_start (int active_);
    void stop();
    ev_loop *get_loop();

private:
    int active;
    int pending;
    int priority;
    void *data;
    ev_loop *loop;
    std::function<void(ev_loop *loop, ev_watcher *w, int)>cb;
};

using ev_watcher_list = std::forward_list<ev_watcher *>;










class ev_async : public ev_watcher
{
public:
    sig_atomic_t get_sent(){
        return sent;
    }
    void set_sent(sig_atomic_t sent_){
        sent = sent_;
    }
private:
    sig_atomic_t volatile sent; /* private */
} ;

#endif //LIBEVCPP_WATCHER_H
