#include <stdio.h>
#include "cl_thread.h"

cl_thread_t *t1 = NULL;
cl_thread_t *t2 = NULL;

int work_timer1(cl_thread_t *t)
{
	printf("work timer1\n");
}

int work_timer2(cl_thread_t *t)
{
	printf("work timer2\n");
}


int main()
{
	cl_thread_master_t master;
	cl_thread_t fetch;

	cl_thread_init(&master);


	CL_THREAD_TIMER_ON(&master, t1, work_timer1, NULL, 0);
	CL_THREAD_TIMER_ON(&master, t2, work_timer2, NULL, 0);

	while(cl_thread_fetch(&master, &fetch)) {
		cl_thread_call(&fetch);
	}
}
