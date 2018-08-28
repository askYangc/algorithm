#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>
#include <assert.h>
#include <sys/syscall.h>


#include "stlc_list.h"
#include "itimer.h"
#include "itimer_internal.h"

#define DEF_ITIMER_POOL_ADD_SIZE 20
#define MAX_THREAD_TASK 48
#define MAX_POOL_BUCKET 1024


#define itimer_pool_lock(evt_pool) pthread_mutex_lock(&((evt_pool)->lock));
#define itimer_pool_unlock(evt_pool) pthread_mutex_unlock(&((evt_pool)->lock));

enum {
	ITIMER_TASK_USE = 1,
	ITIMER_TASK_UNUSE = 2,
};

typedef struct itimer_task {
	int state;		/* ITIMER_TASK_USE or ITIMER_TASK_UNUSE */

	pid_t tid;
	pthread_mutex_t lock;
	struct stlc_list_head _task;
}itimer_task_t;

typedef struct {
	itimer_mgr mgr;
	itimer_task_t task[MAX_THREAD_TASK];
}itimer_thread_mgr_t;


typedef struct {
	pthread_mutex_t lock;
	struct stlc_list_head _free;
	u_int32_t alloc;
	u_int32_t unuse;
	u_int64_t ghandle;

	struct stlc_list_head bucket[MAX_POOL_BUCKET];
}itimer_evt_pool_t;

static itimer_thread_mgr_t *thread_mgr = NULL;   /* just gcc */
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

static pid_t GetPid()
{
    static __thread pid_t pid = 0;
    static __thread pid_t tid = 0;
    if( !pid || !tid || pid != getpid() )
    {
        pid = getpid();
#if defined( __APPLE__ )
		tid = syscall( SYS_gettid );
		if( -1 == (long)tid )
		{
			tid = pid;
		}
#elif defined( __FreeBSD__ )
		syscall(SYS_thr_self, &tid);
		if( tid < 0 )
		{
			tid = pid;
		}
#else 
        tid = syscall( __NR_gettid );
#endif

    }
    return tid;
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

#define itimer_bucket_hash(handle) ((handle)%MAX_POOL_BUCKET)
static void itimer_evt_add_bucket(itimer_evt *evt)
{
	if(evt) {
		int key = itimer_bucket_hash(evt->handle);
		stlc_list_add_tail(&evt->link, &evt_pool->bucket[key]);
	}
}

static itimer_evt *itimer_evt_bucket_lookup(u_int64_t handle)
{
	itimer_evt *evt = NULL;
	int key = itimer_bucket_hash(handle);

	itimer_pool_lock(evt_pool);
	stlc_list_for_each_entry(evt, &evt_pool->bucket[key], link) {
		if(evt->handle == handle) {
			itimer_pool_unlock(evt_pool);
			return evt;
		}
	}

	itimer_pool_unlock(evt_pool);
	return NULL;
}


static void itimer_evt_del_bucket(itimer_evt *evt)
{
	if(evt) {
		stlc_list_del(&evt->link);
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
	evt->handle = evt_pool->ghandle++;	/* not care ghandle is 0 */

	itimer_evt_add_bucket(evt);
	
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
		itimer_evt_del_bucket(evt);
		stlc_list_add_tail(&evt->link, &evt_pool->_free);
		evt_pool->unuse++;
		itimer_pool_unlock(evt_pool);
	}
}


u_int64_t itimer_set(itimer_func_t f, void *data, void *user, u_int32_t timer, int repeat)
{
	itimer_evt *evt;

	evt = itimer_evt_pool_alloc();
	if(evt == NULL) {
		return 0;
	}

	evt->evt_free = itimer_unset;
	itimer_evt_init(evt, f, data, user);
	itimer_evt_start(&thread_mgr->mgr, evt, timer, repeat);	

	return evt->handle;
}

void itimer_unset(u_int64_t handle)
{
	itimer_evt *evt;

	if(handle == 0) {
		return ;
	}

	evt = itimer_evt_bucket_lookup(handle);
	if(evt) {
		itimer_evt_destroy(evt);
		itimer_evt_pool_free(evt);
	}
}

void itimer_run()
{
	itimer_mgr_run(&thread_mgr->mgr, GetTickMS());
}

void *itimer_thread_timer(void *p)
{

	for(;;) {
		itimer_run();
		usleep(2500);
	}

	return NULL;	
}


void itimer_evt_pool_init()
{
	int i = 0;
	assert(!evt_pool);
	evt_pool = (itimer_evt_pool_t*)calloc(1, sizeof(itimer_evt_pool_t));
	assert(evt_pool);

	evt_pool->ghandle = 1;
	evt_pool->alloc = evt_pool->unuse = 0;
	STLC_INIT_LIST_HEAD(&evt_pool->_free);

	for(i = 0; i < MAX_POOL_BUCKET; i++) {
		STLC_INIT_LIST_HEAD(&evt_pool->bucket[i]);
	}

	pthread_mutex_init(&evt_pool->lock, NULL);
}

void itimer_task_init(itimer_thread_mgr_t *mgr)
{
	int i = 0;
	itimer_task_t *t;
	for(i = 0; i < MAX_THREAD_TASK; i++) {
		t = &mgr->task[i];
		t->state = ITIMER_TASK_UNUSE;
		STLC_INIT_LIST_HEAD(&t->_task);
		pthread_mutex_init(&t->lock, NULL);
	}
}

void itimer_mgr_thread_init(unsigned       interval)
{
	thread_mgr = (itimer_thread_mgr_t*)calloc(1, sizeof(itimer_thread_mgr_t));
	assert(thread_mgr);

	itimer_task_init(thread_mgr);
	
	itimer_mgr_init(&thread_mgr->mgr, GetTickMS(), interval);
}

void itimer_work_init(unsigned     interval)
{
	pthread_t pid;
	itimer_evt_pool_init();
	itimer_mgr_thread_init(interval);
	pthread_create(&pid, NULL, itimer_thread_timer, NULL);
}

