//
// Created by feng on 2020/12/8.
//

#ifndef LIBEVCPP_EV_TIMER_H
#define LIBEVCPP_EV_TIMER_H

#include <queue>

#include "watcher.h"
#include "utils.h"
#include "ev_loop.h"
class ev_loop;

class ev_timer : public ev_watcher{

public:
    ev_timer();

    void init(std::function<void(ev_loop *loop, ev_timer *w, int)> cb, double at_, double repeat_);
    void start() noexcept ;
    void set_at(double );
    double get_at() const;
    void stop() ;
    void call_back(ev_loop *loop, ev_watcher *w, int) override;

    void set_repeat(double );
    double get_repeat() const;

    std::function<void(ev_loop *loop, ev_timer *w, int)>cb;
private:
    double at;
    double repeat;
};

class ev_periodic : public ev_watcher{

public:
    ev_periodic();
    void init(std::function<void(ev_loop *loop, ev_periodic *w, int)> cb, tm* t);
    void start() noexcept ;
    void set_at(double );
    double get_at() const;
    void stop();
    void again();
    void call_back(ev_loop *loop, ev_watcher *w, int) override;
    void set_t(tm* );
    tm* get_t();

private:
    std::function<void(ev_loop *loop, ev_periodic *w, int)>cb;
    double at;
    tm* t;
};

struct periodic_cb{
    bool operator()(ev_periodic* left, ev_periodic* right) { return left->get_at() > right->get_at();}
};
struct timer_cb{
    bool operator()(ev_timer* left, ev_timer* right) {  return left->get_at() > right->get_at();}
};

class TimeManaher{
public:
    TimeManaher(){};
    static TimeManaher * GetThis();

    void timers_reify();
    void periodics_reify();
    void periodics_reschedule();

    void push(bool isev_timer, ev_watcher *w);
    ev_watcher * top(bool isev_timer);
    size_t size(bool isev_timer);

private:


    std::priority_queue<ev_periodic *, std::vector<ev_periodic *>, periodic_cb > periodic_queue;
    std::priority_queue<ev_timer *, std::vector<ev_timer *>, timer_cb >  timer_queue;
};



#endif //LIBEVCPP_EV_TIMER_H
