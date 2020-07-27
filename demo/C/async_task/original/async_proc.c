#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "async_proc.h"


/*
普通写不连续数据流(加锁)
*/
void async_proc_append_stream(asyncProc_t *p, async_stream_t *buffer)
{
    int i = 0;
	pthread_mutex_lock(&p->mutex_);

    for(i = 0; i < buffer->cout; i++) {
        flex_append(p->currentBuffer_, buffer->data[i], buffer->len[i]);  //直接写进去,一个buffer不宜太长
    }

    if(readableBytes(p->currentBuffer_) >= ASYNCPROC_BUFMAX) {
        //可以替换下一个缓冲了
        alignflexbuffer_add_tail(&p->buffers_, p->currentBuffer_);  /* 加入缓冲 */
        
		if(flex_buffer_usable(p->nextBuffer_)) {
			flex_buffer_move(p->currentBuffer_, p->nextBuffer_, 0);   /* 难得calloc，直接从已经分配好的nextBuffer_里面替换 */
		}else {
		    //没有则直接重新分配一个
		    flex_buffer_recalloc(p->currentBuffer_, ASYNCPROC_BUFMAX);
		}

        condition_notify(p->cond_);
    }

	pthread_mutex_unlock(&p->mutex_);
}

/*
普通写数据(加锁)
*/
void async_proc_append(asyncProc_t *p, char* data, int len)
{
	pthread_mutex_lock(&p->mutex_);
    flex_append(p->currentBuffer_, data, len);  //直接写进去,一个buffer不宜太长

    if(readableBytes(p->currentBuffer_) >= ASYNCPROC_BUFMAX) {
        //可以替换下一个缓冲了
        alignflexbuffer_add_tail(&p->buffers_, p->currentBuffer_);  /* 加入缓冲 */
        
		if(flex_buffer_usable(p->nextBuffer_)) {
			flex_buffer_move(p->currentBuffer_, p->nextBuffer_, 0);   /* 难得calloc，直接从已经分配好的nextBuffer_里面替换 */
		}else {
		    //没有则直接重新分配一个
		    flex_buffer_recalloc(p->currentBuffer_, ASYNCPROC_BUFMAX);
		}

        condition_notify(p->cond_);
    }

	pthread_mutex_unlock(&p->mutex_);
}

void *async_proc_threadFunc(void *args)
{
    asyncProc_t *p = (asyncProc_t*)args;
	int i = 0;
	//assert(aslog->running_ == true);
	flex_buffer_t *newBuffer1;
	flex_buffer_t *newBuffer2;
	alignflexbuffer_t buffersToWrite;
    flex_buffer_t *cur;

	buffersToWrite.cout = 0;
    newBuffer1 = flex_buffer_init(ASYNCPROC_BUFMAX);
    newBuffer2 = flex_buffer_init(ASYNCPROC_BUFMAX);
	alignflexbuffer_init(&buffersToWrite, ASYNCPROC_MAXBUFFER + 1);

	countdownlatch_countdown(p->latch_);
	while (p->running_) {
		//assert(newBuffer1 && newBuffer1->length() == 0);
		//assert(newBuffer2 && newBuffer2->length() == 0);
		//assert(buffersToWrite.empty());

		{
			pthread_mutex_lock(&p->mutex_);
			if (p->buffers_.cout <= 0)  { // unusual usage!
				/* 如果缓冲队列为空，则等待最多3秒时间或者被append唤醒来继续下一步操作 */
				condition_waitForSeconds(p->cond_, p->flushInterval_);
			}
			alignflexbuffer_add_tail(&p->buffers_, p->currentBuffer_); /* 不管有没有数据都将currentBuffer_的数据加入队列，有可能会加入一个空的队列 */
			alignflexbuffer_move(&buffersToWrite, &p->buffers_);		/* 临界的buffers_被交换为本地的buffters */
			flex_buffer_move(p->currentBuffer_, newBuffer1, 0);		/* 将本地的buffer替换当前缓冲，应该是为了append快速结束 */
			if (!flex_buffer_usable(p->nextBuffer_)) {
				flex_buffer_move(p->nextBuffer_, newBuffer2, 0);		/* 将本地的buffer2替换备用缓冲，应该是为了append快速结束 */
			}
			pthread_mutex_unlock(&p->mutex_);
		}

    	//assert(!buffersToWrite.empty());

		if (buffersToWrite.cout > ASYNCPROC_MAXBUFFER){
			//超过MAXBUFFER的可能是生产者出问题了，直接只保留前面2个
			alignflexbuffer_erase(&buffersToWrite, 2, ASYNCPROC_MAXBUFFER+1);
		}
		for (i = 0; i < buffersToWrite.cout; ++i) {
			// FIXME: use unbuffered stdio FILE ? or use ::writev ?
			cur = &buffersToWrite.buffers_[i];
			p->proc(p, flex_peek(cur), readableBytes(cur));
		}

		if (buffersToWrite.cout > 2){
			// drop non-bzero-ed buffers, avoid trashing
			/* 在短时间内大量的append操作产生的多个队列只保留2个就可以了，用来分配给本地buffer1和buffer2 */
			alignflexbuffer_erase(&buffersToWrite, 2, ASYNCPROC_MAXBUFFER+1);
		}

		if (!flex_buffer_usable(newBuffer1)) {
			//assert(!buffersToWrite.empty());
			flex_buffer_move(newBuffer1, &buffersToWrite.buffers_[buffersToWrite.cout-1], 0); /* 保留的第一块buffer，分给本地buffer1 */
			flex_retrieveAll(newBuffer1);
			buffersToWrite.cout--;
		}

		if (!flex_buffer_usable(newBuffer2)) {
			//assert(!buffersToWrite.empty());
			flex_buffer_move(newBuffer2, &buffersToWrite.buffers_[buffersToWrite.cout-1], 0);
			flex_retrieveAll(newBuffer2);						/* 保留的第一块buffer，分给本地buffer2 */
			buffersToWrite.cout--;
		}

		alignflexbuffer_clear(&buffersToWrite);			/* 有可能nextBuffer_不是空的，所以newBuffers没有赋值给nextBuffer_，所以这里还多了一块，需要清除掉 */
	}
	alignflexbuffer_clear(&buffersToWrite);
    flex_buffer_free(newBuffer1);
    flex_buffer_free(newBuffer2);
	return NULL;
}

void async_proc_start(asyncProc_t *p)
{
  	p->running_ = 1;
	pthread_create(&p->pid, NULL, async_proc_threadFunc, p);
  	countdownlatch_wait(p->latch_);
}

void async_proc_stop(asyncProc_t *p)
{
	p->running_ = 0;
	condition_notify(p->cond_);
	pthread_join(p->pid, NULL);
}

void async_proc_free(asyncProc_t *p)
{
    pthread_mutex_destroy(&p->mutex_);
    if(p->latch_) {
        countdownlatch_free(p->latch_);
    }
    if(p->cond_) {
        condition_free(p->cond_);
    }
    flex_buffer_free(p->currentBuffer_);
    flex_buffer_free(p->nextBuffer_);
    alignflexbuffer_free(&p->buffers_);
    free(p);
}

asyncProc_t *async_proc_def_init(asyncProc_func_t f, void *data, int flushInterval_)
{
	asyncProc_t *p = calloc(1, sizeof(asyncProc_t));	
	if(p) {
		p->running_ = 0;
		p->flushInterval_ = flushInterval_;
        p->proc = f;
        p->data = data;
    
		pthread_mutex_init(&p->mutex_, NULL);
		p->latch_ = countdownlatch_init(&p->mutex_, 1);
		p->cond_ = condition_init(&p->mutex_);
        p->currentBuffer_ = flex_buffer_init(ASYNCPROC_BUFMAX);
        p->nextBuffer_ = flex_buffer_init(ASYNCPROC_BUFMAX);
		alignflexbuffer_init(&p->buffers_, ASYNCPROC_MAXBUFFER + 1);
	}

    return p;
}

asyncProc_t *async_proc_init(asyncProc_func_t f, void *data)
{
	return async_proc_def_init(f, data, ASYNCPROC_FLUSHTIME);
}
