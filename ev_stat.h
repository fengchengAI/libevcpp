//
// Created by feng on 2020/12/16.
//

#ifndef LIBEVCPP_EV_STAT_H
#define LIBEVCPP_EV_STAT_H

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/inotify.h>
#include "climits"
#include <array>
#include <vector>
#include <map>
#include "watcher.h"
#include "ev_loop.h"
#define DEF_STAT_INTERVAL  5.0074891
#define NFS_STAT_INTERVAL 30.1074891 /* for filesystems potentially failing inotify */
#define MIN_STAT_INTERVAL  0.1074891
#define EV_INOTIFY_BUFSIZE (sizeof (struct inotify_event) * 2 + NAME_MAX)

class ev_stat : public ev_watcher{
public:

    ev_stat(std::function<void(ev_loop*, ev_stat*,int)>, std::string);

    void start(ev_loop* loop);
    void stop();
    void stat();
    int get_wd();
    int get_fd();

    void set_wd(int wd_);
    void infy_add();
    void infy_del();
    void call_back(ev_loop *loop, void *w, int) override;

    struct stat attr;
    struct stat prev;
private:
    std::forward_list<ev_watcher* >list;
    //double interval;
    const std::string path;
    File_Stat *file_stat;
    std::function<void(ev_loop*, ev_stat*, int)> cb;
    int fs_fd;

    int wd; /* wd for inotify, fd for kqueue */
};



class File_Stat{
public:

    explicit File_Stat(ev_loop * loop_);
    int infy_newfd();
    int infy_init();
    void infy_wd(int fd, struct inotify_event *ev);
    void remove(int fd, ev_io* w);
    void push_front(int fd, ev_io* w);
    size_t size();
    int get_fd();
    ev_io *get_ev_io();

    int fs_fd;

    std::map<int, std::forward_list<ev_stat*>> fs_hash ;
    ev_io* fs_w;

    ev_loop * loop;


};

void stat_timer_cb (ev_loop* loop, ev_stat *w, int revents);

#endif //LIBEVCPP_EV_STAT_H
