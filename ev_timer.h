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
    void start(ev_loop *loop) noexcept ;
    void set_at(double );
    double get_at();
    void stop() ;
    void call_back(ev_loop *loop, void *w, int) override;

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

    void init(std::function<void(ev_loop *loop, ev_periodic *w, int)> cb, tm* t);
    void start(ev_loop *loop) noexcept ;
    void set_at(double );
    double get_at();
    void stop();
    void again();
    void call_back(ev_loop *loop, void *w, int) override;
    void set_t(tm* );
    tm* get_t();

private:
    std::function<void(ev_loop *loop, ev_periodic *w, int)>cb;
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


template <typename Type>
Timer<Type>::Timer(ev_loop * loop_): timer_queue([](Type* a1, Type* a2) { return a1->get_at() > a2->get_at(); })
{
    loop = loop_;
}
template <typename Type>
void Timer<Type>::push(Type * w){
    timer_queue.push(w);
}

template <typename Type>
void Timer<Type>::pop(){
    timer_queue.pop();
}

template <typename Type>
Type * Timer<Type>::top(){
    return timer_queue.top();
}

template <typename Type>
size_t Timer<Type>::size(){
    return timer_queue.size();
}

template <typename Type>
void Timer<Type>::timers_reify()
{
    //printf("ANHE_at(timers [HEAP0]%f\n",timer_queue.top()->get_at());
    //printf("mn_now%f\n",loop->mn_now);
    if(timer_queue.size() && timer_queue.top()->get_at() < loop->mn_now)  // 过期了一件事件，
    {
        do
        {
            ev_timer * temp = timer_queue.top();
            timer_queue.pop();

            loop->ev_feed_event(temp,EV_TIMER);
            if (temp->get_repeat()){
                temp->set_at(temp->get_at()+temp->get_repeat());
                timer_queue.push(temp);
            }
            //timer_queue.pop();

        }while(timer_queue.size() && timer_queue.top()->get_at() < loop->mn_now);
    }
}
template <typename Type>
void Timer<Type>::periodics_reify()
{
    //printf("ANHE_at(timers [HEAP0]%f\n",timer_queue.top()->get_at());
    //printf("mn_now%f\n",loop->mn_now);
    //std::cout<<timer_queue.top()->get_at()<<"  --  "<<loop->ev_rt_now<<std::endl;
    if(timer_queue.size() && timer_queue.top()->get_at() < loop->ev_rt_now)  // 过期了一件事件，
    {
        do
        {
            loop->ev_feed_event(timer_queue.top(),EV_PERIODIC);
            timer_queue.pop();

        }while(timer_queue.size() && timer_queue.top()->get_at() < loop->ev_rt_now);
    }
}

template <typename Type>
void Timer<Type>::periodics_reschedule()
{
    std::vector<ev_periodic *> temp;
    while(timer_queue.size()){
        temp.push_back(timer_queue.top());
        timer_queue.pop();
    }
    for(auto t:temp){
        t->set_at(tm_to_time(t->get_t()));
        timer_queue.push(t);
    }
}

#endif //LIBEVCPP_EV_TIMER_H
