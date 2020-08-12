#VERBOSE=1
#CFLAGS	+= -O -I../udpserver -I../app_login -I./
#LDFLAGS	+= -lpthread -L../common_open -lsvrlib -lssl -lrt -L../app_login -lapploginlib -Wl,--export-dynamic

#DEPLIB = ../common_open/libsvrlib.a ../app_login/libapploginlib.a
#LDFLAGS += -lpthread -lm -g -Wl,-wrap,malloc -Wl,-wrap,free -Wl,-wrap,calloc -Wl,-wrap,realloc
LDFLAGS += -lpthread -lm -g


SLIB=libmemcheck.a


include ../Makefile.inc
