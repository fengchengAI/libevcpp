//
// Created by feng on 2020/12/2.
//

#ifndef LIBEVCPP_EV_LOOP_H
#define LIBEVCPP_EV_LOOP_H
#include <array>
#include <memory>
#include <csignal>
#include "ev.h"
//#include "ev_timer.h"

class ev_timer;
class File_Stat;
class ev_periodic;
template <typename Type>
class Timer;
class ev_loop;

#define NUMPRI 5

extern ev_loop * ev_default_loop_ptr;
class Signal;
class ev_watcher;
class FdWatcher;
class Multiplexing;
class ev_async;
class ev_signal;
class ev_io;
struct ANPENDING
{
    ev_watcher * w;
    int events; /* the pending event set for the given watcher */
};

enum {
    /* the default */
    EVFLAG_AUTO       = 0x00000000U, /* not quite a mask */
    /* flag bits */
    EVFLAG_NOENV      = 0x01000000U, /* do NOT consult environment */
    EVFLAG_FORKCHECK  = 0x02000000U, /* check for a fork in each iteration */
    /* debugging/feature disable */
    EVFLAG_NOINOTIFY  = 0x00100000U, /* do not attempt to use inotify */
#if EV_COMPAT3
    EVFLAG_NOSIGFD    = 0, /* compatibility to pre-3.9 */
#endif
    EVFLAG_SIGNALFD   = 0x00200000U, /* attempt to use signalfd */
    EVFLAG_NOSIGMASK  = 0x00400000U, /* avoid modifying the signal mask */
    EVFLAG_NOTIMERFD  = 0x00800000U  /* avoid creating a timerfd */
};

/* method bits to be ored together */
enum {
    EVBACKEND_SELECT   = 0x00000001U, /* available just about anywhere */
    EVBACKEND_POLL     = 0x00000002U, /* !win, !aix, broken on osx */
    EVBACKEND_EPOLL    = 0x00000004U, /* linux */
    EVBACKEND_DEVPOLL  = 0x00000010U, /* solaris 8 */ /* NYI */
    EVBACKEND_LINUXAIO = 0x00000040U, /* linux AIO, 4.19+ */
    EVBACKEND_IOURING  = 0x00000080U, /* linux io_uring, 5.1+ */
    EVBACKEND_ALL      = 0x000000FFU, /* all known backends */
    EVBACKEND_MASK     = 0x0000FFFFU  /* all future backends */
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

typedef ev_watcher ev_prepare;
typedef ev_watcher ev_idle;
typedef ev_watcher ev_check;

typedef ev_watcher ev_fork;

typedef ev_watcher ev_cleanup;

static ev_signal *childev;

class ev_loop {
public:
    ev_loop();
    int run(int flags);
    void ev_invoke_pending();
    void queue_events(std::vector<ev_watcher *>events, int type);
    void destroy();
    void event_init();
    void loop_init(unsigned int flags ) noexcept;
    void ev_feed_event(ev_watcher *w, int revents) noexcept;
    void time_update(double max_block);
    void evtimerfd_init();
    void idle_reify();
    void loop_fork();
    void ev_break(int how);
    double ev_rt_now;
    double now_floor; /* last time we refreshed rt_time */
    double mn_now;    /* monotonic clock "now" 在创建loop的时候初始化，每次update_time更新这个*/
    double rtmn_diff; /* difference realtime - monotonic time */

    // TODO ? 这里是什么？
    /* 用于事件的反向馈送对于一个到期的时间事件，将它放进这里 */

    
    // ANPENDING *pendings [NUMPRI];  // 二维数组，不同的等级监视的事件类型。指向NUMPRI大小的数组，每个数组指向pendings类型地址
    std::array<std::vector<ANPENDING>, NUMPRI> pendings;
    //int pendingmax [NUMPRI];
    //std::array<int, NUMPRI> pendingcnt; //记录的是每个等级已经记录的监视器个数。也是用这个对 w_->pending进行排序，即使用++pendingcnt [NUMPRI]对w_->pending赋值
    int pendingpri; /* highest priority currently pending */

    // TODO ？ 这是什么
    //ev_watcher* pending_w; /* dummy pending watcher */

    double io_blocktime;
    double timeout_blocktime;

    int backend;  //一个flag指向后台哪个方法，select，epoll, poll
    int activecnt; /* total number of active events("refcount"; */ //loop挂载的事件数，当调用ev_TYPE_start中后再会在子函数内加一
    sig_atomic_t loop_done; /* signal by ev_break */

    //int backend_fd;  // epoll_creat创建的结果
    //double backend_mintime; /* assumed typical timer resolution */


    //void(*backend_modify)(ev_loop * loop, int fd, int oev ,int nev);
    //void(*backend_poll)(ev_loop * loop, double );


// anfds是一个指向ANFD大小为anfdmax的数组，其索引值为文件描述符
// 比如有两个ev_io事件用到stdin标准输入文件描述符，则对于第一个ev_io将anfds的头指向这个，此时记为为anfds->1
// 将三个ev_io按照123顺序加入，最终结果为anfds->3->2->1
    //ANFD * anfds;
    FdWatcher* fdwtcher;
    Timer<ev_timer> *timer;

#if    EV_ASYNC_ENABLE
    int event_fd;
    //int evpipe [2];
    ev_io *event_io;
#endif
    //sig_atomic_t pipe_write_wanted;
    //sig_atomic_t pipe_write_skipped;

    pid_t curpid;

    char postfork;  /* true if we need to recreate kernel state after fork */

    Multiplexing *mutilplexing;


    //ev_watcher_time ** timers;  // 所有的定时器，其按照ev_timer_start的顺序，给EV_WATCHER的active顺序赋值，用这个active也可以指向其挂在的ev_time事件
#if EV_FORK_ENABLE
    std::vector<ev_fork *> forks;
#endif

#if EV_IDLE_ENABLE
    std::array<std::vector<ev_idle *>,NUMPRI> idles;
#endif

#if EV_PERIODIC_ENABLE
    Timer<ev_periodic> *periodic;
#endif

    int idleall; /* total number */
    std::vector<ev_prepare *> prepares;
    std::vector<ev_check *> checks;

#if EV_ASYNC_ENABLE 
    sig_atomic_t async_pending;
    std::vector<ev_async *> asyncs;
#endif

#if EV_USE_INOTIFY
    File_Stat* file_stat;
#endif

    //sig_atomic_t sig_pending;

#if EV_USE_SIGNALFD
    Signal *sigs;
#endif

#if EV_USE_TIMERFD 
    int timerfd; /* timerfd for time jump detection */
    ev_io* timerfd_w;
#endif

    unsigned int origflags; /* original loop flags */ //loop_init(;传入的

#if EV_FEATURE_API 
    unsigned int loop_count; /* total number of loop iterations/blocks */
    unsigned int loop_depth; /* #ev_run enters - #ev_run leaves */
    std::function<void()> invoke_cb;
#endif

};

ev_loop *ev_default_loop(unsigned int flags = 0);

#endif //LIBEVCPP_EV_LOOP_H