#include "template_model.h"


/*
模板方法模式
这个主要是在父类的代码里面实现了大概的逻辑，规划好了执行函数的流程，
而子类则具体实现具体的逻辑，将父类的抽象延迟到了子类的具体函数中
即将不变的步骤移动到父类，可变的操作移动到子类

*/

//如果在cpp里面包含c定义的头文件
extern "C" {
}

int main(int argc, char **argv)
{
	boy a;
	a.do_lunch();

	girl b;
	b.do_lunch();

	return 0;
}

