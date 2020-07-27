#include <string.h>

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

int mc_set_history_index(memcached_st *mc, u_int64_t sn, u_int8_t type, u_int32_t max_index)
{
	int key_length;
	char key[MEMCACHED_MAX_KEY] = {0};

	key_length = u64KeyEx(MC_KEY_TYPE_DEV_MAX_INDEX, sn, type, key);
	return memcached_set(mc, key, key_length, (char *)&max_index, sizeof(u_int32_t), 0 ,0);		
}

u_int32_t mc_get_history_index(memcached_st *mc, u_int64_t sn, u_int8_t type)
{
	int key_length;
	char *result;
	char key[MEMCACHED_MAX_KEY] = {0};
	size_t value_length;
	memcached_return rc;
	u_int32_t index;
	
	key_length = u64KeyEx(MC_KEY_TYPE_DEV_MAX_INDEX, sn, type, key);
	result = memcached_get(mc, key, key_length, &value_length, 0, &rc);
	if(result){
		index = *(u_int32_t *)result;
		free(result);
		return index;
	}
	return 0;
}

int mc_set_history_context(memcached_st *mc, u_int64_t sn, u_int8_t type, u_int32_t index, dev_log_t history)
{
	int key_length;
	char key[MEMCACHED_MAX_KEY] = {0};

	key_length = u64KeyExEx(MC_KEY_TYPE_DEV_MAX_INDEX, sn, type, index, key);
	return memcached_set(mc, key, key_length, (char *)&history, sizeof(u_int32_t), 0 ,0);		
}

dev_log_t*  mc_get_history_context(memcached_st *mc, u_int64_t sn, u_int8_t type, u_int32_t index)
{
	int key_length;
	char key[MEMCACHED_MAX_KEY] = {0};
	size_t value_length;
	memcached_return rc;
	
	key_length = u64KeyExEx(MC_KEY_TYPE_DEV_MAX_INDEX, sn, type, index, key);
	return (dev_log_t*)memcached_get(mc, key, key_length, &value_length, 0, &rc);
}

int mc_set_server_wanip(memcached_st *mc, u_int16_t id, u_int32_t ip)
{
	int key_length;
	char key[MEMCACHED_MAX_KEY];

	if (mc == NULL)
		return -1;
	
	key_length = u16Key(MC_KEY_TYPE_SERVER_WANIP, id, key);
	return memcached_set(mc, key, key_length, (char *)&ip, sizeof(u_int32_t), 0 ,0);
}

u_int32_t mc_get_server_wanip(memcached_st *mc, u_int16_t id)
{
	int key_length;
	size_t value_length;
	memcached_return rc;	
	char key[MEMCACHED_MAX_KEY];
	char *result;
	u_int32_t value = 0;
	
	if (mc == NULL)
		return 0;
	
	key_length = u16Key(MC_KEY_TYPE_SERVER_WANIP, id, key);
	result = memcached_get(mc, key, key_length, &value_length, 0, &rc);
	if(result){
		value = *(u_int32_t *)result;
		free(result);
	}
	return value;
}


int mc_set_server_lanip(memcached_st *mc, u_int16_t id, u_int32_t ip)
{
	int key_length;
	char key[MEMCACHED_MAX_KEY];

	if (mc == NULL)
		return -1;
	
	key_length = u16Key(MC_KEY_TYPE_SERVER_LANIP, id, key);
	return memcached_set(mc, key, key_length, (char *)&ip, sizeof(u_int32_t), 0 ,0);
}

u_int32_t mc_get_server_lanip(memcached_st *mc, u_int16_t id)
{
	int key_length;
	size_t value_length;
	memcached_return rc;	
	char key[MEMCACHED_MAX_KEY];
	char *result;
	u_int32_t value = 0;
	
	if (mc == NULL)
		return 0;
	
	key_length = u16Key(MC_KEY_TYPE_SERVER_LANIP, id, key);
	result = memcached_get(mc, key, key_length, &value_length, 0, &rc);
	if(result){
		value = *(u_int32_t *)result;
		free(result);
	}
	return value;
}

int mc_set_tcponline(memcached_st *mc, u_int64_t sn, mc_tcpdev_online_t *mc_devinfo)
{
	int key_length;
	char key[MEMCACHED_MAX_KEY];

	if (mc == NULL)
		return -1;
	
	key_length = u64Key(MC_KEY_TYPE_TCPONLINE, sn, key);
	return memcached_set(mc, key, key_length, (char *)mc_devinfo, sizeof(mc_tcpdev_online_t), 0 ,0);
}

mc_tcpdev_online_t* mc_get_tcponline(memcached_st *mc, u_int64_t sn)
{	
	int key_length;
	size_t value_length;
	memcached_return rc;	
	char key[MEMCACHED_MAX_KEY];

	if (mc == NULL)
		return NULL;
	
	key_length = u64Key(MC_KEY_TYPE_TCPONLINE, sn, key);

	return (mc_tcpdev_online_t *)memcached_get(mc, key, key_length, &value_length, 0, &rc);
}

int mc_set_devinfo(memcached_st *mc, u_int64_t sn, uc_dev_info_t *mc_devinfo)
{
	int key_length;
	char key[MEMCACHED_MAX_KEY];

	if (mc == NULL)
		return -1;
	
	key_length = u64Key(MC_KEY_TYPE_DEVINFO, sn, key);
	return memcached_set(mc, key, key_length, (char *)mc_devinfo, sizeof(uc_dev_info_t), 0 ,0);
}

uc_dev_info_t* mc_get_devinfo(memcached_st *mc, u_int64_t sn)
{	
	int key_length;
	size_t value_length;
	memcached_return rc;	
	char key[MEMCACHED_MAX_KEY];

	if (mc == NULL)
		return NULL;
	
	key_length = u64Key(MC_KEY_TYPE_DEVINFO, sn, key);

	return (uc_dev_info_t *)memcached_get(mc, key, key_length, &value_length, 0, &rc);
}


void mc_delete_devinfo(memcached_st *mc, u_int64_t sn)
{	
	int key_length;
	char key[MEMCACHED_MAX_KEY];

	if (mc == NULL)
		return ;
	
	key_length = u64Key(MC_KEY_TYPE_DEVINFO, sn, key);

	memcached_delete(mc, key, key_length, 0);
    return;
}


int mc_set_link_support(memcached_st *mc, u_int64_t sn, ucp_dev_online_t *mc_link)
{
	int key_length;
	char key[MEMCACHED_MAX_KEY];

	if (mc == NULL)
		return -1;
	
	key_length = u64Key(MC_KEY_TYPE_LINK_SUPPORT, sn, key);
	return memcached_set(mc, key, key_length, (char *)mc_link, sizeof(ucp_dev_online_t), 0 ,0);
}

ucp_dev_misc_t *mc_get_dev_misc(memcached_st *mc, u_int64_t sn)
{	
	int key_length;
	size_t value_length;
	memcached_return rc;	
	char key[MEMCACHED_MAX_KEY];

	if (mc == NULL)
		return NULL;
	
	key_length = u64Key(MC_KEY_TYPE_DEV_MISC, sn, key);

	return (ucp_dev_misc_t *)memcached_get(mc, key, key_length, &value_length, 0, &rc);
}

int mc_set_dev_misc(memcached_st *mc, u_int64_t sn, ucp_dev_misc_t *misc)
{
	int key_length;
	char key[MEMCACHED_MAX_KEY];

	if (mc == NULL)
		return -1;
	
	key_length = u64Key(MC_KEY_TYPE_DEV_MISC, sn, key);
	return memcached_set(mc, key, key_length, (char *)misc, sizeof(ucp_dev_misc_t), 0 ,0);
}

int mc_set_doorbell(memcached_st *mc, u_int64_t sn, ucp_mc_doorbell_t *misc)
{
	int key_length;
	char key[MEMCACHED_MAX_KEY];

	if (mc == NULL)
		return -1;
	
	key_length = u64Key(MC_KEY_TYPE_DOORBELL, sn, key);
	return memcached_set(mc, key, key_length, (char *)misc, sizeof(ucp_mc_doorbell_t), 0 ,0);
}

ucp_mc_doorbell_t *mc_get_doorbell(memcached_st *mc, u_int64_t sn)
{	
	int key_length;
	size_t value_length;
	memcached_return rc;	
	char key[MEMCACHED_MAX_KEY];

	if (mc == NULL)
		return NULL;
	
	key_length = u64Key(MC_KEY_TYPE_DOORBELL, sn, key);

	return (ucp_mc_doorbell_t *)memcached_get(mc, key, key_length, &value_length, 0, &rc);
}


ucp_dev_online_t *mc_get_link_support(memcached_st *mc, u_int64_t sn)
{	
	int key_length;
	size_t value_length;
	memcached_return rc;	
	char key[MEMCACHED_MAX_KEY];

	if (mc == NULL)
		return NULL;
	
	key_length = u64Key(MC_KEY_TYPE_LINK_SUPPORT, sn, key);

	return (ucp_dev_online_t *)memcached_get(mc, key, key_length, &value_length, 0, &rc);
}

int mc_set_home_info(memcached_st *mc, u_int32_t home_id, mc_home_info_t *homeinfo)
{
	int key_length;
	char key[MEMCACHED_MAX_KEY];

	if (mc == NULL)
		return -1;
	
	key_length = u32Key(MC_KEY_TYPE_HOME_INFO, home_id, key);
	return memcached_set(mc, key, key_length, (char *)homeinfo, sizeof(mc_home_info_t), 0 ,0);
}
	
mc_home_info_t* mc_get_home_info(memcached_st *mc, u_int32_t home_id)
{
	int key_length;
	size_t value_length;
	memcached_return rc;	
	char key[MEMCACHED_MAX_KEY];

	if (mc == NULL)
		return NULL;
	
	key_length = u32Key(MC_KEY_TYPE_HOME_INFO, home_id, key);

	return (mc_home_info_t *)memcached_get(mc, key, key_length, &value_length, 0, &rc);
}

int mc_set_lansrv_info(memcached_st *mc, u_int64_t sn, u_int32_t lanip)
{
	int key_length;
	char key[MEMCACHED_MAX_KEY];

	if (mc == NULL)
		return -1;
	
	key_length = u64Key(MC_KEY_TYPE_LANSRV_INFO, sn, key);
	return memcached_set(mc, key, key_length, (char *)&lanip, sizeof(u_int32_t), 0 ,0);
}
	
u_int32_t mc_get_lansrv_info(memcached_st *mc, u_int64_t sn)
{
	int key_length;
	size_t value_length;
	memcached_return rc;	
	char key[MEMCACHED_MAX_KEY];
	u_int32_t *ip;
	u_int32_t lanip = 0;

	if (mc == NULL)
		return 0;
	
	key_length = u64Key(MC_KEY_TYPE_LANSRV_INFO, sn, key);

	ip = (u_int32_t *)memcached_get(mc, key, key_length, &value_length, 0, &rc);
	if(ip) {
		lanip = *ip;
		free(ip);
	}

	return lanip;
}


int mc_set_vender(memcached_st *mc, u_int16_t vender_id, const char *vender_str)
{
	int key_length;
	memcached_return rc;
	char key[MEMCACHED_MAX_KEY];

	if (mc == NULL)
		return -1;
	
	key_length = u16Key(MC_KEY_TYPE_VENDERID, vender_id, key);
	rc = memcached_set(mc, key, key_length, vender_str, strlen(vender_str) + 1, 0 ,0);
	if(rc != MEMCACHED_SUCCESS)
		return rc;
	
	key_length = strKey(MC_KEY_TYPE_VENDERSTR, vender_str, key);
	return memcached_set(mc, key, key_length, (char *)&vender_id, sizeof(u_int16_t), 0 ,0);
}

const char* mc_get_vender_str(memcached_st *mc, u_int16_t vender_id)
{
	int key_length;
	size_t value_length;
	memcached_return rc;	
	char key[MEMCACHED_MAX_KEY];
	char *result;

	if (mc == NULL)
		return NULL;
	
	key_length = u16Key(MC_KEY_TYPE_VENDERID, vender_id, key);
	result = memcached_get(mc, key, key_length, &value_length, 0, &rc);
	if(result){
		result[value_length - 1] = '\0';
		return result;
	}
	return NULL;
}

u_int8_t mc_get_vender_id(memcached_st *mc, const char *vender_str)
{
	int key_length;
	size_t value_length;
	memcached_return rc;	
	char key[MEMCACHED_MAX_KEY];
	char *result;
	u_int32_t value;

	if (mc == NULL)
		return -1;
	
	key_length = strKey(MC_KEY_TYPE_VENDERSTR, vender_str, key);
	result = memcached_get(mc, key, key_length, &value_length, 0, &rc);
	if(result){
		value = *(u_int16_t *)result;
		free(result);
		return value;
	}
	return 0;
}

int mc_add_sn_arry(memcached_st *mc, u_int64_t sn)
{
	int key_length;
	memcached_return rc;	
	char key[MEMCACHED_MAX_KEY];

	if (mc == NULL)
		return -1;
	
	key_length = snArrayKey(MC_KEY_TYPE_UPDATESN, key);
	rc = memcached_append(mc, key, key_length, (char *)&sn, sizeof(u_int64_t), 0, 0);
	if(rc == MEMCACHED_NOTSTORED)
		rc = memcached_set(mc, key, key_length, (char *)&sn, sizeof(u_int64_t), 0, 0);

	return rc;
}

u_int64_t* mc_get_clear_sn_array(memcached_st *mc, u_int32_t *count)
{
	int key_length;
	size_t value_length;
	memcached_return rc;	
	char key[MEMCACHED_MAX_KEY];
	char *result;

	if (mc == NULL)
		return NULL;
	
	key_length = snArrayKey(MC_KEY_TYPE_UPDATESN, key);

conflict_retry:	
	result = memcached_get(mc, key, key_length, &value_length, 0, &rc);
	if(result){
		if(memcached_cas(mc, key, key_length, NULL, 0, 0, 0, mc->result.item_cas) == MEMCACHED_DATA_EXISTS){
			free(result);
			goto conflict_retry; //清除前发现更新，重新get
		}
			
		*count = value_length / sizeof(u_int64_t);
		return (u_int64_t *)result;
	}

	return NULL;
}

//车载悟空数据统计接口
static inline int carIndexKey(MC_KEY_TYPE type, char *key)
{
	return snprintf(key, MEMCACHED_MAX_KEY, "%02X_%s", type, "carIndex");
}

//自增carindex，返回key
static inline int mc_get_car_index_key(memcached_st *mc ,char *indexKey)
{
	int key_length;
	memcached_return rc;	
	char key[MEMCACHED_MAX_KEY];
	u_int64_t value = 0;

	if (mc == NULL)
		return -1;
	
	key_length = carIndexKey(MC_KEY_TYPE_CAR_INDEX, key);
	if(memcached_increment(mc, key, key_length, 1, &value) != MEMCACHED_SUCCESS){
		rc = memcached_set(mc, key, key_length, "0", 1, 0, 0);
		if(rc != MEMCACHED_SUCCESS)
			return -1;
	}

	return u64Key(MC_KEY_TYPE_CAR_KEY, value, indexKey);
}

int mc_cache_packet(memcached_st *mc, const char *ptr, int len)
{
	int key_length;
	char key[MEMCACHED_MAX_KEY];

	if(mc == NULL)
		return -1;
	
	key_length = mc_get_car_index_key(mc, key);
	if(key_length < 0)
		return -1;
	
	if(memcached_set(mc, key, key_length, (char *)ptr, len, 8*3600, 0) == MEMCACHED_SUCCESS)
		return 0;
	else
		return -1;
}

//子设备信息缓存
int mc_set_slave_dev(memcached_st *mc, mc_slave_dev_t *slave)
{
	int key_length;
	char key[MEMCACHED_MAX_KEY];

	if (mc == NULL)
		return -1;
	
	key_length = u64Key(MC_KEY_TYPE_SLAVEINFO, slave->slavesn, key);
	return memcached_set(mc, key, key_length, (char *)slave, sizeof(mc_slave_dev_t), 0 ,0);
}

mc_slave_dev_t * mc_get_slave_dev(memcached_st *mc, u_int64_t sn)
{
	int key_length;
	size_t value_length;
	memcached_return rc;	
	char key[MEMCACHED_MAX_KEY];

	if (mc == NULL)
		return NULL;
	
	key_length = u64Key(MC_KEY_TYPE_SLAVEINFO, sn, key);

	return (mc_slave_dev_t *)memcached_get(mc, key, key_length, &value_length, 0, &rc);
}


//子设备列表
int mc_set_slave_list(memcached_st *mc, mc_slave_list_t *list)
{
	int key_length;
	char key[MEMCACHED_MAX_KEY];

	if (mc == NULL)
		return -1;
	
	key_length = u64Key(MC_KEY_TYPE_SLAVELIST, list->gwsn, key);
	return memcached_set(mc, key, key_length, (char *)list, sizeof(mc_slave_list_t) + sizeof(u_int64_t)*list->slave_count, 0 ,0);
}

mc_slave_list_t * mc_get_slave_list(memcached_st *mc, u_int64_t sn)
{
	int key_length;
	size_t value_length;
	memcached_return rc;	
	char key[MEMCACHED_MAX_KEY];

	if (mc == NULL)
		return NULL;
	
	key_length = u64Key(MC_KEY_TYPE_SLAVELIST, sn, key);

	return (mc_slave_list_t *)memcached_get(mc, key, key_length, &value_length, 0, &rc);
}

int mc_set_slave_info(memcached_st *mc, u_int64_t slave_sn, mc_slave_info_t * slave_info)
{
	int key_length;
	char key[MEMCACHED_MAX_KEY];

	if (mc == NULL)
		return -1;
	
	key_length = u64Key(MC_KEY_TYPE_SLAVE_INFO, slave_sn, key);
	return memcached_set(mc, key, key_length, (char *)slave_info, sizeof(mc_slave_info_t), 0 ,0);
}

mc_slave_info_t *mc_get_slave_info(memcached_st *mc, u_int64_t slave_sn)
{
	int key_length;
	size_t value_length;
	memcached_return rc;	
	char key[MEMCACHED_MAX_KEY];

	if (mc == NULL)
		return NULL;
	
	key_length = u64Key(MC_KEY_TYPE_SLAVE_INFO, slave_sn, key);

	return (mc_slave_info_t *)memcached_get(mc, key, key_length, &value_length, 0, &rc);
}

int mc_set_cache_st(memcached_st *mc, u_int64_t sn, u_int16_t index, u_int16_t len, mc_cache_st_t *st)
{
	int key_length;
	char key[MEMCACHED_MAX_KEY];

	if (mc == NULL)
		return -1;
	
	key_length = u64Keyindex(MC_KEY_TYPE_CACHE_ST, sn, index, key);
	return memcached_set(mc, key, key_length, (char *)st, len, 0 ,0);
}

mc_cache_st_t *mc_get_cache_st(memcached_st *mc, u_int64_t sn, u_int16_t index, u_int16_t *len)
{
	int key_length;
	memcached_return rc;	
	char key[MEMCACHED_MAX_KEY];

	if (mc == NULL)
		return NULL;
	
	key_length = u64Keyindex(MC_KEY_TYPE_CACHE_ST, sn, index, key);

	return (mc_cache_st_t *)memcached_get(mc, key, key_length, (size_t *)len, 0, &rc);
}

int mc_set_rfdev_info(memcached_st *mc, u_int64_t gwsn, u_int16_t devid, u_int16_t len, u_int8_t *rfdev)
{
	int key_length;
	char key[MEMCACHED_MAX_KEY];

	if (mc == NULL)
		return -1;
	
	key_length = u64Keyindex(MC_KEY_TYPE_RFDEV_LIST, gwsn, devid, key);
	return memcached_set(mc, key, key_length, (char *)rfdev, len, 0 ,0);
}

u_int8_t *mc_get_rfdev_info(memcached_st *mc, u_int64_t gwsn, u_int16_t devid, u_int16_t *len)
{
	int key_length;
	memcached_return rc;	
	char key[MEMCACHED_MAX_KEY];

	if (mc == NULL)
		return NULL;
	
	key_length = u64Keyindex(MC_KEY_TYPE_RFDEV_LIST, gwsn, devid, key);

	return (u_int8_t *)memcached_get(mc, key, key_length, (size_t *)len, 0, &rc);
}

//缓存在线设备总数
int mc_set_dev_total(memcached_st *mc, char *srv_name, mc_dev_statcs_t * statcs)
{
	int key_length;
	char key[MEMCACHED_MAX_KEY];

	if (mc == NULL)
		return -1;
	
	key_length = snprintf(key, MEMCACHED_MAX_KEY, "%02X_%s", MC_KEY_TYPE_DEV_TOTAL, srv_name);
	return memcached_set(mc, key, key_length, (char *)statcs, sizeof(mc_dev_statcs_t), 0 ,0);
}

int mc_set_dev_snapshoot(memcached_st *mc, char *srv_name, mc_onlinedev_t * devlist, u_int32_t len)
{
	int key_length;
	char key[MEMCACHED_MAX_KEY];

	if (mc == NULL)
		return -1;
	
	key_length = snprintf(key, MEMCACHED_MAX_KEY, "%02X_%s", MC_KEY_DEV_SNAPSHOT, srv_name);
	return memcached_set(mc, key, key_length, (char *)devlist, len, 0 ,0);
}

mc_linksrv_userid_t *mc_get_linksrv_userid(memcached_st *mc)
{
	int key_length;
	size_t value_length;
	memcached_return rc;	
	char key[MEMCACHED_MAX_KEY];
    mc_linksrv_userid_t *t;

	if (mc == NULL)
		return NULL;
	
    key_length = sprintf(key, "%02X_linkserver_userid", MC_KEY_TYPE_SRV);

	t = (mc_linksrv_userid_t *)memcached_get(mc, key, key_length, &value_length, 0, &rc);
    if(t) {
        t->cas = mc->result.item_cas;
    }
    
    return t;
}

int mc_set_linksrv_userid(memcached_st *mc, mc_linksrv_userid_t *srv)
{
	int key_length;
	char key[MEMCACHED_MAX_KEY];

	if (mc == NULL)
		return -1;

    key_length = sprintf(key, "%02X_linkserver_userid", MC_KEY_TYPE_SRV);
    return memcached_set(mc, key, key_length, (char *)srv, sizeof(srv), 0 ,0);
}

int mc_set_linksrv_userid_cas(memcached_st *mc, mc_linksrv_userid_t *srv)
{
	int key_length;
	char key[MEMCACHED_MAX_KEY];

	if (mc == NULL)
		return -1;

    key_length = sprintf(key, "%02X_linkserver_userid", MC_KEY_TYPE_SRV);
    if(memcached_cas(mc, key, key_length, (char*)srv, sizeof(srv), 0, 0, srv->cas) == MEMCACHED_DATA_EXISTS){
        return -1;
    }
    return 0;
}

mc_linksrv_homeid_t *mc_get_linksrv_homeid(memcached_st *mc)
{
	int key_length;
	size_t value_length;
	memcached_return rc;	
	char key[MEMCACHED_MAX_KEY];
    mc_linksrv_homeid_t *t;

	if (mc == NULL)
		return NULL;
	
    key_length = sprintf(key, "%02X_linkserver_homeid", MC_KEY_TYPE_SRV);

	t = (mc_linksrv_homeid_t *)memcached_get(mc, key, key_length, &value_length, 0, &rc);
    if(t) {
        t->cas = mc->result.item_cas;
    }
    
    return t;
}

int mc_set_linksrv_homeid(memcached_st *mc, mc_linksrv_homeid_t *srv)
{
	int key_length;
	char key[MEMCACHED_MAX_KEY];

	if (mc == NULL)
		return -1;

    key_length = sprintf(key, "%02X_linkserver_homeid", MC_KEY_TYPE_SRV);
    return memcached_set(mc, key, key_length, (char *)srv, sizeof(srv), 0 ,0);
}

int mc_set_linksrv_homeid_cas(memcached_st *mc, mc_linksrv_homeid_t *srv)
{
	int key_length;
	char key[MEMCACHED_MAX_KEY];

	if (mc == NULL)
		return -1;

    key_length = sprintf(key, "%02X_linkserver_homeid", MC_KEY_TYPE_SRV);
    if(memcached_cas(mc, key, key_length, (char*)srv, sizeof(srv), 0, 0, srv->cas) == MEMCACHED_DATA_EXISTS){
        return -1;
    }
    return 0;
}

int mc_set_xslock_auth(memcached_st *mc, u_int64_t lock_sn, mc_xslock_auth_t *auth)
{
	int key_length;
	char key[MEMCACHED_MAX_KEY];

	if (mc == NULL)
		return -1;
	
	key_length = u64Keyindex(MC_KEY_TYPE_XSLOCK, lock_sn, MC_KEY_TYPE_XSLOCK_AUTH, key);
	return memcached_set(mc, key, key_length, (char *)auth, sizeof(mc_xslock_auth_t), 0 ,0);
}

mc_xslock_auth_t *mc_get_xslock_auth(memcached_st *mc, u_int64_t lock_sn)
{
	int key_length;
	memcached_return rc;	
	char key[MEMCACHED_MAX_KEY];
	size_t value_length;

	if (mc == NULL)
		return NULL;
	
	key_length = u64Keyindex(MC_KEY_TYPE_XSLOCK, lock_sn, MC_KEY_TYPE_XSLOCK_AUTH, key);
	return (mc_xslock_auth_t *)memcached_get(mc, key, key_length, &value_length, 0, &rc);
}

int mc_set_auto_bind(memcached_st *mc, u_int64_t slave_sn, mc_auto_bind_t *autobind)
{
	int key_length;
	char key[MEMCACHED_MAX_KEY];

	if (mc == NULL)
		return -1;
		
	key_length = u64Key(MC_KEY_TYPE_AUTOBIND, slave_sn, key);
	return memcached_set(mc, key, key_length, (char *)autobind, sizeof(mc_auto_bind_t), 0 ,0);
}

mc_auto_bind_t* mc_get_auto_bind(memcached_st *mc, u_int64_t slave_sn)
{
	int key_length;
	size_t value_length;
	memcached_return rc;	
	char key[MEMCACHED_MAX_KEY];

	if (mc == NULL)
		return NULL;
	
	key_length = u64Key(MC_KEY_TYPE_AUTOBIND, slave_sn, key);

	return (mc_auto_bind_t *)memcached_get(mc, key, key_length, &value_length, 0, &rc);
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

