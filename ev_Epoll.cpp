//
// Created by feng on 2020/12/2.
//

/*
 * libev epoll fd activity backend
 *
 * Copyright(c) 2007,2008,2009,2010,2011,2016,2017,2019 Marc Alexander Lehmann <libev@schmorp.de>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modifica-
 * tion, are permitted provided that the following conditions are met:
 *
 *   1.  Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *
 *   2.  Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MER-
 * CHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPE-
 * CIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES(INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT(INCLUDING NEGLIGENCE OR OTH-
 * ERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * the GNU General Public License("GPL") version 2 or any later version,
 * in which case the provisions of the GPL are applicable instead of
 * the above. If you wish to allow the use of your version of this file
 * only under the terms of the GPL and not to allow others to use your
 * version of this file under the BSD license, indicate your decision
 * by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL. If you do not delete the
 * provisions above, a recipient may use your version of this file under
 * either the BSD or the GPL.
 */

/*
 * general notes about epoll:
 *
 * a) epoll silently removes fds from the fd set. as nothing tells us
 *    that an fd has been removed otherwise, we have to continually
 *    "rearm" fds that we suspect *might* have changed(same
 *    problem with kqueue, but much less costly there).
 * b) the fact that ADD != MOD creates a lot of extra syscalls due to a)
 *    and seems not to have any advantage.
 * c) the inability to handle fork or file descriptors(think dup)
 *    limits the applicability over poll, so this is not a generic
 *    poll replacement.
 * d) epoll doesn't work the same as select with many file descriptors
 *   (such as files). while not critical, no other advanced interface
 *    seems to share this(rather non-unixy) limitation.
 * e) epoll claims to be embeddable, but in practise you never get
 *    a ready event for the epoll fd(broken: <=2.6.26, working: >=2.6.32).
 * f) epoll_ctl returning EPERM means the fd is always ready.
 *
 * lots of "weird code" and complication handling in this file is due
 * to these design problems with epoll, as we try very hard to avoid
 * epoll_ctl syscalls for common usage patterns and handle the breakage
 * ensuing from receiving events for closed and otherwise long gone
 * file descriptors.
 */

#include <sys/epoll.h>
#include <fcntl.h>
#include <unistd.h>
#include "ev_loop.h"
#include "ev_Epoll.h"
#include "watcher.h"
#include "ev.h"
#include "ev_fdmanager.h"
#include "utils.h"

ev_Epoll * t_ev_epoll = nullptr;
ev_Epoll * ev_Epoll::GetThis(){
    if (!t_ev_epoll)
        t_ev_epoll = new ev_Epoll();
    return t_ev_epoll;
}

class FdManaher;
class ev_loop;
const int  EV_EMASK_EPERM = 0x80;

void ev_Epoll::backend_modify(int fd, int oev, int nev) {
    struct epoll_event ev;
    unsigned char oldmask;

    /*
    我们通过假设fd无论如何都是错误的，在这里忽略它来处理EPOLL_CTL_DEL，
    我们必须处理epoll_poll中的虚假事件。
    如果再次添加fd，则尝试添加它，如果失败，则假定它仍具有相同的事件掩码。
    */
    if(!nev)
        return;
    FdManaher::GetThis()->get_anfd(fd).events  = nev;

    /* store the generation counter in the upper 32 bits, the fd in the lower 32 bits */
    ev.data.fd = fd;
    ev.events =(nev & EV_READ ? EPOLLIN : 0)
                |(nev & EV_WRITE ? EPOLLOUT : 0);

    if(!epoll_ctl(backend_fd, oldmask != nev ? EPOLL_CTL_MOD : EPOLL_CTL_ADD, fd, &ev))
        return;
    do{
        if(errno == ENOENT) {
            /* if ENOENT then the fd went away, so try to do the right thing */

            if(!epoll_ctl(backend_fd, EPOLL_CTL_ADD, fd, &ev))
                return;
        } else if(errno == EEXIST) {
            /* EEXIST means we ignored a previous DEL, but the fd is still active */
            /* if the kernel mask is the same as the( mask, we assume it hasn't changed */
            if(oev == nev) {
                break;
            }
            if(!epoll_ctl(backend_fd, EPOLL_CTL_MOD, fd, &ev))
                return;
        } else if(errno == EPERM) {
            /* EPERM means the fd is always ready, but epoll is too snobbish */
            /* to handle it, unlike select or poll. */
            std::cerr<<"目标文件fd不支持epoll"<<std::endl;

            return;
        } else
            assert(("libev: I/O watcher with invalid fd found in epoll_ctl", errno != EBADF && errno != ELOOP &&
                                                                              errno != EINVAL));
    }while(0);

    // 没有调用成功
    FdManaher::GetThis()->fd_kill(fd);
}

void ev_Epoll::backend_poll(double timeout)
{
    int i;
    int eventcnt;


    /* epoll wait times cannot be larger than(LONG_MAX - 999UL) / HZ msecs, which is below */
    /* the default libev max wait time, however. */
    eventcnt = epoll_wait(backend_fd, epoll_events, epoll_eventmax, timeout * 1e3 + 0.9999);

    if(eventcnt < 0)
    {
        if(errno != EINTR)
            std::cerr<<"(libev) epoll_wait"<<std::endl;
        return;
    }

    for(i = 0; i < eventcnt; ++i)
    {
        struct epoll_event *ev = epoll_events + i;

        int fd = ev->data.fd; /* mask out the lower 32 bits */
        int want = FdManaher::GetThis()->get_anfd(fd).events;
        int got  =(ev->events &(EPOLLOUT | EPOLLERR | EPOLLHUP) ? EV_WRITE : 0)
                   |(ev->events &(EPOLLIN  | EPOLLERR | EPOLLHUP) ? EV_READ  : 0);

        /*
         检查虚假通知。
         这只会在egen更新中找到虚假通知，其他虚假通知将通过epoll_ctl找到，
         下面我们假设fd始终在范围内，因为我们从不缩小anfds数组,
         */

        if((got & ~want))
        {
            std::cout<<"未监听的事件类型被触发"<<std::endl;
//            FdManaher::GetThis()->get_anfd(fd).emask = want;
//
//            /*
//             我们收到了一个事件，但对它不感兴趣，请尝试使用mod或del这种巧合情况，
//             因为当我们不再对fds感兴趣时，而且当我们从另一个进程中收到有关fds的虚假通知时，我们也不愿取消注册fds。
//             这在上面用gencounter检查部分处理（==我们的fd不是事件fd），在这里部分是当epoll_ctl返回错误（==一个孩子有fd但我们关闭了它）。
//            注意：对于诸如POLLHUP之类的事件，我们不知道它是指EV_READ还是EV_WRITE，我们可能会发出冗余的EPOLL_CTL_MOD调用。
//             */
//            ev->events =(want & EV_READ  ? EPOLLIN  : 0)
//                         |(want & EV_WRITE ? EPOLLOUT : 0);
//
//            /* pre-2.6.9 kernels require a non-null pointer with EPOLL_CTL_DEL, */
//            /* which is fortunately easy to do for us. */
//            if(epoll_ctl(backend_fd, want ? EPOLL_CTL_MOD : EPOLL_CTL_DEL, fd, ev))
//            {
//                loop->postfork |= 2; /* an error occurred, recreate kernel state */
//                continue;
//            }
        }
        FdManaher::GetThis()->fd_event_nocheck(fd, got);
    }

    /* if the receive array was full, increase its size */
    if(eventcnt == epoll_eventmax)
    {
        free(epoll_events);
        epoll_events = nullptr;

        epoll_eventmax = array_nextsize(sizeof(struct epoll_event), epoll_eventmax, epoll_eventmax + 1);
        epoll_events =(struct epoll_event *)malloc(sizeof(struct epoll_event) * epoll_eventmax);
    }

    /* 现在可以在epoll失败而select可以工作的情况下为所有fds合成事件，... */
    /*
    for(i = epoll_epermcnt; i--; )
    {
        int fd = epoll_eperms[i];
        unsigned char events = FdManaher::GetThis()->get_anfd(fd).events &(EV_READ | EV_WRITE);

        if(FdManaher::GetThis()->get_anfd(fd).emask & EV_EMASK_EPERM && events)
            FdManaher::GetThis()->fd_event(fd,events);
        else
        {
            epoll_eperms[i] = epoll_eperms[--epoll_epermcnt];
            FdManaher::GetThis()->get_anfd(fd).emask = 0;
        }
    }
     */
}

int ev_Epoll::epoll_epoll_create()
{
    int fd;
    fd = epoll_create1(EPOLL_CLOEXEC);

    if(fd < 0 &&(errno == EINVAL || errno == ENOSYS))
    {
        fd = epoll_create(256);

        if(fd >= 0)
            fcntl(fd, F_SETFD, FD_CLOEXEC);
    }

    return fd;
}


void ev_Epoll::backend_init()
{
    backend_fd = epoll_epoll_create();
    if(backend_fd  < 0)
        return ;

    backend_mintime = 1e-3; /* epoll does sometimes return early, this is just to avoid the worst */

    epoll_eventmax = 64; /* initial number of events receivable per poll */
    epoll_events =(struct epoll_event *)malloc(sizeof(struct epoll_event) * epoll_eventmax);
}

void ev_Epoll::destroy()
{
    free(epoll_events);
    epoll_events = nullptr;
    free(epoll_eperms);
    epoll_eperms = nullptr;
}
