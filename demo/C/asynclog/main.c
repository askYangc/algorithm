#include <stdio.h>
#include <pthread.h>
#include "condition.h"
#include <unistd.h>

#include "asynclog.h"

condition_t *t1;

#define BUF_MAX 30

int buf[BUF_MAX] = {0};

int head = 0;
int tail = 0;


void *consumer(void *p)
{
	int timeout;
	while(1) {
		pthread_mutex_lock(condition_getLock(t1));
		while(head == tail) {
			//condition_wait(t1);
			timeout = condition_waitForSeconds(t1, 1);
			if(timeout) {
				printf("I'm timeout\n");
			}
		}
		printf("buf[%d]: %d\n", head, buf[head]);
		head++;
		pthread_mutex_unlock(condition_getLock(t1));
	}
	return NULL;
}

void *producer(void *p)
{
	int run = 1;
	while(run) {
		pthread_mutex_lock(condition_getLock(t1));
		buf[tail] = tail;
		tail++;
		if(tail == BUF_MAX) {
			run = 0;
		}
		condition_notify(t1);
		pthread_mutex_unlock(condition_getLock(t1));
		usleep(2000000);
	}
	printf("producer over\n");
	return NULL;
}

int condition_test()
{
	pthread_t pid1, pid2;

	t1 = condition_init(NULL);

	pthread_create(&pid1, NULL, producer, NULL);
	pthread_create(&pid2, NULL, consumer, NULL);

	
	pthread_join(pid1, NULL);
	pthread_join(pid2, NULL);

	condition_free(t1);
	
	return 0;
}

void do_log()
{
	int i = 0;
	asynclog_init("test-out");
	asynclog_start();

	for(i = 0; i < 100000; i++) {
		log_append("hello world %d\n", i);
	}
	
	asynclog_stop();
}

int main()
{
	do_log();

	return 0;
}
