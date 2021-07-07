//
// Created by feng on 2020/12/2.
//

#ifndef LIBEVCPP_EV_FDMANAGER_H
#define LIBEVCPP_EV_FDMANAGER_H

#include <vector>
#include <unordered_map>
#include <unordered_set>

#include "ev_loop.h"
#include "ev_Epoll.h"
#include "ev.h"
#include "ev_io.h"
#include "watcher.h"


struct ANFD
{
    ANFD():events(0), newevent(0){}
    std::forward_list<ev_io* > list;  // 允许多个ev_io监听同一个fd， TODO 这样会不会没有意义。
    unsigned char events; // list对应所有事件的事件类型的或运算, 表示已经添加监听的类型，
    unsigned char newevent;  // 表示在一次添加ev_io中，需要的类型，也就是有可能要通过epoll_ctl修改的类型。

};

class FdManaher{
public:
    static FdManaher * GetThis();
    FdManaher();
    ANFD &get_anfd(int index);
    void fd_event_nocheck(int fd, int revents);
    void fd_kill(int fd);

    void fd_change(int fd, int flags);
    void fd_reify();
    void remove(int fd, ev_io* w);
    void push_front(int fd, ev_io* w);
    size_t size();
private:
    std::unordered_map<int, ANFD> anfd; //因为一个fd，可能对应多个事件，eg，多个ev_io使用stdin。
    std::unordered_set<int> fdchanges;  // 存放的是需要被修改权限的fd

};

#endif //LIBEVCPP_EV_FDMANAGER_H
