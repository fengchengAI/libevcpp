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

void stat_cb(struct ev_loop *loop, ev_stat* w,int revents)
{
    std::cout<<"file change"<<std::endl;
    if(w->attr.st_nlink){
        std::cout<<"current size "<<(long)w->attr.st_size<<std::endl;
    }
}
void async_cb(struct ev_loop *loop, ev_timer *watcher, int revents)
{
    fprintf(stdout, "ring on\n");
}
void sigint_cb (struct ev_loop *loop, ev_signal *w, int revents)
{
    puts ( "signal ....." );
}
void io_cb (struct ev_loop *loop, ev_io *w, int revents)
{
    printf ("input ready\n");
    w->stop();
}
int main ()
{


    ev_loop *loop = ev_default_loop(EVFLAG_SIGNALFD);

    ev_io t;
    t.init(io_cb,0 ,EV_READ);
    t.start(loop);
        loop->run(0);
        loop->destroy();
        return 0;
}