#ifndef _MEMCACHED_API_H_
#define _MEMCACHED_API_H_
#include <libmemcached/memcached.h>

#include "ds_server.h"
#include "udp_ctrl.h"
#include "../coapserver/xslock_common.h"



/*memcached
key						value
===================================
uuidType+sn				uuid
passType+sn				passwd
serveridType+sn			server_id
devinfoType+sn			mc_devinfo_t
nickType+sn				sn

venderidType+id			vender_string
venderstrType+str		vender_id
updateStatType			sn_array
*/
typedef enum{
	MC_KEY_TYPE_UUID = 0, /*UUID*/
	MC_KEY_TYPE_PASS = 1, /*PASSWD*/
	MC_KEY_TYPE_SERVERID = 2, /*ServerID 合并在线状态，serverid为0表示离线，有serverid表示在线*/
	MC_KEY_TYPE_DEVINFO = 3, /*mc_devinfo_t*/	
	MC_KEY_TYPE_NICK = 4, /*SN*/	
	MC_KEY_TYPE_VENDERID = 5, /*VENDER_ID*/	
	MC_KEY_TYPE_VENDERSTR = 6, /*VENDER_STRING*/
	MC_KEY_TYPE_UPDATESN = 7, /*需要写回数据库的SN_ARRAY*/
	MC_KEY_TYPE_CAR_INDEX = 8, /*车载悟空cache index*/
	MC_KEY_TYPE_CAR_KEY = 9, /*车载悟空cache key*/
	MC_KEY_TYPE_SERVER_LANIP = 10,/*通过serverid获取serverip*/
	MC_KEY_TYPE_TCPONLINE = 11,/*tcp设备记录上下线*/
	MC_KEY_TYPE_LINK_SUPPORT = 12, /* 记录设备是否支持联动 */
	MC_KEY_TYPE_SERVER_WANIP = 13,/*通过serverid获取serverwanip*/
	MC_KEY_TYPE_HOME_INFO = 14,/*通过homeid获取home信息*/
	MC_KEY_TYPE_LANSRV_INFO = 15,/*通过sn获取lansrv信息*/
	MC_KEY_TYPE_SLAVE_INFO = 16,/*通过slave_sn获取子设备信息，数据来自网关周期上报*/
	MC_KEY_TYPE_DEV_MAX_INDEX = 17,/*通过sn和设备提供的类型，获取历史记录中的最大值*/
	MC_KEY_TYPE_DEV_HISTORY = 18,/*通过sn、设备类型和索引，返回历史记录*/
	MC_KEY_TYPE_CACHE_ST = 19,/*缓存(wifi设备、rf子设备)设备状态*/
	MC_KEY_TYPE_DEV_MISC = 20,/*通过sn存放相关设备的杂项数据*/
	MC_KEY_TYPE_RFDEV_LIST = 21,/*缓存macbee子设备列表信息*/
	MC_KEY_TYPE_DEV_TOTAL = 22,/*缓存设备总数*/
	MC_KEY_DEV_SNAPSHOT = 23,/*凌晨在线设备快照*/
	MC_KEY_TYPE_SRV = 24, /* server相关信息缓存 */
	MC_KEY_TYPE_XSLOCK = 25, /* xslock相关信息缓存 */
	MC_KEY_TYPE_AUTOBIND = 26, /*网关自动绑定子设备信息*/
	MC_KEY_TYPE_DOORBELL = 27,/* doorbell */
	MC_KEY_TYPE_SMS = 28,/* 联动设备告警相关 */
	MC_KEY_TYPE_SN_AS = 29,/* 设备的AS缓存 */
	MC_KEY_TYPE_SLAVEINFO = 30,/* 在设备状态信息，来自网关上报的摘要和子设备列表查询  mc_slave_dev_t*/
	MC_KEY_TYPE_SLAVELIST = 31,/* 网关下的子设备，包含新发现的子设备 mc_slave_list_t */
} MC_KEY_TYPE;

typedef enum {
    MC_KEY_TYPE_XSLOCK_AUTH = 1, /* xslock auth */
	MC_KEY_TYPE_XSLOCK_DEV , /* xslock dev */
	MC_KEY_TYPE_XSLOCK_TOKEN, /* xslock token */
}MC_KEY_SUBTYPE_XSLOCK;

#pragma pack(push,1)

typedef struct {
	// 主版本号
	u_int8_t major;
	// 次版本号
	u_int8_t minor;
	// 修订版本号
	u_int8_t revise;
	u_int8_t reserved;
} version_t;

typedef struct {
	version_t stm_ver;
	version_t stm_hard_ver;
	version_t hard_ver;	
	u_int8_t pad[2];
} udp_t;

typedef struct {
	u_int8_t unbind_login;
	u_int8_t pad[1];	
} tcp_t;

typedef struct {
	u_int64_t mastersn;
	u_int32_t ip;						//area可以通过ip查询，保存的意义不大
	u_int32_t online_time;
	u_int32_t offline_time;	
	version_t soft_ver;
	u_int16_t vender_id; 				//可以通过mc_get_vender_str接口取得vender_str
	u_int8_t devserver_id;				//id为0表示离线，否则为对应的devserver
	u_int8_t dev_type;					//设备类型
	u_int8_t ext_type;					//扩展类型
	u_int8_t proto_type; 				//链接类型0 udp 1 tcp
	union 								//udp和tcp差异化状态数据，根据proto_type区分对应结构体。
	{
		udp_t udp;
		tcp_t tcp;
	} u;
} mc_devinfo_t;

/* uc device structure */
typedef struct{
	u_int64_t sn;
	u_int32_t online;
	u_int32_t devserver_id;
	u_int32_t updatetime;
}mc_tcpdev_online_t;

/* uc device structure */
typedef struct{
	u_int64_t homeserver_sn;
	u_int32_t homeserver_devserverid;
	u_int32_t homeserver_lanip;
}mc_home_info_t;
typedef struct{
	u_int8_t rsv;
	u_int8_t log[8];
}dev_log_t;
typedef struct{
	//子设备相关
	u_int8_t online;		// 1 在线，0离线
	u_int32_t time;			//最后一次上线/下线时间
	ucp_dev_info_t dev_info;//子设备类型、版本信息
	//主设备相关
	u_int64_t master_sn;	//主设备sn
	u_int32_t udpserver_id;//主设备所在udpserver_id
}mc_slave_info_t;

typedef struct{
	u_int8_t digest;	//块的摘要/ttcache的cache_count
	u_int8_t pad[3];
}mc_cached_st_head_t;

typedef struct{
	u_int8_t digest;	//块的摘要/ttcache的cache_count
	u_int8_t pad[3];
	u_int8_t data[256];//ttcache/块数据,变长
}mc_cache_st_t;

typedef struct{
	u_int32_t  timestamp;
	u_int32_t  total;
}mc_dev_statcs_t;

typedef struct{
	u_int32_t  timestamp;
	u_int32_t  count;
	u_int64_t  sn[0];
}mc_onlinedev_t;

typedef struct{
	u_int32_t  user_id;
    u_int64_t cas;
}mc_linksrv_userid_t;

typedef struct{
	u_int32_t  home_id;
    u_int64_t cas;
}mc_linksrv_homeid_t;


typedef struct{
	u_int8_t token[16];
    u_int8_t cipher[16];
}mc_xslock_auth_t;

/* gw slave dev auto bind info  */
typedef struct{
	u_int64_t gw_sn;
	u_int32_t time;
}mc_auto_bind_t;


typedef struct{
	u_int16_t asid;				
    u_int8_t pad[2];
    u_int64_t gw_sn;
}ucp_mc_as_t;

//子设备状态信息
typedef struct {
	//status
	u_int32_t uptime;
	u_int16_t dev_id;
	u_int8_t dev_count;
	u_int8_t cache_count;
	u_int8_t flag;
	//sn type
	u_int64_t slavesn;
	u_int64_t gwsn;		//子设备未绑定时，gwsn = 0
	u_int8_t subtype;
    u_int8_t extype;
	//version
	u_int8_t rf_major;
	u_int8_t rf_minor;
	u_int8_t rf_revsion;
	u_int8_t app_major;
	u_int8_t app_minor;
	u_int8_t app_revsion;
	//模板版本
	u_int8_t tmplt_ver; 
}mc_slave_dev_t;

//网关子设备列表信息(含新发现未绑定的子设备)
typedef struct {
	u_int32_t uptime;
	u_int64_t gwsn;
	u_int16_t slave_count;
	u_int8_t pad[2];
	u_int64_t slavelist[0];	//未绑定的子设备sn可能会出现在多个网关的子设备列表中
}mc_slave_list_t;


#pragma pack(pop)

//初始化memcached 使用memcached_free释放memcached资源 所有get方法用后使用free方法释放内存
memcached_st* memcached_init();

//存取serverid对应server_wanip 
int mc_set_server_wanip(memcached_st *mc, u_int16_t id, u_int32_t ip);
u_int32_t mc_get_server_wanip(memcached_st *mc, u_int16_t id);

//存取serverid对应server_lanip 
int mc_set_server_lanip(memcached_st *mc, u_int16_t id, u_int32_t ip);
u_int32_t mc_get_server_lanip(memcached_st *mc, u_int16_t id);


//添加需要写回数据库的sn进入sn_array
int mc_add_sn_arry(memcached_st *mc, u_int64_t sn);

//读取sn_array 读后清
u_int64_t* mc_get_clear_sn_array(memcached_st *mc, u_int32_t *count);

//序列号+原始报文缓存到memcached
int mc_cache_packet(memcached_st *mc, const char *ptr, int len);

//缓存tcp设备上下线信息，提供给dispatcher
mc_tcpdev_online_t* mc_get_tcponline(memcached_st *mc, u_int64_t sn);
int mc_set_tcponline(memcached_st *mc, u_int64_t sn, mc_tcpdev_online_t *mc_devinfo);

//缓存设备是否支持联动
int mc_set_link_support(memcached_st *mc, u_int64_t sn, ucp_dev_online_t *mc_link);
ucp_dev_online_t *mc_get_link_support(memcached_st *mc, u_int64_t sn);

//缓存设备的杂项信息
int mc_set_dev_misc(memcached_st *mc, u_int64_t sn, ucp_dev_misc_t *misc);
ucp_dev_misc_t *mc_get_dev_misc(memcached_st *mc, u_int64_t sn);


//缓存家庭信息
int mc_set_home_info(memcached_st *mc, u_int32_t home_id, mc_home_info_t *homeinfo);
mc_home_info_t* mc_get_home_info(memcached_st *mc, u_int32_t home_id);

//缓存lansrv信息
int mc_set_lansrv_info(memcached_st *mc, u_int64_t sn, u_int32_t lanip);
u_int32_t mc_get_lansrv_info(memcached_st *mc, u_int64_t sn);
int mc_set_history_index(memcached_st *mc, u_int64_t sn, u_int8_t type, u_int32_t max_index);
u_int32_t mc_get_history_index(memcached_st *mc, u_int64_t sn, u_int8_t type);
int mc_set_history_context(memcached_st *mc, u_int64_t sn, u_int8_t type, u_int32_t index, dev_log_t history);
dev_log_t*	mc_get_history_context(memcached_st *mc, u_int64_t sn, u_int8_t type, u_int32_t index);

/********************************** 网关/wifi设备、子设备、子设备列表等信息缓存 **********************************/
//	MC_KEY_TYPE_SLAVEINFO = 30,/* 在设备状态信息，来自网关上报的摘要和子设备列表查询 */
//	MC_KEY_TYPE_SLAVELIST = 31,/* 网关下的子设备，包含新发现的子设备 */

//网关/wifi设备(含udp/tcp设备)信息缓存, json格式
int mc_set_devinfo(memcached_st *mc, u_int64_t sn, uc_dev_info_t *mc_devinfo);
uc_dev_info_t* mc_get_devinfo(memcached_st *mc, u_int64_t sn);
void mc_delete_devinfo(memcached_st *mc, u_int64_t sn);

//子设备信息缓存
int mc_set_slave_dev(memcached_st *mc, mc_slave_dev_t *slave);
mc_slave_dev_t * mc_get_slave_dev(memcached_st *mc, u_int64_t sn);

//子设备列表
int mc_set_slave_list(memcached_st *mc, mc_slave_list_t *list);
mc_slave_list_t * mc_get_slave_list(memcached_st *mc, u_int64_t sn);



//缓存子设备信息
int mc_set_slave_info(memcached_st *mc, u_int64_t slave_sn, mc_slave_info_t * slave_info);
mc_slave_info_t *mc_get_slave_info(memcached_st *mc, u_int64_t slave_sn);

//缓存设备列表信息
int mc_set_rfdev_info(memcached_st *mc, u_int64_t gwsn, u_int16_t devid, u_int16_t len, u_int8_t *rfdev);
u_int8_t  *mc_get_rfdev_info(memcached_st *mc, u_int64_t gwsn, u_int16_t devid, u_int16_t *len);



//缓存块数据(tt_cache)
int mc_set_cache_st(memcached_st *mc, u_int64_t sn, u_int16_t index, u_int16_t len, mc_cache_st_t *st);
mc_cache_st_t *mc_get_cache_st(memcached_st *mc, u_int64_t sn, u_int16_t index, u_int16_t *len);

//缓存在线设备总数
int mc_set_dev_total(memcached_st *mc, char *srv_name, mc_dev_statcs_t * statcs);
//每日凌晨在线设备快照
int mc_set_dev_snapshoot(memcached_st *mc, char *srv_name, mc_onlinedev_t * devlist, u_int32_t len);

//linkserver的缓存数据
mc_linksrv_userid_t *mc_get_linksrv_userid(memcached_st *mc);
int mc_set_linksrv_userid(memcached_st *mc, mc_linksrv_userid_t *srv);
int mc_set_linksrv_userid_cas(memcached_st *mc, mc_linksrv_userid_t *srv);

mc_linksrv_homeid_t *mc_get_linksrv_homeid(memcached_st *mc);
int mc_set_linksrv_homeid(memcached_st *mc, mc_linksrv_homeid_t *srv);
int mc_set_linksrv_homeid_cas(memcached_st *mc, mc_linksrv_homeid_t *srv);

//xslock auth
int mc_set_xslock_auth(memcached_st *mc, u_int64_t lock_sn, mc_xslock_auth_t *auth);
mc_xslock_auth_t *mc_get_xslock_auth(memcached_st *mc, u_int64_t lock_sn);


//缓存网关自动绑定子设备信息
int mc_set_auto_bind(memcached_st *mc, u_int64_t slave_sn, mc_auto_bind_t *autobind);
mc_auto_bind_t* mc_get_auto_bind(memcached_st *mc, u_int64_t slave_sn);

int mc_set_doorbell(memcached_st *mc, u_int64_t sn, ucp_mc_doorbell_t *misc);
ucp_mc_doorbell_t *mc_get_doorbell(memcached_st *mc, u_int64_t sn);

int mc_set_dev_as(memcached_st *mc, u_int64_t sn, ucp_mc_as_t *as, time_t expired);
ucp_mc_as_t *mc_get_dev_as(memcached_st *mc, u_int64_t sn);

char *mc_get(memcached_st *mc, char *key, size_t key_length, size_t *value_length);
int mc_set(memcached_st *mc, char *key, size_t key_length, 
    char *value, size_t value_length,
    time_t expiration, u_int32_t flags);
int mc_cas(memcached_st *mc, char *key, size_t key_length, 
    char *value, size_t value_length,
    time_t expiration, u_int32_t flags, 
    u_int64_t cas);

#endif
