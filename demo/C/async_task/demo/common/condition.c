#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>

#include "condition.h"

void condition_free(condition_t *t)
{
	if(t) {
		if(t->lock_free) {
			pthread_mutex_destroy(t->lock_);
			free(t->lock_);
		}
		pthread_cond_destroy(&t->pcond_);
		free(t);
	}
}

condition_t *condition_init(pthread_mutex_t *lock)
{
	condition_t *t = calloc(1, sizeof(condition_t));
	if(t == NULL)
		return NULL;

	if(lock) {
		t->lock_free = 0;
		t->lock_ = lock;
	}else {
		t->lock_ = calloc(1, sizeof(pthread_mutex_t));
		if(t->lock_ == NULL) {
			free(t);
			return NULL;
		}
		pthread_mutex_init(t->lock_, NULL);
		t->lock_free = 1;
	}
	
	pthread_cond_init(&t->pcond_, NULL);

	return t;
}

pthread_mutex_t *condition_getLock(condition_t *t)
{
	return t->lock_;
}

void condition_wait(condition_t *t)
{
	pthread_cond_wait(&t->pcond_, t->lock_);
}

int condition_waitForSeconds(condition_t *t, double seconds)
{
	struct timespec abstime;
	// FIXME: use CLOCK_MONOTONIC or CLOCK_MONOTONIC_RAW to prevent time rewind.
	clock_gettime(CLOCK_REALTIME, &abstime);

	const int64_t kNanoSecondsPerSecond = 1000000000;
	int64_t nanoseconds = seconds * kNanoSecondsPerSecond;

	abstime.tv_sec += (abstime.tv_nsec + nanoseconds) / kNanoSecondsPerSecond;
	abstime.tv_nsec = (abstime.tv_nsec + nanoseconds) % kNanoSecondsPerSecond;

	return ETIMEDOUT == pthread_cond_timedwait(&t->pcond_, t->lock_, &abstime);
}

void condition_notify(condition_t *t)
{
	pthread_cond_signal(&t->pcond_);
}

void condition_notifyAll(condition_t *t)
{
	pthread_cond_broadcast(&t->pcond_);
}


//countdownlatch_t
void countdownlatch_wait(countdownlatch_t *t)
{
	pthread_mutex_lock(t->condition_->lock_);
	while(t->count > 0) {
		condition_wait(t->condition_);
	}
	pthread_mutex_unlock(t->condition_->lock_);
}

void countdownlatch_countdown(countdownlatch_t *t)
{
	pthread_mutex_lock(t->condition_->lock_);
	t->count--;
	if(t->count == 0) {
		condition_notifyAll(t->condition_);
	}
	pthread_mutex_unlock(t->condition_->lock_);
}

void countdownlatch_free(countdownlatch_t *t)
{
	if(t) {
		if(t->condition_)
			condition_free(t->condition_);
		free(t);
	}
}

countdownlatch_t *countdownlatch_init(pthread_mutex_t *lock, int count)
{
	countdownlatch_t *t = (countdownlatch_t*)calloc(1, sizeof(condition_t));
	if(t) {
		t->count = count;
		t->condition_ = condition_init(lock);
	}

	return t;
}
