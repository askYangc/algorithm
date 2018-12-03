#ifndef _BASE_ARBUFFER_H_
#define _BASE_ARBUFFER_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct {
	char *buffer;
	char *cur;
	int len;
}arraybuffers_t;

typedef struct {
	arraybuffers_t *buffers_;
	int cout;
	int max;
}arraybufferalign_t;


static inline int arbuffer_usable(arraybuffers_t *buffer)
{
	return buffer->buffer?1:0;
}


static inline char *arbuffer_end(arraybuffers_t *buffer)
{
	return buffer->buffer + buffer->len;
}

static inline int arbuffer_avail(arraybuffers_t *buffer)
{
	return arbuffer_end(buffer) - buffer->cur;
}

static inline int arbuffer_used(arraybuffers_t *buffer)
{
	return buffer->cur - buffer->buffer;
}

static inline void arbuffer_append(arraybuffers_t *buffer, const char* logline, int len)
{
	memcpy(buffer->cur, logline, len);
	buffer->cur += len;
}

static inline void arbuffer_move(arraybuffers_t *dst, arraybuffers_t *src)
{
	dst->buffer = src->buffer;
	dst->cur = dst->buffer;
	dst->len = src->len;
	src->buffer = NULL;
	src->cur = NULL;
	src->len = 0;
}

static inline void arbuffer_reset(arraybuffers_t *buffer)
{
	buffer->cur = buffer->buffer;
}

static inline void arbuffer_clear(arraybuffers_t *buffer)
{
	if(buffer) {
		if(buffer->buffer) 
			free(buffer->buffer);
		buffer->buffer = buffer->cur = NULL;
		buffer->len = 0;
	}
}

static inline int arbuffer_space(arraybuffers_t *buffer)
{
	return buffer->len;
}

static inline void arbuffer_init(arraybuffers_t *buffer, int len)
{
	if(buffer) {
		buffer->buffer = (char*)calloc(1, len);
		buffer->cur = buffer->buffer;
		buffer->len = len;
	}
}

static inline void arraybufferalign_add_tail(arraybufferalign_t *t, arraybuffers_t *buffer)
{
	if(t->cout >= t->max)
		return ;
	t->buffers_[t->cout++] = *buffer;
	buffer->buffer = NULL;
	buffer->cur = NULL;
	buffer->len = 0;
}

static inline void arraybufferalign_erase(arraybufferalign_t *t, int begin, int end)
{
	int i = 0; 
	for(i = begin; i < end; i++) {
		arbuffer_clear(&t->buffers_[i]);
	}
	if(begin >= 0) {
		t->cout = begin;
	}
}

static inline void arraybufferalign_move(arraybufferalign_t *dst, arraybufferalign_t *src)
{
	int max = (dst->max>src->cout)?src->cout:dst->max;

	arraybufferalign_erase(dst, 0, dst->cout);
	memcpy(dst->buffers_, src->buffers_, max*sizeof(arraybuffers_t));
	memset(src->buffers_, 0, max*sizeof(arraybuffers_t));
	dst->cout = max;
	src->cout = 0;	
}

static inline void arraybufferalign_free(arraybufferalign_t *t)
{
	if(t) {
		if(t->buffers_) {
			arraybufferalign_erase(t, 0, t->cout);
			free(t->buffers_);
			t->buffers_ = NULL;
		}
		t->cout = t->max = 0;
	}
}

static inline void arraybufferalign_clear(arraybufferalign_t *t)
{
	arraybufferalign_erase(t, 0, t->cout);
}

static inline void arraybufferalign_init(arraybufferalign_t *t, int max)
{
	t->buffers_ = calloc(1, sizeof(arraybuffers_t)*max);
	t->max = max;
}

#endif
