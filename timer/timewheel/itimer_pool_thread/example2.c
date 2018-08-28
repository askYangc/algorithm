#include <stdio.h>
#include <sys/time.h>
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


void evt1_timer(void * data, void * user)
{
	printf("evt1_timer\n");
	
	//ITIMER_THREAD_OFF(handle1);
}

void evt2_timer(void * data, void * user)
{
	printf("evt2_timer\n");
}


int main()
{
	int count = 4;
	
	itimer_work_init(1);

	ITIMER_THREAD_ON(handle1, evt1_timer,  NULL, 5, 2);
	ITIMER_THREAD_ON(handle2, evt2_timer,  NULL, 9, 1);
	
	
	return 0;
}

