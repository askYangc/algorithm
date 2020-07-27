#ifndef _ASYNC_TASK_CFG_H_
#define _ASYNC_TASK_CFG_H_

#include <limits.h>


#define DS_DIR "/dsserver/"
#define SERVER_CONF "dsserver.conf"

#define ds_info printf
#define ds_error printf
#define ds_inout printf

//memcached
#define MEMCACHED_IP "memcached_ip"
#define DEF_MEMCACHED_IP "127.0.0.1"
#define MEMCACHED_PORT "memcached_port"
#define DEF_MEMCACHED_PORT 11211


// mysql
#define DB_IP "db_ip"
#define DEF_DB_IP "127.0.0.1"
#define DB_PORT "db_port"
#define DEF_DB_PORT 3306
#define DB_NAME "db_name"
#define DEF_DB_NAME ""
#define DB_USER "db_user"
#define DEF_DB_USER ""
#define DB_PASS "db_pass"
#define DEF_DB_PASS ""


#define DB_READ_TIMEOUT "db_read_timeout"
#define DEF_DB_READ_TIMEOUT 20

//account
#define DB_ACCOUNT_IP "db_account_ip"
#define DB_ACCOUNT_PORT "db_account_port"
#define DB_ACCOUNT_NAME "db_account_name"
#define DB_ACCOUNT_USER "db_account_user"
#define DB_ACCOUNT_PASS "db_account_pass"


//rds
#define DB_RDS_IP "db_rds_ip"
#define DB_RDS_PORT "db_rds_port"
#define DB_RDS_NAME "db_rds_name"
#define DB_RDS_USER "db_rds_user"
#define DB_RDS_PASS "db_rds_pass"

//local
#define DB_LOCAL_IP "db_local_ip"
#define DB_LOCAL_PORT "db_local_port"
#define DB_LOCAL_NAME "db_local_name"
#define DB_LOCAL_USER "db_local_user"
#define DB_LOCAL_PASS "db_local_pass"

//cloud
#define DB_CLOUD_IP "db_cloud_ip"
#define DB_CLOUD_PORT "db_cloud_port"
#define DB_CLOUD_NAME "db_cloud_name"
#define DB_CLOUD_USER "db_cloud_user"
#define DB_CLOUD_PASS "db_cloud_pass"

//global
#define GLOBAL_DB_IP "global_db_ip"
#define GLOBAL_DB_PORT "global_db_port"
#define GLOBAL_DB_NAME "global_db_name"
#define DEF_GLOBAL_DB_NAME ""
#define GLOBAL_DB_USER "global_db_user"
#define DEF_GLOBAL_DB_USER ""
#define GLOBAL_DB_PASS "global_db_pass"
#define DEF_GLOBAL_DB_PASS ""
#define SSL_ENABLE "ssl-enable"
#define SSL_CA 	"ssl-ca"
#define SSL_CLICERT "ssl-clicert"
#define SSL_CLIKEY 	"ssl-clikey"
#define SSL_CAPATH 	"ssl-capath"
#define DEF_SSL_CA ""
#define DEF_SSL_CLICERT ""
#define DEF_SSL_CLIKEY ""
#define DEF_SSL_CAPATH ""
#define DB_READ_TIMEOUT "db_read_timeout"
#define DEF_DB_READ_TIMEOUT 20
#define KA_WANIP "ka_wanip"
#define KA_PORT "ka_port"
#define DEF_KA_PORT 51206

//sync mysql
#define SYNC_DB_IP "sync_db_ip"
#define SYNC_DB_PORT "sync_db_port"
#define SYNC_DB_NAME "sync_db_name"
#define DEF_SYNC_DB_NAME ""
#define SYNC_DB_USER "sync_db_user"
#define DEF_SYNC_DB_USER ""
#define SYNC_DB_PASS "sync_db_pass"
#define DEF_SYNC_DB_PASS ""



#endif
