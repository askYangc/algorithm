AS      = $(CROSS_COMPILE)as
LD      = $(CROSS_COMPILE)ld
CC      = $(CROSS_COMPILE)gcc
CXX      = $(CROSS_COMPILE)g++
CPP     = $(CC) -E
AR      = $(CROSS_COMPILE)ar
NM      = $(CROSS_COMPILE)nm
STRIP   = $(CROSS_COMPILE)strip
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump
RANLIB  = $(CROSS_COMPILE)ranlib


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

EXEC = main

CFLAGS	+= -Wall
LDFLAGS += -lpthread

#.PHONY: $(SUBDIRS) $(DEPS) $(OBJS) $(EXEC) $(SLIB) $(SHLIB)

VERBOSE=0

all: $(SUBDIRS) $(DEPS) $(OBJS) $(EXEC) $(SLIB) $(SHLIB)

ifeq ($(VERBOSE), 0)

%.o : %.c
	@echo -e "  CC\t$@"
	@$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@
%.o : %.cpp
	@echo -e "  CXX\t$@"
	@$(CXX) -c $(CFLAGS) $(CPPFLAGS) $< -o $@

.%.d : %.c
	@echo -e "  DEP\t$@" 
	@$(CC) -c -MM $(CFLAGS) $(CPPFLAGS) $< -o $@
.%.d : %.cpp
	@echo -e "  DEP\t$@" 
	@$(CXX) -c -MM $(CFLAGS) $(CPPFLAGS) $< -o $@

$(EXEC): $(DEPLIB) $(OBJS)
	@echo -e "  LD\t$@"
	@$(CXX) $(OBJS) $(LDFLAGS) -o $@

$(SHLIB): $(SRCS)
	@echo -e "  LD\t$@"
	@$(CC) -fPIC -shared $(SRCS) $(CFLAGS) $(LDFLAGS) -o $@

$(SLIB): $(OBJS)
	@echo -e "  AR\t$@"
	@$(AR) cr $@ $(OBJS)
	@$(RANLIB) $@

$(SUBDIRS)::
	@echo -e "MAKE\t[$@]"
	@$(MAKE) --no-print-directory -C $@ 

else

%.o : %.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@
%.o : %.cpp
	$(CXX) -c $(CFLAGS) $(CPPFLAGS) $< -o $@

.%.d : %.c
	$(CC) -c -MM $(CFLAGS) $(CPPFLAGS) $< -o $@
.%.d : %.cpp
	$(CXX) -c -MM $(CFLAGS) $(CPPFLAGS) $< -o $@

$(EXEC): $(DEPLIB) $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o $@

$(SHLIB): $(SRCS)
	$(CC) -fPIC -shared $(SRCS) $(CFLAGS) $(LDFLAGS) -o $@

$(SLIB): $(OBJS)
	$(AR) cr $@ $(OBJS)
	$(RANLIB) $@

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
	@rm -f .*.d *.o *.a *.so $(EXEC) $(SHLIB) $(SLIB)


-include $(DEPS)
