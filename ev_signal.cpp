//
// Created by feng on 2020/12/8.
//

#include "ev_signal.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

void ev_signal::set_signum(int sig_){
    signum = sig_;
}
int ev_signal::get_signum(){
    return signum;
}
void ev_signal::init(std::function<void(ev_loop &loop, ev_watcher *w, int)> cb, int sig_)
{
    ev_watcher::init(cb);
    signum = sig_;
}

void ev_feed_signal_event (ev_loop* loop, int signum)
{
    ev_watcher_list w;

    if (signum <= 0 || signum >= NSIG)
        return;

    --signum;

    #if EV_MULTIPLICITY
    /* it is permissible to try to feed a signal to the wrong loop */
    /* or, likely more useful, feeding a signal nobody is waiting for */

    if (signals [signum].loop != loop)
        return;
    #endif

    signals [signum].pending = 0;
    for (auto &i : signals [signum].head)
        loop->ev_feed_event(i,EV_SIGNAL);
}

void sigfdcb (ev_loop*loop, ev_io *iow, int revents)
{
    struct signalfd_siginfo si[2], *sip; /* these structs are big */

    for (;;)
    {
        ssize_t res = read (loop->sigfd, si, sizeof (si));

        /* not ISO-C, as res might be -1, but works with SuS */
        for (sip = si; (char *)sip < (char *)si + res; ++sip)
            ev_feed_signal_event (loop, sip->ssi_signo);

        if (res < (ssize_t)sizeof (si))
            break;
    }
}
void ev_signal::start(ev_loop &loop){
    if (get_active())
        return;

    assert (("libev: ev_signal_start called with illegal signal number", signum > 0 && signum < NSIG));

    assert (("libev: a signal must not be attached to two different loops",
            !signals[get_signum() - 1].loop || signals[signum - 1].loop == &loop));

    signals[get_signum() - 1].loop = &loop;

    // TODO 这里使用的是signalfd
    if (loop.sigfd == -2)
    {
      loop.sigfd = signalfd (-1, &loop.sigfd_set, SFD_NONBLOCK | SFD_CLOEXEC);
      if (loop.sigfd < 0 && errno == EINVAL)
        loop.sigfd = signalfd (-1, &loop.sigfd_set, 0); /* retry without flags */

      if (loop.sigfd >= 0)
        {
          fd_intern (loop.sigfd); /* doing it twice will not hurt */

          sigemptyset (&loop.sigfd_set);

          loop.sigfd_w.init(sigfdcb, loop.sigfd, EV_READ);
          ev_io_init (&loop.sigfd_w, sigfdcb, loop.sigfd, EV_READ);
          ev_set_priority (&loop.sigfd_w, EV_MAXPRI);
          loop.sigfd_w.start(loop);
          loop.activecnt--;
          //ev_unref (EV_A); /* signalfd watcher should not keep loop alive */
        }
    }

  if (loop.sigfd >= 0)
    {
      /* TODO: check .head */
      sigaddset (&loop.sigfd_set, signum);
      sigprocmask (SIG_BLOCK, &loop.sigfd_set, 0);

      signalfd (loop.sigfd, &loop.sigfd_set, 0);
    }

    ev_start (1);
    signals [signum - 1].head.push_front(this);
}

void ev_signal::childcb (int revents)
{
    int pid, status;

    /* some systems define WCONTINUED but then fail to support it (linux 2.4) */
    if (0 >= (pid = waitpid (-1, &status, WNOHANG | WUNTRACED | WCONTINUED)))
        if (!WCONTINUED
        || errno != EINVAL
        || 0 >= (pid = waitpid (-1, &status, WNOHANG | WUNTRACED)))
            return;

    /* make sure we are called again until all children have been reaped */
    /* we need to do it this way so that the callback gets called before we continue */
    ev_feed_event (EV_A_ (W)sw, EV_SIGNAL);
    get_loop()->ev_feed_event(this,EV_SIGNAL);
    child_reap (EV_A_ pid, pid, status);
    if ((EV_PID_HASHSIZE) > 1)
    child_reap (EV_A_ 0, pid, status); /* this might trigger a watcher twice, but feed_event catches that */
}