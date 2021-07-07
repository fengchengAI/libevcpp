//
// Created by feng on 2020/12/2.
//

#ifndef LIBEVCPP_EV_H
#define LIBEVCPP_EV_H

#include <cerrno>
#include <cassert>
#include <iostream>
#include <vector>
#include <functional>

/* these priorities are inclusive, higher priorities will be invoked earlier */
# define EV_MINPRI 0
# define EV_MAXPRI 4



#define MIN_TIMEJUMP   1. /* minimum timejump that gets detected (if monotonic clock available) */
#define MAX_BLOCKTIME  59.743 /* never wait longer than this time (to detect time jumps) */
#define MAX_BLOCKTIME2 1500001.07 /* same, but when timerfd is used to detect jumps, also safe delay to not overflow */


#ifndef EV_PID_HASHSIZE
# define EV_PID_HASHSIZE EV_FEATURE_DATA ? 16 : 1
#endif

enum {
    EV_UNDEF    = (int)0xFFFFFFFF, /* guaranteed to be invalid */
    EV_NONE     =            0x00, /* no events */
    EV_READ     =            0x01, /* ev_io detected read will not block */
    EV_WRITE    =            0x02, /* ev_io detected write will not block */
    EV__IOFDSET =            0x80, /* internal use only */
    EV_IO       =         EV_READ, /* alias for type-detection */
    EV_TIMER    =      0x00000100, /* 相对定时器 timer timed out */

    EV_PERIODIC =      0x00000200, /*绝对定时器 periodic timer timed out */
    EV_SIGNAL   =      0x00000400, /* signal was received */
    EV_CHILD    =      0x00000800, /* child/pid had status change */
    EV_STAT     =      0x00001000, /* stat data changed */
    EV_IDLE     =      0x00002000, /* event loop is idling */
    EV_PREPARE  =      0x00004000, /* event loop about to poll */
    EV_CHECK    =      0x00008000, /* event loop finished poll */
    EV_EMBED    =      0x00010000, /* embedded event loop needs sweep */
    EV_FORK     =      0x00020000, /* event loop resumed in child */
    EV_CLEANUP  =      0x00040000, /* 退出触发事件 event loop resumed in child */
    EV_ASYNC    =      0x00080000, /* async intra-loop signal */
    EV_CUSTOM   =      0x01000000, /* for use by user code */
    EV_ERROR    = (int)0x80000000  /* sent when an error occurs */
};

#ifndef EV_INOTIFY_HASHSIZE
# define EV_INOTIFY_HASHSIZE 16
#endif

#endif //LIBEVCPP_EV_H





