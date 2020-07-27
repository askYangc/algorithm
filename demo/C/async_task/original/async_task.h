#ifndef _BASE_ASYNC_TASK_H_
#define _BASE_ASYNC_TASK_H_

#include "db_tool.h"
#include "async_proc.h"
#include "memcached_api.h"

#pragma pack(push,1)

enum {
    ASYNC_STORAGE_SQL = 65535,
};

typedef struct {
	u_int8_t ver;
	u_int8_t pad[3];

	u_int32_t ds_command;
	u_int32_t len;    
}async_storage_hdr_t;

#define async_storage_hdr_size (sizeof(async_storage_hdr_t))
#define	get_async_storage_payload(hdr, type) (type *)((async_storage_hdr_t*)hdr+1)


typedef struct {
    u_int8_t db_type;
    u_int8_t pad[3];
    u_int32_t len;
    u_int8_t sql[0];
}async_storage_sql_t;

typedef struct {
    int mc;
    int account;
    int sync;
}asyncConfig_t;

struct asyncTask_s;

typedef int (* asyncTask_func_t)(struct asyncTask_s *, async_storage_hdr_t *);


typedef struct asyncTask_s{
    asyncProc_t *_proc;
    MYSQL account_mysql;
    MYSQL sync_mysql;
    memcached_st *mc;
    asyncConfig_t cfg;
    asyncTask_func_t _callback;
}asyncTask_t;



#pragma pack(pop)


//通用使用配置
int task_async(asyncTask_t *task, u_int32_t cmd, u_int8_t *param, u_int32_t param_len) ;
int task_sql_async(asyncTask_t *task, int db_type, u_int8_t *sql, u_int32_t sql_len); 

asyncTask_t *async_task_def_func_init(asyncConfig_t *config, asyncTask_func_t _cb);
int async_task_stop(asyncTask_t *task);

#define async_task_safe_stop(task) do{\
    if(task){\
        async_task_stop(task);\
        task = NULL;\
    }\
}while(0)


#endif
