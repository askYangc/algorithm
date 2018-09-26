#ifndef _MY_DELEGATE_H_
#define _MY_DELEGATE_H_

#include <iostream>
#include <map>
#include <pthread.h>
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include "boost/noncopyable.hpp"
#include "boost/shared_ptr.hpp"
//#include "boost/enable_shared_from_this.hpp"

/*
notice:
	mydelegate.add用于添加永久对象，不用考虑观察者对象声明周期的。
	
	如果要考虑到对端对象生命周期，建议对端对象继承enable_shared_from_this,bind时传递shared_from_this()
	并用shared_ptr管理对象,并且在shared_ptr.reset()之前显式调用mydelegate.remove函数。
	这样才不会出现对象的race condition。
	重点:
	1,用shared_ptr管理对象，传递shared_from_this()
	2,释放对象前显式调用remove函数取消回调。
*/

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
	mydelegate():id(0),observers(new std::map<int, T>()){}
	~mydelegate(){}


	int add(T func) {
		MutexLockGuard guard(mutex);
		int id = getId();
		if(observers.use_count() != 1) {
			observers.reset(new std::map<int, T>(*observers));
		}
		observers->insert(std::pair<int,T>(id, func));
		return id;
	}
	
    void remove(int id) {
		MutexLockGuard guard(mutex);
		if(observers.use_count() != 1) {
			observers.reset(new std::map<int, T>(*observers));
		}
		observers->erase(id);
	}

	typedef typename std::map<int,T>::iterator obsIter;
	void notifyAll() {
		boost::shared_ptr<std::map<int, T> > obs;
		{
			MutexLockGuard guard(mutex);
			obs = observers;
		}
		
		for(obsIter it = obs->begin();it != obs->end();it++) {
			it->second();
		}
	}
	
	template<typename T1>
	void notifyAll(T1 t1) {		
		boost::shared_ptr<std::map<int, T> > obs;
		{
			MutexLockGuard guard(mutex);
			usleep(100);
			obs = observers;
			std::cout << "start notifyall" << std::endl;
		}
		
		for(obsIter it = obs->begin();it != obs->end();it++) {
			it->second(t1);
		}
		std::cout << "notifyAll over" << std::endl;		
	}	
	
	template<typename T1, typename T2>
	void notifyAll(T1 t1, T2 t2) {
		boost::shared_ptr<std::map<int, T> > obs;
		{
			MutexLockGuard guard(mutex);
			obs = observers;
		}
		
		for(obsIter it = obs->begin();it != obs->end();it++) {
			it->second(t1, t2);
		}
	}		
	
private:
	int getId() {
		return ++id;
	}

	int id;
	boost::shared_ptr<std::map<int, T> > observers;
	MutexLock mutex;
};

#endif
