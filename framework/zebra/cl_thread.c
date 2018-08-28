#include "cl_thread.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

static cl_thread_t *cl_thread_trim_head(struct stlc_list_head *list)
{
	cl_thread_t *t;
	
	if (stlc_list_empty(list))
		return NULL;

	t = stlc_list_entry(list->next, cl_thread_t, link);
	stlc_list_del(&t->link);

	return t;
}

static cl_thread_t *cl_thread_get(cl_thread_master_t *m, u_int8_t type,
		cl_thread_func_t func, void *arg)
{
	cl_thread_t *thread;
	
	thread = cl_thread_trim_head(&m->unuse);
	if (thread == NULL) {
		thread = calloc(sizeof(cl_thread_t), 1);
	} 
	
	thread->type = type;
	thread->master = m;
	thread->func = func;
	thread->arg = arg;

	return thread;
}

/* Add new read thread. */
cl_thread_t *cl_thread_add_read(cl_thread_master_t *m, 
		cl_thread_func_t func, void *arg, SOCKET sock)
{
	cl_thread_t *thread;
	
	if (FD_ISSET(sock, &m->readfd)) {
		printf("There is already read fd [%d]\n", sock);
		return NULL;
	}

	thread = cl_thread_get(m, CL_THREAD_READ, func, arg);
	FD_SET(sock, &m->readfd);
	thread->u.fd = sock;
	stlc_list_add(&thread->link, &m->read);

	if (sock > (SOCKET)m->max_fd)
		m->max_fd = sock;

	return thread;
}

/* Add new write thread. */
cl_thread_t *cl_thread_add_write(cl_thread_master_t *m, 
		cl_thread_func_t func, void *arg, SOCKET sock)
{
	cl_thread_t *thread;
	
	if (FD_ISSET(sock, &m->writefd)) {
		printf("There is already write fd [%d]\n", sock);
		return NULL;
	}

	thread = cl_thread_get(m, CL_THREAD_WRITE, func, arg);
	FD_SET(sock, &m->writefd);
	thread->u.fd = sock;
	stlc_list_add(&thread->link, &m->write);

	if (sock > (SOCKET)m->max_fd)
		m->max_fd = sock;

	return thread;
}

u_int32_t getmsec()
{
	struct timespec tp;
	u_int64_t t;

	if (clock_gettime(CLOCK_MONOTONIC, &tp) == 0) {
		t = tp.tv_sec*1000 + tp.tv_nsec/1000000;
	} else {
		printf("clock_gettime failed\n");
		t = 0;
	}


	return (u_int32_t)t;
}

#define u32_is_bigger(a,b) \
	(((a) != (b)) && ((u_int32_t)(a)-(u_int32_t)(b)) < 0x7FFFFFFF)

/* Add timer event thread. */
cl_thread_t *cl_thread_add_timer(cl_thread_master_t *m, 
		cl_thread_func_t func, void *arg, u_int32_t timer)
{
	time_node_t *tn;
	cl_thread_t *thread;
	
	thread = cl_thread_get(m, CL_THREAD_TIMER, func, arg);
	thread->u.timeout = getmsec() + timer;

	stlc_list_for_each_entry(tn, &m->timer, link) {
		if(tn->interval == timer){
			stlc_list_add_tail(&thread->link, &tn->th_head);
			return thread;
		}
	}
	tn = calloc(sizeof(*tn), 1);
	STLC_INIT_LIST_HEAD(&tn->th_head);
	tn->interval = timer;
	tn->count = 0;
	printf("<%s, %d> new timer list with interval: %u\n",
		__FILE__, __LINE__, timer);
	
	stlc_list_add_tail(&tn->link, &m->timer);
	stlc_list_add_tail(&thread->link, &tn->th_head);

	return thread;
}

/* Add simple event thread. */
cl_thread_t *cl_thread_add_event(cl_thread_master_t *m, 
		cl_thread_func_t func, void *arg, u_int32_t val)
{
	cl_thread_t *thread;
	
	thread = cl_thread_get(m, CL_THREAD_EVENT, func, arg);
	thread->u.val = val;

	stlc_list_add_tail(&thread->link, &m->event);

	return thread;
}

#define	cl_assert(b) _cl_assert(!(!(b)), __FILE__, __LINE__)

void _cl_assert(int b, const char *file, int line)
{
	if ( ! b ) {
		printf("%s line %d assert failed!!!\n", file, line);
		*(char *)0 = 0;
	}
}

/* Cancel thread from scheduler. */
void cl_thread_cancel(cl_thread_t *t)
{
	switch (t->type) {
	case CL_THREAD_READ:
		cl_assert(FD_ISSET(t->u.fd, &t->master->readfd));
		FD_CLR(t->u.fd, &t->master->readfd);
		stlc_list_del(&t->link);
		break;
	case CL_THREAD_WRITE:
		cl_assert(FD_ISSET(t->u.fd, &t->master->writefd));
		FD_CLR(t->u.fd, &t->master->writefd);
		stlc_list_del(&t->link);
		break;
	case CL_THREAD_TIMER:
	case CL_THREAD_EVENT:
	case CL_THREAD_READY:
		stlc_list_del(&t->link);
		break;
	default:
		*(u_int32_t *)0 = 0;
		break;
	}

	t->type = CL_THREAD_UNUSED;
	stlc_list_add(&t->link, &t->master->unuse);
}

struct timeval *
cl_thread_timer_wait(cl_thread_master_t *m, struct timeval *timer_val)
{
	timer_val->tv_sec = 0;
	timer_val->tv_usec = 1000;
	return timer_val;
	
}

int cl_thread_process_fd(cl_thread_master_t *m, struct stlc_list_head *list,
		fd_set *fdset, fd_set *mfdset)
{
	int ready = 0;
	cl_thread_t *thread, *next;

	stlc_list_for_each_entry_safe(thread, next, list, link) {
		if (FD_ISSET(CL_THREAD_FD(thread), fdset)) {
			cl_assert(FD_ISSET(CL_THREAD_FD(thread), mfdset));
			FD_CLR(CL_THREAD_FD(thread), mfdset);
			stlc_list_del(&thread->link);
			
			stlc_list_add(&thread->link, &m->ready);
			thread->type = CL_THREAD_READY;
			ready++;
		}
	}

	return ready;
}


//////////////////////////////////////////////////////////////////////////////////////

static cl_thread_t *
cl_thread_run (cl_thread_master_t *m, cl_thread_t *thread,
	    cl_thread_t *fetch)
{
	*fetch = *thread;
	thread->type = CL_THREAD_UNUSED;
	stlc_list_add(&thread->link, &m->unuse);
	
	return fetch;
}

cl_thread_t *cl_thread_fetch(cl_thread_master_t *m, cl_thread_t *fetch)
{
	int n;
	cl_thread_t *thread;
	time_node_t *tn;
	u_int32_t now;
	fd_set readfd;
	fd_set writefd;
	fd_set exceptfd;
	struct timeval timer_val, *timer_wait;

	while (1) {
		/* Normal event is the highest priority.  */
		if ((thread = cl_thread_trim_head(&m->event)) != NULL) {
			return cl_thread_run(m, thread, fetch);
		}

		/* Execute timer.  */
		now = getmsec();
		stlc_list_for_each_entry(tn, &m->timer, link) {
			stlc_list_for_each_entry(thread, &tn->th_head, link){
				if (u32_is_bigger(now, thread->u.timeout)) {
					stlc_list_del(&thread->link);
					return cl_thread_run(m, thread, fetch);
				}
				break;
			}
		}

		/* If there are any ready threads, process top of them.  */
		if ((thread = cl_thread_trim_head(&m->ready)) != NULL) {
			return cl_thread_run(m, thread, fetch);
		}

		/* Structure copy.  */
		readfd = m->readfd;
		writefd = m->writefd;
		exceptfd = m->exceptfd;

		/* Calculate select wait timer. */
		timer_wait = cl_thread_timer_wait(m, &timer_val);

		n = select((int)m->max_fd + 1, &readfd, &writefd, &exceptfd, timer_wait);
		if (n <= 0)
		 	continue;

		/* Normal priority read thead. */
		cl_thread_process_fd(m, &m->read, &readfd, &m->readfd);

		/* Write thead. */
		cl_thread_process_fd(m, &m->write, &writefd, &m->writefd);
		
		if ((thread = cl_thread_trim_head(&m->ready)) != NULL) {
			return cl_thread_run(m, thread, fetch);
		}
	}

	return NULL;
}

cl_thread_t *cl_thread_fetch_lock(cl_thread_master_t *m, cl_thread_t *fetch)
{
	int n;
	cl_thread_t *thread;
	time_node_t *tn;
	u_int32_t now;
	fd_set readfd;
	fd_set writefd;
	fd_set exceptfd;
	struct timeval timer_val, *timer_wait;

	while (1) {
		/* Normal event is the highest priority.  */
		if ((thread = cl_thread_trim_head(&m->event)) != NULL) {
			return cl_thread_run(m, thread, fetch);
		}

		pthread_mutex_lock(&m->lock);
		/* Execute timer.  */
		now = getmsec();
		stlc_list_for_each_entry(tn, &m->timer, link) {
			stlc_list_for_each_entry(thread, &tn->th_head, link){
				if (u32_is_bigger(now, thread->u.timeout)) {
					stlc_list_del(&thread->link);
					cl_thread_run(m, thread, fetch);
					pthread_mutex_unlock(&m->lock);
					return fetch;
				}
				break;
			}
		}
		pthread_mutex_unlock(&m->lock);

		/* If there are any ready threads, process top of them.  */
		if ((thread = cl_thread_trim_head(&m->ready)) != NULL) {
			return cl_thread_run(m, thread, fetch);
		}

		/* Structure copy.  */
		readfd = m->readfd;
		writefd = m->writefd;
		exceptfd = m->exceptfd;

		/* Calculate select wait timer. */
		timer_wait = cl_thread_timer_wait(m, &timer_val);

		n = select((int)m->max_fd + 1, &readfd, &writefd, &exceptfd, timer_wait);
		if (n <= 0)
		 	continue;

		/* Normal priority read thead. */
		cl_thread_process_fd(m, &m->read, &readfd, &m->readfd);

		/* Write thead. */
		cl_thread_process_fd(m, &m->write, &writefd, &m->writefd);
		
		if ((thread = cl_thread_trim_head(&m->ready)) != NULL) {
			return cl_thread_run(m, thread, fetch);
		}
	}

	return NULL;
}

/* We check thread consumed time. If the system has getrusage, we'll
   use that to get indepth stats on the performance of the thread.  If
   not - we'll use gettimeofday for some guestimation.  */
void cl_thread_call(cl_thread_t *thread)
{
	(*thread->func)(thread);
}

//////////////////////////////////////////////////////////////////////////////////////

RS cl_thread_init(cl_thread_master_t *m)
{
	memset(m, 0, sizeof(cl_thread_master_t));

	STLC_INIT_LIST_HEAD(&m->read);
	STLC_INIT_LIST_HEAD(&m->write);
	STLC_INIT_LIST_HEAD(&m->timer);
	STLC_INIT_LIST_HEAD(&m->event);
	STLC_INIT_LIST_HEAD(&m->ready);
	STLC_INIT_LIST_HEAD(&m->unuse);
	pthread_mutex_init(&m->lock, NULL);
	return RS_OK;
}

/*
	free all threads
*/
static void cl_thread_free_all_thread(struct stlc_list_head *list)
{
	cl_thread_t *t, *next;

	stlc_list_for_each_entry_safe(t, next, list, link) {
		stlc_list_del(&t->link);
		free(t);
	}
}

static void cl_thread_free_all_timer(struct stlc_list_head *list)
{
	time_node_t *tn, *n;

	stlc_list_for_each_entry_safe(tn, n, list, link) {
		cl_thread_free_all_thread(&tn->th_head);
		stlc_list_del(&tn->link);
		free(tn);
	}
}


void cl_thread_stop(cl_thread_master_t *m)
{
	cl_thread_free_all_thread(&m->read);
	cl_thread_free_all_thread(&m->write);
	cl_thread_free_all_timer(&m->timer);
	cl_thread_free_all_thread(&m->event);
	cl_thread_free_all_thread(&m->ready);
	cl_thread_free_all_thread(&m->unuse);

	pthread_mutex_destroy(&m->lock);
}


