# algorithm
good algorithm from others or myself

# 算法
timer/timewheel/itimer 来源于https://github.com/skywind3000/AsyncNet/blob/master/system/ 将该算法记录下来，以备用学习


itimer_pool 是修改itimer，增加了一个内存池管理和简化设置和去掉定时器的操作


itimer_pool_thread 是修改itimer为一个独立线程来管理定时器，通过2个简单的宏和一个函数就可以嵌入到其他线程去处理定时器


# 框架
zebra thread 是开源软件zebra剥离出来的运行框架，用在单线程的程序里面效果很好
