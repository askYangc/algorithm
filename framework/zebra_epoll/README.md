# zebra_epoll

zebra_epoll是参考zebra的格式编写的框架，将zebra的响应部分替换了。

定时器替换为最小堆管理。

io的read和write链表替换为红黑树节点管理。

CL_THREAD_TIMER_ON是一次性定时器，需要自己重新设置定时器。

CL_THREAD_READ_ON和WRITE_ON默认是非永久监听。需要进入回调函数后自己重新设置定时器（和zebra一样）。可以更新参数设置为永久监听。设置永久监听后调用回调函数时对应的cl_thread_t就不会指向unuse链表，无需重新设置。


zebra_epoll需要cl_thread.* rbtree.* min_heap.* stlc_list.h
