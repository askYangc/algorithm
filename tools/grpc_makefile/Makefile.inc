ARCH = $(shell getconf LONG_BIT)

ifeq ($(ARCH),64)
      ARCHGCC = -m64
      CFLAGS += -DBITS64
      MYSQLLIB = /usr/lib64/mysql
else
      ARCHGCC = -m32
      MYSQLLIB = /usr/lib/mysql
endif

AS      = $(CROSS_COMPILE)as
LD      = $(CROSS_COMPILE)ld
CC      = $(CROSS_COMPILE)gcc
CE      = $(CC) -E
CPP     = $(CROSS_COMPILE)g++
AR      = $(CROSS_COMPILE)ar
NM      = $(CROSS_COMPILE)nm
STRIP   = $(CROSS_COMPILE)strip
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump
RANLIB  = $(CROSS_COMPILE)ranlib

#CFLAGS	+= -Wall -I../include_open -I../zookeeper/include/c-client-src -g $(ARCHGCC) -D_USE_SYSLOG_
#LDFLAGS +=  -lm -ldl -lcrypto -lpthread -lmemcached\
            -L$(MYSQLLIB) -lmysqlclient\
            -L../zookeeper/lib -lzookeeper_mt	

CFLAGS	+= -Wall -g $(ARCHGCC) 
LDFLAGS += 

VERBOSE	= 0

ifeq ($(SRCS), )
SRCS    = $(wildcard *.c)
endif

ifeq ($(SRCSXX), )
SRCSXX    = $(wildcard *.cpp)
endif

ifeq ($(SHLIB), )
OBJS    = $(patsubst %.c, %.o, $(SRCS))
DEPS    = $(patsubst %.c, .%.d, $(SRCS))
OBJS    += $(patsubst %.cpp, %.o, $(SRCSXX))
DEPS    += $(patsubst %.cpp, .%.d, $(SRCSXX))
endif

PROTOS_PATH = ./
PROTOC_INC = /usr/local/protobuf3.8/include
PROTOC_LIB = /usr/local/protobuf3.8/lib
PROTOC = /usr/local/protobuf3.8/bin/protoc
GRPC_CPP_PLUGIN = grpc_cpp_plugin
GRPC_CPP_PLUGIN_PATH ?= `which $(GRPC_CPP_PLUGIN)`

PB = $(wildcard *.proto)
GB = $(wildcard *.proto)
PBSRC = $(patsubst %.proto, %.pb.cc, $(PB))
GBSRC = $(patsubst %.proto, %.grpc.pb.cc, $(GB))
PBOBJ = $(patsubst %.pb.cc, %.pb.o, $(PBSRC))
GBOBJ = $(patsubst %.grpc.pb.cc, %.grpc.pb.o, $(GBSRC))
PBHEAD = $(patsubst %.pb.cc, %.pb.h, $(PBSRC))
GBHEAD = $(patsubst %.grpc.pb.cc, %.grpc.pb.h, $(GBSRC))
#$(info $(GBHEAD))


PBFLAGS = `pkg-config --cflags protobuf grpc` -I$(PROTOC_INC)

#.PHONY: $(SUBDIRS) $(DEPS) $(OBJS) $(EXEC) $(CPPEXEC) $(SLIB) $(SHLIB)

all: $(SUBDIRS) $(PBOBJ) $(GBOBJ) $(DEP) $(OBJS) $(EXEC) $(CPPEXEC) $(SLIB) $(SHLIB)

%.o : %.c
	@echo -e "  CC\t$@"
	@$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@
%.o : %.cpp
	@echo -e "  CPP\t$@"
	@$(CPP) -c $(CFLAGS) $(CPPFLAGS) $< -o $@

.%.d : %.c
	@echo -e "  DEP\t$@" 
	@$(CC) -c -MM $(CFLAGS) $(CPPFLAGS) $< -o $@
.%.d : %.cpp
	@echo -e "  DEP\t$@" 
	@$(CC) -c -MM $(CFLAGS) $(CPPFLAGS) $< -o $@

%.grpc.pb.o: %.grpc.pb.cc
	@echo -e "  GB\t$@" 
	@$(CPP) -c $(CFLAGS) $(CPPFLAGS) $(PBFLAGS) $< -o $@

%.pb.o: %.pb.cc
	@echo -e "  PB\t$@" 
	@$(CPP) -c $(CFLAGS) $(CPPFLAGS) $(PBFLAGS) $< -o $@
	

.PRECIOUS: %.grpc.pb.cc
%.grpc.pb.cc: %.proto
	@echo -e "  PROTOC-GRPC\t$@" 
	@$(PROTOC) -I $(PROTOS_PATH) --grpc_out=. --plugin=protoc-gen-grpc=$(GRPC_CPP_PLUGIN_PATH) $<

.PRECIOUS: %.pb.cc
%.pb.cc: %.proto
	@echo -e "  PROTOC\t$@" 
	@$(PROTOC) -I $(PROTOS_PATH) --cpp_out=. $<


$(EXEC): $(DEPLIB) $(OBJS)
	@echo -e "  LD\t$@"
	@$(CC) $(OBJS) $(LDFLAGS) -o $@

$(CPPEXEC): $(DEPLIB) $(OBJS)
	@echo -e "  LD\t$@"
	@$(CPP) $(OBJS) $(LDFLAGS) -o $@

$(SHLIB): $(SRCS)
	@echo -e "  LD\t$@"
	@$(CC) -fPIC -shared $(SRCS) $(CFLAGS) $(LDFLAGS) -o $@

$(SLIB): $(OBJS)
	@echo -e "  AR\t$@"
	$(AR) cr $@ $(OBJS)
	$(RANLIB) $@

ifeq ($(VERBOSE), 0)
$(SUBDIRS)::
	@echo -e "MAKE\t[$@]"
	@$(MAKE) --no-print-directory -C $@ 
else
$(SUBDIRS)::
	$(MAKE) -C $@ 
endif

install_dirs := $(addprefix _install_, $(SUBDIRS))
$(install_dirs):
	@[ -d dsserver ] || mkdir dsserver
	@$(MAKE) -C $(patsubst _install_%, %, $@) INSDIR=$(INSDIR) install

install: $(install_dirs)
ifneq ($(EXEC), )
	cp $(EXEC) $(INSDIR)/
endif
ifneq ($(SHLIB), )
	cp $(SHLIB) $(INSDIR)/
endif

clean_dirs := $(addprefix _clean_, $(SUBDIRS))

$(clean_dirs):
	@$(MAKE) -C $(patsubst _clean_%, %, $@) clean

clean: $(clean_dirs)
	@rm -f .*.d *.o *.a *.so $(EXEC) $(CPPEXEC) $(SHLIB) $(SLIB)
cleanall: $(clean_dirs)
	@rm -f .*.d *.o *.a *.so $(EXEC) $(CPPEXEC) $(SHLIB) $(PBSRC) $(GBSRC) $(PBHEAD) $(GBHEAD)


-include $(DEPS)
