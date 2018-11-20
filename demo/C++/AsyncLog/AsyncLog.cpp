#include <stdio.h>
#include "AsyncLog.h"
#include "LogFile.h"

void get_assign_time(char *buf, int buf_size, time_t t)
{
	struct tm tm;

	localtime_r (&t, &tm);
	strftime (buf, buf_size, "%Y-%m-%d-%H-%M-%S", &tm);
}


AsyncLogging::AsyncLogging(const string& basename,
                           size_t rollSize,
                           int flushInterval)
  : flushInterval_(flushInterval),
    running_(false),
    basename_(basename),
    rollSize_(rollSize),
    mutex_(),
    cond_(mutex_),
    latch_(1),
    currentBuffer_(new Buffer),
    nextBuffer_(new Buffer),
    buffers_()
{
	currentBuffer_->bzero();
	nextBuffer_->bzero();
	buffers_.reserve(16);
}

void AsyncLogging::append(const char* logline, int len)
{
	MutexLockGuard lock(mutex_);
	if (currentBuffer_->avail() > len) {
		currentBuffer_->append(logline, len);	/* 长度能写，就直接加入到当前缓冲 */
	}else {
		buffers_.push_back(currentBuffer_.release());	/* 长度不够则直接将当前缓存给弄进一个缓冲队列buffers_ */

		if (nextBuffer_) {
			currentBuffer_ = boost::ptr_container::move(nextBuffer_);
		}else {
		  	currentBuffer_.reset(new Buffer); // Rarely happens
		}
		currentBuffer_->append(logline, len);
		cond_.notify();
	}
}

void AsyncLogging::threadFunc()
{
	assert(running_ == true);
	latch_.countDown();
	LogFile output(basename_, rollSize_, false);
	BufferPtr newBuffer1(new Buffer);
	BufferPtr newBuffer2(new Buffer);
	newBuffer1->bzero();
	newBuffer2->bzero();
	BufferVector buffersToWrite;
	buffersToWrite.reserve(16);
	
	while (running_) {
		assert(newBuffer1 && newBuffer1->length() == 0);
		assert(newBuffer2 && newBuffer2->length() == 0);
		assert(buffersToWrite.empty());

		{
			MutexLockGuard lock(mutex_);
			if (buffers_.empty()) {  // unusual usage!
				cond_.waitForSeconds(flushInterval_);		/* 如果缓冲队列为空，则等待最多3秒时间或者被append唤醒来继续下一步操作 */
			}

			buffers_.push_back(currentBuffer_.release());	/* 不管有没有数据都讲currentBuffer_的数据加入队列 */
			currentBuffer_ = boost::ptr_container::move(newBuffer1);	/* 将本地的buffer替换当前缓冲，应该是为了append快速结束 */
			buffersToWrite.swap(buffers_);				/* 临界的buffers_被交换为本地的buffters */

			if (!nextBuffer_) {
				nextBuffer_ = boost::ptr_container::move(newBuffer2);	/* 将本地的buffer2替换备用缓冲，应该是为了append快速结束 */
			}
		}

		assert(!buffersToWrite.empty());

		if (buffersToWrite.size() > 25) {
			char buf[256];
			char tbuf[100] = {0};
			::get_assign_time(tbuf, 100, ::time(NULL));
			snprintf(buf, sizeof buf, "Dropped log messages at %s, %zd larger buffers\n",
			tbuf,
			buffersToWrite.size()-2);
			fputs(buf, stderr);
			output.append(buf, static_cast<int>(strlen(buf)));
			buffersToWrite.erase(buffersToWrite.begin()+2, buffersToWrite.end());
		}

		for (size_t i = 0; i < buffersToWrite.size(); ++i) {
			// FIXME: use unbuffered stdio FILE ? or use ::writev ?
			output.append(buffersToWrite[i].data(), buffersToWrite[i].length());	/* 将数据缓冲进文件 */
		}

		if (buffersToWrite.size() > 2) {
			// drop non-bzero-ed buffers, avoid trashing
			buffersToWrite.resize(2);			/* 在短时间内大量的append操作产生的多个队列只保留2个就可以了，用来分配给本地buffer1和buffer2 */
		}

		if (!newBuffer1) {
			assert(!buffersToWrite.empty());
			newBuffer1 = buffersToWrite.pop_back();	/* 保留的第一块buffer，分给本地buffer1 */
			newBuffer1->reset();
		}

		if (!newBuffer2)
		{
			assert(!buffersToWrite.empty());
			newBuffer2 = buffersToWrite.pop_back();	/* 保留的第一块buffer，分给本地buffer2 */
			newBuffer2->reset();
		}

		buffersToWrite.clear();			/* 有可能nextBuffer_不是空的，所以newBuffers没有赋值给nextBuffer_，所以这里还多了一块，需要清除掉 */
		output.flush();
	}
	output.flush();
}


void *Logthread(void *p)
{
	AsyncLogging *t = (AsyncLogging*)p;
	//boost::function<void ()> f = boost::bind(&AsyncLogging::threadFunc, t);

	t->threadFunc();

	return NULL;
}

void AsyncLogging::start()
{
	running_ = true;
	pthread_create(&pid_, NULL, Logthread, this);
	latch_.wait();
}

void AsyncLogging::stop()
{
	if(running_) {
		running_ = false;
		cond_.notify();
		printf("start join\n");
		pthread_join(pid_, NULL);
		printf("end join\n");
	}
}
