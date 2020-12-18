//
// Created by feng on 2020/12/8.
//

#include "ev_timer.h"
#include "utils.h"
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

    at = at+loop->mn_now; // 这里的到期时间是，创建后的时间间隔

    assert (("libev: ev_timer_start called with negative timer repeat value", get_repeat() >= 0.));

    ;
    // TODO ? 应该将++timercnt;移到这个函数的后面，否则timers的第一个元素无法赋值
    // 这就会堆吧，从索引1开始而不是0
    //++timercnt;
    loop->timer->push(this);
    ev_start (loop->timer->size());

}

void ev_timer::call_back(ev_loop *loop, void *w, int event){
    cb(loop, static_cast<ev_timer*>(w), event);
}

void ev_periodic::init(std::function<void(ev_loop *, ev_periodic *, int)> cb_, tm *t_) {
    cb = cb_;
    t = t_;
}

void ev_periodic::set_at(double at_) {
    at = at_;
}

double ev_periodic::get_at() {
    return at;
}

void ev_periodic::start(ev_loop *loop) noexcept {
    set_loop(loop);

    if (get_active())
        return;

#if EV_USE_TIMERFD
    if (loop->timerfd == -1)
        get_loop()->evtimerfd_init ();
#endif

    at = tm_to_time(t);

    loop->periodic->push(this);
    ev_start(loop->periodic->size());
}

void ev_periodic::stop() {
    ev_watcher::clear_pending();

    if (!get_active())
        return;
    ev_watcher::stop();
}

void ev_periodic::again() {
    stop();
    start(get_loop());
}

tm *ev_periodic::get_t() {
    return t;
}

void ev_periodic::set_t(tm * t_) {
    t = t_;
}

ev_periodic::ev_periodic():ev_watcher() {

}

void ev_periodic::call_back(ev_loop *loop, void *w, int event) {
    cb(loop,static_cast<ev_periodic*>(w),event);
}
