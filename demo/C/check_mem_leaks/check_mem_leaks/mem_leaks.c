#include "mem_leaks.h"
#include <string.h>
#include <malloc.h>
#include <execinfo.h>
#include <pthread.h>
#include <assert.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/syscall.h>

#define MAX_STACKTRACE 10

#ifndef COLOR_RED
#define	COLOR_RED	"\033[31m\033[1m"
#define	COLOR_GREEN	"\033[32m\033[1m"
#define	COLOR_YELLOW	"\033[33m\033[1m"
#define	COLOR_END	"\033[0m"
#endif


//pthread_mutex_t m_lock;
static pthread_once_t my_key_once = PTHREAD_ONCE_INIT;
mem_leaks_hash_t *g_alloc_hash_b;


#define MEM_LEAKS_MAX_THREADS 102400
/* 开启/关闭钩子函数 */
int pids[MEM_LEAKS_MAX_THREADS] = {0};

static int mem_check = 0;
/* 开启/关闭钩子 */
//int hook_active = 1;





#ifdef __cplusplus
extern "C" {
#endif
extern void *__libc_malloc(size_t size);
extern void __libc_free(void*);
extern void*  __libc_calloc(size_t, size_t);
extern void*  __libc_realloc(void*, size_t);


/*
如果使用my_backtrace
如果源代码编译时使用了-O1或-O2优化选项，可执行代码会把ebp/rbp/rsp寄存器当作普通寄存器使用，导致backtrace失败。
为了防止这种情况发生，可以在编译时使用-O2  -fno-omit-frame-pointer  或-Og 来避免优化中使用上述寄存器。
backtrace_symbols的实现需要符号名称的支持，在gcc编译过程中需要加入-rdynamic参数
系统自带的backtrace函数是通过读取操作系统的一个全局信息区，在多线程并发调用时，会造成严重的锁冲突
*/
#define STACKCALL __attribute__((regparm(1),noinline))  
static void ** STACKCALL getEBP(void){  
        void **ebp=NULL;  
        __asm__ __volatile__("mov %%rbp, %0;\n\t"  
                    :"=m"(ebp)      /* 输出 */  
                    :      /* 输入 */  
                    :"memory");     /* 不受影响的寄存器 */  
        return (void **)(*ebp);  
}  
static int my_backtrace(void **buffer,int size){  
      
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

static mem_leaks_hash_t *mem_leaks_hash_init()
{
	int i;
	mem_leaks_hash_t *hash;
	hash = (mem_leaks_hash_t *)__libc_calloc(sizeof(*hash), 1);
	if(hash){
		hash->total = 0;
		
		for(i = 0; i < MEM_HASH_SIZE; i++){
			STLC_INIT_HLIST_HEAD(&hash->bucket[i]);
		}
        pthread_mutex_init(&hash->lock, NULL);
	}
	return hash;
}

#define  mem_hash_key(ptr)  (int)((unsigned long)(ptr)%MEM_HASH_SIZE)
static inline void mem_hash_add(mem_leaks_info_t *info, mem_leaks_hash_t *hash)
{
	stlc_hlist_add_head(&info->hash_link, &hash->bucket[mem_hash_key(info->p)]);
	hash->count[mem_hash_key(info->p)]++;
	hash->total++;
}

static inline void mem_hash_del(mem_leaks_info_t *info, mem_leaks_hash_t *hash)
{
	hash->total--;
	hash->count[mem_hash_key(info->p)]--;
	stlc_hlist_del(&info->hash_link);
}
static inline mem_leaks_info_t *mem_hash_lookup(mem_leaks_hash_t *hash, void *ptr)
{
	mem_leaks_info_t *info = NULL;
	struct stlc_hlist_node *pos;
	int key = mem_hash_key(ptr);

	stlc_hlist_for_each_entry(info, pos, &hash->bucket[key], hash_link){
		if(info->p == ptr)
			return info;
	}
	return NULL;
}



mem_leaks_info_t *mem_alloc_info_alloc(void *p, char **stacktrace, u_int8_t stacktrace_num, size_t size)
{
    mem_leaks_info_t *info = __libc_calloc(1, sizeof(mem_leaks_info_t));
    assert(info);
    info->p = p;
    info->stacktrace = stacktrace;
    info->stacktrace_num = stacktrace_num;
    info->size = size;
    return info;
}


void mem_alloc_info_free(mem_leaks_info_t *info)
{
    if(info) {
        if(info->stacktrace) {
            __libc_free(info->stacktrace);
        }
        __libc_free(info);
    }
}

void mem_alloc_info_show(mem_leaks_info_t *info)
{
    int i;
    if(info) {
        printf("===========%salloc%s===============\n", COLOR_YELLOW, COLOR_END);
        printf("addr: %s%p%s, stack_num: %u, size: %s%u%s\n", COLOR_GREEN, info->p, COLOR_END, info->stacktrace_num, COLOR_GREEN, info->size, COLOR_END);
        if(info->stacktrace) {
            printf("stacktrace:\n");
            for(i = 0; i < info->stacktrace_num; i++) {
                printf("\tframe %d :%s\n", i, info->stacktrace[i]);
            }
        }
    }
}

mem_leaks_info_t *mem_alloc_info_add(void *p, char **stacktrace, u_int8_t stacktrace_num, size_t size)
{
    mem_leaks_info_t *info = mem_alloc_info_alloc(p, stacktrace, stacktrace_num, size);
    pthread_mutex_lock(&g_alloc_hash_b->lock);
    mem_hash_add(info, g_alloc_hash_b);
    pthread_mutex_unlock(&g_alloc_hash_b->lock);
    return info;
}

void mem_alloc_info_del(void *p)
{
    pthread_mutex_lock(&g_alloc_hash_b->lock);
    mem_leaks_info_t *pos = mem_hash_lookup(g_alloc_hash_b, p);
    if(pos) {
        mem_hash_del(pos, g_alloc_hash_b);
        mem_alloc_info_free(pos);
    }
    pthread_mutex_unlock(&g_alloc_hash_b->lock);
}


static void mem_leaks_init()
{   
#if 0
    pthread_mutexattr_t attr;    
    pthread_mutexattr_init(&attr);    
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);       
    pthread_mutex_init(&m_lock, &attr);
#endif
    //pthread_mutex_init(&m_lock, NULL);

    int i;
    for(i = 0; i < MEM_LEAKS_MAX_THREADS; i++) {
        pids[i] = 1;
    }
    
    g_alloc_hash_b = mem_leaks_hash_init();
    //__free_hook = my_free;
}



#if 0
static void *__wrap_memalign(size_t boundary, size_t size)
{
    void *p = NULL;
    void *array[MAX_STACKTRACE];
    pthread_once(&my_key_once, mem_leaks_init);   
    pthread_mutex_lock(&m_lock);  
    
    p = __real_memalign(boundary, size);
    int stack_num = my_backtrace(array, MAX_STACKTRACE);
    char **stacktrace = backtrace_symbols(array, stack_num);
    mem_alloc_info_add(p, stacktrace, stack_num, size);

    pthread_mutex_unlock(&m_lock);  
    return p;
}
#endif

/*
如果是将分配的内存扩大，则有以下情况：
1）如果当前内存段后面有需要的内存空间，则直接扩展这段内存空间，realloc()将返回原指针。
2）如果当前内存段后面的空闲字节不够，那么就使用堆中的第一个能够满足这一要求的内存块，将目前的数据复制到新的位置，并将原来的数据块释放掉，返回新的内存块位置。
3）如果申请失败，将返回NULL，此时，原来的指针仍然有效。
*/
void *realloc(void *__ptr, size_t size)
{
    //printf("in realloc\n");
    void *p = NULL;
    void *array[MAX_STACKTRACE];
    pthread_once(&my_key_once, mem_leaks_init);   

    if(!mem_check) {
        p = __libc_realloc(__ptr, size);
        if(p) {
            mem_alloc_info_del(__ptr);
        }
        return p;
    }

    if(pids[GetPid()]) {
        pids[GetPid()] = 0;
        p = __libc_realloc(__ptr, size);
        if(p) {
            mem_alloc_info_del(__ptr);
            int stack_num = my_backtrace(array, MAX_STACKTRACE);
            char **stacktrace = backtrace_symbols(array, stack_num);
            mem_alloc_info_add(p, stacktrace, stack_num, size);           
        }
        pids[GetPid()] = 1;
        return p;
    }
    
    p = __libc_realloc(__ptr, size);
    if(p) {
        mem_alloc_info_del(__ptr);
    }
    return p;
}


void *calloc(size_t numElements, size_t sizeOfElement)
{
    void *p = NULL;
    void *array[MAX_STACKTRACE];
    pthread_once(&my_key_once, mem_leaks_init);   
    
    if(!mem_check) {
        p = __libc_calloc(numElements, sizeOfElement);
        return p;
    }

    if(pids[GetPid()]) {
        pids[GetPid()] = 0;
        p = __libc_calloc(numElements, sizeOfElement);

        int stack_num = my_backtrace(array, MAX_STACKTRACE);
        char **stacktrace = backtrace_symbols(array, stack_num);
        mem_alloc_info_add(p, stacktrace, stack_num, numElements*sizeOfElement);
        
        pids[GetPid()] = 1;
        return p;
    }

    p = __libc_calloc(numElements, sizeOfElement);
    return p;
}

void *malloc(size_t size)
{
    void *p = NULL;
    void *array[MAX_STACKTRACE];
    pthread_once(&my_key_once, mem_leaks_init);   

    if(!mem_check) {
        p = __libc_malloc(size);
        return p;
    }
    
    if(pids[GetPid()]) {
        pids[GetPid()] = 0;
        p = __libc_malloc(size);

        int stack_num = my_backtrace(array, MAX_STACKTRACE);
        char **stacktrace = backtrace_symbols(array, stack_num);
        mem_alloc_info_show(mem_alloc_info_add(p, stacktrace, stack_num, size));
        
        pids[GetPid()] = 1;
        return p;
    }

    p = __libc_malloc(size);
    return p;
}

void free(void *ptr)
{
    pthread_once(&my_key_once, mem_leaks_init);   

    if(ptr){
        mem_alloc_info_del(ptr);
    }
    __libc_free(ptr);
}


void mem_leaks_show()
{
    int i;
    mem_leaks_info_t *tpos;
    struct stlc_hlist_node *pos;
    pthread_once(&my_key_once, mem_leaks_init);   

    printf("\n%s===================mem_alloc_show====================%s\n\n", COLOR_YELLOW, COLOR_END);
    pthread_mutex_lock(&g_alloc_hash_b->lock);
    for(i = 0; i < MEM_HASH_SIZE; i++) {
        stlc_hlist_for_each_entry(tpos, pos, &g_alloc_hash_b->bucket[i], hash_link) {
            mem_alloc_info_show(tpos);
        }
    }
    pthread_mutex_unlock(&g_alloc_hash_b->lock);
    printf("\n%s=========================end======================!!!%s\n\n", COLOR_YELLOW, COLOR_END);
    
}

void mem_leaks_start()
{
    pthread_once(&my_key_once, mem_leaks_init);   
    mem_check = 1;   
}

void mem_leaks_stop()
{
    pthread_once(&my_key_once, mem_leaks_init);   
    mem_check = 0;    
}

void mem_leaks_cmd(char *param)
{
    if(param == NULL){
        printf("usage: mem_check start|stop|show, now mem_check: %d\n", mem_check);
       return ;
    }

	if(strcmp(param, "start") == 0){
		printf("\nmem_leaks_start\n");
		mem_leaks_start();
		return;
	}else if(strcmp(param, "stop") == 0){
		printf("\nmem_leaks_stop\n");
		mem_leaks_stop();
		return;
	}else if(strcmp(param, "show") == 0){
		printf("\nmem_leaks_show\n");
		mem_leaks_show();
		return;
	} else {
        printf("usage: mem_check start|stop|show\n");
    }

}


#ifdef __cplusplus
}
#endif


