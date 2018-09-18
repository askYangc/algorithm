#include "state.h"


/*

状态模式
其实就是状态机，根据当前状态执行不同功能
将复杂的行为分离开，解除耦合


*/

//如果在cpp里面包含c定义的头文件
extern "C" {
}

obj::obj()
{
	s = new state_idle();
}

obj::~obj()
{
	delete s;
}

void obj::request()
{
	s->setState(this);
	s->show();
}

void state_idle::setState(obj *j)
{
	j->s = new state_estab();
}

void state_estab::setState(obj *j)
{
	j->s = new state_idle();
}


int main(int argc, char **argv)
{
	obj o;

	o.request();
	o.request();
	o.request();
	o.request();
	o.request();
	

	return 0;
}

