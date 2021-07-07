//
// Created by feng on 2020/12/2.
//

#ifndef LIBEVCPP_WATCHER_H
#define LIBEVCPP_WATCHER_H

#include <functional>
#include <forward_list>

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
    virtual ~ev_watcher();
    void init(std::function<void(ev_loop *loop, ev_watcher *w, int)>);
    void set_priority(int pri_);
    void set_pending(int pen_);
    void set_data(void *d_);
    int get_priority() const;
    bool get_active() const;
    int get_pending() const;
    void clear_pending();
    void *get_data();
    void pri_adjust();
    void ev_start();
    virtual void stop();
    virtual void call_back(ev_loop *loop, ev_watcher *w, int) = 0;

private:
    bool active ;
    int pending;
    int priority;
    void *data;
    std::function<void(ev_loop *loop, ev_watcher *w, int)>cb;
};
#endif //LIBEVCPP_WATCHER_H
