#ifndef _SINGLETON_H_
#define _SINGLETON_H_

#include <iostream>
#include <pthread.h>

class person {
private:
	person() {}
	person(const person &b) {}
	person &operator=(const person &b) { return *this;}
public:
	static person *getInstance(){
		if(p == NULL) {
			pthread_mutex_lock(&mutex);
			if(p == NULL) {
				p = new person();
			}
			pthread_mutex_unlock(&mutex);
		}
		return p;
	}

public:
	static pthread_mutex_t mutex;

private:
	static person *p;
};


pthread_mutex_t person::mutex = PTHREAD_MUTEX_INITIALIZER;
person *person::p = NULL;

#endif
