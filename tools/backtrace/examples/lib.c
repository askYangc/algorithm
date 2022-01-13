#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <execinfo.h>
#include <unistd.h>

#include "lib.h"


void exec_shell(char *cmd, char *buf, int buf_size)
{
	FILE *fp = NULL;
	fp = popen(cmd, "r");
	if(fp == NULL) {
		return;
	}

	while(fgets(buf, buf_size, fp) != NULL);

	pclose(fp);
	return ;
}

int get_backtrace_string(void* bt,char* buff,int buff_size)
{
	char cmd[128] = {0};
	char property[256]={0};
	char not_care1[128]={0},not_care2[128]={0},not_care3[128]={0};
	char library_path[256]={0};
	char exe_path[256],function_name[256];
	char maps_line[1024]={0};
	void* offset_start,*offset_end;
	int maps_column_num=0, n;
	FILE* fd_maps=NULL;
	fd_maps=fopen("/proc/self/maps","r");
	unsigned long exe_symbol_offset=0;
	char unknow_position[12] ="??:?\n";
	unsigned char unknow_position_len = strlen(unknow_position);
	if(fd_maps==NULL)
	{
		return -1;
	}
	while(NULL!=fgets(maps_line,sizeof(maps_line),fd_maps))
	{
		maps_column_num=sscanf(maps_line,"%p-%p\t%s\t%s\t%s\t%s\t%s"
														,&offset_start
														,&offset_end
														,property
														,not_care1
														,not_care2
														,not_care3
														,library_path);
		
		if(maps_column_num==7&&bt>=offset_start&&bt<=offset_end)
		{
			//printf("=== get in bt: %p, maps_line: %s\n", bt, maps_line);
			break;
		}
		
	}
	fclose(fd_maps);
	n = readlink("/proc/self/exe", exe_path, sizeof(exe_path));
	exe_path[n] = 0;
	if(0==strcmp(exe_path,library_path))
	{
		exe_symbol_offset=(unsigned long)bt;
	}
	else
	{
		exe_symbol_offset=(char*)bt-(char*)offset_start;
	}
	snprintf(cmd,sizeof(cmd),"addr2line -Cfp -e %s 0x%lx",library_path,exe_symbol_offset);
	//printf("cmd: %s\n", cmd);
	exec_shell(cmd, buff, buff_size);
	//printf("buff: %s\n", buff);
	if(0==memcmp(&buff[strlen(buff) - unknow_position_len],unknow_position, unknow_position_len))
	{
		snprintf(cmd,sizeof(cmd),"addr2line -Cifp -e %s 0x%lx",library_path,exe_symbol_offset);
		//printf("cmd1: %s\n", cmd);
		exec_shell(cmd, function_name, sizeof(function_name));
		function_name[strlen(function_name)-1]='\0';
		snprintf(buff,buff_size,"%s(%s+0x%lx)\n",library_path,function_name,exe_symbol_offset);
	}
	return 0;
}


/*
如果使用my_backtrace
如果源代码编译时使用了-O1或-O2优化选项，可执行代码会把ebp/rbp/rsp寄存器当作普通寄存器使用，导致backtrace失败。
为了防止这种情况发生，可以在编译时使用-O2  -fno-omit-frame-pointer  或-Og 来避免优化中使用上述寄存器。
系统自带的backtrace函数是通过读取操作系统的一个全局信息区，在多线程并发调用时，会造成严重的锁冲突
*/
#define STACKCALL __attribute__((regparm(1),noinline))  
void ** STACKCALL getEBP(void){  
        void **ebp=NULL;  
        __asm__ __volatile__("mov %%rbp, %0;\n\t"  
                    :"=m"(ebp)      /* 输出 */  
                    :      /* 输入 */  
                    :"memory");     /* 不受影响的寄存器 */  
        return (void **)(*ebp);  
}  
int my_backtrace(void **buffer,int size){  
      
    int frame=0;  
    void ** ebp;  
    void **ret=NULL;  
    unsigned long long func_frame_distance=0;  
    if(buffer!=NULL && size >0)  
    {  
        ebp=getEBP();  
        func_frame_distance=(unsigned long long)(*ebp) - (unsigned long long)ebp;  
        while(ebp&& frame<size  
            &&(func_frame_distance< (1ULL<<24))//assume function ebp more than 16M  
            &&(func_frame_distance>0))  
        {  
            ret=ebp+1;  
            buffer[frame++]=*ret;  
            ebp=(void**)(*ebp);  
            func_frame_distance=(unsigned long long)(*ebp) - (unsigned long long)ebp;  
        }  
    }  
    return frame;  
}  



#define BACKTRACE_SIZE 30
void bt_show()
{
	int i;
	void *array[BACKTRACE_SIZE];
	char buf[2048] = {0};

#if 1
	//用get_backtrace_string
	//backtrace or my_backtrace
	int stack_num = backtrace(array, BACKTRACE_SIZE);
	for(i = 0; i < stack_num; i++) {
		get_backtrace_string(array[i], buf, 2048);
		printf("  [%02d] %s\n", i, buf);
	}
#else 
	//用系统的backtrace_symbols,需要释放
	//backtrace or my_backtrace
	int stack_num = backtrace(array, BACKTRACE_SIZE);
	printf("stack_num: %d\n", stack_num);
	char **stacktrace = backtrace_symbols(array, stack_num);	
	if(stacktrace == NULL){
		return;
	}
	for(i = 0; i < stack_num; i++) {
		printf("  [%02d] %s\n", i, stacktrace[i]);
	}
	free(stacktrace);
#endif
}

void hello()
{
	printf("hello world\n");
	test_inline();
}
