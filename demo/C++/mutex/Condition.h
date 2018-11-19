#ifndef _BASE_CONDITION_H_
#define _BASE_CONDITION_H_

#include "Mutex.h"

#include <stdint.h>
#include <boost/noncopyable.hpp>
#include <pthread.h>
#include <errno.h>

class Condition : boost::noncopyable {
public:
	explicit Condition(MutexLock& mutex) : mutex_(mutex) {
		pthread_cond_init(&pcond_, NULL);
	}

	~Condition() {
		pthread_cond_destroy(&pcond_);
	}

	void wait() {
		pthread_cond_wait(&pcond_, mutex_.getPthreadMutex());
	}

	// returns true if time out, false otherwise.
	bool waitForSeconds(double seconds) {
		struct timespec abstime;
		// FIXME: use CLOCK_MONOTONIC or CLOCK_MONOTONIC_RAW to prevent time rewind.
		clock_gettime(CLOCK_REALTIME, &abstime);

		const int64_t kNanoSecondsPerSecond = 1000000000;
		int64_t nanoseconds = static_cast<int64_t>(seconds * kNanoSecondsPerSecond);

		abstime.tv_sec += static_cast<time_t>((abstime.tv_nsec + nanoseconds) / kNanoSecondsPerSecond);
		abstime.tv_nsec = static_cast<long>((abstime.tv_nsec + nanoseconds) % kNanoSecondsPerSecond);

		return ETIMEDOUT == pthread_cond_timedwait(&pcond_, mutex_.getPthreadMutex(), &abstime);
	}

	void notify()
	{
		pthread_cond_signal(&pcond_);
	}

	void notifyAll() {
		pthread_cond_broadcast(&pcond_);
	}

private:
	MutexLock& mutex_;
	pthread_cond_t pcond_;
};

#endif  // MUDUO_BASE_CONDITION_H
