//
// Created by feng on 2020/12/2.
//
#include <unistd.h>
#include <sys/timerfd.h>
#include <sys/eventfd.h>
#include "ev_loop.h"
#include "utils.h"
#include <cstdlib>
#include "anfd.h"
#include "ev_signal.h"
#include "watcher.h"
#include "ev_stat.h"
#include "anfd.h"
#include "watcher.h"

#include "ev_epoll.h"
#include "ev_timer.h"
#include "ev_io.h"
#include "ev_signal.h"
#include "ev_other_watcher.h"
#define EVBREAK_RECURSE 0x80

ev_loop * ev_default_loop_ptr = nullptr;
void pendingcb(ev_loop *loop, ev_watcher *w, int revents)
{
}



#define EV_TSTAMP_HUGE \
 (sizeof(time_t) >= 8     ? 10000000000000.  \
   : 0 <(time_t)4294967295 ?     4294967295.  :   2147483647.)



ev_loop *ev_default_loop(unsigned int flags )
{
    if(!ev_default_loop_ptr)
    {
        ev_loop *loop  = ev_default_loop_ptr = new ev_loop();

        loop->loop_init(flags);

        #if EV_CHILD_ENABLE
            // 监视子进程退出(实际上是一个ev_signal事件，监视SIGCHLD)
            childev = new ev_signal();
            childev->init(childcb, SIGCHLD);
            childev->set_priority(EV_MAXPRI);
            childev->start(loop);
            loop->activecnt--;
        #endif

    }

    return ev_default_loop_ptr;
}

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
void ev_loop::ev_break(int how)
{
    loop_done = how;
}

void ev_loop::loop_init(unsigned int flags) noexcept
{
        activecnt = 0;
        origflags = flags;
        idleall = 0;
        /* pid check not overridable via env */
        curpid = getpid();

        if(!(flags & EVFLAG_NOENV)
            && !enable_secure()
            && getenv("LIBEV_FLAGS"))
            flags = atoi(getenv("LIBEV_FLAGS"));

        ev_rt_now          = ev_time();
        mn_now             = get_clock();
        now_floor          = mn_now;
        rtmn_diff          = ev_rt_now - mn_now;
        #if EV_FEATURE_API
        /*
        invoke_cb          = [this](){
                                ev_invoke_pending();
                            };
        */
        #endif

        io_blocktime       = 0.;
        timeout_blocktime  = 0.;
        backend            = 0;
        //sig_pending        = 0;
        #if EV_ASYNC_ENABLE
        async_pending      = 0;
        #endif
        //pipe_write_skipped = 0;
        //pipe_write_wanted  = 0;
        event_fd = -1;

        fdwtcher = new FdWatcher(this);


        #if EV_USE_INOTIFY
        file_stat = new File_Stat(this);
        //file_stat->fs_fd   = flags & EVFLAG_NOINOTIFY ? -1 : -2;
        #endif
        #if EV_USE_SIGNALFD
        sigs = new Signal(this);
        #endif
        #if EV_USE_TIMERFD
        timerfd = -1;
        timerfd_w = new ev_io();
        #endif

        if(!(flags & EVBACKEND_MASK))
        flags |= EVBACKEND_EPOLL;

        if(!backend &&(flags & EVBACKEND_EPOLL   ))
            backend = EVBACKEND_EPOLL;
        mutilplexing = selectMultiplexing(EVBACKEND_EPOLL);
        mutilplexing->backend_init(this,flags);

        file_stat = new File_Stat(this);
        timer = new Timer<ev_timer>(this);
        periodic = new Timer<ev_periodic>(this);
        //pending_w = new ev_watcher();
        //pending_w->init(pendingcb);

        #if EV_ASYNC_ENABLE
        event_io = new ev_io();
        event_io->init(pipecb,0,0);
        event_io->set_priority(EV_MAXPRI);
        #endif

}

void ev_loop::queue_events( std::vector<ev_watcher *>events, int type)
{
    for(auto w : events)
        ev_feed_event(w, type);
}

void ev_loop::ev_feed_event(ev_watcher *w, int revents) noexcept
{

    int pri = w->get_priority() - EV_MINPRI;

    if(w->get_pending())
        pendings[pri][w->get_pending() - 1].events |= revents;
    else
    {
        w->set_pending(pendings[pri].size()+1);
        pendings[pri].push_back({w,revents});
    }

    pendingpri = NUMPRI - 1;   // TODO ?  这里为什么-1；
}
void ev_loop::time_update(double max_block)
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

        /* no timer adjustment, as the monotonic clock doesn't jump */
        /* timers_reschedule(EV_A_ rtmn_diff - odiff) */
# if EV_PERIODIC_ENABLE
    periodic->periodics_reschedule();
# endif

}
ev_loop::ev_loop(){}

int ev_loop::run(int flags)
{
#if EV_FEATURE_API
    ++loop_depth;
#endif

    assert(("libev: ev_loop recursion during release detected", loop_done != EVBREAK_RECURSE));

    loop_done = EVBREAK_CANCEL;
    ev_invoke_pending();

    do  // 这里会一直循环下去
    {

#if EV_VERIFY >= 2
        //ev_verify(mullexing->);
#endif

        if(curpid) /* penalise the forking check even more */
            if(getpid() != curpid)
            {
                curpid = getpid();
                postfork = 1;
            }

#if EV_FORK_ENABLE
        /* we might have forked, so queue fork handlers */
        if(postfork)
            if(!forks.empty())
            {
                queue_events(forks, EV_FORK);
                ev_invoke_pending();
            }
#endif

#if EV_PREPARE_ENABLE
        /* queue prepare watchers(and execute them) */
        if(!prepares.empty())
        {
            queue_events(prepares, EV_PREPARE);
            ev_invoke_pending();
        }
#endif

        if(loop_done)
            break;

        /* we might have forked, so reify kernel state if necessary */

        if(postfork)
            loop_fork();

        /* update fd-related kernel structures */
        fdwtcher->fd_reify();

        /* calculate blocking time */
        {
            double waittime  = 0.;
            double sleeptime = 0.;

            /* remember old timestamp for io_blocktime calculation */
            double prev_mn_now = mn_now;

            /* 更新时间以取消回调处理开销 */
            time_update(EV_TSTAMP_HUGE);

            /* from now on, we want a pipe-wake-up */
            //pipe_write_wanted = 1;

            if(!(flags & EVRUN_NOWAIT || idleall || !activecnt ))
            {// 这个函数主要设置waittime等待时间

                waittime = MAX_BLOCKTIME;

#if EV_USE_TIMERFD
                /* sleep a lot longer when we can reliably detect timejumps */
            if(timerfd >= 0)
              waittime = MAX_BLOCKTIME2;
#endif
#if !EV_PERIODIC_ENABLE
                /* 没有周期但具有单调时钟，无需任何时间跳变检测，因此睡眠时间更长 */
            if(1)
              waittime = MAX_BLOCKTIME2;
#endif

                if(timer->size())
                {
                    double to = timer->top()->get_at() - mn_now;
                    if(waittime > to) waittime = to;
                }
#if EV_PERIODIC_ENABLE
                if(periodic->size())
                {
                    double to = periodic->top()->get_at() - ev_rt_now;
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
                if(waittime < mutilplexing->backend_mintime)
                    waittime = waittime <= 0.
                               ? 0.
                               : mutilplexing->backend_mintime;

                /* extra check because io_blocktime is commonly 0 */
                if(io_blocktime)
                {
                    sleeptime = io_blocktime -(mn_now - prev_mn_now);

                    if(sleeptime > waittime - mutilplexing->backend_mintime)
                        sleeptime = waittime - mutilplexing->backend_mintime;

                    if(sleeptime > 0.)
                    {
                        ev_sleep(sleeptime);
                        waittime -= sleeptime;
                    }
                }
            }

#if EV_FEATURE_API
            ++loop_count;
#endif
            assert((loop_done = EVBREAK_RECURSE, 1)); /* assert for side effect */
            //printf("befoe_backend_poll%f\n", get_clock());
            printf("waittime%f\n",waittime);
            mutilplexing->backend_poll(this, waittime);   // 里面有epoll_wait
            assert((loop_done = EVBREAK_CANCEL, 1)); /* assert for side effect */
            //printf("after_backend_poll%f\n", get_clock());

            //pipe_write_wanted = 0; /* just an optimisation, no fence needed */
            /*
            if(pipe_write_skipped)
            {
                assert(("libev: pipe_w not active, but pipe not written", event_io->get_active()));
                ev_feed_event(event_io, EV_CUSTOM);
            }
            */
            /* update ev_rt_now, do magic */
            time_update(waittime + sleeptime);
        }

        /* queue pending timers and reschedule them */
        timer->timers_reify(); /* relative timers called last */
#if EV_PERIODIC_ENABLE
        periodic->periodics_reify(); /* absolute timers called first */
#endif

#if EV_IDLE_ENABLE
        /* queue idle watchers unless other events are pending */
        idle_reify();
#endif

#if EV_CHECK_ENABLE
        /* queue check watchers, to be executed first */
        if(!checks.empty())
            queue_events(checks, EV_CHECK);
#endif

        ev_invoke_pending();  // 最后的事件触发是在这里
    }
    while(
            activecnt
            && !loop_done
            && !(flags &(EVRUN_ONCE | EVRUN_NOWAIT))
    );

    if(loop_done == EVBREAK_ONE)
        loop_done = EVBREAK_CANCEL;

#if EV_FEATURE_API
    --loop_depth;
#endif

    return activecnt;
}


void ev_loop::ev_invoke_pending()
{

    pendingpri = NUMPRI;
    //printf("%d\n",pendingpri);
    do
    {
        --pendingpri;

        /* pendingpri possibly gets modified in the inner loop */
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

void timerfdcb(ev_loop* loop, ev_io *iow, int revents)
{
    struct itimerspec its = { 0 };

    its.it_value.tv_sec = loop->ev_rt_now +(int)MAX_BLOCKTIME2;
    timerfd_settime(loop->timerfd, TFD_TIMER_ABSTIME | TFD_TIMER_CANCEL_ON_SET, &its, 0);

    loop->ev_rt_now = ev_time();
    /* periodics_reschedule only needs ev_rt_now */
    /* but maybe in the future we want the full treatment. */
    /*
    now_floor = EV_TS_CONST(0.);
    time_update(EV_A_ EV_TSTAMP_HUGE);
    */
    #if EV_PERIODIC_ENABLE
    loop->periodic->periodics_reschedule();
    #endif
}

void ev_loop::evtimerfd_init()
{
    if(!timerfd_w->get_active())
    {
        timerfd = timerfd_create(CLOCK_REALTIME, TFD_NONBLOCK | TFD_CLOEXEC);

        if(timerfd >= 0)
        {
            fd_intern(timerfd); /* just to be sure */
            timerfd_w->init(timerfdcb, timerfd, EV_READ);
            timerfd_w->set_priority(EV_MINPRI);
            timerfd_w->start(this);
            activecnt--;

            /*(re-) arm timer */
            timerfdcb(this, 0, 0);
        }
    }
}

void ev_loop::idle_reify()
{
    if(idleall)
    {
        int pri;

        for(pri = NUMPRI; pri--; )
        {
            if(pendings[pri].size())
                break;
            if(idles[pri].size())
            {
                queue_events(idles [pri],  EV_IDLE);
                break;
            }
        }
    }
}

void ev_loop::destroy() {

    #if EV_CLEANUP_ENABLE
        /* queue cleanup watchers(and execute them) */
        /*
        if(ecb_expect_false(cleanupcnt))
        {
            queue_events(EV_A_(W *)cleanups, cleanupcnt, EV_CLEANUP);
            EV_INVOKE_PENDING;
        }
        */
    #endif

    #if EV_CHILD_ENABLE
        if(this==ev_default_loop_ptr && childev->get_active())
        {
            ++activecnt;
            childev->stop();
        }
    #endif

        if(event_io->get_active())
        {
            if(event_fd >= 0) close(event_fd);
        }

    #if EV_USE_SIGNALFD
        delete(sigs);
        sigs = nullptr;

    #endif

    #if EV_USE_TIMERFD
        if(timerfd_w->get_active())
            close(timerfd);
    #endif

    #if EV_USE_INOTIFY
        if(file_stat->get_fd() >= 0)
        close(file_stat->get_fd());
        delete(file_stat);
        file_stat = nullptr;
    #endif

        if(mutilplexing->backend_fd >= 0){
            close(mutilplexing->backend_fd);
        }

    #if EV_USE_EPOLL
        if(backend == EVBACKEND_EPOLL   ) mutilplexing->destroy();
        delete(mutilplexing);
        mutilplexing = nullptr;
    #endif

        delete(timer);
    #if EV_PERIODIC_ENABLE
        delete(periodic);
    #endif

        backend = 0;

    #if EV_MULTIPLICITY
        if(this==ev_default_loop_ptr)
    #endif
        ev_default_loop_ptr = nullptr;
    #if EV_MULTIPLICITY
        else
            delete(this);
    #endif

    delete(fdwtcher);
    fdwtcher = nullptr;

#if    EV_ASYNC_ENABLE
    if(event_fd) {
        close(event_fd);
        delete(event_io);
        event_io= nullptr;
    }
#endif
}

void ev_loop::loop_fork() {

#if EV_USE_EPOLL
    if(backend == EVBACKEND_EPOLL) mutilplexing->fork(this);
#endif
#if EV_USE_INOTIFY
    //infy_fork(EV_A);
#endif

    if(postfork != 2)
    {


#if EV_USE_TIMERFD
        if(timerfd_w->get_active())
        {
            activecnt++;
            timerfd_w->stop();

            close(timerfd);
            timerfd = -1;

            evtimerfd_init();
            /* reschedule periodics, in case we missed something */
            ev_feed_event(timerfd_w, EV_CUSTOM);
        }
#endif

#if EV_SIGNAL_ENABLE || EV_ASYNC_ENABLE
        if(event_io->get_active())
        {
            /* pipe_write_wanted must be false now, so modifying fd vars should be safe */

            activecnt++;
            event_io->stop();


            event_init();
            /* iterate over everything, in case we missed something before */
            ev_feed_event(event_io, EV_CUSTOM);
        }
#endif
    }

    postfork = 0;
}

void ev_loop::event_init() {
    if(!event_io->get_active()){

        int temp;
        temp = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
        if(temp < 0 && errno == EINVAL)
            temp = eventfd(0, 0);
        if(event_fd < 0)
            event_fd = temp;
        else{
            dup2(temp, event_fd);
            close(temp);
        }

        fd_intern(event_fd);
        event_io->set_fd(event_fd);
        event_io->set_event(EV_READ);
        event_io->start(this);
        activecnt--;
    }
}
