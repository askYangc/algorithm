#ifndef _BASE_ASYNC_PROC_H_
#define _BASE_ASYNC_PROC_H_

#include <stdio.h>
#include <pthread.h>
#include "condition.h"
#include "flex_buffer.h"

#define ASYNCPROC_BUFMAX 1024*1024*4

#define ASYNCPROC_MAXBUFFER 23
#define ASYNCPROC_FLUSHTIME 2

//非连续数据流
#define ASYNCPROC_MAXSTREAM 4


typedef struct {
    char *data[ASYNCPROC_MAXSTREAM];
    int len[ASYNCPROC_MAXSTREAM];
    int cout;                       //个数
}async_stream_t;


struct asyncProc_s;

typedef int (* asyncProc_func_t)(struct asyncProc_s *, char *, int);


typedef struct asyncProc_s {
	int running_;
	int flushInterval_;
	pthread_t pid;
	
	pthread_mutex_t mutex_;
	countdownlatch_t *latch_;
	condition_t *cond_;
	flex_buffer_t *currentBuffer_;
	flex_buffer_t *nextBuffer_;
	alignflexbuffer_t buffers_;
    asyncProc_func_t proc;
	//char logline[LINKAGEASYNCLOG_BUFMAX];
	void *data;
}asyncProc_t;


asyncProc_t *async_proc_init(asyncProc_func_t f, void *data);
asyncProc_t *async_proc_def_init(asyncProc_func_t f, void *data, int flushInterval_);
void async_proc_free(asyncProc_t *p);

void async_proc_start(asyncProc_t *p);
void async_proc_stop(asyncProc_t *p);


void async_proc_append(asyncProc_t *p, char *data, int len);
void async_proc_append_stream(asyncProc_t *p, async_stream_t *buffer);


#define async_proc_safe_stop(p) do{\
    if(p){\
        async_proc_stop(p);\
        async_proc_free(p);\
        p = NULL;\
    }\
}while(0)


#endif
