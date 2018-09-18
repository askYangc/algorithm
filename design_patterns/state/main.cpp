#include "state.h"


/*

状态模式
这个就是状态机，这里C++更封装了一层，将具体的各个状态的行为都封装到一个子类去。
这样修改某一状态的代码就只需要修改这个子类就可以了。这样大大降低了耦合



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

