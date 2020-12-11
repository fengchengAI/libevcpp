//
// Created by feng on 2020/12/8.
//

#ifndef LIBEVCPP_EV_TIMER_H
#define LIBEVCPP_EV_TIMER_H

#include <queue>
#include "watcher.h"

class ev_timer : public ev_watcher{

public:
    ev_timer();

    void init(std::function<void(ev_loop *loop, ev_timer *w, int)> cb, double at_, double repeat_);
    void start (ev_loop *loop) noexcept ;
    void set_at(double );
    double get_at();
    void stop() ;
    void call_back(ev_loop *loop, ev_timer *w, int) ;

    void set_repeat(double );
    double get_repeat();

    std::function<void(ev_loop *loop, ev_timer *w, int)>cb;
private:
    double at;
    double repeat;
};

/*
//  ev_watcher_time是用来做堆的。
struct ev_watcher_time{
    int active;
    int pending;
    int priority;
    void *data;
    std::function<void(ev_loop &loop, ev_watcher_time *w, int)>cb;
    double at;
};
*/
/*
struct cmp{
    bool operator ()( ev_timer * a, ev_timer * b ) {//返回true，a的优先级大于b
        //x大的排在队前部；x相同时，y大的排在队前部
        return a->get_at() > b->get_at();
    }
};
*/
class Timer{
public:
    Timer(ev_loop *loop);
    std::priority_queue<ev_timer *, std::vector<ev_timer *>,  std::function<bool(ev_timer * a1, ev_timer * a2)> > timer_queue;

    void timers_reify ();

   // auto cmp =

    void push(ev_timer *);
    ev_timer * top();
    void pop();
    size_t size();

private: ev_loop *loop;
};
#endif //LIBEVCPP_EV_TIMER_H
