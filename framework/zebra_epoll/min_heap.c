#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>

#include "ds_types.h"
#include "min_heap.h"

#define TIME_HEAP_NODE_ISIZE 100

static u_int64_t gettime() {
    u_int64_t t;
#if !defined(__APPLE__) || defined(AVAILABLE_MAC_OS_X_VERSION_10_12_AND_LATER)
    struct timespec ti;
    clock_gettime(CLOCK_MONOTONIC, &ti);
    t = (u_int64_t)ti.tv_sec * 1000;
    t += ti.tv_nsec / 1000000;
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    t = (u_int64_t)tv.tv_sec * 1000;
    t += tv.tv_usec / 1000;
#endif
    return t;
}


static inline int time_heap_empty(time_heap_t *h)
{
	return h->cursize == 0;
}


void time_heap_resize(time_heap_t *h)
{
	int i = 0;
	minheap_node_t **tmp = (minheap_node_t**)calloc(1, sizeof(minheap_node_t*)*h->capacity*2);
	assert(tmp);

	h->capacity = h->capacity*2;
	for(i = 0; i < h->cursize; i++) {
		tmp[i] = h->nodes[i];
	}

	free(h->nodes);
	h->nodes = tmp;
}

minheap_node_t *minheap_node_init(cb_func_t cb, void *args, u_int32_t timeout)
{
	minheap_node_t *node = (minheap_node_t*)calloc(1, sizeof(minheap_node_t));
	if(node) {
		node->cb = cb;
		node->args = args;
		node->expire = (u_int32_t)(gettime() + timeout);
	}

	return node;
}

void minheap_node_free(minheap_node_t *node)
{
	if(node) free(node);
}


static void percolate_down(time_heap_t *h, int hole)
{
	int child = 0;
	minheap_node_t *tmp = h->nodes[hole];
	
	for (; ((hole*2)+1) <= (h->cursize-1); hole = child) {
		child = hole * 2 + 1;
	
		if (child < (h->cursize-1) &&
				h->nodes[child+1]->expire < h->nodes[child]->expire) {
			++child;
		}
	
		if (h->nodes[child]->expire < tmp->expire)
			h->nodes[hole] = h->nodes[child];
		else
			break;
	}
	
	h->nodes[hole] = tmp;
}

static void insert_time_heap(minheap_node_t **nodes, minheap_node_t *n, int hole)
{
	int parent = 0;
	for( ; hole > 0; hole=parent) {
		parent = (hole-1)/2; 
		if(nodes[parent]->expire <= n->expire) {
			break;
		}

		nodes[hole] = nodes[parent];  
	}  
	
	nodes[hole] = n;  
}


minheap_node_t *time_heap_add_timer(time_heap_t *h, cb_func_t cb, void *args, u_int32_t timeout)
{
	minheap_node_t *node = NULL;
	if(!h) {
		return NULL;
	}

	if(h->cursize >= h->capacity) {
		time_heap_resize(h);
	}
	node = minheap_node_init(cb, args, timeout);

	insert_time_heap(h->nodes, node, h->cursize++);
	return node;
}

int time_heap_del_timer(time_heap_t *h, minheap_node_t *node)
{
	if(node) {
		/* lazy delete */
		node->cb = NULL;
		node->args = NULL;
	}
	
	return 0;
}

void pop_timer(time_heap_t *h)
{
	if(time_heap_empty(h)) {
		return ;
	}
	if(h->nodes[0]) {
		minheap_node_free(h->nodes[0]);
		h->nodes[0] = NULL;
		h->nodes[0] = h->nodes[--h->cursize];
		percolate_down(h, 0);
	}
}

minheap_node_t *time_heap_top(time_heap_t *h)
{
	if(time_heap_empty(h)) {
		return NULL;
	}
	return h->nodes[0];
}

int time_heap_top_expire(time_heap_t *h, u_int32_t *expire)
{
	if(time_heap_empty(h)) {
		return 0;
	}
	*expire = h->nodes[0]->expire;
	return 1;
}

int time_heap_time_wait(time_heap_t *h, int *time_wait)
{
	u_int32_t expire = 0;
	if(time_heap_top_expire(h, &expire)) {
		u_int32_t t = (u_int32_t)gettime();
		*time_wait = (int)(expire > t? expire-t:1);
		return 1;
	}

	return 0;
}

void time_heap_tick(time_heap_t *h)
{
	 minheap_node_t *tmp = h->nodes[0];
	 u_int32_t cur = (u_int32_t)gettime();
	
	 //循环处理到期定时器
	 while (!time_heap_empty(h)) {
		 if (!tmp)
			 break;
	
		 //如果堆顶定时期没到期，则退出循环
		 if (tmp->expire > cur)
			 break;

		
		 //否则就执行堆顶定时器中的回调函数
		 if (h->nodes[0]->cb) {
			 h->nodes[0]->cb(h->nodes[0]->args);
		 }
	
		 //删除堆元素，同时生成新的堆顶定时器
		 pop_timer(h);
		 
		 tmp = h->nodes[0];
	 }
}


void *cl_time_heap_tick(time_heap_t *h)
{
	 minheap_node_t *tmp = h->nodes[0];
	 u_int32_t cur = (u_int32_t)gettime();
	 void *args = NULL;
	 
	 //循环处理到期定时器
	 while (!time_heap_empty(h)) {
		 if (!tmp)
			 break;
	
		 //如果堆顶定时期没到期，则退出循环
		 if (tmp->expire > cur)
			 break;

		
		 //否则就执行堆顶定时器中的回调函数，不执行回调，返回参数
		 if (h->nodes[0]->args) {
			 args = h->nodes[0]->args;
			 pop_timer(h);
		 	 return args;
			 //h->nodes[0]->cb(h->nodes[0]->args);
		 }
	
		 //删除堆元素，同时生成新的堆顶定时器
		 pop_timer(h);
		 
		 tmp = h->nodes[0];
	 }
	 return args;
}


time_heap_t *time_heap_init()
{
	time_heap_t *h = (time_heap_t*)calloc(1, sizeof(time_heap_t));
	assert(h);
	h->cursize = 0;
	h->capacity = TIME_HEAP_NODE_ISIZE;

	h->nodes = (minheap_node_t**)calloc(1, sizeof(minheap_node_t*)*h->capacity);
	assert(h->nodes);

	return h;
}

void time_head_free(time_heap_t *heap)
{
	int i;
	if(heap) {
		for(i = 0; i < heap->cursize; i++) {
			minheap_node_free(heap->nodes[i]);
		}
		if(heap->nodes) {
			free(heap->nodes);
			heap->nodes = NULL;
		}
		free(heap);
	}
}

