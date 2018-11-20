#include <stdio.h>
#include "AsyncLog.h"

AsyncLogging *glog = NULL;


int main()
{
	char logline[1024];
	int n = 0;
	glog = new AsyncLogging("11test", 4000);

	glog->start();

	n = sprintf(logline, "I'm a test\n");

	glog->append(logline, n);

	printf("wait asynclog goto while\n");
	sleep(1);
	glog->stop();
	printf("stop it\n");
	

	return 0;
}

