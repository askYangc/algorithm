#include "flyweight.h"


/*
享元模式
其实具体实现就是一个内存池。当不同的功能使用的是相同的代码，或有大量过多的对象的时候，可以使用。
那么就可以利用已经实例化过后的对象来代替大量生成的对象。因为对象其实逻辑相同，只是一些参数不同，
没必要重新实例化对象了。这样就节省了资源。
功能之间如果相同，相同的逻辑称为享元的内部状态，
如果发现除了几个参数以外其他都相同的，那就可以使用享元模式。
不同的参数叫做享元模式的外部状态。
在发现使用了很多相同逻辑的时候，也可以考虑把不变的和变化的专门提出来
弄成内在状态和外在状态。

感觉这个模式有点像单例模式的集合。尽量少的生产对象。
根据对象的特征将变化的和不变化的区分开


*/

using namespace std;


//如果在cpp里面包含c定义的头文件
extern "C" {
}

int main(int argc, char **argv)
{
	fac c;
	string b("boke");
	string s("shopping");
	user u1((char*)"小明");
	user u2((char*)"小强");
	website *w = c.getWebSite(b);
 	w->webshow(u1);

	w = c.getWebSite(s);
	w->webshow(u2);
	w = c.getWebSite(s);
	w->webshow(u1);
	c.getwebscount();

	return 0;
}

