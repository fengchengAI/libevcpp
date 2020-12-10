//
// Created by feng on 2020/12/11.
//
#include "utils.h"
#include <bits/types.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctime>
int enable_secure (void)
{

    return getuid () != geteuid ()
           || getgid () != getegid ();
}

double ev_time (void)
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

void ev_sleep (double delay)
{
    if (delay > 0.)
    {
        struct timespec ts;
        ts.tv_sec = (long)delay;
        ts.tv_nsec = (long)((delay - ts.tv_sec) * 1e9);
        nanosleep (&ts, 0);
    }
}
#define MALLOC_ROUND 4096 /* prefer to allocate in chunks of this size, must be 2**n and >> 4 longs */

int array_nextsize (int elem, int cur, int cnt)
{
    int ncur = cur + 1;

    do
        ncur <<= 1;   // ncur乘以2； ，等于说每次增加两个
    while (cnt > ncur);

    // 当elem * ncur > MALLOC_ROUND - sizeof (void *) * 4每次当内存不够时直接加1024,而不是加2
    /* if size is large, round to MALLOC_ROUND - 4 * longs to accommodate malloc overhead */
    if (elem * ncur > MALLOC_ROUND - sizeof (void *) * 4)
    {
        ncur *= elem;
        ncur = (ncur + elem + (MALLOC_ROUND - 1) + sizeof (void *) * 4) & ~(MALLOC_ROUND - 1);
        ncur = ncur - sizeof (void *) * 4;
        ncur /= elem;
    }

    return ncur;
}

/* used to prepare libev internal fd's */
/* this is not fork-safe */
void fd_intern (int fd)
{
    fcntl (fd, F_SETFD, FD_CLOEXEC);
    fcntl (fd, F_SETFL, O_NONBLOCK);
}

