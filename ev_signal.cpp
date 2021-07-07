//
// Created by feng on 2020/12/8.
//

#include "ev_signal.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "ev_loop.h"
#include "utils.h"
#include "ev_other_watcher.h"
#include "ev_io.h"
Signal * t_signalfd_p = nullptr;
void ev_signal::set_signum(int sig_)
{
    signum = sig_;
}

int ev_signal::get_signum()
{
    return signum;
}

void ev_signal::init(std::function<void(ev_loop *loop, ev_signal *w, int)> cb_, int sig_)
{
    cb = cb_;
    signum = sig_;
}

void ev_feed_signal_event(ev_loop* loop, int signum)
{
    if(signum <= 0 || signum >= NSIG)
        return;

    --signum;

    signals[signum].pending = 0;
    for(auto i : signals [signum].head)
        loop->ev_feed_event(i,EV_SIGNAL);
}


void sigfdcb(ev_loop*loop, ev_io *iow, int revents)
{

    struct signalfd_siginfo si[2], *sip; /* these structs are big */

    for(;;)
    {
        ssize_t res = read(iow->get_fd(), si, sizeof(si));

        /* not ISO-C, as res might be -1, but works with SuS */
        for(sip = si;(char *)sip <(char *)si + res; ++sip)
            ev_feed_signal_event(loop, sip->ssi_signo);

        if(res <(ssize_t)sizeof(si))
            break;
    }
}
void ev_signal::start()
{

    // sigs = Signal::GetThis();
    if(get_active())
        return;

    assert(("libev: ev_signal_start called with illegal signal number", signum > 0 && signum < NSIG));

    // assert(("libev: a signal must not be attached to two different loops",
    //         !signals[signum - 1].loop || signals[signum - 1].loop == loop));

    // signals[signum - 1].loop = loop;

    if(Signal::GetThis()->get_fd() >= 0)
    {
        Signal::GetThis()->sigaddset(signum);
        sigprocmask(SIG_BLOCK, Signal::GetThis()->fd_set_ptr(), 0);
        signalfd(Signal::GetThis()->get_fd(), Signal::GetThis()->fd_set_ptr(), 0);
    }

    ev_start();
    signals[signum - 1].head.push_front(this);
}


void ev_signal::stop()
{
    clear_pending();

    if(!get_active())
        return;

    signals[signum-1].head.remove(this);
    ev_watcher::stop();

    if(signals[signum - 1].head.empty())
    {

#if EV_MULTIPLICITY
        signals[signum - 1].loop = nullptr; /* unattach from signal */
#endif

        if(Signal::GetThis()->get_fd() >= 0)
        {
            sigset_t ss;

            sigemptyset(&ss);
            sigaddset(&ss, signum);
            sigdelset(Signal::GetThis()->fd_set_ptr(), signum);

            signalfd(Signal::GetThis()->get_fd(), Signal::GetThis()->fd_set_ptr(), 0);
            sigprocmask(SIG_UNBLOCK, &ss, 0);
        }
    }
}

void ev_signal::call_back(ev_loop *loop, ev_watcher *w, int sig_ )
{
    cb(loop, dynamic_cast<ev_signal*>(w), sig_);
}

ev_signal::ev_signal() :ev_watcher(),signum(-1){

}

ev_signal::~ev_signal(){

}

Signal::Signal() :sigfd_w(nullptr)
{
    //loop = loop_;
    sigfd = signalfd(-1, &sigfd_set, SFD_NONBLOCK | SFD_CLOEXEC);
    if(sigfd < 0 && errno == EINVAL)
        sigfd = signalfd(-1, &sigfd_set, 0); /* retry without flags */

    if(sigfd >= 0)
    {
        fd_intern(sigfd); /* doing it twice will not hurt */

        sigemptyset(&sigfd_set);
        sigfd_w = new ev_io();
        sigfd_w->init(sigfdcb, sigfd, EV_READ);
        sigfd_w->set_priority(EV_MAXPRI);
        sigfd_w->start();
        ev_loop::GetThis()->activecnt--;
    }
}

int Signal::get_fd()
{
    return sigfd;
}

void Signal::sigaddset(int signum)
{   // 为什么这里可以运行多次
    if(::sigaddset(&sigfd_set, signum)<0)
    {
        perror("sigemptyset");
    }
}

sigset_t *Signal::fd_set_ptr()
{
    return &sigfd_set;
}

Signal::~Signal()
{
    delete(sigfd_w);
    sigfd_w = nullptr;
}

Signal *Signal::GetThis() {
    return t_signalfd_p;
}
