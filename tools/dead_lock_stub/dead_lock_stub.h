#ifndef __DEAD_LOCK_STUB_H__
#define __DEAD_LOCK_STUB_H__

#include <stdio.h>

#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <execinfo.h>
#include "stlc_list.h"
#include "ds_types.h"


/* use this need #include "dead_lock_stub.h" and add code DeadLockGraphic::getInstance().start_check(); */


#ifndef COLOR_END

#define	COLOR_RED	"\033[31m\033[1m"
#define	COLOR_GREEN	"\033[32m\033[1m"
#define	COLOR_YELLOW	"\033[33m\033[1m"
#define	COLOR_END	"\033[0m"

#endif

#define WAIT_MUTEXLOCK_TIMEOUT 5
#define WAIT_HOLD_THREAD_MAX 10
#define MAX_DEAD_MUTEXLOCK_CHECK 100
#define MAX_STACKTRACE	10

typedef struct {
	struct stlc_list_head link;
    int indegress;
	u_int64_t thread_id;

	int thread_count;
    u_int64_t threads[10];
}thread_graphic_vertex_t;

typedef struct {
	struct stlc_list_head link;
    u_int8_t valid;
    u_int32_t t;
	u_int64_t thread_id;
    u_int64_t lock_addr;
    char lock_name[32];
    char file[32];
    int line;
    char funcname[32];       
	char **stacktrace;
	u_int8_t stacktrace_num;
}thread_req_lock_t;


typedef struct {
    struct stlc_list_head link;
    char file[20];
    int line;
    char funcname[32];   
	char **stacktrace;
	u_int8_t stacktrace_num;
}hold_lock_point_t;


typedef struct {
	struct stlc_list_head link;
    struct stlc_list_head point_list;   //hold_lock_point_t
    int count;
    u_int64_t thread_id; 
	u_int64_t lock_addr;
    char lock_name[32];    
}thread_hold_lock_t;

static inline int thread_time_2_str(char *s, int len,  time_t t) 
{
    struct tm tm;

    localtime_r (&t, &tm);
    return strftime (s, len, "%y-%m-%d %T", &tm);
}


extern struct stlc_list_head m_thread_apply_lock;   //thread_req_lock_t
extern struct stlc_list_head m_lock_belong_thread;  //thread_hold_lock_t
extern pthread_mutex_t deadlock_mutex;

thread_req_lock_t *thread_req_lock_lookup(u_int64_t thread_id);
thread_req_lock_t *thread_req_lock_lookup_or_alloc(u_int64_t thread_id);
void thread_req_lock_init(thread_req_lock_t *l, 
	u_int64_t thread_id, u_int64_t lock_addr, const char *file, int line, const char *funcname, const char *lock_name, char **stacktrace, u_int8_t num);


thread_hold_lock_t *thread_hold_lock_lookup(u_int64_t lock_addr);
thread_hold_lock_t *thread_hold_lock_lookup_or_alloc(u_int64_t lock_addr);
void thread_hold_lock_pushstack(thread_hold_lock_t *h, const char *file, int line, const char *funcname, char **stacktrace, u_int8_t stacktrace_num);
void thread_hold_lock_popstack(thread_hold_lock_t *h);
int my_backtrace(void **buffer,int size);

void _check_dead_lock_stub();
void _show_thread_locks_info();


static inline void deadlockcheck_before(u_int64_t thread_id, u_int64_t lock_addr, const char *file, int line, const char *funcname, const char *lock_name, char **stacktrace, u_int8_t num)
{
	thread_req_lock_t *l;
    pthread_mutex_lock(&deadlock_mutex);


	// (A) m_thread_apply_lock, 添加 thread_id => lock_addr
    l = thread_req_lock_lookup_or_alloc(thread_id);
	if(l == NULL) {
		return;
	}

	thread_req_lock_init(l, thread_id, lock_addr, file, line, funcname, lock_name, stacktrace, num);

	l->valid = 1;
    pthread_mutex_unlock(&deadlock_mutex);
}

static inline void deadlockcheck_after(u_int64_t thread_id, u_int64_t lock_addr, const char *file, int line, const char *funcname, const char *lock_name, char **stacktrace, u_int8_t num)
{
	thread_req_lock_t *l;
	thread_hold_lock_t *h;
    pthread_mutex_lock(&deadlock_mutex);     

	// (B)m_thread_apply_lock, 去除 thread_id => lock_addr     
	l = thread_req_lock_lookup(thread_id);
	if(l) {
		l->valid = 0;
		l->stacktrace_num = 0;
		l->stacktrace = NULL;
	}
	              
	// (A)m_lock_belong_thread, add lock_addr => thread_id        
	h = thread_hold_lock_lookup_or_alloc(lock_addr);
	if(h) {
		h->thread_id = thread_id;
		memset(h->lock_name, 0, sizeof(h->lock_name));
		memcpy(h->lock_name, lock_name, sizeof(h->lock_name) -1);
	}

	thread_hold_lock_pushstack(h, file, line, funcname, stacktrace, num);
            
	//m_lock_belong_thread[lock_addr] = thread_id;        
	pthread_mutex_unlock(&deadlock_mutex);
}

static inline void deadlockcheck_unlock_after(u_int64_t thread_id, u_int64_t lock_addr)
{
	thread_hold_lock_t *h;
    pthread_mutex_lock(&deadlock_mutex);
	
	// (B)m_lock_belong_thread, remove lock_addr => thread_id      
	h = thread_hold_lock_lookup_or_alloc(lock_addr);
	if(h) {
		thread_hold_lock_popstack(h);
	}
       
	pthread_mutex_unlock(&deadlock_mutex);
}

static inline void check_dead_lock_stub()
{
	pthread_mutex_lock(&deadlock_mutex);
	_check_dead_lock_stub();
	pthread_mutex_unlock(&deadlock_mutex);
}

static inline void show_thread_locks_info()
{
	pthread_mutex_lock(&deadlock_mutex);
	_show_thread_locks_info();
	pthread_mutex_unlock(&deadlock_mutex);
}

#include <sys/syscall.h>

#define gettid() syscall(__NR_gettid)

// 拦截lock, 添加before, after操作, 记录锁与线程的关系
#define pthread_mutex_lock(x)                                                                       \
    do {                                                                                            \
		void * array[MAX_STACKTRACE];																			\
		int stack_num = my_backtrace(array, MAX_STACKTRACE);													\
		char **stacktrace = backtrace_symbols(array, stack_num);								\
		deadlockcheck_before(gettid(), (u_int64_t)(x), __FILE__, __LINE__, __FUNCTION__, #x, stacktrace, stack_num);        \
        pthread_mutex_lock(x);                                                                      \
        deadlockcheck_after(gettid(), (u_int64_t)(x), __FILE__, __LINE__, __FUNCTION__, #x, stacktrace, stack_num);         \
    } while (0);

// 拦截unlock, 添加after操作, 解除锁和线程的关系
#define pthread_mutex_unlock(x)                                                                     \
    do {                                                                                            \
        pthread_mutex_unlock(x);                                                                    \
        deadlockcheck_unlock_after(gettid(), (u_int64_t)(x));                                        \
    } while(0);


//* Outside calls the function *//
void dead_lock_stub_start_check(int run);


#endif
