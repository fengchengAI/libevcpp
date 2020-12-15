//
// Created by feng on 2020/12/16.
//

#ifndef LIBEVCPP_EV_STAT_H
#define LIBEVCPP_EV_STAT_H

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "watcher.h"
#include "ev_loop.h"


class ev_stat : public ev_watcher{
public:
    ev_stat():ev_watcher(){};
    //ev_stat(std::function<void(ev_loop*, ev_stat*,int)>, std::string, double);

    void start(ev_loop* loop);
    void stop();
    //void stat();

private:
    //std::forward_list<ev_watcher* >list;
    double interval;
    std::string path;
    struct stat prev;
    struct stat attr;
    std::function<void(ev_loop*, ev_stat*, int)> cb;

    int wd; /* wd for inotify, fd for kqueue */
};


#endif //LIBEVCPP_EV_STAT_H
