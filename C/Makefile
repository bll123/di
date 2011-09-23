#
#  di makefile
#
#  Copyright 2001-2010 Brad Lanam Walnut Creek CA, USA
#
#  $Id$
#

CC = cc
# for tests.done
DC = gdc

SHELL = /bin/sh
OBJ_EXT = .o
EXE_EXT =
MAKE = make

###
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
# installation options
#
prefix = /usr/local
LOCALEDIR = $(prefix)/share/locale
PROG = di
MPROG = mi

###
# additional flags/libraries
#
DI_CPPFLAGS = -DPROG=\"$(PROG)\" -DMPROG=\"$(MPROG)\" \
	-DDI_LOCALE_DIR=\"$(LOCALEDIR)\"

###
# mkconfig variables

MKCONFIGPATH = ../mkconfig
MKCONFIG_TYPE = sh
MKC_REQLIB = mkconfig.reqlibs

###
# main

# have to get various environment variables set up.

.PHONY: all
all:	all-sh

.PHONY: all-sh
all-sh:	di.env
	. ./di.env;$(MAKE) -e MKCONFIG_TYPE=sh di-programs

.PHONY: all-perl
all-perl:	di.env
	. ./di.env;$(MAKE) -e MKCONFIG_TYPE=perl di-programs

###
# environment

di.env:	features/env.dat
	@-$(RM) -f di.env tests.done
	CC=$(CC) $(_MKCONFIG_SHELL) $(MKCONFIGPATH)/mkconfig.sh features/env.dat

###
# specific builds

# This was tested using vc++ 5.0. (many years ago)
# seriously out of date; config.win is incorrect for this build.
.PHONY: windows
windows:
	@$(MAKE) ../dioptions.dat
	copy /y NUL: di.env
	-del config.h
	copy /y /b NUL:+config.win config.h
	copy /y NUL: $(MKC_REQLIB)
	$(MAKE) \
		CC=cl LD=cl EXE_EXT=".exe" OBJ_EXT=".obj" \
		CFLAGS="$(CFLAGS) -nologo -O2 \
		-DDEFAULT_FORMAT=\\\"MbuvpT\\\"" \
		LDFLAGS="-nologo -O2" di-programs

# This was tested using cygwin
.PHONY: windows-gcc
windows-gcc:
	@$(MAKE) ../dioptions.dat
	@echo ':' > di.env;chmod a+rx di.env
	@-$(RM) -f config.h mkconfig.cache mkconfig_*.vars tests.done
	$(CP) -f config.win config.h
	@echo '-lintl' > $(MKC_REQLIB)
	$(MAKE) \
		CC=gcc LD=gcc EXE_EXT=".exe" OBJ_EXT=".o" \
		CFLAGS="$(CFLAGS) -g -O2 \
		-DDEFAULT_FORMAT=\\\"MbuvpT\\\"" \
		LDFLAGS="-g -O2" di-programs

.PHONY: windows-mingw
windows-mingw:
	copy /y /b NUL:+..\features\dioptions.dat ..\dioptions.dat
	copy /y NUL: di.env
	-del config.h
	copy /y /b NUL:+config.mingw config.h
	copy /y NUL: $(MKC_REQLIB)
	$(MAKE) \
		CC=mingw32-gcc LD=mingw32-gcc \
		EXE_EXT=".exe" OBJ_EXT=".o" \
		CFLAGS="$(CFLAGS) -g -O2 \
		-DDEFAULT_FORMAT=\\\"MbuvpT\\\"" \
		LDFLAGS="-g -O2" mingw-$(PROG).exe

.PHONY: windows-dmc
windows-dmc:
	copy /y /b NUL:+..\features\dioptions.dat ..\dioptions.dat
	copy /y NUL: di.env
	-del config.h
	copy /y /b NUL:+config.dmc config.h
	copy /y NUL: $(MKC_REQLIB)
	$(MAKE) \
		CC=dmc LD=dmc \
		EXE_EXT=".exe" OBJ_EXT=".o" \
		CFLAGS="$(CFLAGS) -o \
		-DDEFAULT_FORMAT=\\\"MbuvpT\\\"" \
		LDFLAGS="-o" dmc-target

.PHONY: os2-gcc
os2-gcc:
	@echo ':' > di.env;chmod a+rx di.env
	$(MAKE) MKCONFIG_TYPE=perl \
		CC=gcc LD=gcc EXE_EXT=".exe" OBJ_EXT=".o" \
		CFLAGS="$(CFLAGS) -g -O2 \
		LDFLAGS="-g -O2 -Zexe" $(PROG).exe

.PHONY: bsd43-cc
bsd43-cc:
	@$(MAKE) ../dioptions.dat
	@echo ':' > di.env;chmod a+rx di.env
	@-$(RM) -f config.h mkconfig.cache mkconfig_*.vars tests.done
	$(CP) config.bsd43.cc config.h
	@touch $(MKC_REQLIB)
	$(MAKE) _MKCONFIG_SHELL=$(MKCONFIG_SHELL) \
		CC=cc LD=cc EXE_EXT="" OBJ_EXT=".o" \
		CFLAGS="$(CFLAGS) -O" \
		LDFLAGS="-O" di-programs

.PHONY: bsd43-gcc
bsd43-gcc:
	@$(MAKE) ../dioptions.dat
	@echo ':' > di.env;chmod a+rx di.env
	@-$(RM) -f config.h mkconfig.cache mkconfig_*.vars tests.done
	$(CP) config.bsd43.gcc config.h
	@touch $(MKC_REQLIB)
	$(MAKE) _MKCONFIG_SHELL=$(MKCONFIG_SHELL) \
		CC=gcc LD=gcc EXE_EXT="" OBJ_EXT=".o" \
		CFLAGS="$(CFLAGS) -O2" \
		LDFLAGS="-O2" di-programs

###
# cleaning

# leaves:
#   config.h
#   ../dioptions.dat, tests.done, test_di, di.env
.PHONY: clean
clean:
	@-$(RM) -rf $(PROG) $(MPROG) \
		mingw-$(PROG).exe dmc-$(PROG).exe $(PROG).exe $(MPROG).exe \
		*.o *.obj mkconfig.log \
		tests.done _tmp_mkconfig tests.d/chksh* \
		mkconfig.cache mkconfig_*.vars \
		>/dev/null 2>&1

# leaves:
#   _mkconfig_runtests, _tmp_mkconfig, ../dioptions.dat
#   tests.done, test_di
.PHONY: realclean
realclean:
	@$(MAKE) clean > /dev/null 2>&1
	@-$(RM) -rf config.h di.env mkconfig.reqlibs \
		>/dev/null 2>&1

# leaves:
#   ../dioptions.dat
.PHONY: distclean
distclean:
	@$(MAKE) realclean > /dev/null 2>&1
	@-$(RM) -rf tests.done test_di \
		_mkconfig_runtests _tmp_mkconfig \
		>/dev/null 2>&1

###
# installation

.PHONY: install
install:	di-programs di.env
	. ./di.env;cd ..;$(MAKE) -e prefix=$(prefix) LOCALEDIR=$(LOCALEDIR) install

###
# programs

.PHONY: di-programs
di-programs:	$(PROG)$(EXE_EXT)

###
# configuration file

../dioptions.dat:	../features/dioptions.dat
	cd ../;$(MAKE) dioptions.dat

config.h:	di.env ../dioptions.dat features/mkconfig.dat
	@-$(RM) -f config.h tests.done
	@if [ "$(DI_NO_NLS)" != "" ]; then \
		echo "*** User requested no NLS"; \
		$(MKCONFIGPATH)/mkcsetopt.sh -o ../dioptions.dat NLS F; fi
	@if [ "$(MKCONFIG_TYPE)" = "sh" -o "$(MKCONFIG_TYPE)" = "" ]; then \
		. ./di.env;$(_MKCONFIG_SHELL) \
		$(MKCONFIGPATH)/mkconfig.sh \
		features/mkconfig.dat; fi
	@if [ "$(MKCONFIG_TYPE)" = "perl" ]; then \
		. ./di.env;perl \
		$(MKCONFIGPATH)/mkconfig.pl \
		features/mkconfig.dat; fi

$(MKC_REQLIB):	di.env config.h
	$(_MKCONFIG_SHELL) $(MKCONFIGPATH)/mkreqlib.sh config.h


###
# executables

OBJECTS = di$(OBJ_EXT) didiskutil$(OBJ_EXT) \
		digetentries$(OBJ_EXT) digetinfo$(OBJ_EXT) \
		diquota$(OBJ_EXT) \
		realloc$(OBJ_EXT) strdup$(OBJ_EXT) \
		strstr$(OBJ_EXT) trimchar$(OBJ_EXT) getopt$(OBJ_EXT)

$(PROG)$(EXE_EXT):	$(MKC_REQLIB) $(OBJECTS)
	@echo "*** Using libs: `$(CAT) $(MKC_REQLIB)`"
	$(CC) -o $(PROG)$(EXE_EXT) \
		$(LDFLAGS) $(OBJECTS) $(LIBS) \
		`$(CAT) $(MKC_REQLIB)`

mingw-$(PROG)$(EXE_EXT):	$(OBJECTS)
	$(CC) -o mingw-$(PROG)$(EXE_EXT) \
		$(CFLAGS) $(LDFLAGS) $(OBJECTS) $(LIBS)

.PHONY: dmc-target
dmc-target:	$(OBJECTS) dmc-$(PROG)$(EXE_EXT)

dmc-$(PROG)$(EXE_EXT):
	$(CC) -o$(PROG)$(EXE_EXT) \
		$(LDFLAGS) $(OBJECTS) $(LIBS)

###
# objects

di$(OBJ_EXT):		di.c config.h di.h version.h
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $(DI_CPPFLAGS) di.c

digetinfo$(OBJ_EXT):	digetinfo.c config.h di.h dimntopt.h
	$(CC) -c $(CFLAGS) $(CPPFLAGS) digetinfo.c

didiskutil$(OBJ_EXT):	didiskutil.c config.h di.h dimntopt.h
	$(CC) -c $(CFLAGS) $(CPPFLAGS) didiskutil.c

digetentries$(OBJ_EXT):	digetentries.c config.h di.h dimntopt.h
	$(CC) -c $(CFLAGS) $(CPPFLAGS) digetentries.c

diquota$(OBJ_EXT):	diquota.c config.h di.h
	$(CC) -c $(CFLAGS) $(CPPFLAGS) diquota.c

getopt$(OBJ_EXT):	getopt.c config.h
	$(CC) -c $(CFLAGS) $(CPPFLAGS) getopt.c

realloc$(OBJ_EXT):	realloc.c config.h di.h
	$(CC) -c $(CFLAGS) $(CPPFLAGS) realloc.c

strdup$(OBJ_EXT):	strdup.c config.h di.h
	$(CC) -c $(CFLAGS) $(CPPFLAGS) strdup.c

strstr$(OBJ_EXT):	strstr.c config.h di.h
	$(CC) -c $(CFLAGS) $(CPPFLAGS) strstr.c

trimchar$(OBJ_EXT):	trimchar.c config.h di.h
	$(CC) -c $(CFLAGS) $(CPPFLAGS) trimchar.c

###
# regression testing

.PHONY: all-test
all-test:	tests.done

tests.done: $(MKCONFIGPATH)/runtests.sh
	@echo "## running tests"
	CC=$(CC) DC=$(DC) $(_MKCONFIG_SHELL) \
		$(MKCONFIGPATH)/runtests.sh ./tests.d
	touch tests.done

# needs environment
.PHONY: testrpmbuild
testrpmbuild:
	-$(TEST) -d ./rpmbuild && rm -rf ./rpmbuild
	$(MKDIR) -p ./rpmbuild/SOURCES
	$(MKDIR) -p ./rpmbuild/BUILD
	$(CP) -f $(DI_DIR)/di-$(DI_VERSION).tar.gz ./rpmbuild/SOURCES
	$(RPMBUILD) --define="_topdir `pwd`/rpmbuild" -ba ../di.spec
	$(TEST) -f ./rpmbuild/RPMS/$(MARCH)/di-$(DI_VERSION)-1.$(MARCH).rpm || exit 1
	$(TEST) -f ./rpmbuild/SRPMS/di-$(DI_VERSION)-1.src.rpm || exit 1
	$(TEST) -d ./rpmbuild && rm -rf ./rpmbuild

# needs environment
.PHONY: rtest-env
rtest-env:
	@echo "$(_MKCONFIG_SYSTYPE)"
	@echo "$(_MKCONFIG_SYSREV)"
	@echo "$(_MKCONFIG_SYSARCH)"
	@echo "$(CC)"
	@echo "$(_MKCONFIG_USING_GCC)"
	@echo "$(CFLAGS)"
	@echo "$(LDFLAGS)"
	@echo "$(LIBS)"
	@echo "$(OBJ_EXT)"
	@echo "$(EXE_EXT)"
	@echo "$(XMSGFMT)"

