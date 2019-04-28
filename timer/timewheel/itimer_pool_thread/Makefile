all:example1 example2 example3

example1:itimer_internal.c example1.c
	gcc example1.c itimer_internal.c -o example1

example2:itimer.c itimer_internal.c example2.c
	gcc example2.c itimer.c itimer_internal.c -o example2 -lpthread
example3:itimer.c itimer_internal.c example3.c
	gcc example3.c itimer.c itimer_internal.c -o example3 -lpthread
.PHONY:clean
clean:
	rm -rf *.o example1 example2 example3
