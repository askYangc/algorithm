#include <stdio.h>
#include <unistd.h>
#include "async_task.h"

asyncTask_t *task;


int demo_test(asyncTask_t *task, async_storage_hdr_t *hdr)
{
    printf("demo ok, cmd: %d, len: %d\n", hdr->ds_command, hdr->len);


    return 0;
}

int main()
{
    asyncConfig_t config;
    config.account = 0;
    config.mc = 0;
    config.sync = 1;
    u_int8_t buf[100] = "okok";

    char sql[100] = "insert into test(name,num) values('zeng', 2);";

    task = async_task_def_func_init(&config, demo_test);


    task_async(task, 1, NULL, 0);
    task_async(task, 5, buf, strlen((char*)buf));
    task_sql_async(task, DB_TYPE_SYNC, (u_int8_t*)sql, strlen(sql));

    sleep(5);

    async_task_safe_stop(task);
    
    return 0;
}

