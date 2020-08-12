#ifndef _CHECK_MEM_LEAKS_H_
#define _CHECK_MEM_LEAKS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "ds_types.h"
#include "stlc_list.h"


typedef struct {	
    struct stlc_hlist_node hash_link;    
    void *p;
    char **stacktrace;
    u_int8_t stacktrace_num;
    u_int32_t size;
}mem_leaks_info_t;

#define MEM_HASH_SIZE (1024*64)

typedef struct{
	struct stlc_hlist_head bucket[MEM_HASH_SIZE];
	int count[MEM_HASH_SIZE];
	pthread_mutex_t lock;
	int total;
}mem_leaks_hash_t;


    ///private
    //void *__wrap_malloc(size_t size);
    //void *__wrap_realloc(void *__ptr, size_t size);
    //void *__wrap_calloc(size_t numElements, size_t sizeOfElement);
    //void __wrap_free(void *ptr);

void mem_leaks_start();
void mem_leaks_show();
void mem_leaks_stop();
void mem_leaks_cmd(char *param);


#ifdef __cplusplus
}
#endif

#endif

