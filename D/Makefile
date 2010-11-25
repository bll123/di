#
#  di makefile
#
#  Copyright 2010 Brad Lanam Walnut Creek CA, USA
#
#  $Id$
#

SHELL = /bin/sh
OBJ_EXT = .o
EXE_EXT =
DC = dmd
MAKE = make

#
# common programs
#
CAT = cat
CHGRP = chgrp
CHMOD = chmod
CHOWN = chown
CP = cp
LN = ln
MKDIR = mkdir
MV = mv
RM = rm
RPMBUILD = rpmbuild
SED = sed
TEST = test

###
# mkconfig variables

MKCONFIGPATH = ../mkconfig
MKCONFIG_TYPE = sh
MKC_REQLIB = mkconfig.reqlibs

###
# main

# have to get various environment variables set up.

all:	all-sh

all-sh:	di.env
	. ./di.env;$(MAKE) -e MKCONFIG_TYPE=sh di-programs

###
# environment

#### possibly merge features/env.dat w/C version???

di.env:	features/env.dat
	@-$(RM) -f di.env tests.done
	CC=$(CC) $(_MKCONFIG_SHELL) \
		$(MKCONFIGPATH)/mkconfig.sh features/env.dat

###
# cleaning

# leaves:
#   config.d, di.env, mkconfig.reqlibs,
#   mkconfig.cache, mkconfig_*.vars,
#   dioptions.dat, pretests.done, test_di
clean:
	@-rm -f di di.exe mi mi.exe *.o *.obj \
	    > /dev/null 2>&1

# leaves:
#   _mkconfig_runtests, _tmp_mkconfig, dioptions.dat
#   pretests.done, test_di
realclean:
	@$(MAKE) clean
	@-$(RM) -rf config.d di.env mkconfig.reqlibs \
		mkconfig.cache mkconfig_*.vars \
		>/dev/null 2>&1

# leaves:
#   dioptions.dat
distclean:
	@$(MAKE) realclean
	@-$(RM) -rf pretests.done test_di \
		_mkconfig_runtests _tmp_mkconfig \
		>/dev/null 2>&1

###
# programs

di-programs:	di$(EXE_EXT)

###
# configuration file

dioptions.dat:	../dioptions.dat
	@cp ../dioptions.dat dioptions.dat
	@chmod u+w dioptions.dat

config.d:	di.env dioptions.dat features/mkconfig.dat
	@-$(RM) -f config.d tests.done
	@if [ "$(DI_NO_NLS)" != "" ]; then \
		echo "*** User requested no NLS"; \
		$(MKCONFIGPATH)/mkcsetopt.sh -o dioptions.dat NLS F; fi
	@. ./di.env;$(_MKCONFIG_SHELL) \
		$(MKCONFIGPATH)/mkconfig.sh \
		features/mkconfig.dat; fi

$(MKC_REQLIB):	di.env config.d
	$(_MKCONFIG_SHELL) $(MKCONFIGPATH)/mkreqlib.sh config.d

###
# executables

OBJECTS = getopt$(OBJ_EXT) options$(OBJ_EXT) di$(OBJ_EXT)
#DFLAGS = -debug -unittest 
DFLAGS = -unittest 

di$(EXE_EXT):	$(OBJECTS)
	$(DC) -ofdi$(EXE_EXT) $(OBJECTS)

###
# objects

di$(OBJ_EXT):	di.d options$(OBJ_EXT) getopt$(OBJ_EXT)
	$(DC) -c $(DFLAGS) di.d

options$(OBJ_EXT):	options.d getopt$(OBJ_EXT)
	$(DC) -c $(DFLAGS) options.d

getopt$(OBJ_EXT):	getopt.d
	$(DC) -c $(DFLAGS) getopt.d

###
# regression testing

di-all-test:	pretests.done tests.done

pretests.done:
	cd $(MKCONFIGPATH); $(MAKE) CC=$(CC) tests.done

tests.done: $(MKCONFIGPATH)/runtests.sh
	@echo "## running tests"
	CC=$(CC) $(_MKCONFIG_SHELL) $(MKCONFIGPATH)/runtests.sh ../tests.d
	touch tests.done

testrpmbuild:
	-$(TEST) -d ./rpmbuild && rm -rf ./rpmbuild
	$(MKDIR) -p ./rpmbuild/SOURCES
	$(MKDIR) -p ./rpmbuild/BUILD
	$(CP) -f $(DI_DIR)/di-$(DI_VERSION).tar.gz ./rpmbuild/SOURCES
	$(RPMBUILD) --define="_topdir `pwd`/rpmbuild" -ba di.spec
	$(TEST) -f ./rpmbuild/RPMS/i386/di-$(DI_VERSION)-1.i386.rpm || exit 1
	$(TEST) -f ./rpmbuild/SRPMS/di-$(DI_VERSION)-1.src.rpm || exit 1
	$(TEST) -d ./rpmbuild && rm -rf ./rpmbuild

rtest-env:
	@echo "$(_MKCONFIG_SYSTYPE)"; \
		echo "$(_MKCONFIG_SYSREV)"; \
		echo "$(_MKCONFIG_SYSARCH)"; \
		echo "$(CC)"; \
		echo "$(_MKCONFIG_USING_GCC)"; \
		echo "$(CFLAGS)"; \
		echo "$(LDFLAGS)"; \
		echo "$(LIBS)"; \
		echo "$(OBJ_EXT)"; \
		echo "$(EXE_EXT)"; \
		echo "$(XMSGFMT)"
