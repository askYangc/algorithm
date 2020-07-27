#ifndef _FLEX_BUFFER_H_
#define _FLEX_BUFFER_H_

/// A buffer class modeled after org.jboss.netty.buffer.ChannelBuffer
///
/// @code
/// +-------------------+------------------+------------------+
/// | prependable bytes |  readable bytes  |  writable bytes  |
/// |                   |     (CONTENT)    |                  |
/// +-------------------+------------------+------------------+
/// |                   |                  |                  |
/// 0      <=      readerIndex   <=   writerIndex    <=     size
/// @endcode

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


typedef struct {
    char *_buffer;

    int readerIndex_;
    int writerIndex_;
    int size;
}flex_buffer_t;

typedef struct {
	flex_buffer_t *buffers_;
	int cout;
	int max;
}alignflexbuffer_t;


static inline int readableBytes(flex_buffer_t *b)
{ 
    return b->writerIndex_ - b->readerIndex_; 
}

static inline int writableBytes(flex_buffer_t *b)
{ 
    return b->size - b->writerIndex_; 
}

static inline char* flex_peek(flex_buffer_t *b)
{ 
    return b->_buffer + b->readerIndex_; 
}

static inline const char* flex_beginWrite(flex_buffer_t *b)
{ 
    return b->_buffer + b->writerIndex_; 
}


static inline void flex_retrieveAll(flex_buffer_t *b)
{
  b->readerIndex_ = b->writerIndex_ = 0;
  memset(b->_buffer, 0, b->size);
}

// retrieve returns void, to prevent
// string str(retrieve(readableBytes()), readableBytes());
// the evaluation of two functions are unspecified
static inline void flex_retrieve(flex_buffer_t *b, int len)
{
  assert(len <= readableBytes(b));
  if (len < readableBytes(b))
  {
    memset(&b->_buffer[b->readerIndex_], 0, len);
    b->readerIndex_ += len;
  }
  else
  {
    flex_retrieveAll(b);
  }
}

static inline void retrieveUntil(flex_buffer_t *b, const char* end)
 {
   assert(flex_peek(b) <= end);
   assert(end <= flex_beginWrite(b));
   flex_retrieve(b, end - flex_peek(b));
 }


static inline int retrieveReadData(flex_buffer_t *b, char *data, int len)
{
  if(len > readableBytes(b))
    len = readableBytes(b);
  memcpy(data, flex_peek(b), len);
  flex_retrieve(b, len);
  return len;
}

static inline void flex_makeSpace(flex_buffer_t *b, size_t len)
{
    if(b->readerIndex_ != 0) {
        int readable = readableBytes(b);
        memmove(b->_buffer, b->_buffer+b->readerIndex_, readable);
        b->readerIndex_ = 0;
        b->writerIndex_ = b->readerIndex_ + readable;
    }
    
    if (writableBytes(b) < len) {
        char *tmp = (char*)realloc(b->_buffer, b->writerIndex_ + len);
        assert(tmp != NULL);
        b->_buffer = tmp;
        memset(&b->_buffer[b->size], 0, b->writerIndex_ + len - b->size);
        b->size = b->writerIndex_ + len;
    }
}

static inline void ensureWritableBytes(flex_buffer_t *b, int len)
{
  if (writableBytes(b) < len)
  {
    flex_makeSpace(b, len);
  }
  assert(writableBytes(b) >= len);
}

static inline void flex_hasWritten(flex_buffer_t *b, size_t len)
{
  assert(len <= writableBytes(b));
  b->writerIndex_ += len;
}


static inline int flex_append(flex_buffer_t *b, char *data, int len)
{
    ensureWritableBytes(b, len);
    memcpy(b->_buffer + b->writerIndex_, data, len);    
    flex_hasWritten(b, len);
    return len;
}



static inline void flex_buffer_clean(flex_buffer_t *b, int isfree)
{
    if(b) {
        b->readerIndex_ = b->writerIndex_ = 0;
        b->size = 0;
        if(isfree && b->_buffer) {
            free(b->_buffer);
        }
        b->_buffer = NULL;
    }
}

static inline void flex_buffer_move(flex_buffer_t *dst, flex_buffer_t *src, int clean)
{
    if(clean) {
        flex_buffer_clean(dst, 1);
    }
    memcpy(dst, src, sizeof(flex_buffer_t));
    flex_buffer_clean(src, 0);
}

static inline int flex_buffer_usable(flex_buffer_t *b)
{
    return b->_buffer?1:0;
}


static inline void flex_buffer_free(flex_buffer_t *b)
{
    if(b) {
        if(b->_buffer)
            free(b->_buffer);
        free(b);
    }
}

#define flex_buffer_safe_free(b) do{\
    if(b){\
        flex_buffer_free(b);\
        b = NULL;\
    }\
}while(0)

static inline flex_buffer_t *flex_buffer_init(int size)
{
    flex_buffer_t *b = (flex_buffer_t*)calloc(1, sizeof(flex_buffer_t));
    if(b) {
        b->_buffer = (char*)calloc(1, size);
        if(!b->_buffer) {
            free(b);
            return NULL;
        }
        b->readerIndex_ = b->writerIndex_ = 0;
        b->size = size;
    }
    
    return b;
}


static inline int flex_buffer_recalloc(flex_buffer_t *b, int size)
{
    if(b) {
        if(b->_buffer) {
            free(b->_buffer);
        }
        b->_buffer = (char*)calloc(1, size);
        if(!b->_buffer) {
            free(b);
            return -1;
        }
        b->readerIndex_ = b->writerIndex_ = 0;
        b->size = size;
    }
    
    return 0;
}


static inline void alignflexbuffer_add_tail(alignflexbuffer_t *t, flex_buffer_t *buffer)
{
	if(t->cout >= t->max)
		return ;
	t->buffers_[t->cout++] = *buffer;
    flex_buffer_clean(buffer, 0);
}

static inline void alignflexbuffer_erase(alignflexbuffer_t *t, int begin, int end)
{
	int i = 0; 
	for(i = begin; i < end; i++) {
		flex_buffer_clean(&t->buffers_[i], 1);
	}
	if(begin >= 0) {
		t->cout = begin;
	}
}

static inline void alignflexbuffer_move(alignflexbuffer_t *dst, alignflexbuffer_t *src)
{
	int max = (dst->max>src->cout)?src->cout:dst->max;

	alignflexbuffer_erase(dst, 0, dst->cout);
	memcpy(dst->buffers_, src->buffers_, max*sizeof(flex_buffer_t));
	memset(src->buffers_, 0, max*sizeof(flex_buffer_t));
	dst->cout = max;
	src->cout = 0;	
}

static inline void alignflexbuffer_free(alignflexbuffer_t *t)
{
	if(t) {
		if(t->buffers_) {
			alignflexbuffer_erase(t, 0, t->cout);
			free(t->buffers_);
			t->buffers_ = NULL;
		}
		t->cout = t->max = 0;
	}
}

static inline void alignflexbuffer_clear(alignflexbuffer_t *t)
{
	alignflexbuffer_erase(t, 0, t->cout);
}

static inline void alignflexbuffer_init(alignflexbuffer_t *t, int max)
{
	t->buffers_ = (flex_buffer_t*)calloc(1, sizeof(flex_buffer_t)*max);
	t->max = max;
}



#endif
