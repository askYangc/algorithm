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
#SRCS    = $(wildcard *.c)
SRCS    = $(filter-out test_%,$(wildcard *.c))
endif

ifeq ($(SRCSXX), )
#SRCSXX    = $(wildcard *.cpp)
SRCSXX    = $(filter-out test_%,$(wildcard *.cpp))
endif

ifeq ($(SHLIB), )
OBJS    = $(patsubst %.c, %.o, $(SRCS))
DEPS    = $(patsubst %.c, .%.d, $(SRCS))
OBJS    += $(patsubst %.cpp, %.o, $(SRCSXX))
DEPS    += $(patsubst %.cpp, .%.d, $(SRCSXX))
endif

ifdef ctest

ifeq ($(CTEST_EXEC), )
CTEST_EXEC = ctest_$(EXEC)
endif
$(warning $(CTEST_EXEC))

EXECF = $(EXEC)

ifeq ($(CTEST_SRCS), )
CTEST_SRCS    = $(filter-out $(EXECF).c,$(wildcard *.c))
#$(warning $(CTEST_SRCS))
endif

ifeq ($(CTEST_SRCSXX), )
CTEST_SRCSXX    = $(filter-out $(EXECF).cpp,$(wildcard *.cpp))
endif

ifeq ($(CTEST_SHLIB), )
CTEST_OBJS    = $(patsubst %.c, %.o, $(CTEST_SRCS))
CTEST_DEPS    = $(patsubst %.c, .%.d, $(CTEST_SRCS))
CTEST_OBJS    += $(patsubst %.cpp, %.o, $(CTEST_SRCSXX))
CTEST_DEPS    += $(patsubst %.cpp, .%.d, $(CTEST_SRCSXX))
endif

CTEST_CFLAGS += -fprofile-arcs -ftest-coverage
#CTEST_LDFLAGS += -lcunit -lgcov --coverage
CTEST_LDFLAGS += -lcunit --coverage
endif


#.PHONY: $(SUBDIRS) $(DEPS) $(OBJS) $(CTEST_OBJS) $(EXEC) $(SLIB) $(SHLIB) $(CTEST_EXEC)

all: $(SUBDIRS) $(DEPS) $(OBJS) $(EXEC) $(SLIB) $(SHLIB) $(CTEST_EXEC)

ifndef ctest
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

$(EXEC): $(DEPLIB) $(OBJS)
	@echo -e "  LD\t$@"
	@$(CC) $(OBJS) $(LDFLAGS) -o $@

$(CTEST_EXEC):

$(SHLIB): $(SRCS)
	@echo -e "  LD\t$@"
	@$(CC) -fPIC -shared $(SRCS) $(CFLAGS) $(LDFLAGS) -o $@

$(SLIB): $(OBJS)
	@echo -e "  AR\t$@"
	@$(AR) cr $@ $(OBJS)
	@$(RANLIB) $@

else 

%.o : %.c
	@echo -e "  CC\t$@"
	@$(CC) -c $(CFLAGS) $(CPPFLAGS) $(CTEST_CFLAGS) $< -o $@
%.o : %.cpp
	@echo -e "  CPP\t$@"
	@$(CPP) -c $(CFLAGS) $(CPPFLAGS) $(CTEST_CFLAGS) $< -o $@

.%.d : %.c
	@echo -e "  DEP\t$@" 
	@$(CC) -c -MM $(CFLAGS) $(CPPFLAGS) $(CTEST_CFLAGS) $< -o $@
.%.d : %.cpp
	@echo -e "  DEP\t$@" 
	@$(CC) -c -MM $(CFLAGS) $(CPPFLAGS) $(CTEST_CFLAGS) $< -o $@

$(EXEC): $(DEPLIB) $(OBJS)
	@echo -e "  LD\t$@"
	@$(CC) $(OBJS) $(LDFLAGS) $(CTEST_LDFLAGS) -o $@

$(CTEST_EXEC): $(CTEST_DEPLIB) $(CTEST_OBJS)
	@echo -e "  LD\t$@"
	$(CC) $(CTEST_OBJS) $(LDFLAGS) $(CTEST_LDFLAGS) -o $@

$(SHLIB): $(SRCS)
	@echo -e "  LD\t$@"
	@$(CC) -fPIC -shared $(SRCS) $(CFLAGS) $(LDFLAGS) $(CTEST_CFLAGS) $(CTEST_LDFLAGS) -o $@

$(SLIB): $(OBJS)
	@echo -e "  AR\t$@"
	@$(AR) cr $@ $(OBJS)
	@$(RANLIB) $@


endif

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
	@rm -f .*.d *.o *.a *.so *.gcno *.gcda *.gcov *.xml $(EXEC) $(SHLIB) $(SLIB) $(CTEST_EXEC)


#-include $(DEPS)
-include $(ALLDEPS)
