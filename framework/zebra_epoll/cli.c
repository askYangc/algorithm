#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <errno.h>

#include "ds_types.h"
#include "cl_thread.h"

#define ds_fatal printf
#define	TCP_NORMAL_ERR(e)	((e) == EINTR || (e) == EAGAIN || (e) == EWOULDBLOCK)


#define sockopt_noblock(socket, noblock) do{\
	int val = (noblock);\
	ioctl((socket), FIONBIO, (long)&val);\
}while(0)


typedef struct {
	cl_thread_master_t m;
	cl_thread_t *t_console_read;
	cl_thread_t *t_read;
	cl_thread_t *t_write;
	char buf[100];
	int n;

	int sock;
}cli_t;

cli_t *cli = NULL;

static inline u_int32_t str_to_ip(char *str)
{

	u_int32_t ip = 0;
	struct in_addr addr;
	
	if(inet_aton(str, &addr) != 0){
		ip = htonl(addr.s_addr);
	}
	return ip;
}


int connect_tcp(u_int32_t ip, int port)
{
	int	sock, addr_len;
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
	addr_len = sizeof(addr);

	if (0 !=  connect(sock, (struct sockaddr *)&addr, addr_len)) {
		//ds_info("connect_tcp failed(ip=0x%08x, port=%d): %s\n\r", ip, port, strerror(errno));
		close(sock);
		return -1;
	}
	sockopt_noblock(sock, 1);
	return sock;
}

int tcp_read(cl_thread_t *t)
{
	char buf[100] = {0};
	cli->t_read = NULL;
	CL_THREAD_READ_ON(&cli->m, cli->t_read, tcp_read, NULL, cli->sock);
	
	int n = read(cli->sock, buf, sizeof(buf));
	if(n > 0) {
		printf("Srv:%s", buf);
	}else if(n == 0) {
		printf("Srv is Closed\n");
		CL_THREAD_OFF(cli->t_read);
		CL_THREAD_OFF(cli->t_write);
	}else {
		printf("n:%d\n", n);
		if(!TCP_NORMAL_ERR(errno)) {
			printf("Srv1 is Closed\n");
			CL_THREAD_OFF(cli->t_read);
			CL_THREAD_OFF(cli->t_write);
		}
	}

	return 0;
}

int tcp_write(cl_thread_t *t)
{
	cli->t_write = NULL;
	write(cli->sock, cli->buf, cli->n);

	return 0;
}

int console_read(cl_thread_t *t)
{
	cli->t_console_read = NULL;
	CL_THREAD_READ_ON(&cli->m, cli->t_console_read, console_read, NULL, 0);
	
	cli->n = read(0, cli->buf, sizeof(cli->buf));
	CL_THREAD_WRITE_ON(&cli->m, cli->t_write, tcp_write, NULL, cli->sock);
	return 0;
}


void cli_init()
{
	cli = (cli_t*)calloc(1, sizeof(cli_t));
	assert(cli != NULL);
	
	cl_thread_init(&cli->m);

	cli->sock = connect_tcp(str_to_ip("127.0.0.1"), 4567);
	if(cli->sock <= 0) {
		printf("connect_tcp failed\n");
		return ;
	}

	CL_THREAD_READ_ON(&cli->m, cli->t_read, tcp_read, NULL, cli->sock);
}

int main()
{
	cl_thread_t thread;

	cli_init();
	CL_THREAD_READ_ON(&cli->m, cli->t_console_read, console_read, NULL, 0);

	while(cl_thread_fetch(&cli->m, &thread)) {
		cl_thread_call(&thread);
	}
	

	return 0;
}

