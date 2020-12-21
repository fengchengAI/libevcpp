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

    #if EV_MULTIPLICITY
    /* it is permissible to try to feed a signal to the wrong loop */
    /* or, likely more useful, feeding a signal nobody is waiting for */

    if(signals[signum].loop != loop)
        return;
    #endif

    signals[signum].pending = 0;
    for(auto i : signals [signum].head)
        loop->ev_feed_event(i,EV_SIGNAL);
}

void child_reap(ev_loop* loop, int chain, int pid, int status)
{
    int traced = WIFSTOPPED(status) || WIFCONTINUED(status);
    for(auto w : childs.at(chain))
    {
        if((w->pid == pid || !w->pid)
            &&(!traced ||(w->flags & 1)))
        {
            w->set_priority(EV_MAXPRI);
            w->rpid = pid;
            w->rstatus = status;
            loop->ev_feed_event(w,EV_CHILD);
        }
    }
}

void sigfdcb(ev_loop*loop, ev_io *iow, int revents)
{
    // TODO 在child事件下，为什么会进入这里？
    std::cout<<"sigfdcb "<<std::endl;
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
void ev_signal::start(ev_loop *loop)
{

    set_loop(loop);
    sigs = get_loop()->sigs;
    if(get_active())
        return;

    assert(("libev: ev_signal_start called with illegal signal number", signum > 0 && signum < NSIG));

    assert(("libev: a signal must not be attached to two different loops",
            !signals[signum - 1].loop || signals[signum - 1].loop == loop));

    signals[signum - 1].loop = loop;

    if(sigs->get_fd() >= 0)
    {
        /* TODO: check .head */
        sigs->sigaddset(signum);
       // int aa = (*(sigs->fd_set_ptr())).__val[0];
        sigprocmask(SIG_BLOCK, sigs->fd_set_ptr(), 0);
        signalfd(sigs->get_fd(), sigs->fd_set_ptr(), 0);
    }

    ev_start(1);
    signals[signum - 1].head.push_front(this);
}

void childcb(ev_loop* loop, ev_signal * w, int revents)
{
    std::cout<<"childcb"<<std::endl;
    int pid, status;

    /* some systems define WCONTINUED but then fail to support it(linux 2.4) */
    if(0 >=(pid = waitpid(-1, &status, WNOHANG | WUNTRACED | WCONTINUED)))
        if(!WCONTINUED
        || errno != EINVAL
        || 0 >=(pid = waitpid(-1, &status, WNOHANG | WUNTRACED)))
            return;

    /* make sure we are called again until all children have been reaped */
    /* we need to do it this way so that the callback gets called before we continue */
    // TODO? 下面是什么
    //loop->ev_feed_event(w,EV_SIGNAL);
    child_reap(loop, pid, pid, status);
    /*
    if((EV_PID_HASHSIZE) > 1)
        w->child_reap(0, pid, status); // this might trigger a watcher twice, but feed_event catches that
    */
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

        if(sigs->get_fd() >= 0)
        {
            sigset_t ss;

            sigemptyset(&ss);
            sigaddset(&ss, signum);
            sigdelset(sigs->fd_set_ptr(), signum);

            signalfd(sigs->get_fd(), sigs->fd_set_ptr(), 0);
            sigprocmask(SIG_UNBLOCK, &ss, 0);
        }
    }
}

void ev_signal::call_back(ev_loop *loop, void *w, int sig_ )
{
    cb(loop, static_cast<ev_signal*>(w), sig_);
}

Signal::Signal(ev_loop *loop_) :sigfd(0), sigfd_w(nullptr)
{
    loop = loop_;
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
        sigfd_w->start(loop_);
        loop_->activecnt--;
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
