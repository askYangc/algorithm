#ifndef _MY_DELEGATE_H_
#define _MY_DELEGATE_H_

#include <iostream>
#include <map>
#include <pthread.h>
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include "boost/noncopyable.hpp"

class MutexLock : boost::noncopyable {
public:
	MutexLock() {
		pthread_mutex_init(&_mutex, NULL);
	}
	~MutexLock() {
		pthread_mutex_destroy(&_mutex);
	}
	void lock() {
		pthread_mutex_lock(&_mutex);
	}
	void unlock() {
		pthread_mutex_unlock(&_mutex);
	}	
	
private:
	pthread_mutex_t _mutex;
};

class MutexLockGuard : boost::noncopyable
{
 public:
  explicit MutexLockGuard(MutexLock& mutex)
    : mutex_(mutex)
  {
    mutex_.lock();
  }

  ~MutexLockGuard()
  {
    mutex_.unlock();
  }

 private:

  MutexLock& mutex_;
};



template<typename T>
class mydelegate {
public:	
	mydelegate():id(0){}
	~mydelegate(){}


	int add(T func) {
		MutexLockGuard guard(mutex);
		int id = getid();
		observers.insert(std::pair<int,T>(id, func));
		return id;
	}
	void remove(int id) {
		MutexLockGuard guard(mutex);
		observers.erase(id);
	}

	typedef typename std::map<int,T>::iterator obsIter;
	void notifyAll() {
		MutexLockGuard guard(mutex);
		for(obsIter it = observers.begin();it != observers.end();it++) {
			it->second();
		}
	}
	template<typename T1>
	void notifyAll(T1 t1) {
		for(obsIter it = observers.begin();it != observers.end();it++) {
			it->second(t1);
		}
	}	
	template<typename T1, typename T2>
	void notifyAll(T1 t1, T2 t2) {
		for(obsIter it = observers.begin();it != observers.end();it++) {
			it->second(t1, t2);
		}
	}		
	
private:
	int getid() {
		return ++id;
	}
	std::map<int, T> observers;
	int id;
	MutexLock mutex;
};

#endif
