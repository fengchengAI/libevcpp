//
// Created by feng on 2020/12/8.
//

#include "ev_timer.h"

#include <utility>
#include "utils.h"

TimeManaher * t_timer_p = nullptr;
void ev_timer::set_at(double at_){
    at = at_;
}
double ev_timer::get_at() const{
    return at;
}

void ev_timer::set_repeat(double repeat_){
    repeat = repeat_;
}
double ev_timer::get_repeat() const{
    return repeat;
}



void ev_timer::stop(){

    ev_watcher::clear_pending();

    if(!get_active())
        return;

    set_at(ev_loop::GetThis()->mn_now);

    ev_watcher::stop();

}

ev_timer::ev_timer():ev_watcher(),at(0),repeat(0)
{

}
void ev_timer::init(std::function<void(ev_loop *loop, ev_timer *w, int)> cb_, double at_, double repeat_){
    cb = std::move(cb_);
    at = at_;
    repeat = repeat_;
}
void ev_timer::start() noexcept
{

    if(get_active())
        return;

    at = at+ev_loop::GetThis()->mn_now; // 这里的到期时间是，创建后的时间间隔

    assert(("libev: ev_timer_start called with negative timer repeat value", get_repeat() >= 0.));

    TimeManaher::GetThis()->push(1, this);
    ev_start();

}

void ev_timer::call_back(ev_loop *loop, ev_watcher *w, int event){
    cb(loop, dynamic_cast<ev_timer*>(w), event);
}

void ev_periodic::init(std::function<void(ev_loop *, ev_periodic *, int)> cb_, tm *t_) {
    cb = std::move(cb_);
    t = t_;
}

void ev_periodic::set_at(double at_) {
    at = at_;
}

double ev_periodic::get_at() const {
    return at;
}

void ev_periodic::start() noexcept {
    if(get_active())
        return;

    at = tm_to_time(t);
    TimeManaher::GetThis()->push(0, this);
    ev_start();
}

void ev_periodic::stop() {
    ev_watcher::clear_pending();

    if(!get_active())
        return;
    ev_watcher::stop();
}

void ev_periodic::again() {
    stop();
    start();
}

tm *ev_periodic::get_t() {
    return t;
}

void ev_periodic::set_t(tm * t_) {
    t = t_;
}

ev_periodic::ev_periodic():ev_watcher(),at(0),t(nullptr) {

}

void ev_periodic::call_back(ev_loop *loop, ev_watcher *w, int event) {
    cb(loop, dynamic_cast<ev_periodic*>(w),event);
}


void TimeManaher::push(bool istime, ev_watcher * w){
    if (istime)
        timer_queue.push(dynamic_cast<ev_timer*>(w));
    else
        periodic_queue.push(dynamic_cast<ev_periodic*>(w));
}


ev_watcher * TimeManaher::top(bool istime){
    if (istime)
        return timer_queue.top();
    else
        return periodic_queue.top();
}

size_t TimeManaher::size(bool istime){
    if (istime)
        return timer_queue.size();
    else
        return periodic_queue.size();
}

void TimeManaher::timers_reify()
{

    if(timer_queue.size() && timer_queue.top()->get_at() < ev_loop::GetThis()->mn_now)  // 过期了一件事件，
    {
        do
        {
            ev_timer * temp = timer_queue.top();
            timer_queue.pop();

            ev_loop::GetThis()->ev_feed_event(temp, EV_TIMER);
            if (temp->get_repeat()){
                temp->set_at(temp->get_at()+temp->get_repeat());
                timer_queue.push(temp);
            }
            //timer_queue.pop();

        }while(timer_queue.size() && timer_queue.top()->get_at() < ev_loop::GetThis()->mn_now);
    }
}
void TimeManaher::periodics_reify()
{

    if(timer_queue.size() && timer_queue.top()->get_at() < ev_loop::GetThis()->ev_rt_now)  // 过期了一件事件，
    {
        do
        {
            ev_loop::GetThis()->ev_feed_event(timer_queue.top(), EV_PERIODIC);
            timer_queue.pop();

        }while(timer_queue.size() && timer_queue.top()->get_at() < ev_loop::GetThis()->ev_rt_now);
    }
}

void TimeManaher::periodics_reschedule()
{
    std::vector<ev_periodic *> temp;
    while(periodic_queue.size()){
        temp.push_back(dynamic_cast<ev_periodic*>(timer_queue.top()));
        periodic_queue.pop();
    }
    for(auto t:temp){
        t->set_at(tm_to_time(t->get_t()));
        periodic_queue.push(t);
    }
}

TimeManaher * TimeManaher::GetThis() {
    if (!t_timer_p)
        t_timer_p = new TimeManaher();
    return t_timer_p;
}
