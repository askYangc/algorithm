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

itimer_evt *evt1 = NULL;
itimer_evt *evt2 = NULL;



void evt1_timer(void * data, void * user)
{
	printf("evt1_timer\n");
	
	ITIMER_OFF(evt1);
}

void evt2_timer(void * data, void * user)
{
	printf("evt2_timer\n");
}


int main()
{
	int count = 20;
	
	itimer_evt_pool_init();
	itimer_mgr_thread_init(1);

	ITIMER_ON(evt1, evt1_timer,  NULL, 5, 2);
	ITIMER_ON(evt2, evt2_timer,  NULL, 9, 1);
	

	while(count-- > 0) {
		itimer_run();
		printf("usleep\n");
		usleep(1000);
	}

	
	
	return 0;
}

