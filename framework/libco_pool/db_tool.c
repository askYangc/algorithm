#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <errno.h>
#include "db_tool.h"
#include "ini_file.h"

char file_conf[512] = "/dsserver/"SERVER_CONF;

int get_db_conn_config(char *filename, db_config_t *db_config, u_int8_t db_type)
{
	int value;
	char strip[32];
	if(DB_TYPE_RDS == db_type){//rds数据库
		
		if(ini_read_string(filename, DB_RDS_IP, (char*)db_config->host_ip, sizeof(db_config->host_ip)) != 0){
			strcpy((char*)db_config->host_ip, DEF_DB_IP);
			ini_append_string(filename, DB_RDS_IP, DEF_DB_IP);
		}
		if(ini_read_number(filename, DB_RDS_PORT, &value) != 0){
			value = DEF_DB_PORT;
			sprintf(strip, "%d", value);
			ini_append_string(filename, DB_RDS_PORT, strip);
		}
		db_config->port = value;
	}else if(DB_TYPE_ACCOUNT == db_type){//帐号数据库
		if(ini_read_string(filename, DB_ACCOUNT_IP, (char*)db_config->host_ip, sizeof(db_config->host_ip)) != 0){
			strcpy((char*)db_config->host_ip, DEF_DB_IP);
			ini_append_string(filename, DB_ACCOUNT_IP, DEF_DB_IP);
		}
		if(ini_read_number(filename, DB_ACCOUNT_PORT, &value) != 0){
			value = DEF_DB_PORT;
			sprintf(strip, "%d", value);
			ini_append_string(filename, DB_ACCOUNT_PORT, strip);
		}
		db_config->port = value;
	}else if(DB_TYPE_GLOBAL == db_type) {//全球账号数据库
		if(ini_read_string(filename, GLOBAL_DB_IP, (char*)db_config->host_ip, sizeof(db_config->host_ip)) != 0){
			strcpy((char*)db_config->host_ip, DEF_DB_IP);
			ini_append_string(filename, GLOBAL_DB_IP, DEF_DB_IP);
		}
		if(ini_read_number(filename, GLOBAL_DB_PORT, &value) != 0){
			value = DEF_DB_PORT;
			sprintf(strip, "%d", value);
			ini_append_string(filename, GLOBAL_DB_PORT, strip);
		}
		db_config->port = value;		

		if(ini_read_string(filename, GLOBAL_DB_NAME, (char*)db_config->db_name, sizeof(db_config->db_name)) != 0){
			strcpy((char*)db_config->db_name, DEF_GLOBAL_DB_NAME);
			ini_append_string(filename, GLOBAL_DB_NAME, DEF_GLOBAL_DB_NAME);
		}

		if(ini_read_string(filename, GLOBAL_DB_USER, (char*)db_config->user, sizeof(db_config->user)) != 0){
			strcpy((char*)db_config->user, DEF_GLOBAL_DB_USER);
			ini_append_string(filename, GLOBAL_DB_USER, DEF_GLOBAL_DB_USER);
		}

		if(ini_read_string(filename, GLOBAL_DB_PASS, (char*)db_config->password, sizeof(db_config->password)) != 0){
			strcpy((char*)db_config->password, DEF_GLOBAL_DB_PASS);
			ini_append_string(filename, GLOBAL_DB_PASS, DEF_GLOBAL_DB_PASS);
		}	
		return 0;		
	}else{//本地数据库
		if(ini_read_string(filename, DB_IP, (char*)db_config->host_ip, sizeof(db_config->host_ip)) != 0){
			strcpy((char*)db_config->host_ip, DEF_DB_IP);
			ini_append_string(filename, DB_IP, DEF_DB_IP);
		}		
		
		if(ini_read_number(filename, DB_PORT, &value) != 0){
			value = DEF_DB_PORT;
			sprintf(strip, "%d", value);
			ini_append_string(filename, DB_PORT, strip);
		}
		db_config->port = value;
	}

	if(ini_read_string(filename, DB_NAME, (char*)db_config->db_name, sizeof(db_config->db_name)) != 0){
		strcpy((char*)db_config->db_name, DEF_DB_NAME);
		ini_append_string(filename, DB_NAME, DEF_DB_NAME);
	}

	if(ini_read_string(filename, DB_USER, (char*)db_config->user, sizeof(db_config->user)) != 0){
		strcpy((char*)db_config->user, DEF_DB_USER);
		ini_append_string(filename, DB_USER, DEF_DB_USER);
	}

	if(ini_read_string(filename, DB_PASS, (char*)db_config->password, sizeof(db_config->password)) != 0){
		strcpy((char*)db_config->password, DEF_DB_PASS);
		ini_append_string(filename, DB_PASS, DEF_DB_PASS);
	}
	
	return 0;
}

/**
*连接数据库
*/
int db_open(MYSQL *mysql){

	db_config_t db_config;
	
	if(mysql_init(mysql) == NULL){
		fprintf(stderr, "inital mysql handle errorn\n");
		return -1;
	}
		
	if(get_db_conn_config(file_conf, &db_config,DB_TYPE_LOCAL)==0){
		//printf("host_ip=%s,user=%s,password=%s,db_name=%s,port=%d\n",db_config->host_ip,db_config->user,db_config->password,db_config->db_name,db_config->port);
		if(mysql_real_connect(mysql,(char*)db_config.host_ip,(char*)db_config.user,
			(char*)db_config.password,(char*)db_config.db_name,(int)db_config.port,NULL,0) == NULL){
			fprintf(stderr, "Failed to conect to database ip:%s, port:%d, Error:%s\n",
				db_config.host_ip, db_config.port, mysql_error(mysql));
			return -1;
		}

		db_type_execute_sql(mysql, DB_TYPE_LOCAL, "set names utf8;");
		return 0;
	}
	return -1;
}


/**
*执行查询sql
*/
MYSQL_RES*  db_query(MYSQL *mysql, char *sql)
{
	
	MYSQL_RES *m_res;
	int ret;

	if(mysql == NULL)
	{
		printf("mysql == NULL\n");
		return NULL;
	}
	
	if(sql == NULL || strlen(sql) == 0)
	{
		printf("sql == NULL  or  sql length == 0\n");
		return NULL;	
	}
	
	ret = mysql_query(mysql,sql);
	if(ret != 0)
	{
		printf("mysql_query return %d, Error %d (%s): '%s'\n",ret,
			mysql_errno(mysql), mysql_error(mysql), sql);
		fprintf(stderr, "mysql_query return %d, Error %d (%s): '%s'\n",ret,
			mysql_errno(mysql), mysql_error(mysql), sql);

		if(mysql_errno(mysql) == CR_SERVER_GONE_ERROR) /* 2006 server has gone away */
		{
			db_close(mysql);
			if(db_open(mysql) != 0)
			{
				return NULL;
			}
			if(mysql_query(mysql, sql)!=0)
			{
				printf("Again connect mysql database unsuccessful!\n");
				return NULL;
			}
			printf("Again connect mysql database successful!\n");
		}
		else
			return NULL;
	}

	m_res = mysql_store_result(mysql);

	if(m_res==NULL){
		if(mysql_field_count(mysql) != 0) {
			printf("mysql_store_result return NULL, Error %d (%s): '%s'\n",
				mysql_errno(mysql), mysql_error(mysql), sql);
			fprintf(stderr, "mysql_store_result return NULL, Error %d (%s): '%s'\n",
				mysql_errno(mysql), mysql_error(mysql), sql);
			return NULL;
		}
	}

	return m_res;
}

int db_result_count(MYSQL *mysql)
{
	return mysql_field_count(mysql);
}

/**
*执行添加或删除sql
*/
int execute_sql(MYSQL *mysql, char *sql)
{
	if(mysql == NULL)
	{
		printf("mysql == NULL\n");
		return -1;
	}
	
	if(sql == NULL || strlen(sql) == 0)
	{
		printf("sql == NULL  or  sql length == 0\n");
		return -1;	
	}
		
	if(mysql_query(mysql, sql)!=0)
	{
		printf("mysql_query Error %d (%s): '%s'\n",
			mysql_errno(mysql), mysql_error(mysql), sql);
		fprintf(stderr, "mysql_query Error %d (%s): '%s'\n",
			mysql_errno(mysql), mysql_error(mysql), sql);

		if(mysql_errno(mysql) == CR_SERVER_GONE_ERROR) /* 2006 server has gone away */
		{
			db_close(mysql);
			if(db_open(mysql) != 0)
			{
				return -1;
			}
			if(mysql_query(mysql, sql)!=0)
			{
				printf("Again connect mysql database unsuccessful!\n");
				return -1;
			}
			printf("Again connect mysql database successful!\n");
			return 0;
		}
		else
			return -1;
	}
	return 0;
}




char *db_query_value(MYSQL *mysql, char *sql, char *result, int maxlen)
{
	MYSQL_RES* m_res = NULL;
	MYSQL_ROW m_row;
	char * ret = NULL;

	if (result) {
		memset(result, 0, maxlen);
	}

	if (mysql == NULL || sql == NULL)
		goto end;
	
	m_res = db_query(mysql, sql);
	if(m_res == NULL){
		printf("do query failed: %s\n", sql);
		goto end;
	}

	while((m_row = mysql_fetch_row(m_res))){
		if (m_row && m_row[0]) {
			strncpy(result, m_row[0], maxlen);
			ret = result;
		}
		break;
	}

end:
	if (m_res) mysql_free_result(m_res);
	return ret;
}

/**
*关闭连接
*/
int db_close(MYSQL *mysql)
{
	if(mysql != NULL){
		mysql_close(mysql);
	}
	return 0;
}

/**===================连接--指定类型的数据库=================================*/

/**
*连接--指定类型的数据库
*/

int db_ssl_set(char *filename, MYSQL *mysql, int db_type)
{
	if(db_type == DB_TYPE_GLOBAL) {
		char client_key[256] = {0};		
		char client_cert[256] = {0};
		char cacert[256]  = {0};
		char cartpath[256] = {0};
		
		if(ini_read_string(filename, SSL_CLIKEY, (char*)client_key, sizeof(client_key)) != 0){
			return 0;
			strcpy((char*)client_key, DEF_SSL_CLIKEY);
			ini_append_string(filename, SSL_CLIKEY, DEF_SSL_CLIKEY);
		}
		if(ini_read_string(filename, SSL_CLICERT, (char*)client_cert, sizeof(client_cert)) != 0){
			return 0;
			strcpy((char*)client_cert, DEF_SSL_CLICERT);
			ini_append_string(filename, SSL_CLICERT, DEF_SSL_CLICERT);
		}
		if(ini_read_string(filename, SSL_CA, (char*)cacert, sizeof(cacert)) != 0){
			return 0;
			strcpy((char*)cacert, DEF_SSL_CA);
			ini_append_string(filename, SSL_CA, DEF_SSL_CA);
		}
		if(ini_read_string(filename, SSL_CAPATH, (char*)cartpath, sizeof(cartpath)) != 0){
			return 0;
			strcpy((char*)cartpath, DEF_SSL_CAPATH);
			ini_append_string(filename, SSL_CAPATH, DEF_SSL_CAPATH);
		}
		
		mysql_ssl_set(mysql, client_key, client_cert, cacert, cartpath, NULL);
#if 0		
		mysql_ssl_set(mysql, "/usr/local/mysql/mysql-test/std_data/client-key.pem", "/usr/local/mysql/mysql-test/std_data/client-cert.pem", 
			"/usr/local/mysql/mysql-test/std_data/cacert.pem", "/usr/local/mysql/mysql-test/std_data/", NULL);
#endif
	}
	return 0;
}

int db_type_open(MYSQL *mysql,int db_type)
{

	db_config_t db_config;
	
	if(mysql_init(mysql) == NULL){
		fprintf(stderr, "inital mysql handle errorn\n");
		return -1;
	}
		
	if(get_db_conn_config(file_conf, &db_config,db_type)==0){
		db_ssl_set(file_conf, mysql, db_type);
		if(mysql_real_connect(mysql,(char*)db_config.host_ip,(char*)db_config.user,
			(char*)db_config.password,(char*)db_config.db_name,(int)db_config.port,NULL,0) == NULL){
			fprintf(stderr, "Failed to conect to database ip:%s, port:%d, Error:%s\n",
				db_config.host_ip, db_config.port, mysql_error(mysql));
			return -1;
		}
		db_type_execute_sql(mysql, db_type,"set names utf8;");		
		return 0;
	}
	return -1;
}


/**
*执行查询sql
*/
MYSQL_RES* db_type_query(MYSQL *mysql,int db_type,char *sql)
{
	MYSQL_RES *m_res;
	int ret;

	if(mysql == NULL)
	{
		printf("mysql == NULL\n");
		return NULL;
	}
	
	if(sql == NULL || strlen(sql) == 0)
	{
		printf("sql == NULL  or  sql length == 0\n");
		return NULL;	
	}
	
	ret = mysql_query(mysql,sql);
	if(ret != 0)
	{
		printf("mysql_query return %d, Error %d (%s): '%s'\n",ret,
			mysql_errno(mysql), mysql_error(mysql), sql);
		fprintf(stderr, "mysql_query return %d, Error %d (%s): '%s'\n",ret,
			mysql_errno(mysql), mysql_error(mysql), sql);

		if(mysql_errno(mysql) == CR_SERVER_GONE_ERROR) /* 2006 server has gone away */
		{
			db_close(mysql);
			if(db_type_open(mysql,db_type) != 0)
			{
				return NULL;
			}
			if(mysql_query(mysql, sql)!=0)
			{
				printf("Again connect mysql database unsuccessful!\n");
				return NULL;
			}
			printf("Again connect mysql database successful!\n");
		}
		else
			return NULL;
	}

	m_res = mysql_store_result(mysql);

	if(m_res==NULL){
		if(mysql_field_count(mysql) != 0) {
			printf("mysql_store_result return NULL, Error %d (%s): '%s'\n",
				mysql_errno(mysql), mysql_error(mysql), sql);
			fprintf(stderr, "mysql_store_result return NULL, Error %d (%s): '%s'\n",
				mysql_errno(mysql), mysql_error(mysql), sql);
			return NULL;
		}
	}

	return m_res;
}


char *db_type_query_value(MYSQL *mysql, int db_type, char *sql, char *result, int maxlen)
{
	MYSQL_RES* m_res = NULL;
	MYSQL_ROW m_row;
	char * ret = NULL;

	if (result) {
		memset(result, 0, maxlen);
	}
	if (mysql == NULL || sql == NULL)
		goto end;
	
	m_res = db_type_query(mysql,db_type, sql);
	if(m_res == NULL){
		printf("do query failed: %s\n", sql);
		goto end;
	}

	while((m_row = mysql_fetch_row(m_res))){
		if (m_row && m_row[0]) {
			strncpy(result, m_row[0], maxlen);
			ret = result;
		}
		break;
	}

end:
	if (m_res) mysql_free_result(m_res);
	return ret;
}




/**
*执行添加或删除sql
*/
int db_type_execute_sql(MYSQL *mysql, int db_type, char *sql)
{
	if(mysql == NULL)
	{
		printf("mysql == NULL\n");
		return -1;
	}
	
	if(sql == NULL || strlen(sql) == 0)
	{
		printf("sql == NULL  or  sql length == 0\n");
		return -1;	
	}
		
	if(mysql_query(mysql, sql)!=0)
	{
		printf("mysql_query Error %d (%s): '%s'\n",
			mysql_errno(mysql), mysql_error(mysql), sql);
		fprintf(stderr, "mysql_query Error %d (%s): '%s'\n",
			mysql_errno(mysql), mysql_error(mysql), sql);

		if(mysql_errno(mysql) == CR_SERVER_GONE_ERROR) /* 2006 server has gone away */
		{
			db_close(mysql);
			if(db_type_open(mysql,db_type) != 0)
			{
				return -1;
			}
			if(mysql_query(mysql, sql)!=0)
			{
				printf("Again connect mysql database unsuccessful!\n");
				if(mysql_errno(mysql) == ER_DUP_ENTRY)
					return 1;
				return -1;
			}
			printf("Again connect mysql database successful!\n");
			return 0;
		}else if(mysql_errno(mysql) == ER_DUP_ENTRY){
			return 1;
		}	
		else{
			return -1;
		}
	}
	return 0;
}

#if 0
/**======================RDS数据库操作=======================================*/

/**
*连接RDS数据库
*/
int db_rds_open(MYSQL *mysql){

	db_config_t db_config;
	
	if(mysql_init(mysql) == NULL){
		fprintf(stderr, "inital mysql handle errorn\n");
		return -1;
	}
		
	if(get_db_conn_config(file_conf, &db_config,DB_TYPE_RDS)==0){//区别处
		if(mysql_real_connect(mysql,(char*)db_config.host_ip,(char*)db_config.user,
			(char*)db_config.password,(char*)db_config.db_name,(int)db_config.port,NULL,0) == NULL){
			fprintf(stderr, "Failed to conect to database ip:%s, port:%d, Error:%s\n",
				db_config.host_ip, db_config.port, mysql_error(mysql));
			return -1;
		}

		db_query(mysql, "set names utf8;");
		return 0;
	}
	return -1;
}


/**
*执行RDS 查询sql
*/
MYSQL_RES* db_rds_query(MYSQL *mysql, char *sql)
{
	
	MYSQL_RES *m_res;
	int ret;

	if(mysql == NULL)
	{
		printf("mysql == NULL\n");
		return NULL;
	}
	
	if(sql == NULL || strlen(sql) == 0)
	{
		printf("sql == NULL  or  sql length == 0\n");
		return NULL;	
	}
	
	ret = mysql_query(mysql,sql);
	if(ret != 0)
	{
		printf("mysql_query return %d, Error %d (%s): '%s'\n",ret,
			mysql_errno(mysql), mysql_error(mysql), sql);
		fprintf(stderr, "mysql_query return %d, Error %d (%s): '%s'\n",ret,
			mysql_errno(mysql), mysql_error(mysql), sql);

		if(mysql_errno(mysql) == CR_SERVER_GONE_ERROR) /* 2006 server has gone away */
		{
			db_close(mysql);
			if(db_rds_open(mysql) != 0)//区别处
			{
				return NULL;
			}
			if(mysql_query(mysql, sql)!=0)
			{
				printf("Again connect mysql database unsuccessful!\n");
				return NULL;
			}
			printf("Again connect mysql database successful!\n");
		}
		else
			return NULL;
	}

	m_res = mysql_store_result(mysql);

	if(m_res==NULL){
		if(mysql_field_count(mysql) != 0) {
			printf("mysql_store_result return NULL, Error %d (%s): '%s'\n",
				mysql_errno(mysql), mysql_error(mysql), sql);
			fprintf(stderr, "mysql_store_result return NULL, Error %d (%s): '%s'\n",
				mysql_errno(mysql), mysql_error(mysql), sql);
			return NULL;
		}
	}

	return m_res;
}

/**
*RDS数据库----执行添加或删除sql
*/
int execute_rds_sql(MYSQL *mysql, char *sql)
{
	if(mysql == NULL)
	{
		printf("mysql == NULL\n");
		return -1;
	}
	
	if(sql == NULL || sql[0] == 0)
	{
		printf("sql == NULL  or  sql length == 0\n");
		return -1;	
	}
		
	if(mysql_query(mysql, sql)!=0)
	{
		printf("mysql_query Error %d (%s): '%s'\n",
			mysql_errno(mysql), mysql_error(mysql), sql);
		fprintf(stderr, "mysql_query Error %d (%s): '%s'\n",
			mysql_errno(mysql), mysql_error(mysql), sql);

		if(mysql_errno(mysql) == CR_SERVER_GONE_ERROR) /* 2006 server has gone away */
		{
			db_close(mysql);
			if(db_rds_open(mysql) != 0)//区别处
			{
				return -1;
			}
			if(mysql_query(mysql, sql)!=0)
			{
				printf("Again connect mysql database unsuccessful!\n");
				return -1;
			}
			printf("Again connect mysql database successful!\n");
			return 0;
		}
		else
			return -1;
	}
	return 0;
}

#endif 
