#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>
#include <assert.h>

#include "stlc_list.h"
#include "itimer.h"
#include "itimer_internal.h"

#define DEF_ITIMER_POOL_ADD_SIZE 20

#define itimer_pool_lock(evt_pool) pthread_mutex_lock(&((evt_pool)->lock));
#define itimer_pool_unlock(evt_pool) pthread_mutex_unlock(&((evt_pool)->lock));


typedef struct {
	itimer_mgr mgr;
}itimer_thread_mgr_t;


typedef struct {
	pthread_mutex_t lock;
	struct stlc_list_head _free;
	u_int32_t alloc;
	u_int32_t unuse;
}itimer_evt_pool_t;

static __thread itimer_thread_mgr_t *thread_mgr = NULL;   /* just gcc */
static itimer_evt_pool_t *evt_pool = NULL;

static unsigned long long GetTickMS()
{
#if defined( __LIBCO_RDTSCP__)
    static uint32_t khz = getCpuKhz();
    return counter() / khz;
#else
    struct timeval now = { 0 };
    gettimeofday( &now,NULL );
    unsigned long long u = now.tv_sec;
    u *= 1000;
    u += now.tv_usec / 1000;
    return u;
#endif
}

static void itimer_evt_pool_add(int size) {
	int i = 0;
	itimer_evt *evt;
	for(i = 0; i < size; i++) {
		evt = (itimer_evt*)calloc(1, sizeof(itimer_evt));		
		stlc_list_add_tail(&evt->link, &evt_pool->_free);
	}
	if(size > 0) {
		evt_pool->alloc += size;
		evt_pool->unuse += size;
	}
}


static itimer_evt *itimer_evt_pool_alloc()
{
	itimer_evt *evt = NULL;

	if(!evt_pool)
		return (itimer_evt*)calloc(1, sizeof(itimer_evt));
	
	itimer_pool_lock(evt_pool);

	if(stlc_list_empty(&evt_pool->_free)) {
		itimer_evt_pool_add(DEF_ITIMER_POOL_ADD_SIZE);
	}
	evt = stlc_list_first_entry(&evt_pool->_free, itimer_evt, link);
	stlc_list_del(&evt->link);
	evt_pool->unuse--;
	
	itimer_pool_unlock(evt_pool);

	return evt;
}

static void itimer_evt_pool_free(itimer_evt *evt)
{
	if(evt) {
		if(!evt_pool) {
			free(evt);
			return;
		}

		itimer_pool_lock(evt_pool);
		stlc_list_add_tail(&evt->link, &evt_pool->_free);
		evt_pool->unuse++;
		itimer_pool_unlock(evt_pool);
	}
}


itimer_evt *itimer_set(itimer_func_t f, void *data, void *user, u_int32_t timer, int repeat)
{
	itimer_evt *evt;

	evt = itimer_evt_pool_alloc();
	if(evt == NULL) {
		return NULL;
	}
		
	itimer_evt_init(evt, f, data, user);
	itimer_evt_start(&thread_mgr->mgr, evt, timer, repeat);	

	return evt;
}

void itimer_unset(itimer_evt *evt)
{
	if(evt == NULL)
		return ;

	itimer_evt_destroy(evt);
	itimer_evt_pool_free(evt);
}

void itimer_run()
{
	itimer_mgr_run(&thread_mgr->mgr, GetTickMS());
}


void itimer_evt_pool_init()
{
	assert(!evt_pool);
	evt_pool = (itimer_evt_pool_t*)calloc(1, sizeof(itimer_evt_pool_t));
	assert(evt_pool);

	STLC_INIT_LIST_HEAD(&evt_pool->_free);
	pthread_mutex_init(&evt_pool->lock, NULL);
}


void itimer_mgr_thread_init(unsigned       interval)
{
	thread_mgr = (itimer_thread_mgr_t*)calloc(1, sizeof(itimer_thread_mgr_t));
	assert(thread_mgr);
	itimer_mgr_init(&thread_mgr->mgr, GetTickMS(), interval);
}

