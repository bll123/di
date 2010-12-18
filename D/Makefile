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
MAKE = make

DC = dmd

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

all:	di.env
	. ./di.env; \
	$(MAKE) -e MKCONFIG_TYPE=sh di-programs

all-test: di.env
	. ./di.env; \
	$(MAKE) -e MKCONFIG_TYPE=sh \
        DTESTFLAGS="$$DC_UNITTEST $$DC_COV" di-programs

###
# environment

#### possibly merge features/env.dat w/C version???

di.env:	features/env.dat
	@-$(RM) -f di.env tests.done
	DC=$(DC) $(_MKCONFIG_SHELL) \
		$(MKCONFIGPATH)/mkconfig.sh features/env.dat

###
# cleaning

# leaves:
#   config.d, di.env, mkconfig.reqlibs,
#   mkconfig.cache, mkconfig_*.vars,
#   dioptions.dat
clean:
	@-rm -f di di.exe mi mi.exe *.o *.obj *.lst mkconfig.log \
	    > /dev/null 2>&1

# leaves:
#   _mkconfig_runtests, _tmp_mkconfig, dioptions.dat
realclean:
	@$(MAKE) clean
	@-$(RM) -rf config.d di.env mkconfig.reqlibs \
		mkconfig.cache mkconfig_*.vars \
		>/dev/null 2>&1

# leaves:
#   dioptions.dat
distclean:
	@$(MAKE) realclean
	@-$(RM) -rf _mkconfig_runtests _tmp_mkconfig \
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

OBJECTS = config$(OBJ_EXT) di_getopt$(OBJ_EXT) display$(OBJ_EXT) options$(OBJ_EXT) di$(OBJ_EXT)

di$(EXE_EXT):	$(OBJECTS)
	$(DC) $(DC_OF)di$(EXE_EXT) $(OBJECTS)

###
# objects

config$(OBJ_EXT):	config.d
	$(DC) -c $(DFLAGS) $(DTESTFLAGS) config.d

di$(OBJ_EXT):	di.d config$(OBJ_EXT) \
		options$(OBJ_EXT) display$(OBJ_EXT) di_getopt$(OBJ_EXT)
	$(DC) -c $(DFLAGS) $(DTESTFLAGS) di.d

options$(OBJ_EXT):	options.d config$(OBJ_EXT) \
		display$(OBJ_EXT) di_getopt$(OBJ_EXT)
	$(DC) -c $(DFLAGS) $(DTESTFLAGS) options.d

di_getopt$(OBJ_EXT):	di_getopt.d config$(OBJ_EXT)
	$(DC) -c $(DFLAGS) $(DTESTFLAGS) di_getopt.d

display$(OBJ_EXT):	display.d config$(OBJ_EXT)
	$(DC) -c $(DFLAGS) $(DTESTFLAGS) display.d
