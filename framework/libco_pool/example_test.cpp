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
#include <pthread.h>
#include <dlfcn.h>
#include "stlc_list.h"
#include <assert.h>

#ifdef __FreeBSD__
#include <cstring>
#endif

extern "C" {
#include "db_tool.h"

}


using namespace std;

#if 0
typedef int (*pthread_mutex_init_pfn_t)(pthread_mutex_t * mutex, const pthread_mutexattr_t * attr);
typedef int (*pthread_mutex_lock_pfn_t)(pthread_mutex_t *mutex);
typedef int (*pthread_mutex_unlock_pfn_t)(pthread_mutex_t *mutex);


static pthread_mutex_init_pfn_t g_sys_pthread_mutex_init_func 	= (pthread_mutex_init_pfn_t)dlsym(RTLD_NEXT,"pthread_mutex_init");
static pthread_mutex_lock_pfn_t g_sys_pthread_mutex_lock_func = (pthread_mutex_lock_pfn_t)dlsym(RTLD_NEXT,"pthread_mutex_lock");
static pthread_mutex_unlock_pfn_t g_sys_pthread_mutex_unlock_func 	= (pthread_mutex_unlock_pfn_t)dlsym(RTLD_NEXT,"pthread_mutex_unlock");

#define HOOK_SYS_FUNC(name) if( !g_sys_##name##_func ) { g_sys_##name##_func = (name##_pfn_t)dlsym(RTLD_NEXT,#name); }


int pthread_mutex_init( pthread_mutex_t * mutex, const pthread_mutexattr_t * attr )
{
	HOOK_SYS_FUNC( pthread_mutex_init );

	printf("hello world\n");
	return 0;
}
#endif



struct task_t
{
	stCoRoutine_t *co;
	int fd;
	struct sockaddr_in addr;
};

static int SetNonBlock(int iSock)
{
    int iFlags;

    iFlags = fcntl(iSock, F_GETFL, 0);
    iFlags |= O_NONBLOCK;
    iFlags |= O_NDELAY;
    int ret = fcntl(iSock, F_SETFL, iFlags);
    return ret;
}



static void SetAddr(const char *pszIP,const unsigned short shPort,struct sockaddr_in &addr)
{
	bzero(&addr,sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(shPort);
	int nIP = 0;
	if( !pszIP || '\0' == *pszIP   
	    || 0 == strcmp(pszIP,"0") || 0 == strcmp(pszIP,"0.0.0.0") 
		|| 0 == strcmp(pszIP,"*") 
	  )
	{
		nIP = htonl(INADDR_ANY);
	}
	else
	{
		nIP = inet_addr(pszIP);
	}
	addr.sin_addr.s_addr = nIP;

}

static int CreateTcpSocket(const unsigned short shPort  = 0 ,const char *pszIP  = "*" ,bool bReuse  = false )
{
	int fd = socket(AF_INET,SOCK_STREAM, IPPROTO_TCP);
	if( fd >= 0 )
	{
		if(shPort != 0)
		{
			if(bReuse)
			{
				int nReuseAddr = 1;
				setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&nReuseAddr,sizeof(nReuseAddr));
			}
			struct sockaddr_in addr ;
			SetAddr(pszIP,shPort,addr);
			int ret = bind(fd,(struct sockaddr*)&addr,sizeof(addr));
			if( ret != 0)
			{
				close(fd);
				return -1;
			}
		}
	}
	return fd;
}

static void *poll_routine( void *arg )
{
	//co_enable_hook_sys();

	vector<task_t> &v = *(vector<task_t>*)arg;
	for(size_t i=0;i<v.size();i++)
	{
		int fd = CreateTcpSocket();
		SetNonBlock( fd );
		v[i].fd = fd;

		int ret = connect(fd,(struct sockaddr*)&v[i].addr,sizeof( v[i].addr )); 
		printf("co %p connect i %ld ret %d errno %d (%s)\n",
			co_self(),i,ret,errno,strerror(errno));
	}
	struct pollfd *pf = (struct pollfd*)calloc( 1,sizeof(struct pollfd) * v.size() );

	for(size_t i=0;i<v.size();i++)
	{
		pf[i].fd = v[i].fd;
		pf[i].events = ( POLLOUT | POLLERR | POLLHUP );
	}
	set<int> setRaiseFds;
	size_t iWaitCnt = v.size();
	for(;;)
	{
		int ret = poll( pf,iWaitCnt,1000 );
		printf("co %p poll wait %ld ret %d\n",
				co_self(),iWaitCnt,ret);
		for(int i=0;i<ret;i++)
		{
			printf("co %p fire fd %d revents 0x%X POLLOUT 0x%X POLLERR 0x%X POLLHUP 0x%X\n",
					co_self(),
					pf[i].fd,
					pf[i].revents,
					POLLOUT,
					POLLERR,
					POLLHUP
					);
			setRaiseFds.insert( pf[i].fd );
		}
		if( setRaiseFds.size() == v.size())
		{
			break;
		}
		if( ret <= 0 )
		{
			break;
		}

		iWaitCnt = 0;
		for(size_t i=0;i<v.size();i++)
		{
			if( setRaiseFds.find( v[i].fd ) == setRaiseFds.end() )
			{
				pf[ iWaitCnt ].fd = v[i].fd;
				pf[ iWaitCnt ].events = ( POLLOUT | POLLERR | POLLHUP );
				++iWaitCnt;
			}
		}
	}
	for(size_t i=0;i<v.size();i++)
	{
		close( v[i].fd );
		v[i].fd = -1;
	}

	printf("co %p task cnt %ld fire %ld\n",
			co_self(),v.size(),setRaiseFds.size() );
	return 0;
}
#define	TCP_NORMAL_ERR(e)	((e) == EINTR || (e) == EAGAIN || (e) == EWOULDBLOCK)



int main_bak(int argc,char *argv[])
{
	vector<task_t> v;
	write(0,NULL, 1);
	for(int i=1;i<argc;i+=2)
	{
		task_t task = { 0 };
		SetAddr( argv[i],atoi(argv[i+1]),task.addr );
		v.push_back( task );
	}

//------------------------------------------------------------------------------------
	printf("--------------------- main -------------------\n");
	vector<task_t> v2 = v;
	//poll_routine( &v2 );
	printf("--------------------- routine -------------------\n");

	for(int i=0;i<1;i++)
	{
		stCoRoutine_t *co = 0;
		vector<task_t> *v2 = new vector<task_t>();
		*v2 = v;
		co_create( &co,NULL,poll_routine,v2 );
		printf("routine i %d\n",i);
		co_resume( co );
	}

	co_eventloop( co_get_epoll_ct(),0,0 );

	return 0;
}

int loop(void *)
{
	printf("loop\n");
	return 0;
}

static __thread struct stlc_list_head lock_wait;
static __thread int lock_wait_init;



typedef struct {
	struct stlc_list_head thread_wait;
	
}co_mutex_t;

typedef struct co_mutex_lock_wait_s{
	struct stlc_list_head link;
	co_mutex_t *mutex;

	stCoCond_t *cond;
}co_mutex_lock_wait_t;


int co_mutex_init(co_mutex_t *mutex)
{
	assert(mutex != NULL);
	STLC_INIT_LIST_HEAD(&mutex->thread_wait);

	return 0;	
}

co_mutex_lock_wait_t *co_mutex_lock_wait_calloc(co_mutex_t *mutex)
{
	co_mutex_lock_wait_t *w = (co_mutex_lock_wait_t*)calloc(1, sizeof(co_mutex_lock_wait_t));
	assert(w != NULL);

	w->mutex = mutex;
	w->cond = co_cond_alloc();

	stlc_list_add_tail(&w->link, &lock_wait);

	return w;
}

co_mutex_lock_wait_t *co_mutex_lock_wait_get(co_mutex_t *mutex)
{
	co_mutex_lock_wait_t *w;

	stlc_list_for_each_entry(w, &lock_wait, link) {
		if(w->mutex == mutex) {
			return w;
		}
	}	

	w = co_mutex_lock_wait_calloc(mutex);
	return w;
}


int co_mutex_lock(co_mutex_t *mutex)
{
	//加入一个list,然后wait释放cpu
	co_mutex_lock_wait_t *w;

	if(!lock_wait_init) {
		STLC_INIT_LIST_HEAD(&lock_wait);
		lock_wait_init = 1;
	}

	w = co_mutex_lock_wait_get(mutex);	
	assert(w != NULL);
	
	co_cond_timedwait(w->cond, -1);
	
	return 0;
}

int co_mutex_unlock(co_mutex_t *mutex)
{
	//获取哪些线程在等待这个锁，然后通知这些线程
	return 0;
}

static void *co_mutex_mangager_proc( void *arg )
{
	co_enable_hook_sys();

	//等待socket消息，然后唤醒具体的协程
	while(1) {
		sleep(10);
	}
	return NULL;
}


int co_mutex_thread_init()
{
	stCoRoutine_t *co;
	co_create( &co,NULL,co_mutex_mangager_proc,NULL );
	co_resume(co);

	return 0;
}

static void *routine_func( void * )
{
	stCoEpoll_t * ev = co_get_epoll_ct(); //ct = current thread
	stCoRoutine_t *co;
	printf("int routine_func\n");

	co_create( &co,NULL,co_mutex_mangager_proc,NULL );
	co_resume(co);

	
	printf("there\n");
	co_eventloop( ev,NULL,0 );
	return 0;
}
int main(int argc,char *argv[])
{
	int cnt = atoi( argv[1] );


	pthread_t tid[ cnt ];
	for(int i=0;i<cnt;i++)
	{
		pthread_create( tid + i,NULL,routine_func,0);
	}
	for(;;)
	{
		sleep(1);
	}
	
	return 0;
}
//./example_poll 127.0.0.1 12365 127.0.0.1 12222 192.168.1.1 1000 192.168.1.2 1111

