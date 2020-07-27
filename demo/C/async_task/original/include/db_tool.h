#ifndef _MYSQL_CONFIG_H_
#define _MYSQL_CONFIG_H_

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#include <mysql/mysql.h>
#include <mysql/errmsg.h>
#include <mysql/mysqld_error.h>
typedef struct {
		 u_int8_t host_ip[128];
		 u_int8_t user[128];
		 u_int8_t password[128];
		 u_int32_t port;
		 u_int8_t db_name[128];
		 u_int32_t read_timeout;
	}db_config_t;

enum{
	DB_TYPE_LOCAL = 0,/**本地数据库*/
	DB_TYPE_RDS,    /**RDS数据库*/
	DB_TYPE_ACCOUNT,/**帐号数据库*/
	DB_TYPE_GLOBAL, /* 全球账号数据库 */	
	DB_TYPE_SYNC, /* 同步数据库 */	
	DB_TYPE_CLOUD, /* 云端数据库 */	
	DB_TYPE_MAX
};
	int db_open(MYSQL *mysql);
	MYSQL_RES* db_query(MYSQL *mysql, char *sql);
	int db_result_count(MYSQL *mysql);
	int execute_sql(MYSQL *mysql, char *sql);	
	int db_close(MYSQL *mysql);
	char *db_query_value(MYSQL *mysql, char *sql, char *result, int maxlen);
	
	//RDS数据库接口
	//int db_rds_open(MYSQL *mysql);
	//MYSQL_RES* db_rds_query(MYSQL *mysql, char *sql);
	//int execute_rds_sql(MYSQL *mysql, char *sql);

	int db_type_open(MYSQL *mysql,int db_type);
	MYSQL_RES*  db_type_query(MYSQL *mysql,int db_type, char *sql);
	char *db_type_query_value(MYSQL *mysql,int db_type, char *sql, char *result, int maxlen);	
	int db_type_execute_sql(MYSQL *mysql, int db_type, char *sql);

	MYSQL_RES*  db_type_bin_query(MYSQL *mysql,int db_type, char *sql, u_int32_t sql_len);
	char *db_type_bin_query_value(MYSQL *mysql,int db_type, char *sql, u_int32_t sql_len, char *result, int maxlen);	
	int db_type_bin_execute_sql(MYSQL *mysql, int db_type, char *sql, u_int32_t sql_len);

	MYSQL_RES* db_bin_query(MYSQL *mysql, char *sql, u_int32_t sql_len);
	int execute_bin_sql(MYSQL *mysql, char *sql, u_int32_t sql_len);	
	char *db_bin_query_value(MYSQL *mysql, char *sql, u_int32_t sql_len, char *result, int maxlen);
	int get_db_conn_config(char *filename, db_config_t *db_config, u_int8_t db_type);

#define SAFE_FREE_QUERY(query_res) \
do{\
	if (query_res) {\
		mysql_free_result(query_res);\
		query_res = NULL;\
	}\
}while(0)

#ifdef __cplusplus
}
#endif /* __cplusplus */
	
#endif
