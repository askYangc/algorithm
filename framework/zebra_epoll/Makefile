all:example1 cli srv

example1:cl_thread.c example1.c rbtree.c min_heap.c
	gcc cl_thread.c example1.c rbtree.c min_heap.c -o example1
cli:cl_thread.c cli.c rbtree.c min_heap.c
	gcc cl_thread.c cli.c rbtree.c min_heap.c -o cli
srv:cl_thread.c srv.c rbtree.c min_heap.c
	gcc cl_thread.c srv.c rbtree.c min_heap.c -o srv

.PHONY:clean
clean:
	rm -rf *.o example1 cli srv
