#include <stdio.h>
#include <sys/time.h>
#include <pthread.h>
#include "itimer.h"

static unsigned long long GetTickMS()
{
#if defined( __LIBCO_RDTSCP__)
    static uint32_t khz = getCpuKhz();
    return counter() / khz;
#else
    struct timeval now = { 0 };
    gettimeofday( &now,NULL );
    unsigned long long u = now.tv_sec;
    u *= 1000;
    u += now.tv_usec / 1000;
    return u;
#endif
}

u_int64_t handle1 = 0;
u_int64_t handle2 = 0;
u_int64_t handle3 = 0;
u_int64_t handle4 = 0;


void evt1_timer(void * data, void * user)
{
	printf("evt1_timer, handle1:%u\n", handle1);
	
	//ITIMER_THREAD_OFF(handle1);
    //ITIMER_THREAD_OFF(handle2);
	//ITIMER_THREAD_OFF(handle3);
    ITIMER_THREAD_OFF(handle4);
	printf("handle1: %u\n", handle1);
}

void evt2_timer(void * data, void * user)
{
	printf("evt2_timer, handle2:%u\n", handle2);
}

void evt3_timer(void * data, void * user)
{
	printf("evt3_timer, handle3:%u\n", handle3);
	
	ITIMER_THREAD_OFF(handle2);
    //ITIMER_THREAD_OFF(handle4);
	printf("handle1: %u\n", handle3);
}

void evt4_timer(void * data, void * user)
{
	printf("evt4_timer, handle4:%u\n", handle4);
}


void *thread_proc(void *p)
{

	ITIMER_THREAD_ON(handle3, evt3_timer,  NULL, 5, 5);
	ITIMER_THREAD_ON(handle4, evt4_timer,  NULL, 9, 2);

    while(1) {
        itimer_update_timer();
        usleep(1000);
    }

    return NULL;
}

int main()
{
	int count = 4;
    pthread_t pid;
	
	itimer_work_init(1);

    pthread_create(&pid, NULL, thread_proc, NULL);

	ITIMER_THREAD_ON(handle1, evt1_timer,  NULL, 5, 5);
	ITIMER_THREAD_ON(handle2, evt2_timer,  NULL, 9, 2);


	while(1) {
		itimer_update_timer();
		usleep(1000);
	}
	
	return 0;
}

