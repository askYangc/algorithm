CPPEXEC=1
#VERBOSE=1
CFLAGS	+= -g -I./ -I../check_mem_leaks
#LDFLAGS	+= -lpthread -L../check_mem_leaks -lmemcheck -Wl,--export-dynamic -fno-omit-frame-pointer -rdynamic -Wl,-wrap,malloc -Wl,-wrap,free -Wl,-wrap,calloc -Wl,-wrap,realloc
#LDFLAGS	+= -lpthread -L../test -ltest -L../check_mem_leaks -lmemcheck -Wl,--export-dynamic -fno-omit-frame-pointer -rdynamic -Wl,-wrap,malloc -Wl,-wrap,free -Wl,-wrap,calloc -Wl,-wrap,realloc
#LDFLAGS	+= -lpthread -L../check_mem_leaks -lmemcheck -Wl,--export-dynamic -fno-omit-frame-pointer -rdynamic

#DEPLIB = ../check_mem_leaks/libmemcheck.a 
#DEPLIB = ../test/libtest.a 

include ../Makefile.inc
