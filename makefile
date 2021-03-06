#############################################################  
# Generic Makefile for C/C++ Program  
#  
# License: GPL (General Public License)  
# Author:  whyglinux <whyglinux AT gmail DOT com>  
# Date:    2006/03/04 (version 0.1)  
#          2007/03/24 (version 0.2)  
#          2007/04/09 (version 0.3)  
#          2007/06/26 (version 0.4)  
#          2008/04/05 (version 0.5)  
#  
# Description:  
# ------------  
# This is an easily customizable makefile template. The purpose is to  
# provide an instant building environment for C/C++ programs.  
#  
# It searches all the C/C++ source files in the specified directories,  
# makes dependencies, compiles and links to form an executable.  
#  
# Besides its default ability to build C/C++ programs which use only  
# standard C/C++ libraries, you can customize the Makefile to build  
# those using other libraries. Once done, without any changes you can  
# then build programs using the same or less libraries, even if source  
# files are renamed, added or removed. Therefore, it is particularly  
# convenient to use it to build codes for experimental or study use.  
#  
# GNU make is expected to use the Makefile. Other versions of makes  
# may or may not work.  
#  
# Usage:  
# ------  
# 1. Copy the Makefile to your program directory.  
# 2. Customize in the "Customizable Section" only if necessary:  
#    * to use non-standard C/C++ libraries, set pre-processor or compiler  
#      options to <MY_CFLAGS> and linker ones to <MY_LIBS>  
#      (See Makefile.gtk+-2.0 for an example)  
#    * to search sources in more directories, set to <SRCDIRS>  
#    * to specify your favorite program name, set to <PROGRAM>  
# 3. Type make to start building your program.  
#  
# Make Target:  
# ------------  
# The Makefile provides the following targets to make:  
#   $ make           compile and link  
#   $ make NODEP=yes compile and link without generating dependencies  
#   $ make objs      compile only (no linking)  
#   $ make tags      create tags for Emacs editor  
#   $ make ctags     create ctags for VI editor  
#   $ make clean     clean objects and the executable file  
#   $ make distclean clean objects, the executable and dependencies  
#   $ make help      get the usage of the makefile  
#  
#===========================================================================  
  
## Customizable Section: adapt those variables to suit your program.  
##==========================================================================  
 
# The C++ program compiler.  
CXX		= g++


# Add by jio for nfs develop
MY_NFS = /mnt/hgfs/share/
# Add by jio for log4cpp lib
LIB_FLAG = -lssl -lcrypto -pthread  -lz -lsqlite3 -lcurl\
	-L./depends/nxleveldb -lleveldb  \
	-L./depends/utils -lutils \
	-L./depends/network -lnetwork -lev \
	-L./depends/nxrabbitmq/amqp  -lrabbitmq \
	-L./depends/nxrabbitmq -lboost_chrono -lboost_system\
	

MY_CFLAGS =  -I ./ \
	-I depends/nxthread \
	-I depends/nxob \
	-I depends/nxjsoncpp \
	-I depends/nxconfig \
	-I depends/nxleveldb \
	-I depends/nxlog \
	-I depends/nxsqlite3 \
	-I depends/nxtools \
	-I depends/nxcrypto \
	-I depends/nxcurl \
	-I depends/utils \
	-I depends/network \
	-I depends/nxrabbitmq \
	-I depends/nxrabbitmq/amqp \
	-I depends/nxmergedepth \
	-I depends/nxpipe \
	-I pandora/nxpullclient \
	-I pandora/nxhttpclient\
	-I pandora/nxkline \
	-I pandora/nxposeidon \
	-I pandora/nxrpcserver \
	-I pandora/nxmergedepth \
	-I pandora/nxlol \
# The linker options.  
#MY_LIBS   =  
  
# The pre-processor options used by the cpp (man cpp for more).  
DEBUG = YES

ifeq ($(DEBUG),YES)   
	CPPFLAGS  = -g -Wall -O0 -std=c++11 -fpermissive -Wno-strict-aliasing -DDEBUG
else
	CPPFLAGS  = -g -Wall -O2 -std=c++11 -fpermissive -Wno-strict-aliasing 
endif

  
# The options used in linking as well as in any direct use of ld.  
LDFLAGS   = 
  
# The directories in which source files reside.  
# If not specified, only the current directory will be serached.  
SRCDIRS   = ./  \
	depends/nxthread/ \
	depends/nxob \
	depends/nxjsoncpp \
    depends/nxconfig \
	depends/nxleveldb \
	depends/nxlog \
	depends/nxsqlite3 \
	depends/nxtools \
	depends/nxcrypto \
	depends/nxcurl \
	depends/nxrabbitmq \
	depends/nxmergedepth \
	depends/nxpipe \
	pandora/nxpullclient \
	pandora/nxhttpclient\
	pandora/nxkline \
	pandora/nxposeidon \
	pandora/nxrpcserver \
	pandora/nxmergedepth \
	pandora/nxlol \
# The executable file name.  
# If not specified, current directory name or `a.out' will be used.  
PROGRAM   =  Nyx.out
  
## Implicit Section: change the following only when necessary.  
##==========================================================================  
  
# The source file types (headers excluded).  
# .c indicates C source files, and others C++ ones.  
SRCEXTS = .c .C .cc .cpp .CPP .c++ .cxx .cp  
  
# The header file types.  
HDREXTS = .h .H .hh .hpp .HPP .h++ .hxx .hp  
  
# The pre-processor and compiler options.  
# Users can override those variables from the command line.  
#CFLAGS  = -g -O2  
#CXXFLAGS= -g -O2  
   

  
# Un-comment the following line to compile C programs as C++ ones.  
#CC     = $(CXX)  
  
# The command used to delete file.  
#RM     = rm -f  
  
ETAGS = etags  
ETAGSFLAGS =  
  
CTAGS = ctags  
CTAGSFLAGS =  
  
## Stable Section: usually no need to be changed. But you can add more.  
##==========================================================================  
SHELL   = /bin/sh  
EMPTY   =  
SPACE   = $(EMPTY) $(EMPTY)  
ifeq ($(PROGRAM),)  
  CUR_PATH_NAMES = $(subst /,$(SPACE),$(subst $(SPACE),_,$(CURDIR)))  
  PROGRAM = $(word $(words $(CUR_PATH_NAMES)),$(CUR_PATH_NAMES))  
  ifeq ($(PROGRAM),)  
    PROGRAM = a.out  
  endif  
endif  
ifeq ($(SRCDIRS),)  
  SRCDIRS = .  
endif  
SOURCES = $(foreach d,$(SRCDIRS),$(wildcard $(addprefix $(d)/*,$(SRCEXTS))))  
HEADERS = $(foreach d,$(SRCDIRS),$(wildcard $(addprefix $(d)/*,$(HDREXTS))))  
SRC_CXX = $(filter-out %.c,$(SOURCES))  
OBJS    = $(addsuffix .o, $(basename $(SOURCES)))  
  
## Define some useful variables.  
DEP_OPT = $(shell if `$(CC) --version | grep "GCC" >/dev/null`; then \
                  echo "-MM -MP"; else echo "-M"; fi )  
DEPEND.d    = $(subst -g ,,$(DEPEND))  
COMPILE.cxx = $(CXX) $(MY_CFLAGS) $(CXXFLAGS) $(CPPFLAGS) -c  
LINK.cxx    = $(CXX) $(MY_CFLAGS) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS)  
  
.PHONY: all objs tags ctags clean distclean help show  
  
# Delete the default suffixes  
.SUFFIXES:  
  
all: $(PROGRAM)  
  
# Rules for creating dependency files (.d).  
#------------------------------------------  
    
# Rules for generating object files (.o).  
#----------------------------------------  
objs:$(OBJS)  

%.o:%.c  
	$(COMPILE.cxx) $< -o $@  
  
%.o:%.C  
	$(COMPILE.cxx) $< -o $@  
  
%.o:%.cc  
	$(COMPILE.cxx) $< -o $@  
  
%.o:%.cpp  
	$(COMPILE.cxx) $< -o $@  
  
%.o:%.CPP  
	$(COMPILE.cxx) $< -o $@  
  
%.o:%.c++  
	$(COMPILE.cxx) $< -o $@  
  
%.o:%.cp  
	$(COMPILE.cxx) $< -o $@  
  
%.o:%.cxx  
	$(COMPILE.cxx) $< -o $@  
  
# Rules for generating the tags.  
#-------------------------------------  
tags: $(HEADERS) $(SOURCES)  
	$(ETAGS) $(ETAGSFLAGS) $(HEADERS) $(SOURCES)  
  
ctags: $(HEADERS) $(SOURCES)  
	$(CTAGS) $(CTAGSFLAGS) $(HEADERS) $(SOURCES)  
  
# Rules for generating the executable.  
#-------------------------------------  

$(PROGRAM):$(OBJS)  
ifeq ($(SRC_CXX),)              # C program  
    $(LINK.c)   $(OBJS) $(MY_LIBS) -o $@  $(LIB_FLAG)
    @echo Type ./$@ to execute the program.  
    cp ./$@ $(MY_NFS)
else                            # C++ program  
	$(LINK.cxx) $(OBJS) $(MY_LIBS) -o $@  $(LIB_FLAG)
	@echo Type ./$@ to execute the program. 
	cp ./$@ $(MY_NFS)
endif  
  
ifndef NODEP  
ifneq ($(DEPS),)  
  sinclude $(DEPS)  
endif  
endif  
  
clean:  
	$(RM) $(OBJS) $(PROGRAM) $(PROGRAM).out  *.d
  
distclean: clean  
	$(RM) $(DEPS) TAGS 

  
# Show help.  
help:  
	@echo 'Generic Makefile for C/C++ Programs (gcmakefile) version 0.5'  
	@echo 'Copyright (C) 2007, 2008 whyglinux <whyglinux@hotmail.com>'  
	@echo  
	@echo 'Usage: make [TARGET]'  
	@echo 'TARGETS:'  
	@echo '  prepare   prepare lib before make
	@echo '  all       (=make) compile and link.'  
	@echo '  NODEP=yes make without generating dependencies.'  
	@echo '  objs      compile only (no linking).'  
	@echo '  tags      create tags for Emacs editor.'  
	@echo '  ctags     create ctags for VI editor.'  
	@echo '  clean     clean objects and the executable file.'  
	@echo '  distclean clean objects, the executable and dependencies.'  
	@echo '  show      show variables (for debug use only).'  
	@echo '  help      print this message.'  
	@echo  
	@echo 'Report bugs to <whyglinux AT gmail DOT com>.'  
  
# Show variables (for debug use only.)  
show:  
	@echo 'PROGRAM     :' $(PROGRAM)  
	@echo 'SRCDIRS     :' $(SRCDIRS)  
	@echo 'HEADERS     :' $(HEADERS)  
	@echo 'SOURCES     :' $(SOURCES)  
	@echo 'SRC_CXX     :' $(SRC_CXX)  
	@echo 'OBJS        :' $(OBJS)  
	@echo 'DEPS        :' $(DEPS)  
	@echo 'DEPEND      :' $(DEPEND)  
	@echo 'COMPILE.c   :' $(COMPILE.c)  
	@echo 'COMPILE.cxx :' $(COMPILE.cxx)  
	@echo 'link.c      :' $(LINK.c)  
	@echo 'link.cxx    :' $(LINK.cxx)  
  
## End of the Makefile ##  Suggestions are welcome  ## All rights reserved ##  
##############################################################  
