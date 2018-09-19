#include "singleton.h"


/*
单例模式 
单例模式就是防止类实例化多个，一般用于工厂模式的时候，工厂的实例只需要一份就够了
因为工厂生产的数据应该都是相同的，只需要 一个工厂生产就可以了。

*/

//如果在cpp里面包含c定义的头文件
extern "C" {
}

int main(int argc, char **argv)
{
	person *a = person::getInstance();
	person *b = person::getInstance();

	std::cout << "a: " << a << ", b: " << b << std::endl; 

	return 0;
}

