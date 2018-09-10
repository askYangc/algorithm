#include "dead_lock_stub.h"

struct stlc_list_head m_thread_apply_lock;  //thread_req_lock_t
struct stlc_list_head m_lock_belong_thread; //thread_hold_lock_t
pthread_mutex_t deadlock_mutex = PTHREAD_MUTEX_INITIALIZER;


static u_int64_t graphics_queue[MAX_DEAD_MUTEXLOCK_CHECK];
static int graphics_queue_count = 0;

void thread_graphic_vertex_push_back(thread_graphic_vertex_t *v, u_int64_t thread_id);
thread_graphic_vertex_t *thread_graphic_vertex_lookup(struct stlc_list_head *g, u_int64_t thread_id);
thread_graphic_vertex_t *thread_graphic_vertex_lookup_or_alloc(struct stlc_list_head *g, u_int64_t thread_id);
void thread_graphic_vertex_free(struct stlc_list_head *g);
int thread_graphic_vertex_count(struct stlc_list_head *g);

/*
如果使用my_backtrace
如果源代码编译时使用了-O1或-O2优化选项，可执行代码会把ebp/rbp/rsp寄存器当作普通寄存器使用，导致backtrace失败。
为了防止这种情况发生，可以在编译时使用-O2  -fno-omit-frame-pointer  或-Og 来避免优化中使用上述寄存器。
系统自带的backtrace函数是通过读取操作系统的一个全局信息区，在多线程并发调用时，会造成严重的锁冲突
*/
#define STACKCALL __attribute__((regparm(1),noinline))  
void ** STACKCALL getEBP(void){  
        void **ebp=NULL;  
        __asm__ __volatile__("mov %%rbp, %0;\n\t"  
                    :"=m"(ebp)      /* 输出 */  
                    :      /* 输入 */  
                    :"memory");     /* 不受影响的寄存器 */  
        return (void **)(*ebp);  
}  
int my_backtrace(void **buffer,int size){  
      
    int frame=0;  
    void ** ebp;  
    void **ret=NULL;  
    unsigned long long func_frame_distance=0;  
    if(buffer!=NULL && size >0)  
    {  
        ebp=getEBP();  
        func_frame_distance=(unsigned long long)(*ebp) - (unsigned long long)ebp;  
        while(ebp&& frame<size  
            &&(func_frame_distance< (1ULL<<24))//assume function ebp more than 16M  
            &&(func_frame_distance>0))  
        {  
            ret=ebp+1;  
            buffer[frame++]=*ret;  
            ebp=(void**)(*ebp);  
            func_frame_distance=(unsigned long long)(*ebp) - (unsigned long long)ebp;  
        }  
    }  
    return frame;  
}  


int graphics_queue_empty()
{
	return graphics_queue_count<=0?1:0;
}

void graphics_queue_push_back(u_int64_t thread_id)
{
	if(graphics_queue_count < MAX_DEAD_MUTEXLOCK_CHECK) {
		graphics_queue[graphics_queue_count++] = thread_id;	
	}
}

u_int64_t graphics_queue_pop()
{
	if(graphics_queue_count == 0) 
		return 0;
	
	return graphics_queue[--graphics_queue_count];
}

void thread_req_lock_show(thread_req_lock_t *l) 
{   
	int i = 0;
    char _buf[32];
    thread_time_2_str(_buf, sizeof(_buf), l->t);
    printf("%llu %swait lock%s %s%s%s(%llu) at: file %s%s%s, function %s%s%s, line %s%d%s, start_time: %s (%u)\n", (u64)l->thread_id, COLOR_YELLOW, COLOR_END, COLOR_RED, l->lock_name, COLOR_END, 
    (u64)l->lock_addr, COLOR_GREEN, l->file, COLOR_END, COLOR_GREEN, l->funcname, COLOR_END, COLOR_GREEN, l->line, COLOR_END, _buf, l->t);
	if(l->stacktrace) {
		printf("stacktrace:\n");
		for(i = 0; i < l->stacktrace_num; i++) {
			printf("\tframe %d :%s\n", i, l->stacktrace[i]);
		}
	}
	
}  

void thread_hold_lock_show(thread_hold_lock_t *h) 
{
    hold_lock_point_t *p;
	int i = 0;
	if(h->count == 0)
		return;
    printf("%sthread %llu %s %shold lock_name(lock_addr)%s: %s%s%s(%llu), count: %s%d%s\n", 
		COLOR_YELLOW, (u64)h->thread_id, COLOR_END, COLOR_GREEN, COLOR_END, COLOR_RED, 
		h->lock_name, COLOR_END, (u64)h->lock_addr, COLOR_GREEN, h->count, COLOR_END);
    stlc_list_for_each_entry(p, &h->point_list, link) {
        printf("%s  lock at%s: file %s%s%s, function %s%s%s, line %s%d%s\n", COLOR_YELLOW, COLOR_END, COLOR_GREEN, p->file, COLOR_END, COLOR_GREEN, p->funcname, COLOR_END, COLOR_GREEN, p->line, COLOR_END);
		if(p->stacktrace) {
			printf("stacktrace:\n");
			for(i = 0; i < p->stacktrace_num; i++) {
				printf("\tframe %d :%s\n", i, p->stacktrace[i]);
			}
		}
	}
} 

void show_dead_lock_infomation(struct stlc_list_head *graphics) 
{
	thread_graphic_vertex_t *pos;
	thread_req_lock_t *l;
	thread_hold_lock_t *h;
	int i = 0;

	stlc_list_for_each_entry(pos, graphics, link) {
		if(pos->indegress == 0) 
			continue;
            printf("=====================thread info=====================\n");
            printf("thd_id: %s%llu%s, indegress: %d\n", COLOR_RED, (u64)pos->thread_id, COLOR_END, pos->indegress);
            printf("wait lock:\n");		
			l = thread_req_lock_lookup(pos->thread_id);
			if(l == NULL) {
				printf("%snot find %llu in m_thread_apply_lock, it's wrong%s\n", COLOR_RED, (u64)pos->thread_id, COLOR_END);
				continue;
			}
			thread_req_lock_show(l);

			printf("\nwait these threads unlock:%s\n", COLOR_YELLOW);
			for(i = 0; i < pos->thread_count; i++) {
				printf("%llu ", (u64)pos->threads[i]);
			}
			printf("%s\n", COLOR_END); 
			printf("hold these locks:\n");  
			
			stlc_list_for_each_entry(h, &m_lock_belong_thread, link) {
				if(h->thread_id == pos->thread_id) {
					thread_hold_lock_show(h);
				}
			}
			printf("=====================end=====================\n\n");
	}	
}

void show_waitout_lock(thread_req_lock_t *l) 
{
    printf("%sthread %llu is waitting lock%s\n", COLOR_YELLOW, (u64)l->thread_id, COLOR_END);
    thread_req_lock_show(l);
}
  
void check_wait_lock_time() 
{
    u_int32_t t = time(NULL);
	thread_req_lock_t *l;
	thread_hold_lock_t *h;

	stlc_list_for_each_entry(l, &m_thread_apply_lock, link) {
		if(l->valid == 0) 
			continue;
		if( t - l->t > WAIT_MUTEXLOCK_TIMEOUT) {
			show_waitout_lock(l);
			h = thread_hold_lock_lookup(l->lock_addr);
			if(h) {
				thread_hold_lock_show(h);
			}
		}
	}
}


void _check_dead_lock_stub()
{	
	thread_req_lock_t *l;
	thread_hold_lock_t *h;
	thread_graphic_vertex_t *v1, *v2, *v;
	u_int64_t counter = 0;
	struct stlc_list_head graphics;		//thread_graphic_vertex_t
	u_int64_t thd_id1;
	int i = 0;
	
	STLC_INIT_LIST_HEAD(&graphics);
	memset(graphics_queue, 0, sizeof(graphics_queue));
	graphics_queue_count = 0;
	
	// 构建有向图
	stlc_list_for_each_entry(l, &m_thread_apply_lock, link) {
		if(l->valid == 0) {
			continue;
		}
		h = thread_hold_lock_lookup(l->lock_addr);
		if(h == NULL || h->count <= 0)
			continue;
		
		v1 = thread_graphic_vertex_lookup_or_alloc(&graphics, l->thread_id);
		v2 = thread_graphic_vertex_lookup_or_alloc(&graphics, h->thread_id);
		if(v1 && v2) {
			// 保存有向边
			thread_graphic_vertex_push_back(v1, v2->thread_id);
			// 入度 indegress++
			v2->indegress++;
		}
	}
	
	// 检测流程一
	stlc_list_for_each_entry(v, &graphics, link) {
		if(v->indegress == 0) {
			graphics_queue_push_back(v->thread_id);
			counter++;
		}
	}

    // 检测流程二
    while (!graphics_queue_empty()) {
        thd_id1 = graphics_queue_pop();

		v1 = thread_graphic_vertex_lookup(&graphics, thd_id1);
		// 遍历邻近有向边
		for(i = 0; i < v1->thread_count; i++) {
			v2 = thread_graphic_vertex_lookup(&graphics, v1->threads[i]);
			v2->indegress--;
			if(v2->indegress == 0) {
				graphics_queue_push_back(v2->thread_id);
				counter++;
			}
		}
    }

    // 检测流程三
    if ( counter != thread_graphic_vertex_count(&graphics) ) {
        printf("%sFound Dead Lock!!!!!!!!!!!!%s\n", COLOR_RED, COLOR_END);
        show_dead_lock_infomation(&graphics);
    } else {
        printf("No Found Dead Lock.\n");
        check_wait_lock_time();
		//_show_thread_locks_info();
    }   
    
	thread_graphic_vertex_free(&graphics);
}

void *thread_rountine()
{
	while(1) {
		sleep(5);
		check_dead_lock_stub();
	}
	
	return NULL;
}

static hold_lock_point_t *hold_lock_point_init(const char *file, int line, const char *funcname, char **stacktrace, u_int8_t stacktrace_num)
{
    hold_lock_point_t *p = (hold_lock_point_t*)calloc(sizeof(hold_lock_point_t), 1);
    if(p) {
        p->line = line;
		p->stacktrace_num = stacktrace_num;
        memcpy(p->file, file, sizeof(p->file) - 1);
        memcpy(p->funcname, funcname, sizeof(p->funcname) - 1);   
		p->stacktrace = stacktrace;
    }

    return p;
}

static void hold_lock_point_free(hold_lock_point_t *p)
{
    if(p) {
		if(p->stacktrace)
			free(p->stacktrace);
        free(p);
    }
}

#if 0
static void hold_lock_point_all_free(struct stlc_list_head *head)
{
    hold_lock_point_t *pos, *n;
    if(head) {
        stlc_list_for_each_entry_safe(pos, n, head, link) {
            stlc_list_del(&pos->link);
            hold_lock_point_free(pos);
        }
    }
}

static void hold_lock_point_list_cp(const struct stlc_list_head *old, struct stlc_list_head *newx)
{
    hold_lock_point_t *pos, *n;
    if(old && newx) {
        stlc_list_for_each_entry(pos, old, link) {
            n = hold_lock_point_init(pos->file, pos->line, pos->funcname);
            if(n) {
                stlc_list_add(&n->link, newx);
            }
        }
    }
}
#endif

int thread_graphic_vertex_count(struct stlc_list_head *g)
{
    thread_graphic_vertex_t *pos;
	int count = 0;
    stlc_list_for_each_entry(pos, g, link) {
		count++;
    }
	return count;	
}

thread_graphic_vertex_t *thread_graphic_vertex_lookup(struct stlc_list_head *g, u_int64_t thread_id)
{
    thread_graphic_vertex_t *pos;
    stlc_list_for_each_entry(pos, g, link) {
        if(pos->thread_id == thread_id)
			return pos;
    }
	return NULL;
}

thread_graphic_vertex_t *thread_graphic_vertex_alloc(u_int64_t thread_id)
{
    thread_graphic_vertex_t *pos = (thread_graphic_vertex_t*)calloc(sizeof(thread_graphic_vertex_t), 1);
	if(pos) {
		pos->thread_id = thread_id;
	}
	
	return pos;
}

void thread_graphic_vertex_push_back(thread_graphic_vertex_t *v, u_int64_t thread_id)
{
	if(v) {
		if(v->thread_count < WAIT_HOLD_THREAD_MAX) {
			v->threads[v->thread_count] = thread_id;
			v->thread_count++;
		}
	}
}

thread_graphic_vertex_t *thread_graphic_vertex_lookup_or_alloc(struct stlc_list_head *g, u_int64_t thread_id)
{
	thread_graphic_vertex_t *v = thread_graphic_vertex_lookup(g, thread_id);
	if(v == NULL) {
		v = thread_graphic_vertex_alloc(thread_id);
		stlc_list_add(&v->link, g);
	}

	return v;
}

void thread_graphic_vertex_free(struct stlc_list_head *g)
{
	if(g == NULL)
		return;
	thread_graphic_vertex_t *pos, *n;
	stlc_list_for_each_entry_safe(pos, n, g, link) {
		stlc_list_del(&pos->link);
		free(pos);
	}
}

thread_req_lock_t *thread_req_lock_lookup(u_int64_t thread_id)
{
    thread_req_lock_t *pos;
    stlc_list_for_each_entry(pos, &m_thread_apply_lock, link) {
        if(pos->thread_id == thread_id)
			return pos;
    }
	return NULL;
}

thread_req_lock_t *thread_req_lock_alloc(u_int64_t thread_id)
{
    thread_req_lock_t *pos = (thread_req_lock_t*)calloc(sizeof(thread_req_lock_t), 1);
	if(pos) {
		pos->thread_id = thread_id;
		stlc_list_add_tail(&pos->link, &m_thread_apply_lock);
	}
	
	return pos;
}

void thread_req_lock_init(thread_req_lock_t *l, 
	u_int64_t thread_id, u_int64_t lock_addr, const char *file, int line, const char *funcname, const char *lock_name, char **stacktrace, u_int8_t num)
{
	l->thread_id = thread_id;
	l->lock_addr = lock_addr;        
	l->line = line;        
	l->t = time(NULL);  
	l->stacktrace = stacktrace;
	l->stacktrace_num = num;
	memset(l->file, 0, sizeof(l->file));
	memset(l->funcname, 0, sizeof(l->funcname));
	memset(l->lock_name, 0, sizeof(l->lock_name));
	memcpy(l->file, file, sizeof(l->file) - 1);        
	memcpy(l->funcname, funcname, sizeof(l->funcname) - 1);        
	memcpy(l->lock_name, lock_name, sizeof(l->lock_name) - 1);
}

thread_req_lock_t *thread_req_lock_lookup_or_alloc(u_int64_t thread_id)
{	
	thread_req_lock_t *l = thread_req_lock_lookup(thread_id);
	
	if(l == NULL)
		l = thread_req_lock_alloc(thread_id);

	return l;
}

thread_hold_lock_t *thread_hold_lock_lookup(u_int64_t lock_addr)
{
    thread_hold_lock_t *pos;
    stlc_list_for_each_entry(pos, &m_lock_belong_thread, link) {
        if(pos->lock_addr == lock_addr)
			return pos;
    }
	return NULL;
}

thread_hold_lock_t *thread_hold_lock_alloc(u_int64_t lock_addr)
{
    thread_hold_lock_t *pos = (thread_hold_lock_t*)calloc(sizeof(thread_hold_lock_t), 1);
	if(pos) {
		pos->lock_addr = lock_addr;
		STLC_INIT_LIST_HEAD(&pos->point_list);
		stlc_list_add_tail(&pos->link, &m_lock_belong_thread);
	}
	
	return pos;
}

thread_hold_lock_t *thread_hold_lock_lookup_or_alloc(u_int64_t lock_addr)
{
	thread_hold_lock_t *l = thread_hold_lock_lookup(lock_addr);
	if(l == NULL)
		l = thread_hold_lock_alloc(lock_addr);

	return l;
}

void thread_hold_lock_pushstack(thread_hold_lock_t *h, const char *file, int line, const char *funcname, char **stacktrace, u_int8_t stacktrace_num)
{
	
	hold_lock_point_t *p = hold_lock_point_init(file, line, funcname, stacktrace, stacktrace_num);
	if(p) {	
		stlc_list_add(&p->link, &h->point_list);
		h->count++;
	}
}

void thread_hold_lock_popstack(thread_hold_lock_t *h)
{
    if(!stlc_list_empty(&h->point_list)) {            
		hold_lock_point_t *p = stlc_list_entry(h->point_list.next, hold_lock_point_t, link);            
		stlc_list_del(&p->link);            
		hold_lock_point_free(p);            
		h->count--;        
	}	
}

void _show_thread_locks_info()
{
	thread_hold_lock_t *h;
	thread_req_lock_t *l;

	printf("\n======================waited lock======================\n");
	stlc_list_for_each_entry(l, &m_thread_apply_lock, link) {
		if(l->valid == 0)
			continue;
		thread_req_lock_show(l);
	}
	printf("=======================================================\n\n");
	printf("======================holded lock======================\n");
	stlc_list_for_each_entry(h, &m_lock_belong_thread, link) {
		thread_hold_lock_show(h);
	}	
	printf("=======================================================\n");
}


void dead_lock_stub_start_check(int run)
{
	pthread_t tid;
	
    STLC_INIT_LIST_HEAD(&m_thread_apply_lock);
    STLC_INIT_LIST_HEAD(&m_lock_belong_thread);

	if(run) {
		pthread_create(&tid, NULL, thread_rountine, NULL);
	}
}


