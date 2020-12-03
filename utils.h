//
// Created by feng on 2020/12/2.
//

#ifndef LIBEVCPP_UTILS_H
#define LIBEVCPP_UTILS_H
#include <bits/types.h>
#include <sys/time.h>
#include <time.h>

#include <unistd.h>

class noncopyable {
protected:
    noncopyable() = default;
    virtual ~noncopyable() = default;

    noncopyable(const noncopyable &) = delete;
    noncopyable &operator=(const noncopyable &) = delete;
};

int enable_secure (void)
{

    return getuid () != geteuid ()
           || getgid () != getegid ();
}

double ev_time (void) noexcept
{
    struct timeval tv;
    gettimeofday (&tv, 0);
    return tv.tv_sec + tv.tv_usec * 1e-6;
}

double get_clock (void)
{
    struct timespec ts;
    clock_gettime (CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

void ev_sleep (double delay) noexcept
{
    if (delay > 0.)
    {
        struct timespec ts;
        ts.tv_sec = (long)delay;
        ts.tv_nsec = (long)((delay - ts.tv_sec) * 1e9);
        nanosleep (&ts, 0);
    }
}

void array_needsize(type,base,cur,cnt,init)	{

    if (ecb_expect_false ((cnt) > (cur)))
    {
        ecb_unused int ocur_ = (cur);
        (base) = (type *)array_realloc
                (sizeof (type), (base), &(cur), (cnt));
        init ((base), ocur_, ((cur) - ocur_));
    }
}

#endif //LIBEVCPP_UTILS_H
