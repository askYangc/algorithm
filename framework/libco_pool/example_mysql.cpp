/*
* Tencent is pleased to support the open source community by making Libco available.

* Copyright (C) 2014 THL A29 Limited, a Tencent company. All rights reserved.
*
* Licensed under the Apache License, Version 2.0 (the "License"); 
* you may not use this file except in compliance with the License. 
* You may obtain a copy of the License at
*
*	http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, 
* software distributed under the License is distributed on an "AS IS" BASIS, 
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
* See the License for the specific language governing permissions and 
* limitations under the License.
*/

#include "co_routine.h"


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/time.h>
#include <stack>

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <errno.h>
#include <vector>
#include <set>
#include <unistd.h>

#ifdef __FreeBSD__
#include <cstring>
#endif

extern "C" {
#include "db_tool.h"

}

using namespace std;



static void *poll_routine( void *arg )
{
	//return 0;
	//co_enable_hook_sys();
	//printf("poll_routine\n");
	return NULL;
}

static void *poll_total( void *arg )
{
	//return 0;
	co_enable_hook_sys();

	while(1) {
		poll(NULL, 0, 5000);

		stCoRoutine_pool_status();
	}
	
	return NULL;
}

int main(int argc,char *argv[])
{

	stCoRoutine_t *co;

	stCoRoutine_pool_start_gc();

	stCoRoutineAttr_t attr;
	attr.share_stack = co_alloc_sharestack(1, 1024 * 128);
	attr.stack_size = 0;

	co_create(&co, NULL, poll_total, NULL);
	co_resume(co);

	for(int i = 0; i < 1000*20; i++) {
		co_create(&co, &attr, poll_routine, NULL);
		co_resume(co);
	}	

	co_eventloop(co_get_epoll_ct(), NULL, NULL);

	return 0;

}
//./example_poll 127.0.0.1 12365 127.0.0.1 12222 192.168.1.1 1000 192.168.1.2 1111

