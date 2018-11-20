#ifndef BASE_ASYNCLOG_H
#define BASE_ASYNCLOG_H

#include <pthread.h>
#include <string>

#include "../mutex/Mutex.h"
#include "../mutex/Condition.h"
#include "../mutex/CountDownLatch.h"


#include <boost/function.hpp>
#include <boost/noncopyable.hpp>

#include <boost/bind.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/ptr_container/ptr_vector.hpp>


using namespace std;


#define kLargeBuffer 4000*1000

template<typename To, typename From>
inline To implicit_cast(From const &f)
{
  return f;
}


template<int SIZE>
class FixedBuffer : boost::noncopyable
{
 public:
  FixedBuffer()
    : cur_(data_)
  {
    
  }

  ~FixedBuffer()
  {
  
  }

  void append(const char* /*restrict*/ buf, size_t len)
  {
    // FIXME: append partially
    if (implicit_cast<size_t>(avail()) > len)
    {
      memcpy(cur_, buf, len);
      cur_ += len;
    }
  }

  const char* data() const { return data_; }
  int length() const { return static_cast<int>(cur_ - data_); }

  // write to data_ directly
  char* current() { return cur_; }
  int avail() const { return static_cast<int>(end() - cur_); }
  void add(size_t len) { cur_ += len; }

  void reset() { cur_ = data_; }
  void bzero() { ::bzero(data_, sizeof data_); }


 private:
  const char* end() const { return data_ + sizeof data_; }
  // Must be outline function for cookies.

  char data_[SIZE];
  char* cur_;
};


class AsyncLogging : boost::noncopyable {
public:
	AsyncLogging(const string& basename,
	       size_t rollSize,
	       int flushInterval = 3);

	~AsyncLogging() {
		if (running_) {
			stop();
		}
	}	
	void append(const char* logline, int len);
	void start();

	void stop();
	void threadFunc();	
	bool getrun() {return running_;}
private:

	// declare but not define, prevent compiler-synthesized functions
	AsyncLogging(const AsyncLogging&);  // ptr_container
	void operator=(const AsyncLogging&);  // ptr_container



	typedef FixedBuffer<kLargeBuffer> Buffer;
	typedef boost::ptr_vector<Buffer> BufferVector;
	typedef BufferVector::auto_type BufferPtr;

	const int flushInterval_;
	bool running_;
	string basename_;
	size_t rollSize_;
	MutexLock mutex_;
	Condition cond_;
	CountDownLatch latch_;
	BufferPtr currentBuffer_;
	BufferPtr nextBuffer_;
	BufferVector buffers_;	

	pthread_t pid_;
};


#endif
