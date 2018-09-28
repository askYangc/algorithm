#ifndef __INI_FILE_HEADER__
#define __INI_FILE_HEADER__

#include <sys/types.h>
#define SERVER_CONF "dsserver.conf"
#define TRANS_SRV_IP "transsrv_ip"
#define DB_IP "db_ip"
#define DB_RDS_IP "db_rds_ip"
#define DB_ACCOUNT_IP "db_account_ip"
#define DEF_DB_IP "127.0.0.1"
#define DB_PORT "db_port"
#define DB_RDS_PORT "db_rds_port"
#define DB_ACCOUNT_PORT "db_account_port"
#define DEF_DB_PORT 3306
#define DB_NAME "db_name"
#define DEF_DB_NAME "ijdbs"
#define DB_USER "db_user"
#define DEF_DB_USER "ijmaster"
#define DB_PASS "db_pass"
#define DEF_DB_PASS "ijjazhang"
#define SERVER_NAME "server_name"
#define WEB_ROOT_DIR "web_root_dir"
#define WEB_ROOT_DEFAULT "/var/www/"
#define FTP_HOME "/home/ftp"

int ini_read_string(char *filename, char *key, char *value, int value_len);
int ini_read_number(char *filename, char *key, int *value);
int file_exist(char *filename);
int file_size(char *filename, u_int32_t *size);
char *file_name(char *filename);
int get_cpu_count();
int ini_append_string(char *filename, char *key, char *value);

#endif
