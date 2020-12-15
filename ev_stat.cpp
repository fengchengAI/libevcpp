//
// Created by feng on 2020/12/16.
//

#include "ev_stat.h"
#include <sys/stat.h>
#include "watcher.h"
/*
ev_stat::ev_stat(std::function<void(ev_loop *, ev_stat *, int)> cb_, std::string path_, double interval_):
                ev_watcher(),cb(cb_),path(path_),interval(interval_),wd(-2)
                {}
*/
/*
void ev_stat::stat()
{
    if (lstat (path, &attr) < 0)
        attr.st_nlink = 0;
    else if (!attr.st_nlink)
        attr.st_nlink = 1;
}
*/
void ev_stat::start(ev_loop *loop)
{
    if (get_active())
        return;
}
/*
ev_stat::ev_stat() {

}
*/
