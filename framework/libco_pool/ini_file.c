#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "ini_file.h"

/*
ini_trim_left
skip left space or tab character
if first character is '#', this line is comment
return new begin of the string
*/
static inline char *ini_trim_left(char *str)
{
	char *p;

	if(str == NULL)
		return NULL;
	p = str;
	while(*p){
		switch(*p){
			case ' ':
			case '\t':
				p++;
			break;
			case '#':
				return NULL;
			break;
			default:
				return p;
		}
	}
	return NULL;
}

/*
ini_trim_right
skip right space or tab or '\n' or '\r' character
return new end of the string
*/
static inline char *ini_trim_right(char *str)
{
	int len;

	if(str == NULL)
		return NULL;
	
	len = strlen(str)-1;
	while(len){
		if(str[len] == ' ' || str[len] == '\t'
			|| str[len] == '\r' || str[len] == '\n'){
			len--;
		}else{
			return &str[len];
		}
	}
	return NULL;
}

/*
ini_trim_right
skip right space or tab or '\n' or '\r' character
return new end of the string
*/
static inline char *ini_trim(char *str)
{
	
	return ini_trim_right(ini_trim_left(str));
}

int ini_read_string(char *filename, char *key, char *value, int value_len)
{
	FILE *fp;
	char buf[2048];
	int len;
	char *begin, *end;
	int ret = -1;

	fp = fopen(filename, "rb");
	if(fp == NULL)
		return -1;
	
	while(fgets(buf, sizeof(buf)-1, fp)){
		begin = ini_trim_left(buf);
		if(begin == NULL)
			continue;
		end = ini_trim_right(begin);
		if(end == NULL)
			continue;
		len = strlen(key);
		if(memcmp(begin, key, len)!=0)
			continue;
		begin = ini_trim_left(begin+len);
		if(begin == NULL)
			continue;
		begin = ini_trim_left(begin);
		if(begin == NULL)
			continue;
		if(*begin != '=')
			continue;
		begin = ini_trim_left(begin+1);
		if(begin == NULL)
			continue;
		len = end - begin +1;
		if(value_len >= (len+1)){
			memcpy(value, begin, len);
			value[len]=0;
			ret = 0;
			//printf("read config %s = %s\n", key, value);
		}
		goto out;		
		
	}
out:
	fclose(fp);
	return ret;
	
}

int ini_append_string(char *filename, char *key, char *value)
{
	FILE *fp;

	fp = fopen(filename, "ab");
	if(fp == NULL)
		return -1;
	fprintf(fp, "%s=%s\n", key, value);
	fclose(fp);
	return 0;
}

int ini_read_number(char *filename, char *key, int *value)
{
	int ret;
	char buf[128];
	ret =ini_read_string(filename, key, buf, sizeof(buf));
	if(ret == 0){
		*value = atoi(buf);
		return 0;
	}
	return -1;
}

int file_exist(char *filename)
{
	FILE *fp;
	fp = fopen(filename, "r");
	if(fp==NULL)
		return -1;
	fclose(fp);
	return 0;
}

int file_size(char *filename, u_int32_t *size)
{
	struct stat st;
			
	if(stat(filename, &st)==0){
		if(size)
			*size = st.st_size;
		return 0;
	}
	return -1;
}

/*
file_name
返回无路径文件名
*/
char *file_name(char *filename)
{	
	int len;
	if(filename){
		len = strlen(filename);
		if(len <= 0)
			return filename;
		while(--len){
			if(filename[len] == '/')
				return &filename[len + 1];

		}		
	}
	return filename;
}

int get_cpu_count()
{
	return (int)sysconf( _SC_NPROCESSORS_ONLN );
}

