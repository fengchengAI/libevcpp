//
// Created by feng on 2020/12/8.
//

#include "ev_timer.h"
void ev_timer::set_at(double at_){
    at = at_;
}
double ev_timer::get_at(){
    return at;
}

void ev_timer::set_repeat(double repeat_){
    repeat = repeat_;
}
double ev_timer::get_repeat(){
    return repeat;
}

Timer::Timer(ev_loop * loop_): timer_queue( [](ev_timer * a1, ev_timer * a2) { return a1->get_at() > a2->get_at(); })
{
    loop = loop_;
}

void Timer::push(ev_timer * w){
    timer_queue.push(w);
}

void Timer::pop(){
    timer_queue.pop();
}

ev_timer * Timer::top(){
    return timer_queue.top();
}

size_t Timer::size(){
    return timer_queue.size();
}

void Timer::timers_reify ()
{
    printf("ANHE_at (timers [HEAP0]%f\n",timer_queue.top()->get_at());
    printf("mn_now%f\n",loop->mn_now);
    if (timer_queue.size() && timer_queue.top()->get_at() < loop->mn_now)  // 过期了一件事件，
    {
        do
        {
            if(timer_queue.top()->get_repeat())
            {
                auto t = timer_queue.top();
                t->set_at(t->get_at()+t->get_repeat());
                if(t->get_at()<loop->mn_now)
                    t->set_at(loop->mn_now);
                assert (("libev: negative ev_timer repeat value found while processing timers", timer_queue.top()->get_repeat() > 0.));
                timer_queue.push(t);
            }else
            {
                timer_queue.top()->stop();
                //timer_queue.pop();
            }
            loop->ev_feed_event(timer_queue.top(),EV_TIMER);
            timer_queue.pop();

        }while (timer_queue.size() && timer_queue.top()->get_at() < loop->mn_now);
    }
}
void ev_timer::stop(){

    ev_watcher::clear_pending();

    if (!get_active())
        return;

    set_at(get_at()-get_loop()->mn_now);

    ev_watcher::stop();

}

ev_timer::ev_timer():ev_watcher(),at(0),repeat(0)
{

}
void ev_timer::init(std::function<void(ev_loop *loop, ev_timer *w, int)> cb_, double at_, double repeat_){
    cb = cb_;
    at = at_;
    repeat = repeat_;
}
void ev_timer::start (ev_loop *loop) noexcept
{
    set_loop(loop);

    if (get_active())
        return;

    set_at(get_at()+loop->mn_now);  // 这里的到期时间是，创建后的时间间隔

    assert (("libev: ev_timer_start called with negative timer repeat value", get_repeat() >= 0.));

    ;
    // TODO ? 应该将++timercnt;移到这个函数的后面，否则timers的第一个元素无法赋值
    // 这就会堆吧，从索引1开始而不是0
    //++timercnt;
    loop->timer->push(this);
    ev_start (loop->timer->size());

}

void ev_timer::call_back(ev_loop *loop, ev_timer *w, int event){
    cb(loop, w, event);
}
