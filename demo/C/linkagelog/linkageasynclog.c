#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "linkageasynclog.h"
#include "linkage_log.h"

static linkageasynclog_t *link_aslog = NULL;


void linkagelog_append(char* data, int len)
{
	pthread_mutex_lock(&link_aslog->mutex_);
	if (arbuffer_avail(&link_aslog->currentBuffer_) > len) {
		arbuffer_append(&link_aslog->currentBuffer_, data, len);	/* 长度能写，就直接加入到当前缓冲 */
	}else{
		arraybufferalign_add_tail(&link_aslog->buffers_, &link_aslog->currentBuffer_); /* 长度不够则直接将当前缓存给弄进一个缓冲队列buffers_ */

		if(arbuffer_usable(&link_aslog->nextBuffer_)) {
			arbuffer_move(&link_aslog->currentBuffer_, &link_aslog->nextBuffer_);
		}else {
			arbuffer_init(&link_aslog->currentBuffer_, LINKAGEASYNCLOG_BUFMAX);	
		}

		arbuffer_append(&link_aslog->currentBuffer_, data, len);
		condition_notify(link_aslog->cond_);
	}
	pthread_mutex_unlock(&link_aslog->mutex_);
}


/*
void linkagelog_append(char *fmt, ...)
{
	pthread_mutex_lock(&link_aslog->mutex_);

	va_list ap;
	va_start(ap, fmt);
	int n = vsnprintf(link_aslog->logline, ASYNCLOG_BUFMAX - 1, fmt, ap);
	va_end(ap);

	if(n > -1) 
		_log_append(link_aslog->logline, n);

	
	pthread_mutex_unlock(&link_aslog->mutex_);
}
*/

void *linkageasynclog_threadFunc(void *p)
{
	int i = 0;
	//assert(aslog->running_ == true);
	arraybuffers_t newBuffer1;
	arraybuffers_t newBuffer2;
	arraybufferalign_t buffersToWrite;

	buffersToWrite.cout = 0;
	arbuffer_init(&newBuffer1, LINKAGEASYNCLOG_BUFMAX);
	arbuffer_init(&newBuffer2, LINKAGEASYNCLOG_BUFMAX);
	arraybufferalign_init(&buffersToWrite, LINKAGEASYNCLOG_MAXBUFFER + 1);

	countdownlatch_countdown(link_aslog->latch_);
	while (link_aslog->running_) {
		//assert(newBuffer1 && newBuffer1->length() == 0);
		//assert(newBuffer2 && newBuffer2->length() == 0);
		//assert(buffersToWrite.empty());

		{
			pthread_mutex_lock(&link_aslog->mutex_);
			if (link_aslog->buffers_.cout <= 0)  { // unusual usage!
				/* 如果缓冲队列为空，则等待最多3秒时间或者被append唤醒来继续下一步操作 */
				condition_waitForSeconds(link_aslog->cond_, link_aslog->flushInterval_);
			}
			arraybufferalign_add_tail(&link_aslog->buffers_, &link_aslog->currentBuffer_); /* 不管有没有数据都讲currentBuffer_的数据加入队列 */
			arraybufferalign_move(&buffersToWrite, &link_aslog->buffers_);		/* 临界的buffers_被交换为本地的buffters */
			arbuffer_move(&link_aslog->currentBuffer_, &newBuffer1);		/* 将本地的buffer替换当前缓冲，应该是为了append快速结束 */
			if (!arbuffer_usable(&link_aslog->nextBuffer_)) {
				arbuffer_move(&link_aslog->nextBuffer_, &newBuffer2);		/* 将本地的buffer2替换备用缓冲，应该是为了append快速结束 */
			}
			pthread_mutex_unlock(&link_aslog->mutex_);
		}

    	//assert(!buffersToWrite.empty());

		if (buffersToWrite.cout > LINKAGEASYNCLOG_MAXBUFFER){
			//超过MAXBUFFER的可能是生产者出问题了，直接只保留前面2个
			arraybufferalign_erase(&buffersToWrite, 2, LINKAGEASYNCLOG_MAXBUFFER+1);
		}
		for (i = 0; i < buffersToWrite.cout; ++i) {
			// FIXME: use unbuffered stdio FILE ? or use ::writev ?
			do_linkageasynclog(buffersToWrite.buffers_[i].buffer, arbuffer_used(&buffersToWrite.buffers_[i]));
		}

		if (buffersToWrite.cout > 2){
			// drop non-bzero-ed buffers, avoid trashing
			/* 在短时间内大量的append操作产生的多个队列只保留2个就可以了，用来分配给本地buffer1和buffer2 */
			arraybufferalign_erase(&buffersToWrite, 2, LINKAGEASYNCLOG_MAXBUFFER+1);
		}

		if (!arbuffer_usable(&newBuffer1)) {
			//assert(!buffersToWrite.empty());
			arbuffer_move(&newBuffer1, &buffersToWrite.buffers_[buffersToWrite.cout-1]); /* 保留的第一块buffer，分给本地buffer1 */
			arbuffer_reset(&newBuffer1);
			buffersToWrite.cout--;
		}

		if (!arbuffer_usable(&newBuffer2)) {
			//assert(!buffersToWrite.empty());
			arbuffer_move(&newBuffer2, &buffersToWrite.buffers_[buffersToWrite.cout-1]);
			arbuffer_reset(&newBuffer2);						/* 保留的第一块buffer，分给本地buffer2 */
			buffersToWrite.cout--;
		}

		arraybufferalign_clear(&buffersToWrite);			/* 有可能nextBuffer_不是空的，所以newBuffers没有赋值给nextBuffer_，所以这里还多了一块，需要清除掉 */
	}
	arraybufferalign_clear(&buffersToWrite);
	arbuffer_clear(&newBuffer1);
	arbuffer_clear(&newBuffer2);
	return NULL;
}

void linkageasynclog_start()
{
  	link_aslog->running_ = 1;
	pthread_create(&link_aslog->pid, NULL, linkageasynclog_threadFunc, NULL);
  	countdownlatch_wait(link_aslog->latch_);
}

void linkageasynclog_stop()
{
	link_aslog->running_ = 0;
	condition_notify(link_aslog->cond_);
	pthread_join(link_aslog->pid, NULL);
}

void linkageasynclog_def_init(int flushInterval_)
{
	link_aslog = calloc(1, sizeof(linkageasynclog_t));	
	if(link_aslog) {
		link_aslog->running_ = 0;
		link_aslog->flushInterval_ = flushInterval_;
	
		pthread_mutex_init(&link_aslog->mutex_, NULL);
		link_aslog->latch_ = countdownlatch_init(&link_aslog->mutex_, 1);
		link_aslog->cond_ = condition_init(&link_aslog->mutex_);
		arbuffer_init(&link_aslog->currentBuffer_, LINKAGEASYNCLOG_BUFMAX);
		arbuffer_init(&link_aslog->nextBuffer_, LINKAGEASYNCLOG_BUFMAX);
		arraybufferalign_init(&link_aslog->buffers_, LINKAGEASYNCLOG_MAXBUFFER + 1);
	}
	
}

void linkageasynclog_init()
{
	linkageasynclog_def_init(LINKAGEASYNCLOG_FLUSHTIME);
}

