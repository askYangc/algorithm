#include "memo.h"


/*
备忘录模式 
用于存储进度之类很合适，还是要求客户端不需要自己
内部的状态变化。
用于存储某个复杂对象的部分数据，用于可以及时还原备份


*/

//如果在cpp里面包含c定义的头文件
extern "C" {
}

int main(int argc, char **argv)
{
	role p(10, 100);
	memo *m = p.getmemo();

	p.show();

	p.age = 5;
	p.health = 10;

	p.show();

	//recover
	p.setmemo(m);

	p.show();

	delete m;

	return 0;
}

