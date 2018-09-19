#include "duty.h"


/*
责任链模式
主要是让用户的请求经过一系列的传递，而不需要让用户知道到底怎么传递的
这个用于程序通讯的时候，应该可以，这个类似于装饰模式。用于加密解密操作。
并且这个还能指定顺序。指定权限。
为每一个责任链上面的对象指定传递的对象。

*/

using namespace std;

void groupLeader::exectue(oa *o)
{
	if(o->getlevel() <= 1) {
		cout << this->name << " 能批准 " << o->getname() << endl;
	}else {
		cout << this->name << " 不能批准 " << o->getname() <<  "，交给我的上级 " << boss->getname() << " 处理" << endl;
		boss->exectue(o);
	}
}

void itemLeader::exectue(oa *o)
{
	if(o->getlevel() <= 2) {
		cout << this->name << " 能批准 " << o->getname() << endl;
	}else {
		cout << this->name << " 不能批准 " << o->getname() <<  "，交给我的上级 " << boss->getname() << " 处理" << endl;
		boss->exectue(o);
	}
}

void directorLeader::exectue(oa *o)
{
	if(o->getlevel() <= 3) {
		cout << this->name << " 能批准 " << o->getname() << endl;
	}else {		
		cout << this->name << " 不能批准 " << o->getname() <<  "，因为公司制度不允许" << endl;
	}
}



//如果在cpp里面包含c定义的头文件
extern "C" {
}

int main(int argc, char **argv)
{
	groupLeader g((char*)"小明组长");
	itemLeader i((char*)"张经理");
	directorLeader d((char*)"王总监");

	g.setboss(&i);
	i.setboss(&d);

	financialoa fo; 
	vacateoa vo;
	rewardoa ro;
	oa oo(5, (char*)"婚假");

	g.exectue(&vo);
	cout << "\n" << endl;
	g.exectue(&ro);
	cout << "\n" << endl;
	g.exectue(&fo);
	cout << "\n" << endl;
	g.exectue(&oo);

	return 0;
}

