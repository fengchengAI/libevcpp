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
#include <unordered_map>
#include "watcher.h"
#include "ev_loop.h"
#include "ev_io.h"

#define EV_INOTIFY_BUFSIZE (sizeof (struct inotify_event) * 2 + NAME_MAX)

class ev_stat : public ev_watcher{
public:
    ev_stat();
    void init (std::function<void(ev_loop*, ev_stat*, int)>, std::string);

    void start();
    void stop() override;
    void stat();
    int get_wd() const;
    int get_fd() const;

    void set_wd(int wd_);
    void infy_add();
    void infy_del();
    void call_back(ev_loop *loop, ev_watcher *w, int) override;

    struct stat attr;
    struct stat prev;
private:

    std::string path;
    std::function<void(ev_loop*, ev_stat*, int)> cb;
    int fs_fd;
    int wd; /* wd for inotify, fd for kqueue */
};


class FileManaher{
public:
    ~FileManaher();
    static FileManaher * GetThis();
    FileManaher();
    int infy_newfd();
    int infy_init();
    void infy_wd(int fd, struct inotify_event *ev);
    void remove(int fd, ev_stat* w);
    void push_front(int fd, ev_stat* w);
    int get_fd() const;

private:
    int fs_fd;
    std::unordered_map<int, std::forward_list<ev_stat*> > fs_hash;
    ev_io* fs_w;
};

void stat_timer_cb (ev_loop* loop, ev_stat *w, int revents);

#endif //LIBEVCPP_EV_STAT_H
