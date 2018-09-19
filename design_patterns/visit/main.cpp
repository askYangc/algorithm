#include "visit.h"


/*
访问者模式
访问者模式是用于数据结构相对稳定的系统。
目的是要把处理从数据结构分离出来。很多系统可以按照算法和数据结构
分开，如果数据结构很稳定，有有很多容易变化的算法的话，使用访问者
就很适合。因为访问者模式很容易增加算法。
访问者模式给我的感觉是享元模式的变种。
因为访问者模式的大概思路就是，函数是不同的，变化的的只是输入的参
数。由于访问者类的逻辑是不变化的，流程也是不变化的，变化的只是输
入的参数（类），因此就将不变化的和变化的分割开了。

一般这个访问者模式不用

*/

using namespace std;


//如果在cpp里面包含c定义的头文件
extern "C" {
}

void succ::show(human *h) 
{
	cout << h->getname() << "成功了" << endl;	
}

void fail::show(human *h) 
{
	cout << h->getname() << "失败了" << endl;	
}


int main(int argc, char **argv)
{
	man m;
	woman w;

	succ s;
	fail f;

	m.getstate(s);
	w.getstate(s);

	m.getstate(f);
	w.getstate(f);	
	
	return 0;
}

