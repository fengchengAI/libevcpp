//
// Created by feng on 2020/12/2.
//
#include <unistd.h>
#include <sys/timerfd.h>
#include <sys/eventfd.h>
#include "ev_loop.h"
#include "utils.h"
#include <cstdlib>
#include "ev_fdmanager.h"
#include "ev_signal.h"
#include "watcher.h"
#include "ev_stat.h"
#include "ev_fdmanager.h"
#include "watcher.h"

#include "ev_Epoll.h"
#include "ev_timer.h"
#include "ev_io.h"
#include "ev_signal.h"
#include "ev_other_watcher.h"
#define EVBREAK_RECURSE 0x80

ev_loop * ev_default_loop_ptr = nullptr;

#define EV_TSTAMP_HUGE \
 (sizeof(time_t) >= 8     ? 10000000000000.  \
   : 0 <(time_t)4294967295 ?     4294967295.  :   2147483647.)



ev_loop::ev_loop(){
    ev_default_loop_ptr = this;
    init();
};


void pipecb(ev_loop* loop, ev_io *iow, int revents)
{
    int i;
    if(revents & EV_READ)
    {
        if(loop->event_fd > 0)
        {
            uint64_t counter;
            read(loop->event_fd, &counter, sizeof(uint64_t));
        }
    }

    #if EV_ASYNC_ENABLE
    if(loop->async_pending)
    {
        loop->async_pending = 0;

        for(i = loop->asyncs.size(); i--; )
            if(loop->asyncs[i]->get_sent())
            {
                loop->asyncs[i]->set_sent(0);
                loop->ev_feed_event(dynamic_cast<ev_watcher *>(loop->asyncs.at(i)), EV_ASYNC);
            }
    }
    #endif
}



void ev_loop::init()
{
        activecnt = 0;

        ev_rt_now          = ev_time();
        mn_now             = get_clock();
        now_floor          = mn_now;
        rtmn_diff          = ev_rt_now - mn_now;

        io_blocktime       = 0.;
        timeout_blocktime  = 0.;

        async_pending      = 0;

        ev_Epoll::GetThis()->backend_init();

        event_io = new ev_io();
        event_io->init(pipecb,0,0);
        event_io->set_priority(EV_MAXPRI);

}

void ev_loop::ev_feed_event(ev_watcher *w, int revents)
{

    int pri = w->get_priority() - EV_MINPRI;

    if(w->get_pending())
        pendings[pri][w->get_pending() - 1].events |= revents;
    else
    {
        w->set_pending(pendings[pri].size()+1);
        pendings[pri].push_back({w, revents});
    }

}
void ev_loop::time_update()
{

    int i;
    double odiff = rtmn_diff;

    mn_now = get_clock();

    /* only fetch the realtime clock every 0.5*MIN_TIMEJUMP seconds */
    /* interpolate in the meantime */
    /* 当两个时间差很少的时候就不去更新 */
    if(mn_now - now_floor <  MIN_TIMEJUMP * .5)
    {
        // TODO ? 为什么是这样写，而不是用ev_rt_now = ev_time()
        ev_rt_now = rtmn_diff + mn_now;
        return;
    }

    now_floor = mn_now;
    ev_rt_now = ev_time();

    /*循环几次，然后再做出重要的决定。
    关于“ 4”的选择：如果在调用ev_time和get_clock时我们被抢占，则仅进行一次迭代是不够的。
    在这种情况下，几乎可以保证第二次呼叫成功。
    再循环几次也不会造成伤害，因为我们只是在时间跳动时才这样做，或者在极少数情况下被抢占。
    */
    for(i = 4; --i; )
    {
        double diff;
        rtmn_diff = ev_rt_now - mn_now;

        diff = odiff - rtmn_diff;  // 为什么计算两个之间的误差，理应该没什么差距的

        if((diff < 0. ? -diff : diff) < MIN_TIMEJUMP)
            return; /* all is well */

        ev_rt_now = ev_time();
        mn_now    = get_clock();
        now_floor = mn_now;
    }


    TimeManaher::GetThis()->periodics_reschedule();

}
ev_loop * ev_loop::GetThis(){
    return ev_default_loop_ptr;
}

int ev_loop::run(int flags)
{

    assert(("libev: ev_loop recursion during release detected", loop_done != EVBREAK_RECURSE));

    loop_done = EVBREAK_CANCEL;
    ev_invoke_pending();

    do  // 这里会一直循环下去
    {

        if(loop_done)
            break;

        /* update fd-related kernel structures */
        FdManaher::GetThis()->fd_reify();

        /* calculate blocking time */
        {
            double waittime  = 0.;
            double sleeptime = 0.;

            /* remember old timestamp for io_blocktime calculation */
            double prev_mn_now = mn_now;

            /* 更新时间以取消回调处理开销 */
            time_update(EV_TSTAMP_HUGE);

            if(!(flags & EVRUN_NOWAIT || !activecnt ))
            {// 这个函数主要设置waittime等待时间

                waittime = MAX_BLOCKTIME;

#if 0
//#if !EV_PERIODIC_ENABLE
                /* 没有周期但具有单调时钟，无需任何时间跳变检测，因此睡眠时间更长 */
                if(1)
                waittime = MAX_BLOCKTIME2;
#endif

                if(TimeManaher::GetThis()->size(true))
                {
                    double to = dynamic_cast<ev_timer*>(TimeManaher::GetThis()->top(true))->get_at() - mn_now;
                    if(waittime > to) waittime = to;
                }
#if 1
// #if EV_PERIODIC_ENABLE
                if(TimeManaher::GetThis()->size(false))
                {
                    double to = dynamic_cast<ev_periodic*>(TimeManaher::GetThis()->top(false))->get_at() - ev_rt_now;
                    if(waittime > to) waittime = to;
                }
#endif

                /*不要让超时将等待时间减少到timeout_blocktime以下，默认为0 */
                if(waittime < timeout_blocktime)
                    waittime = timeout_blocktime;

                /* 
                现在还有另外两种特殊情况，要么我们的定时器已经过期，所以我们就不应该睡觉，
                要么定时器很快到期，在这种情况下，我们需要等待一些事件循环后端的最短时间 。
                */
                if(waittime < ev_Epoll::GetThis()->getmintime())
                    waittime = waittime <= 0.
                               ? 0.
                               : ev_Epoll::GetThis()->getmintime();

                /* extra check because io_blocktime is commonly 0 */
                if(io_blocktime)
                {
                    sleeptime = io_blocktime -(mn_now - prev_mn_now);

                    if(sleeptime > waittime - ev_Epoll::GetThis()->getmintime())
                        sleeptime = waittime - ev_Epoll::GetThis()->getmintime();

                    if(sleeptime > 0.)
                    {
                        ev_sleep(sleeptime);
                        waittime -= sleeptime;
                    }
                }
            }


            assert((loop_done = EVBREAK_RECURSE, 1)); /* assert for side effect */

            ev_Epoll::GetThis()->backend_poll(waittime);   // 里面有epoll_wait
            assert((loop_done = EVBREAK_CANCEL, 1)); /* assert for side effect */

            /* update ev_rt_now, do magic */
            time_update(waittime + sleeptime);
        }

        /* queue pending timers and reschedule them */
        TimeManaher::GetThis()->timers_reify(); /* relative timers called last */
        TimeManaher::GetThis()->periodics_reify(); /* absolute timers called first */

        ev_invoke_pending();  // 最后的事件触发是在这里
    }
    while(
            activecnt
            && !loop_done
            && !(flags &(EVRUN_ONCE | EVRUN_NOWAIT))
    );

    if(loop_done == EVBREAK_ONE)
        loop_done = EVBREAK_CANCEL;


    return activecnt;
}


void ev_loop::ev_invoke_pending()
{

    pendingpri = NUMPRI;
    do
    {
        --pendingpri;

        while(!pendings[pendingpri].empty())
        {
            auto p = pendings[pendingpri].back();
            pendings[pendingpri].pop_back();
            p.w->set_pending(0);
            p.w->call_back(this, p.w, p.events);
        }
    }
    while(pendingpri);
}


void ev_loop::destroy() {

        if(event_io->get_active())
        {
            if(event_fd >= 0) close(event_fd);
        }



        if(ev_Epoll::GetThis()->getfd() >= 0){
            close(ev_Epoll::GetThis()->getfd());
        }

        ev_Epoll::GetThis()->destroy();


        ev_default_loop_ptr = nullptr;



    if(event_fd) {
        close(event_fd);
        delete(event_io);
        event_io= nullptr;
    }
}


void ev_loop::event_init() {
    if(!event_io->get_active()){

        event_fd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
        if(event_fd < 0 && errno == EINVAL)
            event_fd = eventfd(0, 0);

        fd_intern(event_fd);
        event_io->set_fd(event_fd);
        event_io->set_event(EV_READ);
        event_io->start();
        activecnt--;
    }
}
