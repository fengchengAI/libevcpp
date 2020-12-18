//
// Created by feng on 2020/11/28.
//

// a single header file is required
#include "ev.h"
#include "watcher.h"
#include "ev_timer.h"
#include "ev_loop.h"
#include "ev_io.h"
#include <sys/time.h>
#include "utils.h"
#include "ev_other_watcher.h"
#include "ev_stat.h"
#include <thread>
#include <cstdio> // for puts


 void async_cb(struct ev_loop *loop, ev_periodic *watcher, int revents)
{
    fprintf(stdout, "ring on\n");
}


int main ()
{
    /*
    ev_io stdin_watcher;
    ev_timer timeout_watcher;
    // use the default event loop unless you have special needs
    struct ev_loop *loop = ev_default_loop(EVFLAG_SIGNALFD);

    // initialise an io watcher, then start it
    // this one will watch for stdin to become readable
    stdin_watcher.start(loop);

    // initialise a timer watcher, then start it
    // simple non-repeating 5.5 second timeout
    timeout_watcher.init(timeout_cb, 5.5, 0.);
    timeout_watcher.start(loop);
    */
    // now wait for events to arrive
    auto t = time(0);
    tm *tt = localtime(&t);

    double dd = tm_to_time (tt);
    tt->tm_sec += 30;
    ev_loop *loop = ev_default_loop(EVFLAG_SIGNALFD);


    ev_periodic periodic_watcher;

    periodic_watcher.init(async_cb,tt);
    periodic_watcher.start(loop);
    loop->run(0);


    // break was called, so exit
    return 0;
}