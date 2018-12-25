all:example1 example2

example1:rbtree.c example1.c
	gcc example1.c rbtree.c -o example1
example2:rbtree.c example2.c
	gcc example2.c rbtree.c -o example2

.PHONY:clean
clean:
	rm -rf *.o example1 example2
