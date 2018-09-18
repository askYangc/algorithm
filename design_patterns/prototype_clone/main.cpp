#include "clone.h"


/*
原型模式
原型模式其实就是需要实现一个clone接口，让一个对象可以被另外一个对象被copy，至于是浅拷贝还是深拷贝，可以自己实现。
这个模式的好处是希望让对象封装好自己，不让外部知道自己是怎么拷贝的。
因为有一些对象的new实例化花费的时间太久，不如从已有实例拷贝。


*/

//如果在cpp里面包含c定义的头文件
extern "C" {
}

int main(int argc, char **argv)
{
	student a((char*)"123", 5);

	student *b = a.clone();
	b->setage(10);
	b->setname((char*)"小强");

	a.show();
	b->show();

	delete b;

	return 0;
}

