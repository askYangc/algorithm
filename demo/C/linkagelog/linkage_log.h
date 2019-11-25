#ifndef __BASE_LINKAGE_LOG_H_
#define __BASE_LINKAGE_LOG_H_

#include "ds_types.h"

enum {
	LOG_NONE = 0,
	LOG_ADDDEV,

	LOG_MAX,
};



#pragma pack(push,1)

typedef struct {
	u_int8_t ver;
	u_int8_t pad[3];
	u_int16_t ds_command;
	u_int16_t len;
}linkage_log_header_t;

typedef struct {
	la_estab_user_info_t old;
	la_estab_user_info_t up;
	u_int32_t t;
}log_motify_userinfo_t;

typedef struct {
	u_int32_t home_id;
	u_int32_t doer;
	u_int32_t old_owner;
	u_int32_t new_owner;
	u_int32_t t;
}log_motify_homeowner_t;

typedef struct {
	u_int32_t home_id;
	u_int32_t user_id;
	u_int64_t sn;
	u_int32_t t;
}log_adddev_t;


#pragma pack(pop)

#define	get_logheader_payload(hdr, type) (type *)((linkage_log_header_t*)hdr+1)


typedef void (*linkagelog_func_t)();

typedef struct {
	char *action_name;
	u_int16_t action;
	linkagelog_func_t fuc;
}linkagelog_proc_t;

int do_linkageasynclog(char *data, int len);
void log_header_set(char *data, u_int16_t ds_command, u_int16_t len);
void linkagelog_send(char *data, int len);

void linkagelog_init();
void linkagelog_stop();

#endif
