#include "observer.h"


/*

观察者模式
观察者模式的重点是一个对象变化时需要通知多个对象，
而通知的时候对象并不需要知道是哪些需要被通知，这就实现了解耦
这里需要学习下C++实现委托和C++11的function和bind的使用
不用委托的话，就必须所有观察者得实现相同的接口和参数，这点不好用


*/

//如果在cpp里面包含c定义的头文件
extern "C" {
}

int main(int argc, char **argv)
{
	obj o;

	nbaobserver n1;
	newsobserver n2;

	o.insert(&n1);
	o.insert(&n2);
	o.update();

	return 0;
}

