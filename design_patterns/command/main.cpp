#include "command.h"


/*
命令模式
将具体的命令与一个执行命令的对象绑定，这样调用者就无需知道该怎么
去执行这个命令，可以达到统一管理的目的。在invoker中还可以添加、删除、修改。
将操作的对象和具体的操作给解耦了。
每个命令都绑定了他自己的处理对象。

*/

//如果在cpp里面包含c定义的头文件
extern "C" {
}

int main(int argc, char **argv)
{
	invoker invoke;
	sqlreceiver r;
	namereceiver n;
	
	invoke.addCommand(new sqlcomand(1, &r));
	invoke.addCommand(new namecomand(2, &n));

	//invoke.delCommand(1);

	invoke.doCommands();

	

	return 0;
}

