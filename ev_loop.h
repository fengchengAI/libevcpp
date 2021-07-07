//
// Created by feng on 2020/12/2.
//

#ifndef LIBEVCPP_EV_LOOP_H
#define LIBEVCPP_EV_LOOP_H
#include <array>
#include <memory>
#include <csignal>
#include "ev_other_watcher.h"
#include "ev.h"

#define NUMPRI 5
class ev_watcher;
class ev_io;
class ev_async;
struct ANPENDING
{
    ev_watcher * w;
    int events; /* the pending event set for the given watcher */
};

enum {
    EVBREAK_CANCEL = 0, /* undo unloop */
    EVBREAK_ONE    = 1, /* unloop once */
    EVBREAK_ALL    = 2  /* unloop all loops */
};

/* ev_run flags values */
enum {
    EVRUN_NOWAIT = 1, /* do not block/wait */
    EVRUN_ONCE   = 2  /* block *once* only */
};

class ev_loop {
public:
    static ev_loop * GetThis();
    ev_loop();
    int run(int flags);
    void ev_invoke_pending();
    void destroy();
    void event_init();
    void init();
    void ev_feed_event(ev_watcher *w, int revents);
    void time_update();
    double ev_rt_now;
    double now_floor; /* last time we refreshed rt_time */
    double mn_now;    /* monotonic clock "now" 在创建loop的时候初始化，每次update_time更新这个*/
    double rtmn_diff; /* difference realtime - monotonic time */

    
    std::array<std::vector<ANPENDING>, NUMPRI> pendings;
    //std::array<int, NUMPRI> pendingcnt; //记录的是每个等级已经记录的监视器个数。也是用这个对 w_->pending进行排序，即使用++pendingcnt [NUMPRI]对w_->pending赋值
    int pendingpri; /* highest priority currently pending */

    double io_blocktime;
    double timeout_blocktime;

    int activecnt; /* total number of active events("refcount"; */ //loop挂载的事件数，当调用ev_TYPE_start中后再会在子函数内加一
    sig_atomic_t loop_done; /* signal by ev_break */


    // anfds是一个指向ANFD大小为anfdmax的数组，其索引值为文件描述符
    // 比如有两个ev_io事件用到stdin标准输入文件描述符，则对于第一个ev_io将anfds的头指向这个，此时记为为anfds->1
    // 将三个ev_io按照123顺序加入，最终结果为anfds->3->2->1

    int event_fd;
    ev_io *event_io;

    sig_atomic_t async_pending;
    std::vector<ev_async *> asyncs;

};

#endif //LIBEVCPP_EV_LOOP_H