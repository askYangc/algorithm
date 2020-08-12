#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
//#include "mem_leaks.h"


class hello {
public:
	hello() {
		printf("in hello\n");
	}
	~hello(){
		printf("out hello\n");
	}
	 void  tt() const {
		printf("in const tt\n");
	}
	void tt() {
		printf("in tt\n");
	}
private:
	int a;
};

void test1()
{
	hello *h = new hello();
	const hello *r = h;
	r->tt();
}

int main()
{
    //mem_leaks_start(); 

	test1();
	

    //mem_leaks_show();

    return 0;
}


