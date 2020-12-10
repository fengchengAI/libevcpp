//
// Created by feng on 2020/11/28.
//

// a single header file is required
#include "ev.h"
#include "watcher.h"
#include "ev_timer.h"
#include "ev_loop.h"
#include "ev_io.h"
#include <cstdio> // for puts

// every watcher type has its own typedef'd struct
// with the name ev_TYPE


// all watcher callbacks have a similar signature
// this callback is called when data is readable on stdin
static void
stdin_cb (ev_loop* loop, ev_io *w, int revents)
{
puts ("stdin ready");
// for one-shot events, one must manually stop the watcher
// with its corresponding stop function.


// this causes all nested ev_run's to stop iterating
loop->ev_break ( EVBREAK_ALL);
}

// another callback, this time for a time-out
static void
timeout_cb (ev_loop* loop, ev_timer *w, int revents)
{
puts ("timeout");
// this causes the innermost ev_run to stop iterating
}

int main ()
{
    ev_io stdin_watcher;
    ev_timer timeout_watcher;
    // use the default event loop unless you have special needs
    struct ev_loop *loop = ev_default_loop();

    // initialise an io watcher, then start it
    // this one will watch for stdin to become readable
    stdin_watcher.init(stdin_cb, /*STDIN_FILENO*/ 0, EV_READ);
    stdin_watcher.start(loop);

    // initialise a timer watcher, then start it
    // simple non-repeating 5.5 second timeout
    timeout_watcher.init(timeout_cb, 5.5, 0.);
    timeout_watcher.start(loop);

    // now wait for events to arrive
    loop->run(0);

    // break was called, so exit
    return 0;
}