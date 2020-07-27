#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ds_types.h"
#include "ini_file.h"
#include "config.h"
#include "memcached_api.h"

memcached_st* memcached_init()
{
	memcached_st *memc;   
	memcached_return rc;   
	memcached_server_st *servers;   
	char host_ip[128];
	int port;
	char strip[32];
	char file_conf[PATH_MAX];
	
	sprintf(file_conf, "%s%s", DS_DIR,SERVER_CONF);

	if(ini_read_string(file_conf, MEMCACHED_IP, host_ip, sizeof(host_ip)) != 0){
		strcpy(host_ip, DEF_MEMCACHED_IP);
		ini_append_string(file_conf, MEMCACHED_IP, DEF_MEMCACHED_IP);
	}
	
	if(ini_read_number(file_conf, MEMCACHED_PORT, &port) != 0){
		port = DEF_MEMCACHED_PORT;
		sprintf(strip, "%d", port);
		ini_append_string(file_conf, MEMCACHED_PORT, strip);
	}

	memc = memcached_create(NULL);	
	servers = memcached_server_list_append(NULL, host_ip, port, &rc);   
	rc = memcached_server_push(memc, servers);  
	if (rc != MEMCACHED_SUCCESS) {
		ds_error("memcached init fail, rc %d",rc);
		return NULL;
	}
	memcached_server_free(servers);	  

	memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_BINARY_PROTOCOL, 1) ;  
//	memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_NOREPLY, 1) ;  
	memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_NO_BLOCK, 1) ;  
	
	return memc;
}

static inline int u32Key(MC_KEY_TYPE type, u_int32_t value, char *key)
{
	return snprintf(key, MEMCACHED_MAX_KEY, "%02X_%u", type, value);
}

static inline int u64Key(MC_KEY_TYPE type, u_int64_t value, char *key)
{
	return snprintf(key, MEMCACHED_MAX_KEY, "%02X_%llu", type, (u64)value);
}

static inline int u64KeyEx(MC_KEY_TYPE type, u_int64_t value, u_int8_t dev_type, char *key)
{
	return snprintf(key, MEMCACHED_MAX_KEY, "%02X_%llu_%u", type, (u64)value, dev_type);
}
static inline int u64KeyExEx(MC_KEY_TYPE type, u_int64_t value, u_int8_t dev_type, u_int32_t index,char *key)
{
	return snprintf(key, MEMCACHED_MAX_KEY, "%02X_%llu_%u_%u", type, (u64)value, dev_type, index);
}

static inline int u64Keyindex(MC_KEY_TYPE type, u_int64_t value, u_int16_t index, char *key)
{
	return snprintf(key, MEMCACHED_MAX_KEY, "%02X_%llu_%hu", type, (u64)value, index);
}

static inline int strKey(MC_KEY_TYPE type, const char *str, char *key)
{
	return snprintf(key, MEMCACHED_MAX_KEY, "%02X_%s", type, str);
}

static inline int u16Key(MC_KEY_TYPE type, u_int16_t value, char *key)
{
	return snprintf(key, MEMCACHED_MAX_KEY, "%02X_%u", type, value);
}

static inline int snArrayKey(MC_KEY_TYPE type, char *key)
{
	return snprintf(key, MEMCACHED_MAX_KEY, "%02X_%s", type, "snArray");
}

static inline int char16Key(MC_KEY_TYPE type, u_int8_t *p, char *key)
{
	sprintf(key, "%02X_", type);
	memcpy(key + 3, p, 16);
	return 19;
}



char *mc_get(memcached_st *mc, char *key, size_t key_length, size_t *value_length)
{   
	memcached_return rc;	
	if (mc == NULL)
		return NULL;
    
    return memcached_get(mc, key, key_length, value_length, 0, &rc);
}

int mc_set(memcached_st *mc, char *key, size_t key_length, 
    char *value, size_t value_length,
    time_t expiration, u_int32_t flags)
{
    return memcached_set(mc, key, key_length, value, value_length, expiration, flags);
}

int mc_cas(memcached_st *mc, char *key, size_t key_length, 
    char *value, size_t value_length,
    time_t expiration, u_int32_t flags, 
    u_int64_t cas)
{
    if(memcached_cas(mc, key, key_length, value, value_length, expiration, flags, cas) == MEMCACHED_DATA_EXISTS){
        return -1;
    }
    return 0;
}

/*
int mc_set_dev_as(memcached_st *mc, u_int64_t sn, ucp_mc_as_t *as, time_t expired)
{
	int key_length;
	char key[MEMCACHED_MAX_KEY];

	if (mc == NULL)
		return -1;
	
	key_length = u64Key(MC_KEY_TYPE_SN_AS, sn, key);
	return mc_set(mc, key, key_length, (char *)as, sizeof(ucp_mc_as_t), expired, 0);
}

ucp_mc_as_t *mc_get_dev_as(memcached_st *mc, u_int64_t sn)
{	
	int key_length;
	size_t value_length;
	memcached_return rc;	
	char key[MEMCACHED_MAX_KEY];

	if (mc == NULL)
		return NULL;
	
	key_length = u64Key(MC_KEY_TYPE_SN_AS, sn, key);

	return (ucp_mc_as_t *)memcached_get(mc, key, key_length, &value_length, 0, &rc);
}
*/
