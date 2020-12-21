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
#include "ev_signal.h"
#include <thread>
#include <cstdio> // for puts

void async_cb(struct ev_loop *loop, ev_timer *watcher, int revents)
{
    fprintf(stdout, "ring on\n");
}
void sigint_cb (struct ev_loop *loop, ev_signal *w, int revents)
{
    puts ( "signal ....." );
}
void child_cb (struct ev_loop *loop, ev_child *w, int revents)
{
    w->stop();
    printf ("process %d exited with status %x\n", w->rpid, w->rstatus);
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

    ev_loop *loop = ev_default_loop(EVFLAG_SIGNALFD);

    ev_child cw;
    pid_t pid = fork();
    printf("pid %d\n",pid);
    if (pid < 0)
        printf("cerr\n");
    else if (pid == 0)
    {
        sleep(5);
        exit(1);
    }else {
        cw.init(child_cb, pid, 0);
        cw.start(loop);
        loop->run(0);
        loop->destroy();
        delete(loop);
    }



    // break was called, so exit
    return 0;
}