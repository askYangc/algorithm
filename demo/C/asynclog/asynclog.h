#ifndef _BASE_ASYNCLOG_H_
#define _BASE_ASYNCLOG_H_

#include <stdio.h>
#include <pthread.h>
#include "condition.h"

#define ASYNCLOG_BUFMAX 1024*1024*4
#define ASYNCLOG_KROLLSIZE 500*1000*1000
#define ASYNCLOG_FLUSHTIME 3
#define MAXBUFFER 23

typedef struct {
	char *buffer;
	char *cur;
}asyncbuffers_t;

typedef struct {
	asyncbuffers_t buffers_[MAXBUFFER + 1];
	int cout;
}asyncbufferalign_t;

typedef struct {
	int flushInterval_;
	int running_;
	char basename_[100];
	size_t rollSize_;
	pthread_t pid;
	
	pthread_mutex_t mutex_;
	countdownlatch_t *latch_;
	condition_t *cond_;
	asyncbuffers_t currentBuffer_;
	asyncbuffers_t nextBuffer_;
	asyncbufferalign_t buffers_;
	char logline[ASYNCLOG_BUFMAX];
}asynclog_t;


void asynclog_init(char *basename);
void asynclog_def_init(char *basename, size_t rollSize, int flushInterval);
void asynclog_start();
void asynclog_stop();


void log_append(char *fmt, ...);



#endif
