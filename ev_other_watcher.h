//
// Created by feng on 2020/12/15.
//

#ifndef LIBEVCPP_EV_OTHER_WATCHER_H
#define LIBEVCPP_EV_OTHER_WATCHER_H

#include <csignal>
#include "watcher.h"


class ev_loop;
class ev_watcher;
class ev_async : public ev_watcher
{
public:
    ev_async();
    void init(std::function<void(ev_loop *, ev_async *, int)> cb_);
    void start();
    void stop();
    void async_send();
    void call_back(ev_loop *loop, ev_watcher *w, int) override;

    sig_atomic_t get_sent() const;
    void set_sent(sig_atomic_t sent_);
private:
    std::function<void(ev_loop *, ev_async *, int)> cb;
    sig_atomic_t volatile sent;
};
#endif //LIBEVCPP_EV_OTHER_WATCHER_H
