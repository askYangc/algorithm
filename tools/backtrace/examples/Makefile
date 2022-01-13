all:test main

test:lib.c lib.h
	gcc -g lib.c -fPIC -shared -o libtest.so

main:main.c
	gcc -g main.c -L./ -ltest -o main

.PHONY:clean
clean:
	rm -rf main *.o *.so
