#include <stdio.h>
#include <sys/time.h>
#include "itimer_internal.h"

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

void evt1_timer(void * data, void * user)
{
	printf("evt1_timer\n");
	itimer_evt *evt = (itimer_evt*)user;
	itimer_mgr *mgr = (itimer_mgr*)data;
	//itimer_evt_stop(mgr, evt);
}

void evt2_timer(void * data, void * user)
{
	printf("evt2_timer\n");
}



int main()
{
	itimer_mgr lmgr;
	itimer_mgr *mgr = &lmgr;
	itimer_evt evt1;
	itimer_evt evt2;
	int count = 20;

	itimer_mgr_init(mgr, GetTickMS(), 1);
	itimer_evt_init(&evt1, evt1_timer, &mgr, &evt2);
	itimer_evt_init(&evt2, evt2_timer, NULL, NULL);

	itimer_evt_start(mgr, &evt1, 5, 1);
	itimer_evt_start(mgr, &evt2, 9, 1);


	while(count-- > 0) {
		itimer_mgr_run(mgr, GetTickMS());
		printf("usleep\n");
		usleep(1000);
	}
	
	
	return 0;
}
