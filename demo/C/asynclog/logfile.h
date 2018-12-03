#ifndef _BASE_LOGFILE_H_
#define _BASE_LOGFILE_H_

#include <stdio.h>
#include <pthread.h>

typedef struct {
	char basename_[100];
	size_t rollSize_;
	int flushInterval_;
	int checkEveryN_;
	
	int count_;

	int threadSafe;
	pthread_mutex_t mutex_;
	time_t startOfPeriod_;
	time_t lastRoll_;
	time_t lastFlush_;

	FILE* fp_;
	char buffer_[64*1024];
	size_t writtenBytes_;
	
	int kRollPerSeconds_;
}logfile_t;

logfile_t *logfile_init(char *basename, size_t rollSize, 
	int threadSafe, int flushInterval);
void logfile_free(logfile_t *t);

void logfile_append(logfile_t *t, const char* logline, int len);
void logfile_flush(logfile_t *t);


#endif
