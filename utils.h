//
// Created by feng on 2020/12/2.
//

#ifndef LIBEVCPP_UTILS_H
#define LIBEVCPP_UTILS_H


#include <ctime>

int enable_secure (void);
double ev_time (void);

double get_clock (void);
void ev_sleep (double delay);
#define MALLOC_ROUND 4096 /* prefer to allocate in chunks of this size, must be 2**n and >> 4 longs */

int array_nextsize (int elem, int cur, int cnt);
/* used to prepare libev internal fd's */
/* this is not fork-safe */
void fd_intern (int fd);
double tm_to_time(tm*);

#endif //LIBEVCPP_UTILS_H
