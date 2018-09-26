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
	mydelegate.addonce用于添加只通知一次的对象，如果要考虑到对象生命周期，建议对端对象继承enable_shared_from_this,
	并用shared_ptr管理对象。这样才不会出现对象的race condition。
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
	mydelegate():id(0),observers(new std::map<int, T>()),once_observers(new std::map<int, T>()){}
	~mydelegate(){}


	int add(T func) {
		MutexLockGuard guard(mutex);
		int id = getSingularId();
		if(observers.use_count() != 1) {
			observers.reset(new std::map<int, T>(*observers));
		}
		observers->insert(std::pair<int,T>(id, func));
		return id;
	}

	int addonce(T func) {
		MutexLockGuard guard(mutex);
		int id = getDualId();
		if(once_observers.use_count() != 1) {
			once_observers.reset(new std::map<int, T>());	//只有notifyall函数可能会使use_count!=1,once需要清除
		}		
		once_observers->insert(std::pair<int,T>(id, func));
		return id;
	}
	
    void remove(int id) {
		MutexLockGuard guard(mutex);
		if(id%2) {
			if(observers.use_count() != 1) {
				observers.reset(new std::map<int, T>(*observers));
			}
			observers->erase(id);
		}else {
			if(once_observers.use_count() != 1) {
				once_observers.reset(new std::map<int, T>());	//只有notifyall函数可能会使use_count!=1,once需要清除
				std::cout << "once_observers reset empty" << std::endl;
			}
			once_observers->erase(id);
		}
	}

	typedef typename std::map<int,T>::iterator obsIter;
	void notifyAll() {
		boost::shared_ptr<std::map<int, T> > obs;
		boost::shared_ptr<std::map<int, T> > once_obs;
		{
			MutexLockGuard guard(mutex);
			obs = observers;
			once_obs = once_observers;
		}
		
		for(obsIter it = obs->begin();it != obs->end();it++) {
			it->second();
		}

		for(obsIter it = once_obs->begin();it != once_obs->end();) {
			it->second();
			once_obs->erase(it++);
		}
	}
	template<typename T1>
	void notifyAll(T1 t1) {		
		boost::shared_ptr<std::map<int, T> > obs;
		boost::shared_ptr<std::map<int, T> > once_obs;
		{
			MutexLockGuard guard(mutex);
			usleep(100);
			obs = observers;
			once_obs = once_observers;
			once_observers.reset(new std::map<int, T>());
			std::cout << "start notifyall" << std::endl;
		}
		
		for(obsIter it = obs->begin();it != obs->end();it++) {
			it->second(t1);
		}

		for(obsIter it = once_obs->begin();it != once_obs->end();) {
			it->second(t1);
			once_obs->erase(it++);
		}
		std::cout << "notifyAll over" << std::endl;
		
	}	
	template<typename T1, typename T2>
	void notifyAll(T1 t1, T2 t2) {
		boost::shared_ptr<std::map<int, T> > obs;
		boost::shared_ptr<std::map<int, T> > once_obs;
		{
			MutexLockGuard guard(mutex);
			obs = observers;
			once_obs = once_observers;
		}
		
		for(obsIter it = obs->begin();it != obs->end();it++) {
			it->second(t1, t2);
		}

		for(obsIter it = once_obs->begin();it != once_obs->end();) {
			it->second(t1, t2);
			once_obs->erase(it++);
		}	
	}		
	
private:
	int getSingularId() {
		return (++id%2)?id:++id;
	}

	int getDualId() {
		return (++id%2)?++id:id;
	}

	int id;
	boost::shared_ptr<std::map<int, T> > observers;
	boost::shared_ptr<std::map<int, T> > once_observers;
	MutexLock mutex;
};

#endif
