#include "strategy.h"


/*
策略模式
该模式的核心也是将业务和界面分离。其中context实际上也封装了简单工厂模式
如果遇到了在不同时间要用不同的策略的业务，就可以使用策略模式，因为得到的结果是一样的，只不过实现不一样，就可以通过
封装、继承、多态来实现该功能
这个代码实际上包含了简单工厂模式、策略模式、组合模式
*/

//如果在cpp里面包含c定义的头文件
extern "C" {
}

int main(int argc, char **argv)
{
	context b;
	if(argc == 2) {
		b.set_strategy(*argv[1]);
	}else{
		b.set_strategy('+');
	}
	
	b.get_result(1, 2);
	return 0;
}

