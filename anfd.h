//
// Created by feng on 2020/12/2.
//

#ifndef LIBEVCPP_ANFD_H
#define LIBEVCPP_ANFD_H

#include <vector>
#include <map>
#include "ev_loop.h"
#include "ev_epoll.h"
#include "ev.h"
#include "ev_io.h"
#include "watcher.h"
class Multiplexing;
class ev_loop;
class ev_io;

struct ANFD
{
    std::forward_list<ev_io*> list;
    unsigned char events; /* the events watched for */
    unsigned char reify;  /* flag set when this ANFD needs reification (EV_ANFD_REIFY, EV__IOFDSET) */
    //reify会被设置为w->events & EV__IOFDSET | EV_ANFD_REIFY

    unsigned char emask;  /* some backends store the actual kernel mask in here */
    unsigned char eflags; /* flags field for use by backends */
#if EV_USE_EPOLL
    unsigned int egen;    /* generation counter to counter epoll bugs */
#endif
};

class FdWatcher {
public:

    FdWatcher(ev_loop * loop_){
        loop = loop_;
    };
    ANFD &get_anfd(int index){
        return anfd[index];
    }
    void fd_event (int fd, int revents)
    {
        ANFD *anfd = anfds + fd;

        if (ecb_expect_true (!anfd[fd].reify))
            fd_event_nocheck (fd, revents);
    }
    void fd_event_nocheck (int fd, int revents)
    {

        for(auto i : anfd[fd].list){
            int ev = *i->events & revents;
            if (ev)
                ev_feed_event (EV_A_ *w, ev);
        }
    }
    void fd_kill (int fd)
    {
        ev_io *w;

        while ((w = (ev_io *)anfds [fd].head))
        {
            ev_io_stop (EV_A_ w);
            ev_feed_event (EV_A_ (W)w, EV_ERROR | EV_READ | EV_WRITE);
        }
    }

    void fd_change (int fd, int flags)
    {
        unsigned char reify = anfd[fd].reify;
        anfd[fd].reify = reify | flags;
        if (!reify)
        {
            fdchanges.push_back(fd);
        }
    }

    void fd_reify ()
    {
        int i;
        int changecnt = fdchanges.size();
        /*
        大多数后端不会修改backend_modfiy中的fdchanges列表。 io_uring除外，
        它具有固定大小的缓冲区，这可能会迫使我们处理backend_modify中的事件，从而导致fdchanges被修改，从而可能导致无限循环。
        为了避免这种情况，我们不会动态处理在fd_reify期间添加的fds。
        这意味着对于这些后端，在轮询期间fdchangecnt可能不为零，这必须使它们不会阻塞。
        为了不给其他后端带来太多负担，此细节需要在后端中处理。
         */

        // 事实上这里就是修改epoll_ctl监视的事件类型，比如两个ev_io类型数据，都使用了文件描述符为4的文件，
        // 但是第一个ev_io监视的事read事件，第二个监视的是write事件，于是将两个事件取或，再修改对4的监视条件
        // 也是在这里将监视事件注册到epoll中的。
        while (!fdchanges.empty())
        {
            int fd_ = fdchanges.back();
            fdchanges.pop_back();
            unsigned char o_events = anfd[fd_].events;
            unsigned char o_reify  = anfd[fd_].reify;

            anfd[fd_].reify = 0;

            /*if (ecb_expect_true (o_reify & EV_ANFD_REIFY)) probably a deoptimisation */
            {
                anfd[fd_].events = 0;
                for(auto w : anfd[fd_].list){
                    anfd[fd_].events |= (unsigned char)w->get_event();
                }

                if (o_events != anfd[fd_].events)
                    o_reify = EV__IOFDSET; /* actually |= */
            }

            if (o_reify & EV__IOFDSET)
                loop->mullexing->backend_modify(loop, fd_, o_events, anfd[fd_].events);
        }

        /* 通常，fdchangecnt不会更改。 如果有，则添加了新的fds。
        这是一种罕见的情况（请参见此函数中的开始注释），因此我们将它们复制到前端，并希望后端能够处理这种情况。
         */

    }

    std::map<int,ANFD> anfd;
private:
    ev_loop * loop;
    std::vector<int> fdchanges;
    std::vector<int> fdkills;

};

#endif //LIBEVCPP_ANFD_H
