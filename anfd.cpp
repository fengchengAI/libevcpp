//
// Created by feng on 2020/12/2.
//

#include "anfd.h"

FdWatcher::FdWatcher(ev_loop * loop_){
    loop = loop_;
}

ANFD & FdWatcher::get_anfd(int index){
    return anfd[index];
}

void FdWatcher::fd_event (int fd, int revents)
{
    if (!anfd[fd].reify)
        fd_event_nocheck (fd, revents);
}

void FdWatcher::fd_event_nocheck (int fd, int revents)
{
    for(auto i : anfd[fd].list){
        int ev = i->get_event() & revents;
        if (ev)
            loop->ev_feed_event (dynamic_cast<ev_watcher*>(i), ev);
    }
}

void FdWatcher::push_front(int fd, ev_io* w){
    anfd[fd].list.push_front(w);
}
void FdWatcher::remove(int fd, ev_io* w){
    anfd[fd].list.remove(w);
}

size_t FdWatcher::size(){
    return anfd.size();
}

void FdWatcher::fd_kill (int fd)
{
    for(auto i : anfd[fd].list){
        i->stop();
        loop->ev_feed_event(dynamic_cast<ev_watcher*>(i),EV_ERROR | EV_READ | EV_WRITE);
    }
}

void FdWatcher::fd_change (int fd, int flags)
{
    unsigned char reify = anfd[fd].reify;
    anfd[fd].reify = reify | flags;
    if (!reify)
    {
        fdchanges.push_back(fd);
    }
}

void FdWatcher::fd_reify ()
{
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
            loop->mutilplexing->backend_modify(loop, fd_, o_events, anfd[fd_].events);
    }

    /* 通常，fdchangecnt不会更改。 如果有，则添加了新的fds。
    这是一种罕见的情况（请参见此函数中的开始注释），因此我们将它们复制到前端，并希望后端能够处理这种情况。
    */
}


void FdWatcher::fd_rearm_all ()
{
    for (auto &i:anfd){
        if(i.second.events)
            i.second.events = 0;
            i.second.emask = 0;
            fd_event(i.first, EV__IOFDSET | EV_ANFD_REIFY);
    }
}