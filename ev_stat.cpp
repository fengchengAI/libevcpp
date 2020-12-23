//
// Created by feng on 2020/12/16.
//

#include "ev_stat.h"
#include <sys/stat.h>
#include "watcher.h"
#include "ev_io.h"
#include <cstring>
#include <utility>

#include "utils.h"

void stat_timer_cb(ev_loop* loop, ev_stat *w, int revents)
{

    struct stat prev = w->attr;
    w->stat();

    /* memcmp doesn't work on netbsd, they.... do stuff to their struct stat */
    if(
        prev.st_dev      != w->attr.st_dev
        || prev.st_ino   != w->attr.st_ino
        || prev.st_mode  != w->attr.st_mode
        || prev.st_nlink != w->attr.st_nlink
        || prev.st_uid   != w->attr.st_uid
        || prev.st_gid   != w->attr.st_gid
        || prev.st_rdev  != w->attr.st_rdev
        || prev.st_size  != w->attr.st_size
        || prev.st_atime != w->attr.st_atime
        || prev.st_mtime != w->attr.st_mtime
        || prev.st_ctime != w->attr.st_ctime
        )
    {
    /* we only update w->prev on actual differences */
    /* in case we test more often than invoke the callback, */
    /* to ensure that prev is always different to attr */
        w->prev = prev;
        /*
        #if EV_USE_INOTIFY
        if(w->get_fd() >= 0)
        {
            w->infy_del();
            w->infy_add();
            w->stat();
        }
        #endif
        */

        loop->ev_feed_event( w, EV_STAT);
    }
}

void ev_stat::stat()
{
    if(lstat(path.c_str(), &attr) < 0)
        attr.st_nlink = 0;
    else if(!attr.st_nlink)
        attr.st_nlink = 1;
}
void ev_stat::call_back(ev_loop *loop, void *w, int event){
    cb(loop, static_cast<ev_stat *>(w), event);
}

void ev_stat::start(ev_loop *loop)
{
    set_loop(loop);
    file_stat = get_loop()->file_stat;
    if(get_active())
        return;
    ::ev_stat::stat();

    assert(("ev_stat depend on linux INOTIFY ",EV_USE_INOTIFY));

    fs_fd = file_stat->get_fd();
    if(fs_fd >= 0)
        infy_add();

    ev_start(1);
}

ev_stat::ev_stat():ev_watcher()
{}

void ev_stat::init(std::function<void(ev_loop*, ev_stat*,int)> cb_, std::string str)
{
    cb = std::move(cb_);
    path = std::move(str);
}


File_Stat::File_Stat(ev_loop *loop_)
{
    loop = loop_;
    fs_w  = new ev_io();
    infy_init();
}

int File_Stat::infy_newfd()
{
#if defined IN_CLOEXEC && defined IN_NONBLOCK
    int fd = inotify_init1(IN_CLOEXEC | IN_NONBLOCK);
    if(fd >= 0)
        return fd;
#endif
    return inotify_init();
}

void infy_cb(ev_loop *loop, ev_io *w, int revents)
{
    char buf [EV_INOTIFY_BUFSIZE];
    int ofs;
    int len = read(w->get_fd(), buf, sizeof(buf));

    for(ofs = 0; ofs < len; )
    {
        struct inotify_event *ev =(struct inotify_event *)(buf + ofs);
        loop->file_stat->infy_wd(ev->wd, ev);
        ofs += sizeof(struct inotify_event) + ev->len;
    }
}

int File_Stat::infy_init()
{
    fs_fd = infy_newfd();

    if(fs_fd >= 0)
    {
        fd_intern(fs_fd);
        fs_w->init(infy_cb, fs_fd, EV_READ);
        fs_w->set_priority(EV_MAXPRI);
        fs_w->start(loop);
        --loop->activecnt;
    }
    return fs_fd;
}

int File_Stat::get_fd() const {
    return fs_fd;
}


void File_Stat::infy_wd(int fd, struct inotify_event *ev)
{

    //std::vector<ev_stat*> temp;
    for(auto w : fs_hash.at(fd))
    {
        if(w->get_wd() ==fd || fd ==-1 )
        {
            if(ev->mask &(IN_IGNORED | IN_UNMOUNT | IN_DELETE_SELF))
            {
                //temp.push_back(w);
                w->infy_add();  // 这里一般因为文件已经删除，于是重新添加，此时的wd不等于这个fd，所以不担心在范围循环中对容量做修改
            }
            stat_timer_cb(loop, w, 0);
        }
    }
}

File_Stat::~File_Stat() {
    delete(fs_w);
    fs_w = nullptr;
}

void File_Stat::push_front(int fd, ev_stat *w) {
    fs_hash[fd].push_front(w);
}

void File_Stat::remove(int fd, ev_stat *w) {
    fs_hash[fd].remove(w);
}

void ev_stat::infy_add()
{
    wd = inotify_add_watch(fs_fd, path.c_str(),
                               IN_ATTRIB | IN_DELETE_SELF | IN_MOVE_SELF | IN_MODIFY
                               | IN_CREATE | IN_DELETE | IN_MOVED_FROM | IN_MOVED_TO
                               | IN_DONT_FOLLOW | IN_MASK_ADD);

    if(wd < 0)
    {
        /* can't use inotify, continue to stat */

        /* if path is not there, monitor some parent directory for speedup hints */
        /* note that exceeding the hardcoded path limit is not a correctness issue, */
        /* but an efficiency issue only */
        if((errno == ENOENT || errno == EACCES) && path.size() < 4096)
        {
            char path [4096];
            strcpy(path, this->path.c_str());

            do
            {
                int mask = IN_MASK_ADD | IN_DELETE_SELF | IN_MOVE_SELF
                           |(errno == EACCES ? IN_ATTRIB : IN_CREATE | IN_MOVED_TO);

                char *pend = strrchr(path, '/');

                if(!pend || pend == path)
                    break;

                *pend = 0;
                wd = inotify_add_watch(fs_fd, path, mask);
            }
            while(wd < 0 &&(errno == ENOENT || errno == EACCES));
        }
    }

    if(wd >= 0)
        file_stat->push_front(wd,this);

}

int ev_stat::get_wd() const {
    return wd;
}

void ev_stat::set_wd(int wd_) {
    wd = wd_;
}

int ev_stat::get_fd() const {
    return fs_fd;
}

void ev_stat::infy_del() {
    int slot;
    int wd = this->wd;

    if(wd < 0)
        return;

    this->wd = -2;
    slot = wd &((EV_INOTIFY_HASHSIZE) - 1);
    file_stat->remove(slot,this);

    /* remove this watcher, if others are watching it, they will rearm */
    inotify_rm_watch(fs_fd, wd);
}

void ev_stat::stop() {
    clear_pending();
    if(!get_active())
        return;

#if EV_USE_INOTIFY
    infy_del();
#endif

    ev_watcher::stop();
}
