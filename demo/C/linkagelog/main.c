#include <stdio.h>
#include <pthread.h>
#include "condition.h"
#include <unistd.h>

#include "linkageasynclog.h"
#include "linkage_log.h"

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
	char buf[40] = {0};

	for(i = 0; i < 1000; i++) {
		int n = sprintf(buf, "hello world %d\n", i);
		linkagelog_append(buf, n);
	}
	
}

void test_linkagelog()
{
	char buf[100] = {0};
	linkage_log_header_t *hdr;
	log_adddev_t *t;

	hdr = (linkage_log_header_t*)buf;

	hdr->ver = 1;
	hdr->ds_command = LOG_ADDDEV;
	hdr->len = sizeof(log_adddev_t);

	t = (log_adddev_t*)(hdr+1);
	t->home_id = 5;
	t->user_id = 6;
	t->sn = 808000010001;

	linkagelog_append(buf, sizeof(log_adddev_t) + sizeof(linkage_log_header_t));
}

int main()
{
	linkageasynclog_init();
	linkageasynclog_start();
	test_linkagelog();
	linkageasynclog_stop();

	return 0;
}
