#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "async_task.h"

int db_set_async_sql(asyncTask_t *task, async_storage_hdr_t *hdr, async_storage_sql_t *t)
{
    //return 0;
    MYSQL *mysql = NULL;

    switch(t->db_type) {
        case DB_TYPE_ACCOUNT:
            mysql = &task->account_mysql;
            break;
        case DB_TYPE_SYNC:
            mysql = &task->sync_mysql;
            break;
        default:
            ds_error("unknown db_type %hhu\n", t->db_type);
            return -1;
    }
    
    if(db_type_bin_execute_sql(mysql, t->db_type, (char*)t->sql, t->len) !=0) {
        ds_error("%s execute_sql error\n", __FUNCTION__);
        return -1;
    } 

    return 0;
}


void do_async_proc(asyncProc_t *proc, async_storage_hdr_t *hdr)
{
    asyncTask_t *task = (asyncTask_t*)proc->data;
    switch(hdr->ds_command) {
        case ASYNC_STORAGE_SQL:
            db_set_async_sql(task, hdr, get_async_storage_payload(hdr, async_storage_sql_t));
            break;
        default:
            break;
    }
    if(task->_callback) {
        task->_callback(task, hdr);
    }
}


static int _async_proc(asyncProc_t *proc, char *data, int len)
{
	int remain = len;
	char *cur = data;
	int header_len = sizeof(async_storage_hdr_t);
	async_storage_hdr_t *hdr;

	while(remain >= header_len) {
		hdr = (async_storage_hdr_t*)cur;
		remain -= header_len;
		if(hdr->len > remain) {
			break;
		}
		remain -= hdr->len;
		cur += header_len + hdr->len;
		do_async_proc(proc, hdr);
	}
	
	return 0;
}


asyncTask_t *async_task_def_func_init(asyncConfig_t *config, asyncTask_func_t _cb)
{
    assert(config != NULL);
    asyncTask_t *task = (asyncTask_t*)calloc(1, sizeof(asyncTask_t));
    assert(task != NULL);

    memcpy(&task->cfg, config, sizeof(asyncConfig_t));
    
    if(config->mc) {
    	task->mc = memcached_init();
    	if(task->mc == NULL){
    		printf("open memcached failed\n");
    		goto err;
    	}  
    }

    if(config->account) {
        //open account db
    	if(db_type_open(&task->account_mysql, DB_TYPE_ACCOUNT) < 0){
    		printf("open DB_TYPE_ACCOUNT db failed\n");
    		goto err;
    	}	
    }

    if(config->sync) {
        //open account db
    	if(db_type_open(&task->sync_mysql, DB_TYPE_SYNC) < 0){
    		printf("open DB_TYPE_SYNC db failed\n");
    		goto err;
    	}	
    }

    task->_callback = _cb;
    task->_proc = async_proc_init(_async_proc, task);
    async_proc_start(task->_proc);
    return task;
err:
    if(task) {
        if(task->mc) {
            memcached_free(task->mc);
        }
        db_close(&task->sync_mysql);
        db_close(&task->account_mysql);
        free(task);
    }
    
    return NULL;
}

int async_task_stop(asyncTask_t *task)
{
    if(task) {
        async_proc_safe_stop(task->_proc);
        asyncConfig_t *cfg = &task->cfg;
        if(cfg->mc) {
            memcached_free(task->mc);
        }
        if(cfg->sync) {
            db_close(&task->sync_mysql);
        }
        if(cfg->account) {
            db_close(&task->account_mysql);
        }
        free(task);
    }

    return 0;
}


int task_async(asyncTask_t *task, u_int32_t cmd, u_int8_t *param, u_int32_t param_len) 
{
    async_stream_t stream;
    async_storage_hdr_t local_hdr = {0};
    async_storage_hdr_t *hdr = &local_hdr;

    memset(&stream, 0, sizeof(async_stream_t));
    memset(hdr, 0, sizeof(async_storage_hdr_t));

    stream.data[0] = (char*)hdr;
    stream.len[0] = async_storage_hdr_size;

    stream.data[1] = (char*)param;
    stream.len[1] = param_len;

    stream.cout = 2;

	hdr->ver = 1;
	hdr->ds_command = cmd;
	hdr->len = param_len;
      
	async_proc_append_stream(task->_proc, &stream);    
    return 0;
}


int task_sql_async(asyncTask_t *task, int db_type, u_int8_t *sql, u_int32_t sql_len) 
{
    async_stream_t stream;
    async_storage_hdr_t local_hdr = {0};
    async_storage_sql_t local_storage_sql = {0};
    async_storage_hdr_t *hdr = &local_hdr;
    async_storage_sql_t *ps = &local_storage_sql;

    memset(&stream, 0, sizeof(async_stream_t));
    memset(hdr, 0, sizeof(async_storage_hdr_t));
    memset(ps, 0, sizeof(async_storage_sql_t));

    stream.data[0] = (char*)hdr;
    stream.len[0] = async_storage_hdr_size;

    stream.data[1] = (char*)ps;
    stream.len[1] = sizeof(async_storage_sql_t);

    stream.data[2] = (char*)sql;
    stream.len[2] = sql_len;

    stream.cout = 3;

	hdr->ver = 1;
	hdr->ds_command = ASYNC_STORAGE_SQL;
	hdr->len = sql_len + sizeof(async_storage_sql_t);
    ps->db_type = db_type;
    ps->len = sql_len;

    async_proc_append_stream(task->_proc, &stream);    
    return 0;
}

