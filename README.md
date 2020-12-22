# libevcpp
simple libev implemented with modern C++



## motivation

由于`libev`是一个优秀的io复用框架，但是在实现中使用了大量的宏。这使得代码有一定的复杂性。并且libev为了兼容各个平台使用了多重选择，本项目本着最简单的原则，极大简化了项目。**故本项目可以方便了解libev的框架，如项目使用，请使用libev。** 但在细读libev前可以先阅读本项目。本项目使用c++,方便易懂。



本项目主要是针对 **Linux**，本人在 # Ubuntu20.04 # 下编写。并 **不适用于** MacOS，Windows 或者其他系统

在阅读该项目前，你的系统需要支持，**epoll，signalfd,，timerfd,，inotifyfd，eventfd **函数。

---

### 简化项目如下

* 将ev_watcher结构体用c++类表示，方便实现继承

  在libev中的继承主要是用宏和指针强转做的。

* libev中的结构体都是用c从零实现的，本项目将里面多数结构体使用c++ stl。（map, vector,priority_queue）

* libev的signal的实现顺序为signalfd，eventfd，pipe，本人简化到只使用signalfd
* libev的async的实现顺序为eventfd，pipe，本人只使用eventfd。
* libev的stat使用了inotifyfd和ev_timer，本项目只使用了inotifyfd
* libevcpp的ev_periodic事件只使用时间tm结构体作为事件参数
* libev在linux的io复用框架为epoll，poll，select，linuxaio，iouring，本项目只实现了epoll

* 删掉了所有ecb的代码

* 将ev_loop中的数据分离出来，在libev中绝多数变量在ev_loop结构体中，本项目按照一定的类关系，将数据进行分离。


---



在阅读libev源码时，建议 **生成预编译文件**，方便阅读。



libevcpp的基本事件

```
ev_io  监视io事件
ev_stat  监视文件变化，一般用于服务器热加载
ev_timer 相对定时器，如定时为1分钟后
ev_periodic 绝对定时器，如定时为某一年一月一日的具体一个时间，受系统时间影响
ev_signal 信号处理
```

其他事件，这些事件都只是有限支持

```
EV_CHILD  用来监视子进程，默认监视子进程退出状况
EV_IDLE   当ev_loop队列中没有任务时，执行该事件
EV_PREPARE 每次loop前需要做的事
EV_CHECK 每次loop中需要做的事
EV_CLEANUP 每次loop后要做的事
EV_ASYNC  这是异步操作，当调用特定函数才触发的事件
EV_FORK  当发生fork时，要做的事
```



## example

ev_signal 监视SIGINT。

```c++
void sigint_cb (struct ev_loop *loop, ev_signal *w, int revents)
{
    puts ( "signal ....." );
}
int main ()
{

    ev_loop *loop = ev_default_loop(EVFLAG_SIGNALFD);

    ev_signal s;
    s.init(sigint_cb,SIGINT);
    s.start(loop);
    loop->run(0);
    loop->destroy();
    // break was called, so exit
    return 0;
}
```

---

ev_child  六秒后子进程退出，执行child_cb回调

```c++
void child_cb (struct ev_loop *loop, ev_child *w, int revents)
{
    w->stop();
    printf ("process %d exited with status %x\n", w->get_rpid(), w->get_rstatus());
}
int main ()
{
    ev_loop *loop = ev_default_loop(EVFLAG_SIGNALFD);

    pid_t pid = fork ();
    ev_child cw;
    if (pid == 0)
    {
        sleep(6);
        exit(1);
    } else {
        cw.init(child_cb, pid, 0);
        cw.start(loop);
        loop->run(0);
        loop->destroy();
    }
        return 0;
}
```

---



ev_timer, 10秒后，且每隔5秒触发的事件

```c++
void time_cb (struct ev_loop *loop, ev_timer *w, int revents)
{
    printf ("timer out\n");
}
int main ()
{
    ev_loop *loop = ev_default_loop(EVFLAG_SIGNALFD);

    ev_timer t;
        t.init(time_cb, 10, 5);
        t.start(loop);
        loop->run(0);
        loop->destroy();
        return 0;
}
```

---



ev_stat 对文件监控

```c++
void stat_cb(struct ev_loop *loop, ev_stat* w,int revents)
{
    std::cout<<"file change"<<std::endl;
    if(w->attr.st_nlink){
        std::cout<<"current size "<<(long)w->attr.st_size<<std::endl;
    }
}
int main ()
{
    ev_loop *loop = ev_default_loop(EVFLAG_SIGNALFD);

    ev_stat t;
        t.init(stat_cb, "/home/feng/test.cpp");
        t.start(loop);
        loop->run(0);
        loop->destroy();
        return 0;
```

---



ev_periodic 30秒后触发

```c++
void time_cb (struct ev_loop *loop, ev_periodic *w, int revents)
{
    printf ("timer out\n");
}
int main ()
{

    ev_loop *loop = ev_default_loop(EVFLAG_SIGNALFD);
    time_t tm = time(0);
    struct tm *ttt = localtime(&tm);
    ttt->tm_sec += 30;

    ev_periodic t;
        t.init(time_cb, ttt);
        t.start(loop);
        loop->run(0);
        loop->destroy();
        return 0;
}
```

---



ev_io : 当终端有输入时触发回调

```c++
void io_cb(struct ev_loop *loop, ev_io *w, int revents)
{
    printf ("input ready\n");
    w->stop();
}
int main ()
{
    ev_loop *loop = ev_default_loop(EVFLAG_SIGNALFD);

    ev_io t;
    t.init(io_cb,0 ,EV_READ);
    t.start(loop);
        loop->run(0);
        loop->destroy();
        return 0;
}
```



## 框架

`FdWatcher`是一个监视文件描述符的类，主要针对ev_io

```c++
std::map<int,ANFD> anfd;  // 是其核心

struct ANFD
{
    std::forward_list<ev_io*> list;
    unsigned char events; // list对应所以事件的事件类型之或运算
    unsigned char reify;  //一个标志，指示这个fde需不需要进行fd_reify

    unsigned char emask;  //存储上一次修改的事件类型，每次fd_reify时候判断两个是不是一样。一样就跳过
    unsigned char eflags; /* flags field for use by backends */
#if EV_USE_EPOLL
    unsigned int egen;    /* generation counter to counter epoll bugs */
#endif
};
// 每一个文件描述符都对应一个ANFD结构体，这个结构体中的list是用到这个文件描述符的所有时间，eg, 多个io事件都用到标准输入。
// 每次在loop的时候对所有fd进行判断，然后做对应的修改（epoll_ctl）

```



Timer是一个时间类，主要有一个队列

```c++
template <typename Type>
class Timer{
public:
    std::priority_queue<Type *, std::vector<Type *>,  std::function<bool(Type * a1, Type * a2)> > timer_queue;
private: ev_loop *loop;
};
// 这个设置为了模板,主要是Type可以取值为ev_timer 和ev_periodic.
// 这里使用优先队列对到期时间事件进行排序（依照ev_timer 和ev_periodic中的at进行排序）
```



ev_loop中有一个变量，记录的是已经触发的时间，这些事件按照优先度进行归类，然后在调用ev_invoke_pending会调用call_back函数（虚函数），该函数会动态调用其事件的回调函数。pendings中的事件表示已经触发的事件，需要ev_invoke_pending调用其回调函数，

```c++
std::array<std::vector<ANPENDING>, NUMPRI> pendings;

struct ANPENDING
{
    ev_watcher * w;
    int events; /* the pending event set for the given watcher */
};

```

## loop循环

```c++
int ev_loop::run(int flags)
{

    ev_invoke_pending();

    do  // 这里会一直循环下去
    {

        if(curpid) /* penalise the forking check even more */
            if(getpid() != curpid)
            {   // 这是一个子进程
                curpid = getpid();
                postfork = 1;
            }


        if(postfork)  // 当发生了fork后，会检查其forks有没有值。
            if(!forks.empty())
            {
                queue_events(forks, EV_FORK);
                ev_invoke_pending();
            }


        if(!prepares.empty())  // 进入loop前需要做的事
        {
            queue_events(prepares, EV_PREPARE);
            ev_invoke_pending();
        }

        if(postfork)  // 发生回调后，应该关闭子进程中的一些文件描述符，如eventfd，等等
            loop_fork();

        fdwtcher->fd_reify();  // 检验是否有需要epoll_ctl的fd

        /* calculate blocking time *//计算阻塞时间，epoll_wait
        // 本质上是ev_timer和ev_periodic的最小值
        {
            double waittime  = 0.;
            double sleeptime = 0.;

            /* remember old timestamp for io_blocktime calculation */
            double prev_mn_now = mn_now;

            /* 更新时间以取消回调处理开销 */
            time_update(EV_TSTAMP_HUGE);

            /* from now on, we want a pipe-wake-up */
            //pipe_write_wanted = 1;

            if(!(flags & EVRUN_NOWAIT || idleall || !activecnt ))
            {// 这个函数主要设置waittime等待时间

                waittime = MAX_BLOCKTIME;

                if(timer->size())
                {
                    double to = timer->top()->get_at() - mn_now;
                    if(waittime > to) waittime = to;
                }
                if(periodic->size())
                {
                    double to = periodic->top()->get_at() - ev_rt_now;
                    if(waittime > to) waittime = to;
                }

                /*不要让超时将等待时间减少到timeout_blocktime以下，默认为0 */
                if(waittime < timeout_blocktime)
                    waittime = timeout_blocktime;

                /* 
                现在还有另外两种特殊情况，要么我们的定时器已经过期，所以我们就不应该睡觉，
                要么定时器很快到期，在这种情况下，我们需要等待一些事件循环后端的最短时间 。
                */
                if(waittime < mutilplexing->backend_mintime)
                    waittime = waittime <= 0.
                               ? 0.
                               : mutilplexing->backend_mintime;

            }

            assert((loop_done = EVBREAK_RECURSE, 1)); /* assert for side effect */
            mutilplexing->backend_poll(this, waittime);   // 里面有epoll_wait
            assert((loop_done = EVBREAK_CANCEL, 1)); /* assert for side effect */

            time_update(waittime + sleeptime); // 更新时间。
        }

        /* queue pending timers and reschedule them */
        timer->timers_reify(); //判断是否有ev_timer事件到期
        periodic->periodics_reify(); //判断是否有ev_periodic事件到期

        idle_reify();  // 判断是否有满足ev_idel条件的事件，既目前pendings还有事件未处理


        if(!checks.empty())  // 进入loop中需要做的事
            queue_events(checks, EV_CHECK);

        ev_invoke_pending();  // 最后的事件触发是在这里
    }
    while(
            activecnt
            && !loop_done
            && !(flags &(EVRUN_ONCE | EVRUN_NOWAIT))
    );

    if(loop_done == EVBREAK_ONE)
        loop_done = EVBREAK_CANCEL;


    return activecnt;
}
```



如果有ev_timer或者和ev_periodic，每次在epoll_wait的时间都是取timer_queue的顶层，（最小）

ev_signal事件是通过ev_io监视signalfd。

ev_child事件，是通过ev_signal的SIGCHLD信号

ev_stat事件，是通过ev_io监视inotifd。

ev_async事件是通过ev_io监视eventfd。当事件触发，且掉用了set_sent，会触发其ev_async回调事件。

在ev_periodic中使用timerfd去更新时间



## TODO

* 本人试图分离ev_loop结构体，但是由于经验问题，对设计模式没有深入了解，并没有构造一个合理的类关系

* 本人将添加iouring异步框架

* 由于本人对linux了解不够深入，对epoll的错误代码及其对应处理方法不够了解，所以在epoll代码中并没有一个很好的注释。