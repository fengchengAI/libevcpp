//
// Created by feng on 2020/12/2.
//
#include <unistd.h>
#include "ev_loop.h"
#include "utils.h"
#include <cstdlib>
#include "anfd.h"
static ev_loop *ev_default_loop_ptr = nullptr;
static ev_loop default_loop_struct;

ev_loop *ev_default_loop (unsigned int flags = 0) noexcept
{
    if (!ev_default_loop_ptr)
    {
        ev_loop *loop = ev_default_loop_ptr = &default_loop_struct;


        loop->loop_init (flags);

    if (loop->backend )
    {
        #if EV_CHILD_ENABLE
            ev_signal_init (&childev, childcb, SIGCHLD);
            ev_set_priority (&childev, EV_MAXPRI);
            ev_signal_start (EV_A_ &childev);
            ev_unref (EV_A); /* child watcher should not keep loop alive */
        #endif
    }
    else
    ev_default_loop_ptr = nullptr;
    }

    return ev_default_loop_ptr;
}


void ev_loop::loop_init (unsigned int flags) noexcept
{
    if (!backend)
    {
        origflags = flags;

        /* pid check not overridable via env */
        curpid = getpid ();

        if (!(flags & EVFLAG_NOENV)
            && !enable_secure ()
            && getenv ("LIBEV_FLAGS"))
            flags = atoi (getenv ("LIBEV_FLAGS"));

        ev_rt_now          = ev_time ();
        mn_now             = get_clock ();
        now_floor          = mn_now;
        rtmn_diff          = ev_rt_now - mn_now;
        #if EV_FEATURE_API
        invoke_cb          = ev_invoke_pending;
        #endif

        io_blocktime       = 0.;
        timeout_blocktime  = 0.;
        backend            = 0;
        backend_fd         = -1;
        sig_pending        = 0;
        #if EV_ASYNC_ENABLE
        async_pending      = 0;
        #endif
        pipe_write_skipped = 0;
        pipe_write_wanted  = 0;
        evpipe [0]         = -1;
        evpipe [1]         = -1;
        #if EV_USE_INOTIFY
        fs_fd              = flags & EVFLAG_NOINOTIFY ? -1 : -2;
        #endif
        #if EV_USE_SIGNALFD
        sigfd              = flags & EVFLAG_SIGNALFD  ? -2 : -1;
        #endif
        #if EV_USE_TIMERFD
        timerfd            = flags & EVFLAG_NOTIMERFD ? -1 : -2;
        #endif

        if (!(flags & EVBACKEND_MASK))
        flags |= EVBACKEND_EPOLL;

        if (!backend && (flags & EVBACKEND_EPOLL   )) backend = epoll_init     (EV_A_ flags);
        mutilplexing = selectMultiplexing(EVBACKEND_EPOLL);
        mutilplexing->backend_init(this,flags);


        ev_prepare_init (&pending_w, pendingcb);

        #if EV_SIGNAL_ENABLE || EV_ASYNC_ENABLE
        ev_init (&pipe_w, pipecb);
              ev_set_priority (&pipe_w, EV_MAXPRI);
        #endif
    }
}

int ev_loop::run (int flags)
{
#if EV_FEATURE_API
    ++loop_depth;
#endif

    assert (("libev: ev_loop recursion during release detected", loop_done != EVBREAK_RECURSE));

    loop_done = EVBREAK_CANCEL;

    EV_INVOKE_PENDING; /* in case we recurse, ensure ordering stays nice and clean */

    do  // 这里会一直循环下去
    {
#if EV_VERIFY >= 2
        ev_verify (mullexing->);
#endif

        if (curpid) /* penalise the forking check even more */
            if (getpid () != curpid)
            {
                curpid = getpid ();
                postfork = 1;
            }

#if EV_FORK_ENABLE
        /* we might have forked, so queue fork handlers */
        if (postfork)
            if (forkcnt)
            {
                queue_events (EV_A_ (W *)forks, forkcnt, EV_FORK);
                EV_INVOKE_PENDING;
            }
#endif

#if EV_PREPARE_ENABLE
        /* queue prepare watchers (and execute them) */
        if (preparecnt)
        {
            queue_events (EV_A_ (W *)prepares, preparecnt, EV_PREPARE);
            EV_INVOKE_PENDING;
        }
#endif

        if (loop_done)
            break;

        /* we might have forked, so reify kernel state if necessary */
        if (postfork)
            loop_fork (EV_A);

        /* update fd-related kernel structures */
        fd_reify (EV_A);

        /* calculate blocking time */
        {
            double waittime  = 0.;
            double sleeptime = 0.;

            /* remember old timestamp for io_blocktime calculation */
            double prev_mn_now = mn_now;

            /* 更新时间以取消回调处理开销 */
            time_update (EV_A_ EV_TS_CONST (double_HUGE));

            /* from now on, we want a pipe-wake-up */
            pipe_write_wanted = 1;

            ECB_MEMORY_FENCE; /* make sure pipe_write_wanted is visible before we check for potential skips */

            if (ecb_expect_true (!(flags & EVRUN_NOWAIT || idleall || !activecnt || pipe_write_skipped)))
            {// 这个函数主要设置waittime等待时间

                waittime = EV_TS_CONST (MAX_BLOCKTIME);

#if EV_USE_TIMERFD
                /* sleep a lot longer when we can reliably detect timejumps */
            if (ecb_expect_true (timerfd >= 0))
              waittime = EV_TS_CONST (MAX_BLOCKTIME2);
#endif
#if !EV_PERIODIC_ENABLE
                /* 没有周期但具有单调时钟，无需任何时间跳变检测，因此睡眠时间更长 */
            if (ecb_expect_true (have_monotonic))
              waittime = EV_TS_CONST (MAX_BLOCKTIME2);
#endif

                if (timercnt)
                {
                    double to = ANHE_at (timers [HEAP0]) - mn_now;
                    if (waittime > to) waittime = to;
                }

#if EV_PERIODIC_ENABLE
                if (periodiccnt)
                {
                    double to = ANHE_at (periodics [HEAP0]) - ev_rt_now;
                    if (waittime > to) waittime = to;
                }
#endif

                /*不要让超时将等待时间减少到timeout_blocktime以下，默认为0 */
                if (waittime < timeout_blocktime)
                    waittime = timeout_blocktime;

                /* 
                现在还有另外两种特殊情况，要么我们的定时器已经过期，所以我们就不应该睡觉，
                要么定时器很快到期，在这种情况下，我们需要等待一些事件循环后端的最短时间 。
                */
                if (waittime < backend_mintime)
                    waittime = waittime <= EV_TS_CONST (0.)
                               ? EV_TS_CONST (0.)
                               : backend_mintime;

                /* extra check because io_blocktime is commonly 0 */
                if (io_blocktime)
                {
                    sleeptime = io_blocktime - (mn_now - prev_mn_now);

                    if (sleeptime > waittime - backend_mintime)
                        sleeptime = waittime - backend_mintime;

                    if (ecb_expect_true (sleeptime > EV_TS_CONST (0.)))
                    {
                        ev_sleep (sleeptime);
                        waittime -= sleeptime;
                    }
                }
            }

#if EV_FEATURE_API
            ++loop_count;
#endif
            assert ((loop_done = EVBREAK_RECURSE, 1)); /* assert for side effect */
            printf("befoe_backend_poll%f\n",get_clock());
            printf("waittime%f\n",waittime);
            backend_poll (EV_A_ waittime);   // 里面有epoll_wait
            assert ((loop_done = EVBREAK_CANCEL, 1)); /* assert for side effect */
            printf("after_backend_poll%f\n",get_clock());

            pipe_write_wanted = 0; /* just an optimisation, no fence needed */

            ECB_MEMORY_FENCE_ACQUIRE;
            if (pipe_write_skipped)
            {
                assert (("libev: pipe_w not active, but pipe not written", ev_is_active (&pipe_w)));
                ev_feed_event (EV_A_ &pipe_w, EV_CUSTOM);
            }

            /* update ev_rt_now, do magic */
            time_update (EV_A_ waittime + sleeptime);
        }

        /* queue pending timers and reschedule them */
        timers_reify (EV_A); /* relative timers called last */
#if EV_PERIODIC_ENABLE
        periodics_reify (EV_A); /* absolute timers called first */
#endif

#if EV_IDLE_ENABLE
        /* queue idle watchers unless other events are pending */
        idle_reify (EV_A);
#endif

#if EV_CHECK_ENABLE
        /* queue check watchers, to be executed first */
        if (checkcnt)
            queue_events (EV_A_ (W *)checks, checkcnt, EV_CHECK);
#endif

        EV_INVOKE_PENDING;  // 最后的事件触发是在这里
    }
    while (ecb_expect_true (
            activecnt
            && !loop_done
            && !(flags & (EVRUN_ONCE | EVRUN_NOWAIT))
    ));

    if (loop_done == EVBREAK_ONE)
        loop_done = EVBREAK_CANCEL;

#if EV_FEATURE_API
    --loop_depth;
#endif

    return activecnt;
}