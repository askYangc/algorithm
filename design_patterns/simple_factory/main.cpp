#include "factory.h"


/*
简单工厂模式
该模式的核心就是将业务和界面分离。
将相同功能封装起来，根据业务的需要返回不同的对象。然而调用对象的流程都是一样的。这样就达到了简单封装的功能
*/

//如果在cpp里面包含c定义的头文件
extern "C" {
}

int main()
{
	fac *a = new fac();
	cal *b = a->getcal('-');
	b->setNum(1, 2);
	b->get_result();
	delete b;
	delete a;
	return 0;
}

