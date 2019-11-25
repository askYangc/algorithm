#include <stdio.h>

#include "linkageasynclog.h"
#include "linkage_log.h"


void log_adddev(linkage_log_header_t *hdr)
{
	log_adddev_t *add = (log_adddev_t*)(hdr+1);
	
	printf("log_adddev\n");
	printf("add home_id: %d, user_id: %d, sn:%llu\n", add->home_id, add->user_id, (u64)add->sn);
}


linkagelog_proc_t linkagelog_proc[] = {
	{"ADDDEV", LOG_ADDDEV, log_adddev},
};

int linkagelog_proc_len = sizeof(linkagelog_proc)/sizeof(linkagelog_proc_t);

void do_linkagelog_proc(linkage_log_header_t *hdr)
{
	int i = 0;
	for(i = 0; i < linkagelog_proc_len; i++) {
		if(hdr->ds_command == linkagelog_proc[i].action && linkagelog_proc[i].fuc) {
			linkagelog_proc[i].fuc(hdr);
		} 
	}
}

int do_linkagelog(linkage_log_header_t *hdr)
{
	do_linkagelog_proc(hdr);
	return 0;
}

int do_linkageasynclog(char *data, int len)
{
	int remain = len;
	char *cur = data;
	int header_len = sizeof(linkage_log_header_t);
	linkage_log_header_t *hdr;

	while(remain > header_len) {
		hdr = (linkage_log_header_t*)cur;
		remain -= header_len;
		if(hdr->len > remain) {
			break;
		}
		remain -= hdr->len;
		do_linkagelog(hdr);
	}
	
	return 0;
}

void linkagelog_send(char *data, int len)
{
	linkagelog_append(data, len + sizeof(linkage_log_header_t));
}


void log_header_set(char *data, u_int16_t ds_command, u_int16_t len)
{
	linkage_log_header_t *hdr = (linkage_log_header_t*)data;
	hdr->ver = 1;
	hdr->ds_command = ds_command;
	hdr->len = len;
}

void linkagelog_init()
{
	linkageasynclog_init();
	linkageasynclog_start();
}

void linkagelog_stop()
{
	linkageasynclog_stop();
}


