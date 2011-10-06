#
#  di makefile
#
#  Copyright 2011 Brad Lanam Walnut Creek CA, USA
#
#  $Id$
#

SHELL = /bin/sh
OBJ_EXT = .o
EXE_EXT =
MAKE = make

CC = cc
DC = gdc

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

###
# mkconfig variables

MKCONFIGPATH = ../mkconfig
MKCONFIG_TYPE = sh
MKC_REQLIB = di.reqlibs

###
# main

# have to get various environment variables set up.

#        DTESTFLAGS="$$DC_OPT $$DC_RELEASE $$DC_INLINE" di-programs

all:	di.env
	. ./di.env; \
	$(MAKE) -e MKCONFIG_TYPE=sh \
        DTESTFLAGS="$$DC_OPT $$DC_RELEASE " di-programs


#        DTESTFLAGS="$$DC_UNITTEST $$DC_COV" \

all-test: di.env
	. ./di.env; \
	$(MAKE) -e MKCONFIG_TYPE=sh \
        DTESTFLAGS="$$DC_UNITTEST " \
	LIBS="$(LIBS) $$DC_LIBS" \
	di-programs

###
# environment

#### possibly merge features/env.dat w/C version???

di.env:	features/env.dat
	@-$(RM) -f di.env tests.done
	DC=$(DC) CFLAGS="-I`pwd`/../C" $(_MKCONFIG_SHELL) \
		$(MKCONFIGPATH)/mkconfig.sh features/env.dat

###
# cleaning

# leaves:
#   config.d, di.env, di.reqlibs
clean:
	@-rm -f di di.exe mi mi.exe \
		*.o *.obj *.lst *.gcno *.gcda *.gcov \
		mkconfig.log mkconfig.cache mkc*.vars \
		> /dev/null 2>&1

# leaves:
#   _mkconfig_runtests, _tmp_mkconfig
realclean:
	@$(MAKE) clean > /dev/null 2>&1
	@-$(RM) -rf config.d di.env $(MKC_REQLIB) \
		>/dev/null 2>&1

# leaves:
distclean:
	@$(MAKE) realclean > /dev/null 2>&1
	@-$(RM) -rf _mkconfig_runtests _tmp_mkconfig \
		>/dev/null 2>&1

###
# programs

di-programs:	di$(EXE_EXT)

###
# configuration file

../dioptions.dat:	../features/dioptions.dat
	cd ../;$(MAKE) dioptions.dat

config.d:	di.env ../dioptions.dat features/mkconfig.dat
	@-$(RM) -f config.d tests.done
	@if [ "$(DI_NO_NLS)" != "" ]; then \
		echo "*** User requested no NLS"; \
		$(MKCONFIGPATH)/mkcsetopt.sh -o ../dioptions.dat NLS F; fi
	@. ./di.env;$(_MKCONFIG_SHELL) \
		$(MKCONFIGPATH)/mkconfig.sh \
		features/mkconfig.dat; fi

$(MKC_REQLIB):	di.env config.d
	$(_MKCONFIG_SHELL) $(MKCONFIGPATH)/mkreqlib.sh -l \
		$(MKC_REQLIB) config.d

###
# executables

OBJECTS = config$(OBJ_EXT) digetopt$(OBJ_EXT) dilocale$(OBJ_EXT) \
	diquota$(OBJ_EXT) diskpart$(OBJ_EXT) \
	display$(OBJ_EXT) dispopts$(OBJ_EXT) di$(OBJ_EXT) options$(OBJ_EXT)

di$(EXE_EXT):	$(OBJECTS)
	$(MKCONFIGPATH)/mklink.sh -e -c ${DC} -o di$(EXE_EXT) -- \
		$(OBJECTS) $(LIBS)

###
# objects

config$(OBJ_EXT):	config.d
	$(DC) -c $(DFLAGS) $(DTESTFLAGS) config.d

di$(OBJ_EXT):	di.d config$(OBJ_EXT) \
		diskpart$(OBJ_EXT) dilocale$(OBJ_EXT) \
		diquota$(OBJ_EXT) display$(OBJ_EXT) \
		dispopts$(OBJ_EXT) digetopt$(OBJ_EXT) \
		options$(OBJ_EXT)
	$(DC) -c $(DFLAGS) $(DTESTFLAGS) di.d

digetopt$(OBJ_EXT):	digetopt.d config$(OBJ_EXT)
	$(DC) -c $(DFLAGS) $(DTESTFLAGS) digetopt.d

dilocale$(OBJ_EXT):	dilocale.d config$(OBJ_EXT)
	$(DC) -c $(DFLAGS) $(DTESTFLAGS) dilocale.d

diquota$(OBJ_EXT):	diquota.d config$(OBJ_EXT) diskpart$(OBJ_EXT)
	$(DC) -c $(DFLAGS) $(DTESTFLAGS) diquota.d

diskpart$(OBJ_EXT):	diskpart.d config$(OBJ_EXT)
	$(DC) -c $(DFLAGS) $(DTESTFLAGS) diskpart.d

display$(OBJ_EXT):	display.d config$(OBJ_EXT) dilocale$(OBJ_EXT) \
		diskpart$(OBJ_EXT) dispopts$(OBJ_EXT) \
		options$(OBJ_EXT)
	$(DC) -c $(DFLAGS) $(DTESTFLAGS) display.d

dispopts$(OBJ_EXT):	dispopts.d config$(OBJ_EXT)
	$(DC) -c $(DFLAGS) $(DTESTFLAGS) dispopts.d

options$(OBJ_EXT):	options.d config$(OBJ_EXT) \
		dispopts$(OBJ_EXT) digetopt$(OBJ_EXT)
	$(DC) -c $(DFLAGS) $(DTESTFLAGS) options.d

