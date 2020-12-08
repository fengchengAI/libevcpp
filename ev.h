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
#include "config.h"
#ifndef EV_COMPAT3
# define EV_COMPAT3 1
#endif

#ifndef EV_FEATURES
# if defined __OPTIMIZE_SIZE__
#  define EV_FEATURES 0x7c
# else
#  define EV_FEATURES 0x7f
# endif
#endif

#define EV_FEATURE_CODE     ((EV_FEATURES) &  1)
#define EV_FEATURE_DATA     ((EV_FEATURES) &  2)
#define EV_FEATURE_CONFIG   ((EV_FEATURES) &  4)
#define EV_FEATURE_API      ((EV_FEATURES) &  8)
#define EV_FEATURE_WATCHERS ((EV_FEATURES) & 16)
#define EV_FEATURE_BACKENDS ((EV_FEATURES) & 32)
#define EV_FEATURE_OS       ((EV_FEATURES) & 64)

/* these priorities are inclusive, higher priorities will be invoked earlier */
#ifndef EV_MINPRIEV_FEATURES
# define EV_MINPRI (EV_FEATURE_CONFIG ? -2 : 0)
#endif
#ifndef EV_MAXPRI
# define EV_MAXPRI (EV_FEATURE_CONFIG ? +2 : 0)
#endif

#ifndef EV_MULTIPLICITY
# define EV_MULTIPLICITY EV_FEATURE_CONFIG
#endif

#ifndef EV_PERIODIC_ENABLE
# define EV_PERIODIC_ENABLE EV_FEATURE_WATCHERS
#endif

#ifndef EV_STAT_ENABLE
# define EV_STAT_ENABLE EV_FEATURE_WATCHERS
#endif

#ifndef EV_PREPARE_ENABLE
# define EV_PREPARE_ENABLE EV_FEATURE_WATCHERS
#endif

#ifndef EV_CHECK_ENABLE
# define EV_CHECK_ENABLE EV_FEATURE_WATCHERS
#endif

#ifndef EV_IDLE_ENABLE
# define EV_IDLE_ENABLE EV_FEATURE_WATCHERS
#endif

#ifndef EV_FORK_ENABLE
# define EV_FORK_ENABLE EV_FEATURE_WATCHERS
#endif

#ifndef EV_CLEANUP_ENABLE
# define EV_CLEANUP_ENABLE EV_FEATURE_WATCHERS
#endif

#ifndef EV_SIGNAL_ENABLE
# define EV_SIGNAL_ENABLE EV_FEATURE_WATCHERS
#endif

#ifndef EV_CHILD_ENABLE
# define EV_CHILD_ENABLE EV_FEATURE_WATCHERS
#endif

#ifndef EV_ASYNC_ENABLE
# define EV_ASYNC_ENABLE EV_FEATURE_WATCHERS
#endif

#ifndef EV_EMBED_ENABLE
# define EV_EMBED_ENABLE EV_FEATURE_WATCHERS
#endif

#ifndef EV_WALK_ENABLE
# define EV_WALK_ENABLE 0 /* not yet */
#endif
/* flag bits for ev_default_loop and ev_loop_new */



#ifndef EV_USE_EPOLL
# define EV_USE_EPOLL 1 /* not yet */
#endif



# if HAVE_FLOOR
#  ifndef EV_USE_FLOOR
#   define EV_USE_FLOOR 1
#  endif
# endif

# if HAVE_CLOCK_SYSCALL
#  ifndef EV_USE_CLOCK_SYSCALL
#   define EV_USE_CLOCK_SYSCALL 1
#   ifndef EV_USE_REALTIME
#    define EV_USE_REALTIME  0
#   endif
#   ifndef EV_USE_MONOTONIC
#    define EV_USE_MONOTONIC 1
#   endif
#  endif
# elif !defined EV_USE_CLOCK_SYSCALL
#  define EV_USE_CLOCK_SYSCALL 0
# endif


# if HAVE_CLOCK_GETTIME
#  ifndef EV_USE_MONOTONIC
#   define EV_USE_MONOTONIC 1
#  endif
#  ifndef EV_USE_REALTIME
#   define EV_USE_REALTIME  0
#  endif
# endif

# if HAVE_NANOSLEEP
#  ifndef EV_USE_NANOSLEEP
#    define EV_USE_NANOSLEEP EV_FEATURE_OS
#  endif
# else
#   undef EV_USE_NANOSLEEP
#   define EV_USE_NANOSLEEP 0
# endif

# if HAVE_SELECT && HAVE_SYS_SELECT_H
#  ifndef EV_USE_SELECT
#   define EV_USE_SELECT EV_FEATURE_BACKENDS
#  endif
# else
#  undef EV_USE_SELECT
#  define EV_USE_SELECT 0
# endif

# if HAVE_POLL && HAVE_POLL_H
#  ifndef EV_USE_POLL
#   define EV_USE_POLL EV_FEATURE_BACKENDS
#  endif
# else
#  undef EV_USE_POLL
#  define EV_USE_POLL 0
# endif

# if HAVE_EPOLL_CTL && HAVE_SYS_EPOLL_H
#  ifndef EV_USE_EPOLL
#   define EV_USE_EPOLL EV_FEATURE_BACKENDS
#  endif
# else
#  undef EV_USE_EPOLL
#  define EV_USE_EPOLL 0
# endif

# if HAVE_LINUX_AIO_ABI_H
#  ifndef EV_USE_LINUXAIO
#   define EV_USE_LINUXAIO 0 /* was: EV_FEATURE_BACKENDS, always off by default */
#  endif
# else
#  undef EV_USE_LINUXAIO
#  define EV_USE_LINUXAIO 0
# endif

# if HAVE_LINUX_FS_H && HAVE_SYS_TIMERFD_H && HAVE_KERNEL_RWF_T
#  ifndef EV_USE_IOURING
#   define EV_USE_IOURING EV_FEATURE_BACKENDS
#  endif
# else
#  undef EV_USE_IOURING
#  define EV_USE_IOURING 0
# endif


#  undef EV_USE_KQUEUE
#  define EV_USE_KQUEUE 0


#  undef EV_USE_PORT
#  define EV_USE_PORT 0

# if HAVE_INOTIFY_INIT && HAVE_SYS_INOTIFY_H
#  ifndef EV_USE_INOTIFY
#   define EV_USE_INOTIFY EV_FEATURE_OS
#  endif
# else
#  undef EV_USE_INOTIFY
#  define EV_USE_INOTIFY 0
# endif

# if HAVE_SIGNALFD && HAVE_SYS_SIGNALFD_H
#  ifndef EV_USE_SIGNALFD
#   define EV_USE_SIGNALFD EV_FEATURE_OS
#  endif
# else
#  undef EV_USE_SIGNALFD
#  define EV_USE_SIGNALFD 0
# endif

# if HAVE_EVENTFD
#  ifndef EV_USE_EVENTFD
#   define EV_USE_EVENTFD EV_FEATURE_OS
#  endif
# else
#  undef EV_USE_EVENTFD
#  define EV_USE_EVENTFD 0
# endif

# if HAVE_SYS_TIMERFD_H
#  ifndef EV_USE_TIMERFD
#   define EV_USE_TIMERFD EV_FEATURE_OS
#  endif
# else
#  undef EV_USE_TIMERFD
#  define EV_USE_TIMERFD 0
# endif


#define MIN_TIMEJUMP   1. /* minimum timejump that gets detected (if monotonic clock available) */
#define MAX_BLOCKTIME  59.743 /* never wait longer than this time (to detect time jumps) */
#define MAX_BLOCKTIME2 1500001.07 /* same, but when timerfd is used to detect jumps, also safe delay to not overflow */




#endif //LIBEVCPP_EV_H





