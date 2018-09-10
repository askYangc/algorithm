#include <stdio.h>

#include <unistd.h>
#include <pthread.h>

// *) 引入该头文件即可, 不需要在修改任何代码了
#include "dead_lock_stub.h"

#define true 1
#define false 0

pthread_mutex_t mutex_1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_2 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_3;

void *thread_rountine_1(void *args)
{
	printf("thread_rountine_1 lock mutex_1\n");
    pthread_mutex_lock(&mutex_1);
	printf("thread_rountine_1 lock mutex_1 after\n");
    sleep(1);
    pthread_mutex_lock(&mutex_2);
	printf("thread_rountine_1 lock mutex_2 after\n");

    pthread_mutex_unlock(&mutex_2);
    pthread_mutex_unlock(&mutex_1);
    return (void *)(0);
}

void *thread_rountine_2(void *args)
{
	printf("thread_rountine_2 lock mutex_2\n");
    pthread_mutex_lock(&mutex_2);
	printf("thread_rountine_2 lock after mutex_2\n");
    sleep(1);
    pthread_mutex_lock(&mutex_1);

    pthread_mutex_unlock(&mutex_1);
    pthread_mutex_unlock(&mutex_2);
    return (void *)(0);
}

void *thread_rountine_3(void *args)
{
    pthread_mutex_lock(&mutex_1);
    printf("lock 1\n");
    pthread_mutex_lock(&mutex_2);
    printf("lock 2\n");
    sleep(2);
    pthread_mutex_lock(&mutex_3);
    
    printf("unlock thread_rountine_3\n");
    

    
    while(1) {
           sleep(3);
        
    }
    return (void *)(0);
}

void *thread_rountine_4(void *args)
{
    pthread_mutex_lock(&mutex_3);
    printf("lock 3\n");
    sleep(2);
    pthread_mutex_lock(&mutex_1);
   
    printf("unlock thread_rountine_4\n");
    while(1) {
           sleep(3);
        
    }
    return (void *)(0);
}

void *thread_rountine_5(void *args)
{
    
    pthread_mutex_lock(&mutex_3);
    printf("lock 3 1\n");
    //pthread_mutex_unlock(&mutex_3);
    pthread_mutex_lock(&mutex_3);
    printf("lock 3 2\n");
    sleep(2);
 
   
    printf("unlock thread_rountine_5\n");
    while(1) {
           sleep(3);
        
    }
    return (void *)(0);
}

void *thread_rountine_6(void *args)
{
    sleep(3);
    printf("thread_rountine_6 req 3\n");
    pthread_mutex_lock(&mutex_3);
    pthread_mutex_unlock(&mutex_3);
    printf("thread_rountine_6 unlock 3\n");
   
    while(1) {
           sleep(3);
        
    }
    return (void *)(0);
}

void *thread_rountine_7(void *args)
{
    pthread_mutex_lock(&mutex_3);
    printf("lock 3 1\n");
    //pthread_mutex_unlock(&mutex_3);
    pthread_mutex_lock(&mutex_3);
    printf("lock 3 2\n");
    sleep(2);
	//pthread_mutex_lock(&mutex_1);
 
   
    printf("unlock thread_rountine_5\n");
    while(1) {
           sleep(3);
        
    }
    return (void *)(0);
}

void *thread_rountine_8(void *args)
{
    sleep(1);
    printf("thread_rountine_6 req 3\n");
    pthread_mutex_lock(&mutex_1);
	
    pthread_mutex_lock(&mutex_3);
    printf("thread_rountine_6 unlock 3\n");
   
    while(1) {
           sleep(3);
        
    }
    return (void *)(0);
}


#if 0
int main()
{
    // *) 添加该行， 表示启动死锁检测功能 
    //DeadLockGraphic::getInstance().start_check();
	dead_lock_stub_start_check(1);
	
    pthread_mutex_t t;
    pthread_mutex_init(&t, NULL);
    
    pthread_t tid1, tid2;
    pthread_create(&tid1, NULL, thread_rountine_1, NULL);
    pthread_create(&tid2, NULL, thread_rountine_2, NULL);

    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);
    return 0;
}
#endif
#if 0
int main()
{
 
    // *) 添加该行， 表示启动死锁检测功能 
    dead_lock_stub_start_check(1);
    
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    //pthread_mutex_init(&mutex_3, &attr);
    pthread_mutex_init(&mutex_3, NULL);

    pthread_t tid4, tid3;
    pthread_create(&tid4, NULL, thread_rountine_4, NULL);
    //pthread_create(&tid2, NULL, thread_rountine_2, NULL);
    pthread_create(&tid3, NULL, thread_rountine_3, NULL);

    pthread_join(tid3, NULL);
    pthread_join(tid4, NULL);
    return 0;      
}
#endif

#if 0
int main()
{
 
    // *) 添加该行， 表示启动死锁检测功能 
    dead_lock_stub_start_check(1);
    
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    //pthread_mutex_init(&mutex_3, &attr);
    pthread_mutex_init(&mutex_3, NULL);

    pthread_t tid4, tid3;
    pthread_create(&tid4, NULL, thread_rountine_5, NULL);
    //pthread_create(&tid2, NULL, thread_rountine_2, NULL);
    pthread_create(&tid3, NULL, thread_rountine_6, NULL);

    pthread_join(tid3, NULL);
    pthread_join(tid4, NULL);
    return 0;      
}
#endif

#if 1
int main()
{
 
    // *) 添加该行， 表示启动死锁检测功能 
    dead_lock_stub_start_check(1);
    
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
   	pthread_mutex_init(&mutex_3, &attr);
    //pthread_mutex_init(&mutex_3, NULL);

    pthread_t tid4, tid3;
    pthread_create(&tid4, NULL, thread_rountine_7, NULL);
    //pthread_create(&tid2, NULL, thread_rountine_2, NULL);
    pthread_create(&tid3, NULL, thread_rountine_8, NULL);

    pthread_join(tid3, NULL);
    pthread_join(tid4, NULL);
    return 0;      
}
#endif

