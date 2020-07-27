#ifndef __DS_TOOLS_HEADER__
#define __DS_TOOLS_HEADER__
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define MD5_FORMAT "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x"
#define MD5_ELMENT(buf) \
	((unsigned char*)(buf))[0],((unsigned char*)(buf))[1],((unsigned char*)(buf))[2],((unsigned char*)(buf))[3],\
	((unsigned char*)(buf))[4],((unsigned char*)(buf))[5],((unsigned char*)(buf))[6],((unsigned char*)(buf))[7],\
	((unsigned char*)(buf))[8],((unsigned char*)(buf))[9],((unsigned char*)(buf))[10],((unsigned char*)(buf))[11],\
	((unsigned char*)(buf))[12],((unsigned char*)(buf))[13],((unsigned char*)(buf))[14],((unsigned char*)(buf))[15]

#define UCPREKEY_FORMAT "%02x%02x%02x%02x%02x%02x%02x%02x"
#define UCPREKEY_ELMENT(buf) \
	((unsigned char*)(buf))[0],((unsigned char*)(buf))[1],((unsigned char*)(buf))[2],((unsigned char*)(buf))[3],\
	((unsigned char*)(buf))[4],((unsigned char*)(buf))[5],((unsigned char*)(buf))[6],((unsigned char*)(buf))[7]


#define MAC_FORMAT "%02x%02x%02x%02x%02x%02x"
#define MAC_ELEMENT(buf) \
	((unsigned char*)(buf))[0],((unsigned char*)(buf))[1],((unsigned char*)(buf))[2],\
	((unsigned char*)(buf))[3],((unsigned char*)(buf))[4],((unsigned char*)(buf))[5]

#define NIPQUADS(ip) (((ip) >> 24)&0xFF), (((ip) >> 16)&0xFF), (((ip) >> 8)&0xFF), ((ip)&0xFF)

static inline int asicc_to_hex(unsigned char *str, unsigned char *out)
{
	int len,i, j;
	unsigned int val;
	unsigned char buf[3];
	len = strlen((char*)str);
	if((len%2)!=0)
		return 0;
	
	for(i=0,j=0; i<len;i++,j++)
	{
		buf[0]=str[i++];
		buf[1]=str[i];
		buf[2]=0;
		sscanf((char*)buf, "%02x", &val);
		out[j]=val;	
	}
	return j;
}

static inline void hex_to_asicc(unsigned char *str, int len, unsigned char *out)
{
	int index = 0;
	int i;
	for(i = 0; i < len; i++){
		index = index + sprintf((char*)&out[index], "%02x", str[i]);
	}
}

/*
pwd_to_str
把16字节的passwd md5转换成可以显示字符串
必须保证out内存大于等于33字节
返回值:out字符串长度，等于32
*/
static inline int pwd_to_str(unsigned char *pwd, unsigned char *out)
{
	return sprintf((char*)out, MD5_FORMAT, MD5_ELMENT(pwd));
}

/*
pwd_to_str
把8字节的premary_key转换成可以显示字符串
必须保证out内存大于等于17字节
返回值:out字符串长度，等于16
*/
static inline int ucprekey_to_str(unsigned char *pwd, unsigned char *out)
{
	return sprintf((char*)out, UCPREKEY_FORMAT, UCPREKEY_ELMENT(pwd));
}

/*
str_to_pwd
把字符串形式的passwd md5转换成16字节md5
必须保证out内存大于等于16字节
返回值:成功转换到out的字节数，如果str里面没有无效字符，返回16
*/
static inline int str_to_pwd(unsigned char *str, unsigned char *out)
{
	return asicc_to_hex(str, out);
}

static inline u_int32_t str_to_ip(char *str)
{

	u_int32_t ip = 0;
	struct in_addr addr;
	
	if(inet_aton(str, &addr) != 0){
		ip = htonl(addr.s_addr);
	}
	return ip;
}

static inline void _trim_cmd_tail(char *str)
{
	int len;

	if(str == NULL)
		return;

	len = strlen(str)-1;
	while(len){
		if(str[len] == ' ' || str[len] == '\t'
			|| str[len] == '\r' || str[len] == '\n'){
			str[len] = 0;
			len--;
		}else{
			return ;
		}
	}
	return;
}

static inline char *_trim_cmd_head(char *str)
{
	char *p;

	if(str == NULL)
		return NULL;
	p = str;
	while(*p){
		switch(*p){
			case ' ':
			case '\t':
			case '\n':
			case '\r':
				p++;
			break;
			
			default:
				return p;
		}
	}
	return NULL;
}

static inline u_int32_t len_2_mask(u_int32_t len)
{
	u_int32_t b = 0x80000000;
	u_int32_t m = 0;
	while(len){
		m |= b;
		b = b>>1;
		len--;
	}
	return m;
}

static inline u_int32_t mask_2_len(u_int32_t mask)
{
	u_int32_t i=0,x=0x80000000;
	while(mask&x)
	{
		i++;
		x=x>>1;
	}
	return i;
}

static inline void hex_print_line(u_int8_t *s, int len)
{
	int i;

	for (i = 0; i < 16; i++) {
		if (i % 8 == 0)
			printf(" ");
		
		if (i >= len)
			printf("  ");
		else
			printf("%02x", s[i]);
	}
}

static inline void ascii_print_line(u_int8_t *s, int len)
{
	int i;
	
	for(i = 0; i < 16; i++){
		if (i % 8 == 0)
			printf(" ");

		if (i >= len) {
			printf(" ");
			continue;
		}
		
		if (isprint(s[i]) && s[i] < 128) {
			printf("%c", s[i]);
			continue;
		} else if (isspace(s[i])) {
			if (s[i] == ' ') {
				printf(" ");
				continue;
			} else if (s[i] == '\t') {
				printf("\\t");
				continue;
			} else if (s[i] == '\r') {
				printf("\\r");
				continue;
			} else if(s[i] == '\n') {
				printf("\\n");
				continue;
			}
		}
		
		printf(".");
	}
}

static inline void friendly_hex(u_int8_t *s, int len)
{
	int want, nread = 0;

	while (nread < len) {
		want = (len - nread) >= 16 ? 16 : len - nread;
		hex_print_line(s + nread, want);
		printf("  ");
		ascii_print_line(s + nread, want);
		printf("\n");
		nread += want;
	}
	printf("\n");
}

#define BUFF_SIZE 1024*100
static inline int read_result(char *buff, char *tmpfile)
{	
	int fd = 0;
	struct stat st;
	size_t size;

	if((fd =open(tmpfile, O_RDONLY)) <0){
		printf("open %s failed\n", tmpfile);
		return -1;
	}
	if(fstat(fd, &st)<0){
		printf("fstat %s failed\n", tmpfile);
		close(fd);
		return -1;
	}

	size = st.st_size;
	
	//printf("%s size = %zu\n", tmpfile, size);
	
	if(size <= 0){
		printf("%s is empty file\n", tmpfile);
		close(fd);
		return -1;
	}
	if(size >= BUFF_SIZE){
		printf("%s size is to big: %zu\n", tmpfile, size);
		close(fd);
		return -1;
	}

	if(read(fd, buff, size) != size){
		printf("read %s error\n", tmpfile);
		close(fd);
		return -1;
	}

	close(fd);
	return 0;
}

/*** 返回 buf找到的与expr完全匹配的第一项的第一个字符的索引 
   * buf-源字符串 
   * offset-源字符串偏移-从偏移量往后搜索 
   * expr-匹配字符串 
   * bufMaxLen-源字符串长度 
   */  
static inline int findFirst(const char* buf, int offset, const char* expr, int bufMaxLen)  
{  
    if(buf == NULL || offset<0 || offset>bufMaxLen-1)  
    {  
        return -1;  
    }  
    const char* p1;  
    const char* p2;  
    p1 = buf+offset;  
    p2 = expr;  
    int curoffset = offset;  
    int sameTimes = 0;  
    int exprOffSet = -1;  
    while(curoffset<bufMaxLen)  
    {  
        if(*p1==*p2 && sameTimes==0)  
        {  
            exprOffSet = curoffset;  
            p1++;  
            p2++;  
            curoffset++;  
            sameTimes++;  
        }  
        else if(*p1==*p2 && sameTimes<strlen(expr))  
        {  
            p1++;  
            p2++;  
            curoffset++;  
            sameTimes++;  
            if(sameTimes==strlen(expr))  
            {  
                return exprOffSet;  
            }  
        }  
        else  
        {  
            if(exprOffSet>=0)  
            {  
                p1 = buf+exprOffSet+1;  
                p2 = expr;  
                curoffset = exprOffSet+1;  
                sameTimes = 0;  
                exprOffSet = -1;  
            }  
            else  
            {  
                p1++;  
                curoffset++;  
            }  
        }  
    }  
    return -1;  
}  

#define safe_free(__p)\
do{\
	if(__p){\
		free(__p);\
		(__p) = NULL;\
	}\
}while(0)

#define mem_strcpy(__dst, __src, n)\
do{\
	memcpy((__dst), (__src), n);\
	((char*)(__dst))[n] = 0;\
}while(0)

static inline char * mem_strdup(char *src, int n)
{
	char *p;
	p = (char *)malloc(n+1);
	if(p){
		mem_strcpy(p, src, n);
	}
	return p;
}

static inline int c2i(char ch)  
{          
    if(isdigit(ch))                  
        return ch - 48;             
    if( ch < 'A' || (ch > 'F' && ch < 'a') || ch > 'z' )                  
        return -1;            
    if(isalpha(ch))                  
        return isupper(ch) ? ch - 55 : ch - 87;            
    return -1;  
}  

static inline u_int64_t strhex2int(char *hex)  
{          
    int len, i, temp;          
    u_int64_t num = 0;                    
    len = strlen(hex);            
    for (i=0, temp=0; i<len; i++, temp=0) {                  
        temp = c2i( *(hex + i) );                  
        num |= ((u_int64_t)temp) << ((len - i - 1) * 4);           
    }              
    return num;  
}  

#endif
