all:test thread_test
test:test.cpp Mutex.h Condition.h
	g++ test.cpp -o test
thread_test:thread_test.cpp Mutex.h Condition.h
	g++ thread_test.cpp -o thread_test -lpthread
.PHONY:clean
clean:
	rm -rf test thread_test *.o
