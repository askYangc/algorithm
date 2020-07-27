#ifndef __INI_FILE_HEADER__
#define __INI_FILE_HEADER__

#include <sys/types.h>

#define SERVER_NAME "server_name"

int ini_read_string(char *filename, char *key, char *value, int value_len);
int ini_read_number(char *filename, char *key, int *value);
int file_exist(char *filename);
int file_size(char *filename, u_int32_t *size);
char *file_name(char *filename);
int get_cpu_count();
int ini_append_string(char *filename, char *key, char *value);

#endif
