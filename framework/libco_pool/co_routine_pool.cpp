#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "co_routine_pool.h"


#define DEF_ROUTINE_POOL_MAX 500
#define DEF_ROUTINE_POOL_ADD_SIZE 20

#define DEF_ROUTINE_POOL_RECOVERY_TIME		(600*1000)
#define DEF_ROUTINE_POOL_RECOVERY_MIN_TIME	(10*1000)
#define DEF_ROUTINE_POOL_RECOVERY_MAX	(2000)
#define DEF_ROUTINE_POOL_RECOVERY_ALLOC_LIMIT	(20000)




struct stCoRoutine_pool_t
{
	pthread_mutex_t lock;

	struct stlc_list_head coroutine_free;
	struct stlc_list_head coroutine_end;
	unsigned int alloc;
	unsigned int co_end;
	unsigned int co_unuse;

	stCoRoutine_pool_t() {
		alloc = co_end = co_unuse = 0;
		pthread_mutex_init(&lock, NULL);
		STLC_INIT_LIST_HEAD(&coroutine_free);
		STLC_INIT_LIST_HEAD(&coroutine_end);

		stCoRoutine_add(DEF_ROUTINE_POOL_MAX);
	}

	void stCoRoutine_add(int size) {
		stCoRoutine_t *co;
		for(int i = 0; i < size; i++) {
			co = (stCoRoutine_t*)calloc(1, sizeof(stCoRoutine_t));		
			stlc_list_add_tail(&co->link, &coroutine_free);
		}
		if(size > 0) {
			alloc += size;
			co_unuse += size;
		}
	}

	stCoRoutine_t *stCoRoutine_alloc() {
		stCoRoutine_t *lp = NULL;
		pthread_mutex_lock(&lock);

		if(stlc_list_empty(&coroutine_free)) {
			stCoRoutine_add(DEF_ROUTINE_POOL_ADD_SIZE);
		}
		
		lp = stlc_list_first_entry(&coroutine_free, stCoRoutine_t, link);
		stlc_list_del(&lp->link);
		co_unuse--;
		
		pthread_mutex_unlock(&lock);
		return lp;
	}

	void stCoRoutine_release(stCoRoutine_t *co) {
		//co_release(co);
		pthread_mutex_lock(&lock);
		stlc_list_add_tail(&co->link, &coroutine_end);
		co_end++;
		pthread_mutex_unlock(&lock);
	}

	void stCoRoutine_pool_status() {
		unsigned int palloc;
		unsigned int pco_end;
		unsigned int pco_unuse;
		pthread_mutex_lock(&lock);
		palloc = alloc;
		pco_end = co_end;
		pco_unuse = co_unuse;	
		pthread_mutex_unlock(&lock);

		printf("alloc: %u, unuse_count: %u, end_count: %u\n", palloc, pco_unuse, pco_end);
	}

	int stCoRoutine_recovery();
};


static stCoRoutine_pool_t *stCoRoutine_pool = new stCoRoutine_pool_t();


void get_stCoRoutine_handle()
{
	
}

//nerver free and create
#if 0
stCoRoutine_pool_t *stCoRoutine_pool_alloc(int size)
{
	stCoRoutine_pool_t *pool = (stCoRoutine_pool_t*)calloc(1, sizeof(stCoRoutine_pool_t));
	
	pthread_mutex_init(&pool->lock, NULL);
	STLC_INIT_LIST_HEAD(&pool->coroutine_free);
	STLC_INIT_LIST_HEAD(&pool->coroutine_end);


	if(size > DEF_ROUTINE_POOL_MAX) {
		size = DEF_ROUTINE_POOL_MAX;
	}

	stCoRoutine_add(pool, size);

	return pool;
}

void stCoRoutine_pool_free()
{
	stCoRoutine_pool_t *pool = stCoRoutine_pool;
	if(pool == NULL) {
		return;
	}

	stCoRoutine_t *pos, *n;

	pthread_mutex_lock(&pool->lock);

	stlc_list_for_each_entry_safe(pos, n, &pool->coroutine_free, link) {
		stlc_list_del(&pos->link);
		co_free(pos);
	}

	pthread_mutex_unlock(&pool->lock);
	pthread_mutex_destroy(&pool->lock);
	free(pool);
}
#endif

stCoRoutine_t *stCoRoutine_alloc()
{
	if(!stCoRoutine_pool) {
		return (stCoRoutine_t*)malloc( sizeof(stCoRoutine_t) );
	}
	
	return stCoRoutine_pool->stCoRoutine_alloc();
}

void stCoRoutine_release(stCoRoutine_t *co)
{
	if(!stCoRoutine_pool) {
		//return co_free(co);
		return ;
	}
	return stCoRoutine_pool->stCoRoutine_release(co);	
}

void stCoRoutine_pool_status()
{

	if(!stCoRoutine_pool) {
		printf("not using pool\n");
		return;
	}

	return stCoRoutine_pool->stCoRoutine_pool_status();
}

int stCoRoutine_pool_t::stCoRoutine_recovery()
{
	int charge = 0;
	int count = 0;
	stCoRoutine_t *pos, *n;
	
	pthread_mutex_lock(&lock);
	stlc_list_for_each_entry_safe(pos, n, &coroutine_end, link) {
		if(count++ > DEF_ROUTINE_POOL_RECOVERY_MAX) {
			break;
		}
		stlc_list_del(&pos->link);
		co_end--;
		if(co_unuse >= DEF_ROUTINE_POOL_RECOVERY_ALLOC_LIMIT) {
			alloc--;
			co_free(pos);
			charge = 1;
		}else {
			co_release(pos);
			co_unuse++;
			stlc_list_add_tail(&pos->link, &coroutine_free);
		}
	}
	pthread_mutex_unlock(&lock);
	return charge;
}

static void *stCoRoutine_recovery(void *args)
{
	co_enable_hook_sys();

	int timeout = DEF_ROUTINE_POOL_RECOVERY_TIME;

	for(;;) {
		poll(NULL, 0, timeout);
		if(stCoRoutine_pool->stCoRoutine_recovery()) {
			timeout = DEF_ROUTINE_POOL_RECOVERY_MIN_TIME;
		}else {
			timeout = DEF_ROUTINE_POOL_RECOVERY_TIME;
		}
	}


	return NULL;
}

void stCoRoutine_pool_start_gc()
{
	stCoRoutine_t *co;

	if(!stCoRoutine_pool)
		return;

	co_create(&co, NULL, stCoRoutine_recovery, NULL);
	co_resume(co);
	
	return ;
}
