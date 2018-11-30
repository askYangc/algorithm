#ifndef _BASE_CONDITION_H_
#define _BASE_CONDITION_H_

#include <pthread.h>

typedef struct {
	int lock_free;
	pthread_mutex_t *lock_;
	pthread_cond_t pcond_;
}condition_t;


condition_t *condition_init(pthread_mutex_t *lock);
void condition_free(condition_t *t);

void condition_wait(condition_t *t);
int condition_waitForSeconds(condition_t *t, double seconds);

void condition_notify(condition_t *t);
void condition_notifyAll(condition_t *t);

pthread_mutex_t *condition_getLock(condition_t *t);


#endif
