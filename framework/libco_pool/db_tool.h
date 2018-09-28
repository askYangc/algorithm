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
		 u_int8_t user[16];
		 u_int8_t password[16];
		 u_int32_t port;
		 u_int8_t db_name[16];
	}db_config_t;

enum{
	DB_TYPE_LOCAL = 0,/**本地数据库*/
	DB_TYPE_RDS,    /**RDS数据库*/
	DB_TYPE_ACCOUNT,/**帐号数据库*/
	DB_TYPE_GLOBAL, /* 全球账号数据库 */	
	DB_TYPE_MAX
};

#define GLOBAL_DB_NAME "global_db_name"
#define DEF_GLOBAL_DB_NAME "ijdbs"
#define GLOBAL_DB_USER "global_db_user"
#define DEF_GLOBAL_DB_USER "gb_ijmaster"
#define GLOBAL_DB_PASS "global_db_pass"
#define DEF_GLOBAL_DB_PASS "gb_ijjazhang"
#define GLOBAL_DB_IP "global_db_ip"
#define GLOBAL_DB_PORT "global_db_port"
#define SSL_CA 	"ssl-ca"
#define SSL_CLICERT "ssl-clicert"
#define SSL_CLIKEY 	"ssl-clikey"
#define SSL_CAPATH 	"ssl-capath"
#define DEF_SSL_CA "/usr/local/mysql/mysql-test/std_data/cacert.pem"
#define DEF_SSL_CLICERT "/usr/local/mysql/mysql-test/std_data/client-cert.pem"
#define DEF_SSL_CLIKEY "/usr/local/mysql/mysql-test/std_data/client-key.pem"
#define DEF_SSL_CAPATH "/usr/local/mysql/mysql-test/std_data/"


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
