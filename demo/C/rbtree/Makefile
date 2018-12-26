all:example1 example2 example3

example1:rbtree.c example1.c
	gcc example1.c rbtree.c -o example1
example2:rbtree.c example2.c
	gcc example2.c rbtree.c -o example2
example3:rbtree.c example3.c
	gcc example3.c rbtree.c -o example3

.PHONY:clean
clean:
	rm -rf *.o example1 example2 example3
