//
// Created by feng on 2020/12/2.
//

#ifndef LIBEVCPP_WATCHER_H
#define LIBEVCPP_WATCHER_H

#include <functional>
#include <forward_list>

#include "ev_loop.h"

class ev_loop;


class noncopyable {
protected:
    noncopyable() = default;
    virtual ~noncopyable() = default;

    noncopyable(const noncopyable &) = delete;
    noncopyable &operator=(const noncopyable &) = delete;
};

class ev_watcher : noncopyable
{

public:
    ev_watcher();
    void init(std::function<void(ev_loop *loop, ev_watcher *w, int)> );
    //void set_cb(std::function<void(ev_loop *loop, ev_watcher *w, int)> cb_);
    void set_priority(int pri_);
    void set_active(int act_);
    void set_pending(int pen_);
    void set_data(void *d_);
    //std::function<void(ev_loop *loop, ev_watcher *w, int)>  get_cb();
    int get_priority();
    int get_active();
    int get_pending();
    void clear_pending();
    int get_data();
    void pri_adjust ();
    void ev_start (int active_);
    void stop();
    void call_back(ev_loop *loop, void *w, int) ;
    ev_loop *get_loop();
    void set_loop(ev_loop *);

private:
    int active;
    int pending;
    int priority;
    void *data;
    ev_loop *loop;
    std::function<void(ev_loop *loop, ev_watcher *w, int)>cb;
};
using ev_watcher_list = std::forward_list<ev_watcher*>;
#endif //LIBEVCPP_WATCHER_H
