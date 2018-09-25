#include "mydelegate.h"


/*
用boost::function和boost::bind实现委托

*/

//如果在cpp里面包含c定义的头文件
extern "C" {
}

using namespace std;

class test{
public:
	int tfunc() {
		cout << "tfunc " << endl;
		return 0;
	}
};

int tvobs()
{
	cout << "tvobs get " << endl;
	return 0;
}

int nbaobs(int a, int b)
{
	cout << "nbaobs " << a << " and " << b << endl;
	return 0;
}


int main(int argc, char **argv)
{
	mydelegate<boost::function <int (void)> > a;
	test t;

	a.add(boost::bind(&tvobs));
	a.add(boost::bind(&nbaobs, 2, 5));
	a.add(boost::bind(&test::tfunc, &t));

	a.notifyAll();

	return 0;
}

