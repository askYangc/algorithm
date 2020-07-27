#ifndef _MEMCACHED_API_H_
#define _MEMCACHED_API_H_
#include <libmemcached/memcached.h>

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
} MC_KEY_TYPE;


#pragma pack(push,1)






#pragma pack(pop)

//初始化memcached 使用memcached_free释放memcached资源 所有get方法用后使用free方法释放内存
memcached_st* memcached_init();

char *mc_get(memcached_st *mc, char *key, size_t key_length, size_t *value_length);
int mc_set(memcached_st *mc, char *key, size_t key_length, 
    char *value, size_t value_length,
    time_t expiration, u_int32_t flags);
int mc_cas(memcached_st *mc, char *key, size_t key_length, 
    char *value, size_t value_length,
    time_t expiration, u_int32_t flags, 
    u_int64_t cas);

#endif
