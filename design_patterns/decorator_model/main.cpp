#include "decorator.h"


/*
装饰模式
该模式的特点是为了给指定类对象A增加一些额外的功能，而不需要修改这个类或者继承子类而设计。
核心实现是让原类与一个抽象类共同继承同一个父类A，然后抽象类需要封装一个父类A的对象，这样就可以让
抽象类派生的子类B也拥有原类或者兄弟类C，这样在调用装饰的函数时，无需关心其他对象是怎么实现的，只关心
自己实现的功能就好，最好在自己实现功能的时候先把其他装饰类的功能给先实现了。
*/

//如果在cpp里面包含c定义的头文件
extern "C" {
}

int main(int argc, char **argv)
{
	person p((char*)"小明");
	yifu y;
	kuzi k;
	y.set_decorator_clothes(&p);
	k.set_decorator_clothes(&y);
	k.do_operation();

	return 0;
}

