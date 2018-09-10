调用dead_lock_stub_start_check(1) 到测试函数中即可。
本代码主要是参考网上大神说的通过图的方法来寻找死锁和等待锁，将.c和.h加入到代码中就可以了，没得依赖。
编译选项需要加上-fno-omit-frame-pointer,因为依赖了汇编代码，不然会被优化掉。

两个文件已经在真实的环境里面测试过了

当出现死锁的时候，会打印对应的调用栈。需要通过

addr2line -e linkserver -f -C -a 0x425355 -p

来打印具体函数信息来分析
-e 指定要解析的程序文件（获取符号表）
-f 表明要显示函数名字
-C表明要解析函数Demangle function names
-a 指定函数地址
-p 表明打印在一行，可以不要这个

addr2line是GNU的binutils的一个工具，可能需要安装工具包
