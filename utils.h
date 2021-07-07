//
// Created by feng on 2020/12/2.
//

#ifndef LIBEVCPP_UTILS_H
#define LIBEVCPP_UTILS_H


#include <ctime>

double ev_time(void);

double get_clock(void);
void ev_sleep(double delay);

int array_nextsize(int elem, int cur, int cnt);

void fd_intern(int fd);
double tm_to_time(tm*);

#endif //LIBEVCPP_UTILS_H
