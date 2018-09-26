#include "mydelegate.h"
#include "boost/enable_shared_from_this.hpp"

/*
用boost::function和boost::bind实现委托。
对观察者有对象声明周期的用mydelegate.addonce。该函数只会通知一次，会自动删除回调函数。
需要在bind时传递shared_from_this()，对象应该用shared_ptr管理生命周期

*/

//如果在cpp里面包含c定义的头文件
extern "C" {
}

using namespace std;

class test: public boost::enable_shared_from_this<test>{
public:
	test(mydelegate<boost::function <int (int)> > &a):a(a) {}
	~test() {
		cout << "delete test " << this << endl;
		remove();
	}

	void add(){
		MutexLockGuard guard(mutex);
		id = a.add(boost::bind(&test::tfunc, shared_from_this(), _1, 77));
	}
	void remove(){
		MutexLockGuard guard(mutex);
		a.remove(id);
	}
	int tfunc() {
		MutexLockGuard guard(mutex);
		cout << "tfunc " << endl;
		return 0;
	}
	int tfunc(int a, int b) {
		MutexLockGuard guard(mutex);
		cout << "tfunc " << a << " and " << b << " " << this << endl;
		cout << "use_count " << shared_from_this().use_count() << endl;
		return 0;
	}
	
private:
	mydelegate<boost::function <int (int)> > &a;
	MutexLock mutex;
	int id;
};

int tvobs(int a)
{
	cout << "tvobs get " << a << endl;
	return 0;
}

int nbaobs(int a, int b)
{
	cout << "nbaobs " << a << " and " << b << endl;
	return 0;
}

mydelegate<boost::function <int (int)> > a;


void *thread_func(void *p)
{
	boost::shared_ptr<test> t;
	t.reset(new test(a));
	t->add();
	
	usleep(50);

	cout << "t reset bofore use_cout: " << t.use_count() << endl;
	//t.reset();
	t->remove();
	return NULL;
}


int main(int argc, char **argv)
{
	pthread_t pid;
	pthread_create(&pid, NULL, thread_func, NULL);

	usleep(10);
	a.add(boost::bind(&tvobs, _1));
	a.add(boost::bind(&nbaobs, _1, 5));
	
	cout << "will notify all" << endl;
	a.notifyAll(555);

	return 0;
}

