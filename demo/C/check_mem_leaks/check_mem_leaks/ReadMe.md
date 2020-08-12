本库主要是hook malloc等内存分配函数用于检查内存泄露

连接本库：
-L../check_mem_leaks -lmemcheck -Wl,--export-dynamic -fno-omit-frame-pointer -rdynamic

注意
1，默认内存泄露检查不开启，因为初始化的代码没必要检查内存泄露.
2，mem_leaks_start()用于开启检查(连接库后默认关闭),mem_leaks_stop()用于停止检查。mem_leaks_show()用于展示分配的数据。
3，配合 addr2line 进行检查
    addr2line -e prog -f -C -a 0x425355 -p

4, parse_mem.py是一个分析日志的工具，可以批量使用addr2line分析函数名和行号
    