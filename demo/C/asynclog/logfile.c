#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <time.h>
#include "logfile.h"

void logfile_reopen(logfile_t *t, char *filename)
{
	if(t->fp_) {
		fclose(t->fp_);
	}

	t->fp_ = fopen(filename, "ae");
	if(t->fp_) {
		setbuffer(t->fp_, t->buffer_, sizeof(t->buffer_));
	}
}

static void logfile_gethostname(char *buf, int len)
{
	// HOST_NAME_MAX 64
	// _POSIX_HOST_NAME_MAX 255
	memset(buf, 0, len);
	if (gethostname(buf, len) == 0){
		buf[len - 1] = '\0';
		return ;
	}else{
		sprintf(buf, "unknownhost");
	}
}

static int getLogFileName(char *filename, int len, char *basename, time_t* now)
{
	char buf[256];	
	char timebuf[32];

	struct tm tm;
	*now = time(NULL);
	gmtime_r(now, &tm); // FIXME: localtime_r ?
	logfile_gethostname(buf, sizeof(buf));
	strftime(timebuf, sizeof timebuf, ".%Y%m%d-%H%M%S.", &tm);
	snprintf(filename, len, "%s%s%s.%d.log", basename, timebuf, buf, getpid());

	return 0;
}



void logfile_roll(logfile_t *t)
{
	char filename[512] = {0};
	time_t now = 0;
	getLogFileName(filename, sizeof(filename), t->basename_, &now);
	time_t start = now / t->kRollPerSeconds_ * t->kRollPerSeconds_;
	
	if (now > t->lastRoll_) {
		t->lastRoll_ = now;
		t->lastFlush_ = now;
		t->startOfPeriod_ = start;
		logfile_reopen(t, filename);	  
	}
	
	return ;
}

void _logfile_append(logfile_t *t, const char* logline, int len)
{
	size_t n = fwrite_unlocked(logline, 1, len, t->fp_);
	size_t remain = len - n;
	while (remain > 0) {
		size_t x = fwrite_unlocked(logline + n, 1, remain, t->fp_);
		if (x == 0) {
			int err = ferror(t->fp_);
			if (err) {
				fprintf(stderr, "_logfile_append() failed %s\n", strerror(err));
			}
			break;
		}
		n += x;
		remain = len - n; // remain -= x
	}

	t->writtenBytes_ += len;
}

void logfile_append_unlocked(logfile_t *t, const char* logline, int len)
{
	_logfile_append(t, logline, len);

	if (t->writtenBytes_ > t->rollSize_){
		logfile_roll(t);
	}else{
		++t->count_;
		if (t->count_ >= t->checkEveryN_){
			t->count_ = 0;
			time_t now = time(NULL);
			time_t thisPeriod_ = now / t->kRollPerSeconds_ * t->kRollPerSeconds_;
			if (thisPeriod_ != t->startOfPeriod_){
				logfile_roll(t);
			}else if (now - t->lastFlush_ > t->flushInterval_){
				t->lastFlush_ = now;
				logfile_flush(t);
			}
		}
	}
}

void logfile_append(logfile_t *t, const char* logline, int len)
{
	if(t->threadSafe) {
		pthread_mutex_lock(&t->mutex_);
		logfile_append_unlocked(t, logline, len);
		pthread_mutex_unlock(&t->mutex_);
	}else {
		logfile_append_unlocked(t, logline, len);
	}
}

void logfile_flush(logfile_t *t)
{
 	if(t->threadSafe) {
		pthread_mutex_lock(&t->mutex_);
		fflush(t->fp_);
		pthread_mutex_unlock(&t->mutex_);
	}else {
		fflush(t->fp_);
	} 
}


logfile_t *logfile_init(char *basename, size_t rollSize, int threadSafe, int flushInterval)
{
	logfile_t *f = (logfile_t*)calloc(1, sizeof(logfile_t));
	if(f) {
		memcpy(f->basename_, basename, strlen(basename));
		f->rollSize_ = rollSize;
		f->flushInterval_ = flushInterval;
		f->checkEveryN_ = 1024;
		f->threadSafe = threadSafe;
		f->kRollPerSeconds_ = 60*60*24;
		if(threadSafe) {
			pthread_mutex_init(&f->mutex_, NULL);
		}
		logfile_roll(f);
	}

	return f;
}

void logfile_free(logfile_t *t)
{
	if(t) {
		if(t->fp_) {
			fclose(t->fp_);	
		}
		if(t->threadSafe) {
			pthread_mutex_destroy(&t->mutex_);
		}
		free(t);
	}
}

