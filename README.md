# libevcpp
simple libev implemented with modern c++


# TODO 
watcher系里面的ev_loop使用指针，不能使用引用。且只在start函数调用才传入ev_loop


eventfd初始化的值？？，首先假设-1为初始化，