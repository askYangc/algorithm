#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>

#include "cl_thread.h"


#define ds_fatal printf
#define ds_info printf
#define	TCP_NORMAL_ERR(e)	((e) == EINTR || (e) == EAGAIN || (e) == EWOULDBLOCK)


#define NIPQUADS(ip) (((ip) >> 24)&0xFF), (((ip) >> 16)&0xFF), (((ip) >> 8)&0xFF), ((ip)&0xFF)


typedef struct {
	cl_thread_master_t m;
	cl_thread_t *t_read;
	cl_thread_t *t_write;
	cl_thread_t *t_console_read;
	cl_thread_t *t_listen;

	char buf[100];
	int n;

	int listenfd; 
	int sock;
}srv_t;

srv_t *srv = NULL;

int sockopt_reuseaddr(int sock)
{
	int ret;
	int on = 1;

	ret = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (void *) &on, sizeof(on));
	if (ret < 0)
	{
		ds_info("can't set sockopt SO_REUSEADDR to socket %d errno %s\n", sock, strerror(errno));
		return -1;
	}
	return 0;
}


/* ip和port都是主机序 */
int create_tcp_server(u_int32_t ip, int port)
{
	int	sock, ret;
	struct sockaddr_in addr;
	
	sock = socket(AF_INET, SOCK_STREAM, 0);		
	if (sock < 0) {
		ds_fatal("create_tcp_server socket failed!\n\r");
		return -1;
	}
	
	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(ip);

	sockopt_reuseaddr(sock);

	ret = bind(sock, (struct sockaddr *) &addr, sizeof(struct sockaddr_in));
	if (ret < 0) {
		ds_fatal("create_tcp_server Can't bind to socket(ip=%u.%u.%u.%u, port=%d): %s\n\r", 
			NIPQUADS(ip), port, strerror(errno));
		close(sock);
		return -1;
	}

	ret = listen(sock, SOMAXCONN);
	if (ret < 0) {
		ds_fatal("create_tcp_server Can't listen to socket(port=%d): %s\n\r", port, strerror(errno));
		close(sock);
		return -1;
	}
	sockopt_reuseaddr(sock);
	return sock;
}

void cli_close()
{
	CL_THREAD_OFF(srv->t_read);
	CL_THREAD_OFF(srv->t_write);

	close(srv->sock);
	srv->sock = -1;
}

int tcp_read(cl_thread_t *t)
{
	char buf[100] = {0};
	srv->t_read = NULL;
	CL_THREAD_READ_ON(&srv->m, srv->t_read, tcp_read, NULL, srv->sock);
	
	int n = read(srv->sock, buf, sizeof(buf));
	if(n > 0) {
		printf("Cli:%s", buf);
	}else if(n == 0) {
		printf("Cli is Closed\n");
		cli_close();
	}else {
		if(!TCP_NORMAL_ERR(errno)) {
			printf("Cli1 is Closed\n");
			cli_close();
		}
	}

	return 0;
}

int tcp_write(cl_thread_t *t)
{
	srv->t_write = NULL;
	write(srv->sock, srv->buf, srv->n);

	return 0;
}

int tcp_listen(cl_thread_t *t)
{
	struct sockaddr_in sa;
	socklen_t len;	
	
	srv->t_listen = NULL;	
	CL_THREAD_READ_ON(&srv->m, srv->t_listen, tcp_listen, NULL, srv->listenfd);

	srv->sock = accept(srv->listenfd, (struct sockaddr*)&sa, &len);
	if(srv->sock < 0) {
		printf("accept failed\n");
		return 0;
	}

	printf("accept ok\n");
	CL_THREAD_READ_ON(&srv->m, srv->t_read, tcp_read, NULL, srv->sock);

	return 0;
}

int console_read(cl_thread_t *t)
{
	srv->t_console_read = NULL;
	CL_THREAD_READ_ON(&srv->m, srv->t_console_read, console_read, NULL, 0);
	srv->n = read(0, srv->buf, sizeof(srv->buf));
	CL_THREAD_WRITE_ON(&srv->m, srv->t_write, tcp_write, NULL, srv->sock);
	return 0;
}

void srv_init()
{
	srv = (srv_t*)calloc(1, sizeof(srv_t));
	assert(srv != NULL);
	
	cl_thread_init(&srv->m);

	srv->listenfd = create_tcp_server(0, 4567);
	if(srv->listenfd <= 0) {
		printf("create_tcp_server failed\n");
		return ;
	}

	CL_THREAD_READ_ON(&srv->m, srv->t_listen, tcp_listen, NULL, srv->listenfd);
}

int main()
{
	cl_thread_t thread;

	srv_init();
	CL_THREAD_READ_ON(&srv->m, srv->t_console_read, console_read, NULL, 0);

	while(cl_thread_fetch(&srv->m, &thread)) {
		cl_thread_call(&thread);
	}
	

	return 0;
}



