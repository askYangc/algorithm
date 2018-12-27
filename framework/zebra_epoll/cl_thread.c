#include "cl_thread.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

#include "min_heap.h"

#define	cl_assert(b) _cl_assert(!(!(b)), __FILE__, __LINE__)

static inline void _cl_assert(int b, const char *file, int line)
{
	if ( ! b ) {
		printf("%s line %d assert failed!!!\n", file, line);
		*(char *)0 = 0;
	}
}


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

static int cl_thread_socket_search(cl_thread_socket_t *t, int sock)
{
	if(t->fd < sock) {
		return 1;
	}else if(t->fd > sock) {
		return -1;
	}else {
		return 0;
	}
}

static cl_thread_socket_t *cl_thread_socket_calloc(int sock)
{
	cl_thread_socket_t *t = (cl_thread_socket_t*)calloc(1, sizeof(cl_thread_socket_t));
	if(t) {
		t->fd = sock;
		t->op = CL_EPOLL_ADD;
	}
	return t;
}

static cl_thread_socket_t *cl_thread_socket_lookup(struct rb_root *root, int fd)
{
	cl_thread_socket_t *s;
	rb_search(root, s, cl_thread_socket_t, node, cl_thread_socket_search, fd);
	return s;
}

void cl_thread_socket_show(cl_thread_socket_t *s)
{
	printf("s->fd: %d, s->type: %d, s->op: %d\n", s->fd, s->type, s->op);
	if(s->t_read) {
		printf("t_read->type: %d\n", s->t_read->type);
	}else {
		printf("t_read is NULL\n");
	}
	if(s->t_write) {
		printf("t_write->type: %d\n", s->t_write->type);
	}else {
		printf("t_write is NULL\n");
	}	
}


/* Add new read thread. */
cl_thread_t *cl_thread_add_read(cl_thread_master_t *m, 
		cl_thread_func_t func, void *arg, SOCKET sock, int presist)
{
	cl_thread_t *thread;
	cl_thread_socket_t *s;
	struct epoll_event ev;

	rb_search_or_insert(&m->sockets, s, cl_thread_socket_t, node, 
		cl_thread_socket_search, sock, cl_thread_socket_calloc, sock);
	
	cl_assert(s != NULL);
	if(s->type & CL_SOCKETS_READ) {
		printf("There is already read fd [%d]\n", sock);
		return NULL;
	}

	thread = cl_thread_get(m, CL_THREAD_READ, func, arg);
	thread->u.fd = sock;
	cl_assert(s->t_read == NULL);
	s->t_read = thread;
	s->type |= CL_SOCKETS_READ;

	ev.events = EPOLLIN;
	if(s->type & CL_SOCKETS_WRITE)
		ev.events |= EPOLLOUT;
	ev.data.ptr = s;

	if(s->op == CL_EPOLL_ADD) {
		s->op = CL_EPOLL_ADDED;
		epoll_ctl(m->epollfd, EPOLL_CTL_ADD, sock, &ev);
	}else {
		epoll_ctl(m->epollfd, EPOLL_CTL_MOD, sock, &ev);
	}

	if(presist) {
		s->type |= CL_SOCKETS_READ_PERSIST;
	}else {
		s->type &= ~CL_SOCKETS_READ_PERSIST;
	}

	stlc_list_add_tail(&thread->link, &m->read);

	return thread;
}

/* Add new write thread. */
cl_thread_t *cl_thread_add_write(cl_thread_master_t *m, 
		cl_thread_func_t func, void *arg, SOCKET sock, int presist)
{
	cl_thread_t *thread;
	cl_thread_socket_t *s;
	struct epoll_event ev;

	rb_search_or_insert(&m->sockets, s, cl_thread_socket_t, node, 
		cl_thread_socket_search, sock, cl_thread_socket_calloc, sock);
	
	cl_assert(s != NULL);
	if(s->type & CL_SOCKETS_WRITE) {
		printf("There is already write fd [%d]\n", sock);
		return NULL;
	}

	thread = cl_thread_get(m, CL_THREAD_WRITE, func, arg);
	thread->u.fd = sock;
	cl_assert(s->t_write == NULL);
	s->t_write = thread;
	s->type |= CL_SOCKETS_WRITE;

	ev.events = EPOLLOUT;
	if(s->type & CL_SOCKETS_READ)
		ev.events |= EPOLLIN;
	ev.data.ptr = s;

	if(s->op == CL_EPOLL_ADD) {
		s->op = CL_EPOLL_ADDED;
		epoll_ctl(m->epollfd, EPOLL_CTL_ADD, sock, &ev);
	}else {
		epoll_ctl(m->epollfd, EPOLL_CTL_MOD, sock, &ev);
	}

	if(presist) {
		s->type |= CL_SOCKETS_WRITE_PERSIST;
	}else {
		s->type &= ~CL_SOCKETS_WRITE_PERSIST;
	}

	stlc_list_add_tail(&thread->link, &m->write);

	return thread;
}

/* Add timer event thread. */
cl_thread_t *cl_thread_add_timer(cl_thread_master_t *m, 
		cl_thread_func_t func, void *arg, u_int32_t timer)
{
	cl_thread_t *thread;
	
	thread = cl_thread_get(m, CL_THREAD_TIMER, func, arg);
	thread->u.timer = NULL;
	MINHEAP_TIMER_ON(m->heap, thread->u.timer, NULL, thread, timer);
	stlc_list_add_tail(&thread->link, &m->timer);
	
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

void _cl_thread_cancel_read(cl_thread_socket_t *s)
{
	struct epoll_event ev;
	cl_thread_t *t = s->t_read;
	int op;
	
	s->t_read = NULL;
	s->type = s->type & ~CL_SOCKETS_READ;
	s->type = s->type & ~CL_SOCKETS_READ_PERSIST;
	if(s->type & CL_SOCKETS_WRITE) {
		ev.events = EPOLLOUT;
		ev.data.ptr = s;
		if(s->op == CL_EPOLL_ADD) {
			s->op = CL_EPOLL_ADDED;
			op = EPOLL_CTL_ADD;
		}else {
			op = EPOLL_CTL_MOD;
		}
		epoll_ctl(t->master->epollfd, op, s->fd, &ev);
	}else {
		s->op = CL_EPOLL_ADD;
		epoll_ctl(t->master->epollfd, EPOLL_CTL_DEL, s->fd, NULL);
	}

	stlc_list_del(&t->link);
	return;
	
}


void cl_thread_cancel_read(cl_thread_t *t)
{
	cl_thread_socket_t *s;

	s = cl_thread_socket_lookup(&t->master->sockets, t->u.fd);
	if(s == NULL) {
		stlc_list_del(&t->link);
		return;
	}

	cl_assert(s->t_read == t);
	_cl_thread_cancel_read(s);
	return;
}

void _cl_thread_cancel_write(cl_thread_socket_t *s)
{
	struct epoll_event ev;
	cl_thread_t *t = s->t_write;
	int op;
	
	s->t_write = NULL;
	s->type = s->type & ~CL_SOCKETS_WRITE;
	s->type = s->type & ~CL_SOCKETS_WRITE_PERSIST;
	if(s->type & CL_SOCKETS_READ) {
		ev.events = EPOLLIN;
		ev.data.ptr = s;
		if(s->op == CL_EPOLL_ADD) {
			s->op = CL_EPOLL_ADDED;
			op = EPOLL_CTL_ADD;
		}else {
			op = EPOLL_CTL_MOD;
		}
		epoll_ctl(t->master->epollfd, op, s->fd, &ev);
	}else {
		s->op = CL_EPOLL_ADD;
		epoll_ctl(t->master->epollfd, EPOLL_CTL_DEL, s->fd, NULL);
	}
	
	stlc_list_del(&t->link);
	return;
	
}

void cl_thread_cancel_write(cl_thread_t *t)
{
	cl_thread_socket_t *s;

	s = cl_thread_socket_lookup(&t->master->sockets, t->u.fd);
	if(s == NULL) {
		stlc_list_del(&t->link);
		return;
	}

	cl_assert(s->t_write == t);
	_cl_thread_cancel_write(s);
	return;
}

void cl_thread_cancel_ready(cl_thread_t *t)
{
	cl_thread_socket_t *s;
	s = cl_thread_socket_lookup(&t->master->sockets, t->u.fd);
	if(s == NULL) {
		stlc_list_del(&t->link);
		return;
	}	

	if(s->t_read == t) {
		_cl_thread_cancel_read(s);
	}else if(s->t_write == t) {
		_cl_thread_cancel_write(s);
	}else {
		stlc_list_del(&t->link);
	}

	return;	
}

void cl_thread_cancel_timer(cl_thread_t *t)
{
	MINHEAP_TIMER_OFF(t->master->heap, t->u.timer);
	stlc_list_del(&t->link);
}

/* Cancel thread from scheduler. */
void cl_thread_cancel(cl_thread_t *t)
{
	switch (t->type) {
	case CL_THREAD_READ:
		cl_thread_cancel_read(t);
		break;
	case CL_THREAD_WRITE:
		cl_thread_cancel_write(t);
		break;
	case CL_THREAD_TIMER:
		cl_thread_cancel_timer(t);
		break;
	case CL_THREAD_EVENT:
		stlc_list_del(&t->link);
		break;
	case CL_THREAD_READY:
		cl_thread_cancel_ready(t);
	default:
		*(u_int32_t *)0 = 0;
		break;
	}

	t->type = CL_THREAD_UNUSED;
	stlc_list_add(&t->link, &t->master->unuse);
}

int cl_thread_timer_wait(cl_thread_master_t *m)
{
	int expire = 1;
	time_heap_time_wait(m->heap, &expire);

	return expire;
}

void cl_thread_do_event(cl_thread_master_t *m, struct epoll_event *ev)
{
	int what = 0;
	int ctl = 0;
	cl_thread_socket_t *s = ev->data.ptr;
	cl_thread_t *t;
	struct epoll_event e;
	
	if(ev->events & (EPOLLERR|EPOLLHUP|EPOLLRDHUP)){
		/*
		EPOLLERR：表示对应的文件描述符发生错误；
		EPOLLHUP：表示对应的文件描述符被挂断； 
		EPOLLRDHUP: 表示对端正常关闭
		*/	
		what = EV_READ | EV_WRITE;
	}


	if(ev->events & (EPOLLIN|EPOLLPRI)){
		/*
		EPOLLIN ：表示对应的文件描述符可以读（包括对端SOCKET正常关闭）；
		EPOLLPRI：表示对应的文件描述符有紧急的数据可读（这里应该表示有带外数据到来）；
		*/
		what |= EV_READ;
	}

	if(ev->events & EPOLLOUT) {
		what |= EV_WRITE;
	}
	
	if((what & EV_READ) && (s->type & CL_SOCKETS_READ)) {
		t = s->t_read;
		cl_assert(t != NULL);
		if(!(s->type & CL_SOCKETS_READ_PERSIST)) {
			ctl |= EV_READ;
		}
		if(t->type != CL_THREAD_READY) {
			t->type = CL_THREAD_READY;
			stlc_list_del(&t->link);
			stlc_list_add(&t->link, &m->ready);
		}
	}

	if((what & EV_WRITE) && (s->type & CL_SOCKETS_WRITE)) {
		t = s->t_write;
		cl_assert(t != NULL);
		if(!(s->type & CL_SOCKETS_WRITE_PERSIST)) {
			ctl |= EV_WRITE;
		}
		if(t->type != CL_THREAD_READY) {
			t->type = CL_THREAD_READY;
			stlc_list_del(&t->link);
			stlc_list_add(&t->link, &m->ready);
		}
	}	

	if(ctl == 0) {
		return;
	}

	e.events = 0;
	e.data.ptr = s;
	if(s->type & CL_SOCKETS_READ) {
		e.events |= EPOLLIN;		
	}
	if(s->type & CL_SOCKETS_WRITE) {
		e.events |= EPOLLOUT;		
	}

	if(ctl & EV_READ) {
		e.events &= ~EPOLLIN;
	}	
	
	if(ctl & EV_WRITE) {
		e.events &= ~EPOLLOUT;
	}	

	if(e.events == 0) {
		s->op = CL_EPOLL_ADD;
		epoll_ctl(m->epollfd, EPOLL_CTL_DEL, s->fd, NULL);
	}else {
		int op;
		if(s->op == CL_EPOLL_ADD) {
			s->op = CL_EPOLL_ADDED;
			op = EPOLL_CTL_ADD;
		}else {
			op = EPOLL_CTL_MOD;
		}
		epoll_ctl(m->epollfd, op, s->fd, &e);
	}

	return ;
}

cl_thread_t *cl_thread_get_read(cl_thread_master_t *m, int sock)
{
	cl_thread_socket_t *s = cl_thread_socket_lookup(&m->sockets, sock);
	if(s) {
		return s->t_read;
	}

	return NULL;
}

cl_thread_t *cl_thread_get_write(cl_thread_master_t *m, int sock)
{
	cl_thread_socket_t *s = cl_thread_socket_lookup(&m->sockets, sock);
	if(s) {
		return s->t_write;
	}

	return NULL;
}


static void cl_thread_add_unuse(cl_thread_master_t *m, cl_thread_t *t)
{
	t->type = CL_THREAD_UNUSED;
	stlc_list_add(&t->link, &m->unuse);
}

void cl_thread_socket_set(cl_thread_master_t *m, cl_thread_t *t)
{
	cl_thread_socket_t *s;

	s = cl_thread_socket_lookup(&t->master->sockets, t->u.fd);
	if(s == NULL) {
		cl_thread_add_unuse(m, t);
		return;
	}

	if(s->t_read == t) {
		if(!(s->type & CL_SOCKETS_READ_PERSIST)) {
			//not persist
			s->type &= ~CL_SOCKETS_READ;
			s->t_read = NULL;
			return cl_thread_add_unuse(m, t);
		}
		t->type = CL_THREAD_READ;
		stlc_list_add(&t->link, &m->read);
	}else if(s->t_write == t) {
		if(!(s->type & CL_SOCKETS_WRITE_PERSIST)) {
			//not persist
			s->type &= ~CL_THREAD_WRITE;
			s->t_write = NULL;
			return cl_thread_add_unuse(m, t);
		}
		t->type = CL_THREAD_WRITE;
		stlc_list_add(&t->link, &m->write);	
	}

	return;
}

//////////////////////////////////////////////////////////////////////////////////////

static cl_thread_t *
cl_thread_run (cl_thread_master_t *m, cl_thread_t *thread,
	    cl_thread_t *fetch)
{
	*fetch = *thread;

	if(thread->type == CL_THREAD_READY) {
		cl_thread_socket_set(m, thread);
	}else {
		cl_thread_add_unuse(m, thread);
	}
	
	return fetch;
}

cl_thread_t *cl_thread_fetch(cl_thread_master_t *m, cl_thread_t *fetch)
{
	int i, n;
	cl_thread_t *thread;
	int expire = 0;

	while (1) {
		/* Normal event is the highest priority.  */
		if ((thread = cl_thread_trim_head(&m->event)) != NULL) {
			return cl_thread_run(m, thread, fetch);
		}

		/* Execute timer.  */
		thread = (cl_thread_t*)cl_time_heap_tick(m->heap);
		if(thread) {
			stlc_list_del(&thread->link);
			return cl_thread_run(m, thread, fetch);
		}


		/* If there are any ready threads, process top of them.  */
		if ((thread = cl_thread_trim_head(&m->ready)) != NULL) {
			return cl_thread_run(m, thread, fetch);
		}

		/* Calculate select wait timer. */
		expire = cl_thread_timer_wait(m);

		n = epoll_wait(m->epollfd, m->evs, MAX_EVENT, expire);
		if (n <= 0)
		 	continue;

		for(i = 0; i < n; i++) {
			cl_thread_do_event(m, &m->evs[i]);
		}
			
		if ((thread = cl_thread_trim_head(&m->ready)) != NULL) {
			return cl_thread_run(m, thread, fetch);
		}
	}

	return NULL;
}

cl_thread_t *cl_thread_fetch_lock(cl_thread_master_t *m, cl_thread_t *fetch)
{
	int i, n;
	cl_thread_t *thread;
	int expire = 0;

	while (1) {
		/* Normal event is the highest priority.  */
		if ((thread = cl_thread_trim_head(&m->event)) != NULL) {
			return cl_thread_run(m, thread, fetch);
		}

		pthread_mutex_lock(&m->lock);
		/* Execute timer.  */
		thread = (cl_thread_t*)cl_time_heap_tick(m->heap);
		if(thread) {
			stlc_list_del(&thread->link);
			cl_thread_run(m, thread, fetch);
			pthread_mutex_unlock(&m->lock);
			return fetch;
		}
		pthread_mutex_unlock(&m->lock);

		/* If there are any ready threads, process top of them.  */
		if ((thread = cl_thread_trim_head(&m->ready)) != NULL) {
			return cl_thread_run(m, thread, fetch);
		}

		/* Calculate select wait timer. */
		expire = cl_thread_timer_wait(m);

		n = epoll_wait(m->epollfd, m->evs, MAX_EVENT, expire);
		if (n <= 0)
		 	continue;

		for(i = 0; i < n; i++) {
			cl_thread_do_event(m, &m->evs[i]);
		}
			
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
	STLC_INIT_LIST_HEAD(&m->event);
	STLC_INIT_LIST_HEAD(&m->timer);
	STLC_INIT_LIST_HEAD(&m->ready);
	STLC_INIT_LIST_HEAD(&m->unuse);

	m->epollfd = epoll_create(1);
	if(m->epollfd < 0) {
		printf("epoll_create failed\n");
		return RS_ERROR;
	}

	m->heap = time_heap_init();
	if(m->heap == NULL) {
		printf("time_heap_init failed\n");
		close(m->epollfd);
		return RS_ERROR;
	}

	pthread_mutex_init(&m->lock, NULL);
	
	return RS_OK;
}

static void cl_thread_free(cl_thread_t *t)
{
	stlc_list_del(&t->link);
	free(t);
}

static void cl_thread_socket_free(cl_thread_master_t *m, cl_thread_socket_t *s)
{
	if(s->t_read) {
		cl_thread_free(s->t_read);
	}
	if(s->t_write) {
		cl_thread_free(s->t_write);
	}

	if(s->op == CL_EPOLL_ADDED) {
		s->op = CL_EPOLL_ADD;
		epoll_ctl(m->epollfd, EPOLL_CTL_DEL, s->fd, NULL);
	}
	
	free(s);
}

/*
	free all threads
*/
static void cl_thread_free_all_thread(struct stlc_list_head *list)
{
	cl_thread_t *t, *next;

	stlc_list_for_each_entry_safe(t, next, list, link) {
		cl_thread_free(t);
	}
}

void cl_thread_rb_free(cl_thread_master_t *m, struct rb_root *root)
{
	cl_thread_socket_t *s;
	struct rb_node *pos;
	if(root) {
		for(pos = rb_first(root); pos; pos = rb_first(root)) {
			s = rb_entry(pos, cl_thread_socket_t, node);
			rb_delete_node(root, s, node);
			cl_thread_socket_free(m, s);
		}
	}
	return;
}

void cl_thread_stop(cl_thread_master_t *m)
{
	cl_thread_free_all_thread(&m->event);
	cl_thread_free_all_thread(&m->timer);
	cl_thread_free_all_thread(&m->ready);
	cl_thread_free_all_thread(&m->unuse);

	time_head_free(m->heap);
	m->heap = NULL;
	cl_thread_rb_free(m, &m->sockets);
	
	if(m->epollfd > 0) {
		close(m->epollfd);
		m->epollfd = 0;
	}

	pthread_mutex_destroy(&m->lock);	
}


