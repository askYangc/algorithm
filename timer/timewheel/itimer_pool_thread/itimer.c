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

#define itimer_task_lock(task) pthread_mutex_lock(&((task)->lock));
#define itimer_task_unlock(task) pthread_mutex_unlock(&((task)->lock));



enum {
	ITIMER_TASK_USEED = 1,
	ITIMER_TASK_UNUSE = 2,
};


typedef struct itimer_task_s {
	struct stlc_list_head link;

	u_int64_t handle;
	void (*callback)(void *data, void *user);
	void *data;
	void *user;
}itimer_task_t;


typedef struct itimer_task_q_s {
	int state;		/* ITIMER_TASK_USEED or ITIMER_TASK_UNUSE */

	pid_t tid;
	pthread_mutex_t lock;
	struct stlc_list_head _task;	/* itimer_task_t */
}itimer_task_q_t;

typedef struct {
	itimer_mgr mgr;
	
	itimer_task_q_t task[MAX_THREAD_TASK];
}itimer_thread_mgr_t;


typedef struct {
	pthread_mutex_t lock;
	struct stlc_list_head _free;		/* itimer_evt */
	u_int32_t alloc;
	u_int32_t unuse;
	u_int64_t ghandle;

	struct stlc_list_head bucket[MAX_POOL_BUCKET];
}itimer_evt_pool_t;

typedef struct {
	pthread_mutex_t lock;			
	struct stlc_list_head _free;	/* itimer_task_t */
	u_int32_t alloc;
	u_int32_t unuse;	
}itimer_task_pool_t;


static itimer_thread_mgr_t *thread_mgr = NULL;   /* just gcc */
static itimer_evt_pool_t *evt_pool = NULL;
static itimer_task_pool_t *task_pool = NULL;


static itimer_task_t *itimer_task_pool_alloc();
static void itimer_task_pool_free(itimer_task_t *t);


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


static void itimer_evt_pool_add(int size) 
{
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



static void itimer_task_pool_add(int size) 
{
	int i = 0;
	itimer_task_t *t;
	for(i = 0; i < size; i++) {
		t = (itimer_task_t*)calloc(1, sizeof(itimer_task_t));		
		stlc_list_add_tail(&t->link, &task_pool->_free);
	}
	if(size > 0) {
		task_pool->alloc += size;
		task_pool->unuse += size;
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
    
	stlc_list_for_each_entry(evt, &evt_pool->bucket[key], link) {
		if(evt->handle == handle) {
			return evt;
		}
	}

	return NULL;
}


static void itimer_evt_del_bucket(itimer_evt *evt)
{
	if(evt) {
		stlc_list_del(&evt->link);
	}
}

static int push_task_to_q(itimer_task_t *task, int key)
{
	int ret = -1;
	itimer_task_q_t *q = &thread_mgr->task[key];

	itimer_task_lock(q);

	if(q->state == ITIMER_TASK_USEED) {
		ret = 0;
		stlc_list_add_tail(&task->link, &q->_task);	
	}

	itimer_task_unlock(q);

	return ret;
}

static void itimer_push_task(itimer_evt *evt)
{
	itimer_task_t *task;

	if(evt == NULL || evt->task < 0)
		return;

	task = itimer_task_pool_alloc();
	if(task == NULL) {
		return;
	}
	task->handle = evt->handle;
	task->callback = evt->callback;
	task->data = evt->data;
	task->user = evt->user;

	if(push_task_to_q(task, evt->task) != 0) {
		itimer_task_pool_free(task);
	}
}


static itimer_evt *itimer_evt_pool_alloc()
{
	itimer_evt *evt = NULL;
	
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
		//itimer_pool_lock(evt_pool);
		itimer_evt_del_bucket(evt);
		stlc_list_add_tail(&evt->link, &evt_pool->_free);
		evt_pool->unuse++;
		//itimer_pool_unlock(evt_pool);
	}
}

static itimer_task_t *itimer_task_pool_alloc()
{
	itimer_task_t *t = NULL;
	
	itimer_pool_lock(task_pool);
	if(stlc_list_empty(&task_pool->_free)) {
		itimer_task_pool_add(DEF_ITIMER_POOL_ADD_SIZE);
	}
	t = stlc_list_first_entry(&task_pool->_free, itimer_task_t, link);
	stlc_list_del(&t->link);
	task_pool->unuse--;
	
	itimer_pool_unlock(task_pool);

	return t;
}

static void itimer_task_pool_free(itimer_task_t *t)
{
	if(t) {
		itimer_pool_lock(task_pool);
		stlc_list_add_tail(&t->link, &task_pool->_free);
		task_pool->unuse++;
		itimer_pool_unlock(task_pool);
	}
}

static int GetTask()
{
	itimer_task_q_t *q;
	int okey = 0;
	pid_t tid = 0;
	static __thread int key = 0;

	if(key != 0) {
		return key;
	}
	
	tid = GetPid();	
	okey = key = (tid)%MAX_THREAD_TASK;
	
	do {
		q = &thread_mgr->task[key];
		itimer_task_lock(q);
		if(q->state == ITIMER_TASK_UNUSE) {
			q->state = ITIMER_TASK_USEED;
			q->tid = tid;
			itimer_task_unlock(q);
			break;
		}else if(q->tid == tid) {
			itimer_task_unlock(q);
			break;
		}

		if(++key >= MAX_THREAD_TASK) key = 0;
		
		if(key == okey) {
			key = -1;
			itimer_task_unlock(q);
			printf("key is -1, tid is %u\n", tid);
			break;
		}
        itimer_task_unlock(q);
	}while(1);
		
	return key;	
}


u_int64_t itimer_set(itimer_func_t f, void *data, void *user, u_int32_t timer, int repeat)
{
	itimer_evt *evt;

	evt = itimer_evt_pool_alloc();
	if(evt == NULL) {
		return 0;
	}

	evt->evt_free = itimer_unset;
	evt->push_task = itimer_push_task;
	evt->task = GetTask();
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

    itimer_pool_lock(evt_pool);
	evt = itimer_evt_bucket_lookup(handle);
	if(evt) {
		itimer_evt_destroy(evt);
		itimer_evt_pool_free(evt);
	}
    itimer_pool_unlock(evt_pool);
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


int itimer_update_timer()
{
	itimer_task_q_t *q;
	itimer_task_t *pos, *n;
	struct stlc_list_head _free;
	
	int task = GetTask();
	if(task < 0) {
		return -1;
	}

	STLC_INIT_LIST_HEAD(&_free);
	q = &thread_mgr->task[task];
	itimer_task_lock(q);

	stlc_list_for_each_entry_safe(pos, n, &q->_task, link) {
		if(pos->callback) {
			pos->callback(pos->data, pos->user);
		}
		stlc_list_del(&pos->link);
		stlc_list_add_tail(&pos->link, &_free);
	}

	itimer_task_unlock(q);

	stlc_list_for_each_entry_safe(pos, n, &_free, link) {
		stlc_list_del(&pos->link);
		itimer_task_pool_free(pos);
	}

	return 0;
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
	itimer_task_q_t *t;
	for(i = 0; i < MAX_THREAD_TASK; i++) {
		t = &mgr->task[i];
		t->state = ITIMER_TASK_UNUSE;
		STLC_INIT_LIST_HEAD(&t->_task);
		pthread_mutex_init(&t->lock, NULL);
	}
}

void itimer_task_pool_init()
{
	assert(!task_pool);
	task_pool = (itimer_task_pool_t*)calloc(1, sizeof(itimer_task_pool_t));
	assert(task_pool);

	task_pool->alloc = task_pool->unuse = 0;
	STLC_INIT_LIST_HEAD(&task_pool->_free);

	pthread_mutex_init(&task_pool->lock, NULL);
}

void itimer_mgr_thread_init(unsigned             interval)
{
	thread_mgr = (itimer_thread_mgr_t*)calloc(1, sizeof(itimer_thread_mgr_t));
	assert(thread_mgr);
	

	itimer_task_init(thread_mgr);

	itimer_mgr_init(&thread_mgr->mgr, GetTickMS(), interval);
}

void itimer_work_init(unsigned          interval)
{
	pthread_t pid;
	itimer_evt_pool_init();
	itimer_task_pool_init();
	itimer_mgr_thread_init(interval);
	pthread_create(&pid, NULL, itimer_thread_timer, NULL);
}

