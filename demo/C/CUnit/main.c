#include <stdio.h>
#include "hello.h"


int main()
{
	int a = add(10, 1);
	int b = del(10, 1);

	printf("a:%d, b:%d\n", a, b);
	return 0;
}
