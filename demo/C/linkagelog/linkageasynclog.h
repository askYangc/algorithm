#ifndef _BASE_LINKAGECLOG_H_
#define _BASE_LINKAGECLOG_H_

#include <stdio.h>
#include <pthread.h>
#include "condition.h"
#include "arbuffer.h"

//#define LINKAGEASYNCLOG_BUFMAX 100
#define LINKAGEASYNCLOG_BUFMAX 1024*1024*4

#define LINKAGEASYNCLOG_MAXBUFFER 23
#define LINKAGEASYNCLOG_FLUSHTIME 2


typedef struct {
	int running_;
	int flushInterval_;
	pthread_t pid;
	
	pthread_mutex_t mutex_;
	countdownlatch_t *latch_;
	condition_t *cond_;
	arraybuffers_t currentBuffer_;
	arraybuffers_t nextBuffer_;
	arraybufferalign_t buffers_;
	//char logline[LINKAGEASYNCLOG_BUFMAX];
}linkageasynclog_t;


void linkageasynclog_init();
void linkageasynclog_def_init(int flushInterval_);
void linkageasynclog_start();
void linkageasynclog_stop();


void linkagelog_append(char *data, int len);
//void linkagelog_append(char *fmt, ...);



#endif
