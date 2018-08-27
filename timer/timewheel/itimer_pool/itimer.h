#ifndef _ITIMER_H_
#define _ITIMER_H_

/* timer pool in threads */
/* if use ITIMER_OFF, must use itimer_evt_pool_init() before */
#include "ds_types.h"
#include "itimer_internal.h"

typedef void (*itimer_func_t)(void *data, void *user);
itimer_evt *itimer_set(itimer_func_t f, void *data, void *user, u_int32_t timer, int repeat);
void itimer_unset(itimer_evt *evt);

// 时间单位: 毫秒
#define ITIMER_ON(thread,func,arg,time,repeat) \
	do { \
		if (thread != NULL) \
			itimer_unset(thread); \
		thread = itimer_set (func, arg, NULL, time, repeat); \
	} while (0)

		
#define ITIMER_OFF(thread) \
  do { \
    if (thread) \
      { \
        itimer_unset (thread); \
        thread = NULL; \
      } \
  } while (0)


void itimer_run();
void itimer_evt_pool_init();
void itimer_mgr_thread_init(unsigned       interval);

#endif
