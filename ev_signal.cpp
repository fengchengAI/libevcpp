//
// Created by feng on 2020/12/8.
//

#include "ev_signal.h"

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

void ev_signal::start(ev_loop &loop){
    if (get_active())
        return;

    assert (("libev: ev_signal_start called with illegal signal number", w->signum > 0 && w->signum < EV_NSIG));

#if EV_MULTIPLICITY
    assert (("libev: a signal must not be attached to two different loops",
            !signals [get_signum() - 1].loop || signals [w->signum - 1].loop == loop));

    signals [get_signum() - 1].loop = &loop;
#endif

//#if EV_USE_SIGNALFD
    if (loop.sigfd == -2)
    {
      sigfd = signalfd (-1, &sigfd_set, SFD_NONBLOCK | SFD_CLOEXEC);
      if (sigfd < 0 && errno == EINVAL)
        sigfd = signalfd (-1, &sigfd_set, 0); /* retry without flags */

      if (sigfd >= 0)
        {
          fd_intern (sigfd); /* doing it twice will not hurt */

          sigemptyset (&sigfd_set);

          ev_io_init (&sigfd_w, sigfdcb, sigfd, EV_READ);
          ev_set_priority (&sigfd_w, EV_MAXPRI);
          ev_io_start (EV_A_ &sigfd_w);
          ev_unref (EV_A); /* signalfd watcher should not keep loop alive */
        }
    }

  if (sigfd >= 0)
    {
      /* TODO: check .head */
      sigaddset (&sigfd_set, w->signum);
      sigprocmask (SIG_BLOCK, &sigfd_set, 0);

      signalfd (sigfd, &sigfd_set, 0);
    }
//#endif

    ev_start (EV_A_ (W)w, 1);
    wlist_add (&signals [w->signum - 1].head, (WL)w);

    if (!((WL)w)->next)
# if EV_USE_SIGNALFD
        if (sigfd < 0) /*TODO*/
# endif
    {

        struct sigaction sa;

        evpipe_init (EV_A);

        sa.sa_handler = ev_sighandler;
        sigfillset (&sa.sa_mask);
        sa.sa_flags = SA_RESTART; /* if restarting works we save one iteration */
        sigaction (w->signum, &sa, 0);

        if (origflags & EVFLAG_NOSIGMASK)
        {
            sigemptyset (&sa.sa_mask);
            sigaddset (&sa.sa_mask, w->signum);
            sigprocmask (SIG_UNBLOCK, &sa.sa_mask, 0);
        }
    }

    EV_FREQUENT_CHECK;
}