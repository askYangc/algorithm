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
	u_int32_t home_id;
	u_int32_t user_id;
	u_int64_t sn;
}log_adddev_t;


#pragma pack(pop)

typedef void (*linkagelog_func_t)();

typedef struct {
	char *action_name;
	u_int16_t action;
	linkagelog_func_t fuc;
}linkagelog_proc_t;

int do_linkageasynclog(char *data, int len);

#endif
