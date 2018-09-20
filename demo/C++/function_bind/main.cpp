#include <stdio.h>
#include<boost/function.hpp>
#include<boost/bind.hpp>


void func(int a, int b, int c)
{
	printf("a;%d, b:%d\n", a, b);
	return ;
}

typedef boost::function<void (int)> fc;


int main()
{
	fc w = boost::bind(&func, _1, 2, 5);

	w(4);	
	return 0;	
}
