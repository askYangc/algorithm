all:test

CFLAGS += -I../mutex


test:AsyncLog.cpp test.cpp LogFile.cpp
	g++ $(CFLAGS) test.cpp AsyncLog.cpp LogFile.cpp -o test -lpthread
.PHONY:clean
clean:
	rm -rf test *.o
