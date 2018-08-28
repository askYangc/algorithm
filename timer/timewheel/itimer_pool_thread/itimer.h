#ifndef _ITIMER_H_
#define _ITIMER_H_

/* timer pool in threads */
/* if use ITIMER_OFF, must use itimer_evt_pool_init() before */
#include "ds_types.h"
#include "itimer_internal.h"

typedef void (*itimer_func_t)(void *data, void *user);
u_int64_t itimer_set(itimer_func_t f, void *data, void *user, u_int32_t timer, int repeat);
void itimer_unset(u_int64_t handle);

// 时间单位: 毫秒
#define ITIMER_THREAD_ON(handle,func,arg,time,repeat) \
	do { \
		if (handle != 0) \
			itimer_unset(handle); \
		handle = itimer_set (func, arg, NULL, time, repeat); \
	} while (0)

		
#define ITIMER_THREAD_OFF(handle) \
  do { \
    if (handle) \
      { \
        itimer_unset (handle); \
        handle = 0; \
      } \
  } while (0)


void itimer_run();
void itimer_evt_pool_init();
void itimer_mgr_thread_init(unsigned       interval);

#endif
