all:example1

example1:cl_thread.c example1.c rbtree.c min_heap.c
	gcc cl_thread.c example1.c rbtree.c min_heap.c -o example1

.PHONY:clean
clean:
	rm -rf *.o example1
