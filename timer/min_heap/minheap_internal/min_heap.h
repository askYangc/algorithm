#ifndef _MIN_HEAP_H
#define _MIN_HEAP_H

#include "ds_types.h"

typedef void (*cb_func_t)(void *args);

typedef struct {
	u_int32_t expire;
	cb_func_t cb;
	void *args;
}minheap_node_t;

typedef struct time_heap_s {
	minheap_node_t **nodes;
	int capacity;
	int cursize;
}time_heap_t;

minheap_node_t *time_heap_add_timer(time_heap_t *h, cb_func_t cb, void *args, u_int32_t timeout);
int time_heap_del_timer(time_heap_t *h, minheap_node_t *node);

// 时间单位: 毫秒
#define MINHEAP_TIMER_ON(h, node,func,arg,time) \
	do { \
		if (node != NULL) \
			time_heap_del_timer(h, node); \
		node = time_heap_add_timer (h, func, arg, time); \
	} while (0)

		
#define MINHEAP_TIMER_OFF(h, node) \
  do { \
    if (node) \
      { \
        time_heap_del_timer (h, node); \
        node = NULL; \
      } \
  } while (0)



int time_heap_time_wait(time_heap_t *h, int *time_wait);
void time_heap_tick(time_heap_t *h);

time_heap_t *time_heap_init();


#endif
