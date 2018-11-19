#include <stdio.h>
#include "Condition.h"
#include <vector>
#include <unistd.h>


using namespace std;


class Queue {
public:
	Queue():c(mutex),cout(0) {}
	~Queue() {}

	void push(int d) {
		MutexLockGuard m(mutex);
		q.push_back(d);
		cout++;
		notify();
	}
	
	int pop() {
		MutexLockGuard m(mutex);
		while(q.empty()) {
			wait(1);
		}
		if(!q.empty()) {
			int ret = 0;
			printf("get %d\n", q.front());
			if(q.front() == 9) {
				ret = 1;
			}
			q.erase(q.begin());
			cout--;
			return ret;
		}else {
			printf("count is %d\n", cout);
		}
	}	

	int empty(){
		//MutexLockGuard m(mutex);
		return cout==0?true:false;
	}

	void notify() { c.notify();}
	void wait(int sec) { 
		if(sec == 0) c.wait();
		else c.waitForSeconds(sec);
	}
	
private:
	MutexLock mutex;	
	Condition c;
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
	int do_pop() {
		return cnt.pop();
	}	

	int empty(){ return cnt.empty();}
	void wait(int sec = 0) {
		cnt.wait(sec);
	}
private:
	Queue &cnt;
};

class test_manager {
public:
	test_manager():c(m) {}
	Queue &get_queue () {
		return q;
	}

	void notify() {
		c.notify();
	}

	void wait() {
		c.wait();
	}
private:
	Queue q;
	MutexLock m;
	Condition c;
};

test_manager tm;


void *do_producer(void *args)
{
	Producer p(tm.get_queue());
	for(int i = 0; i < 10; i++) {
		p.do_push(i);
		usleep(500000);
	}

	
	return NULL;
}

void *do_consumer(void *p)
{
	Consumer c(tm.get_queue());

	while(1) {
		int ret = c.do_pop();
		if(ret) break;
	}

	printf("now break\n");
	tm.notify();
	
	return NULL;
}

void mutex_test()
{
	pthread_t pid;

	pthread_create(&pid, NULL, do_producer, NULL);
	pthread_create(&pid, NULL, do_consumer, NULL);

	tm.wait();
}



int main()
{
	mutex_test();
	return 0;
}
