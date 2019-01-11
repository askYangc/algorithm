
CFLAGS	+= -O -I../udpserver -I../app_login -I./
LDFLAGS	+= -lpthread -L../common_open -lsvrlib -lssl -lrt -L../app_login -lapploginlib -Wl,--export-dynamic

DEPLIB = ../common_open/libsvrlib.a ../app_login/libapploginlib.a
EXEC	= linkserver
#CTEST_EXEC=unit_test

include Makefile_ctest.inc
