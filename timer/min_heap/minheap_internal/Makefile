all:example1 example2

example1:min_heap.c example1.c
	gcc example1.c min_heap.c -o example1 -lrt
example2:min_heap.c example2.c
	gcc example2.c min_heap.c -o example2 -lrt

.PHONY:clean
clean:
	rm -rf *.o example1 example2
