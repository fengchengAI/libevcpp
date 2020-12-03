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

Timer::Timer(ev_loop * loop_){
    loop = loop_;
}
void Timer::timers_reify ()
{
    printf("ANHE_at (timers [HEAP0]%f\n",ANHE_at (timers [HEAP0]));
    printf("mn_now%f\n",mn_now);

    if (timercnt && ANHE_at (timers [HEAP0]) < mn_now)  // 过期了一件事件，
    {
        do
        {
            ev_timer *w = (ev_timer *)ANHE_w (timers [HEAP0]);

            /*assert (("libev: inactive timer on timer heap detected", ev_is_active (w)));*/

            /* first reschedule or stop timer */
            if (w->repeat)
            {
                ev_at (w) += w->repeat;
                if (ev_at (w) < mn_now)
                    ev_at (w) = mn_now;

                assert (("libev: negative ev_timer repeat value found while processing timers", w->repeat > EV_TS_CONST (0.)));

                ANHE_at_cache (timers [HEAP0]);
                downheap (timers, timercnt, HEAP0);
            }
            else
                ev_timer_stop (EV_A_ w); /* nonrepeating: stop timer */

            EV_FREQUENT_CHECK;
            feed_reverse (EV_A_ (W)w);
        }
        while (timercnt && ANHE_at (timers [HEAP0]) < mn_now);

        feed_reverse_done (EV_A_ EV_TIMER);
    }
}

void ev_timer::start (ev_loop &loop) noexcept
{
    if (get_active())
        return;

    set_at(get_at()+loop.mn_now);  // 这里的到期时间是，创建后的时间间隔

    assert (("libev: ev_timer_start called with negative timer repeat value", w->repeat >= 0.));

    ;
    // TODO ? 应该将++timercnt;移到这个函数的后面，否则timers的第一个元素无法赋值
    // 这就会堆吧，从索引1开始而不是0
    ++timercnt;
    timer_queue
    ev_start (timercnt + HEAP0 - 1);
    printf("%d\n",sizeof(timers));
    array_needsize (ANHE, timers, timermax, ev_active (w) + 1, array_needsize_noinit);
    ANHE_w (timers [ev_active (w)]) = (WT)w;
    ANHE_at_cache (timers [ev_active (w)]);
    upheap (timers, ev_active (w));

    /*assert (("libev: internal timer heap corruption", timers [ev_active (w)] == (WT)w));*/
}