#include <stdio.h>
#include "Condition.h"
#include <vector>

using namespace std;

class Queue {
public:
	Queue():cout(0) {}
	~Queue() {}

	void push(int d) {
		MutexLockGuard m(mutex);
		q.push_back(d);
		cout++;
	}
	
	int pop() {
		MutexLockGuard m(mutex);
		if(!q.empty()) {
			printf("get %d\n", q.front());
			q.erase(q.begin());
			cout--;
		}else {
			printf("count is %d\n", cout);
		}
	}	
		
private:
	MutexLock mutex;	
	vector<int> q;
	int cout;
};


class Producer {
public:
	Producer(Queue &cnt):cnt(cnt) {}
	void do_push(int d) {
		cnt.push(d);
	}
private:
	Queue &cnt;
};

class Consumer {
public:
	Consumer(Queue &cnt):cnt(cnt) {}
	void do_pop() {
		cnt.pop();
	}	
private:
	Queue &cnt;
};

void mutex_test()
{
	Queue cnt;
	Producer p(cnt);
	Consumer c(cnt);
	for(int i = 0; i < 10; i++) {
		p.do_push(i);
	}

	for(int i = 0; i < 11; i++) {
		c.do_pop();
	}
}



int main()
{
	mutex_test();
	return 0;
}
