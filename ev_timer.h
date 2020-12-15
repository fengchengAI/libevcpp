//
// Created by feng on 2020/12/8.
//

#ifndef LIBEVCPP_EV_TIMER_H
#define LIBEVCPP_EV_TIMER_H

#include <queue>
#include "watcher.h"
#include "ev_loop.h"
class ev_loop;

class ev_timer : public ev_watcher{

public:
    ev_timer();

    void init(std::function<void(ev_loop *loop, ev_timer *w, int)> cb, double at_, double repeat_);
    void start (ev_loop *loop) noexcept ;
    void set_at(double );
    double get_at();
    void stop() ;
    void call_back(ev_loop *loop, void *w, int) ;

    void set_repeat(double );
    double get_repeat();

    std::function<void(ev_loop *loop, ev_timer *w, int)>cb;
private:
    double at;
    double repeat;
};

class ev_periodic : public ev_watcher{

public:
    ev_periodic();

    void init(std::function<void(ev_loop *loop, ev_periodic *w, int)> cb, double at_, tm* t);
    void start (ev_loop *loop) noexcept ;
    void set_at(double );
    double get_at();
    void stop() ;
    void again();
    void call_back(ev_loop *loop, void *w, int) ;
    void set_t(tm* );
    tm* get_t();


    std::function<void(ev_loop *loop, ev_periodic *w, int)>cb;
private:
    double at;
    tm* t;
};
template <typename Type>
class Timer{
public:
    void periodics_reschedule();
    Timer(ev_loop *loop);
    std::priority_queue<Type *, std::vector<Type *>,  std::function<bool(Type * a1, Type * a2)> > timer_queue;

    void timers_reify();
    void periodics_reify();


    void push(Type *);
    Type * top();
    void pop();
    size_t size();

private: ev_loop *loop;
};
#endif //LIBEVCPP_EV_TIMER_H
