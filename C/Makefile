#
#  di makefile
#
#  Copyright 2001-2010 Brad Lanam Walnut Creek CA, USA
#
#  $Id$
#

SHELL = /bin/sh
OBJ_EXT = .o
EXE_EXT =
CC = cc
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

#
# installation options
#
prefix = /usr/local
LOCALEDIR = $(prefix)/share/locale
#
INSTALL_DIR = $(prefix)
INSTALL_BIN_DIR = $(INSTALL_DIR)/bin
INST_LOCALEDIR = $(INSTALL_DIR)/share/locale
PROG = di
MPROG = mi
TARGET = $(INSTALL_BIN_DIR)/$(PROG)$(EXE_EXT)
MTARGET = $(INSTALL_BIN_DIR)/$(MPROG)$(EXE_EXT)

#
# additional flags/libraries
#
DI_CPPFLAGS = -DPROG=\"$(PROG)\" -DMPROG=\"$(MPROG)\" \
	-DDI_LOCALE_DIR=\"$(LOCALEDIR)\"

# if you need permissions other than the default,
# edit these, and do a "make installperm".
USER = root
GROUP = bin
INSTPERM = 4111   # install suid if your system has a mount table only root
#                   can read.  For SysV.4 and Solaris, the mount command may
#                   reset the permissions of /etc/mnttab.

#
# simple man page installation
#
DI_MANINSTDIR = $(INSTALL_DIR)/share/man
DI_MANDIR = $(DI_MANINSTDIR)/man1
MAN_TARGET = $(PROG).1
MANPERM = 644

###
# mkconfig variables

MKCONFIGPATH = ./mkconfig
MKCONFIG_TYPE = sh
MKC_REQLIB = mkconfig.reqlibs

###
# main

# have to get various environment variables set up.

all:	all-sh

all-sh:	di.env
	. ./di.env;$(MAKE) -e MKCONFIG_TYPE=sh di-programs

all-perl:	di.env
	. ./di.env;$(MAKE) -e MKCONFIG_TYPE=perl di-programs


###
# environment

di.env:	$(MKCONFIGPATH)/mkconfig.sh features/env.dat
	@-$(RM) -f di.env tests.done
	CC=$(CC) $(_MKCONFIG_SHELL) \
		$(MKCONFIGPATH)/mkconfig.sh features/env.dat

###
# packaging

tar:
	@-rm -f $(MKCONFIGPATH)/*.tar.gz \
		di-[0-9].[0-9][0-9][a-z].tar.gz > /dev/null 2>&1
	cd $(MKCONFIGPATH);$(MAKE) tar
	./mktar.sh

###
# specific builds

# This was tested using vc++ 5.0. (many years ago)
# seriously out of date; config.win is incorrect for this build.
windows:
	@$(MAKE) dioptions.dat
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
windows-gcc:
	@$(MAKE) dioptions.dat
	@echo ':' > di.env;chmod a+rx di.env
	@-$(RM) -f config.h mkconfig.cache mkconfig_*.vars tests.done
	$(CP) -f config.win config.h
	@echo '-lintl' > $(MKC_REQLIB)
	$(MAKE) \
		CC=gcc LD=gcc EXE_EXT=".exe" OBJ_EXT=".o" \
		CFLAGS="$(CFLAGS) -g -O2 \
		-DDEFAULT_FORMAT=\\\"MbuvpT\\\"" \
		LDFLAGS="-g -O2" di-programs

windows-mingw:
	@$(MAKE) dioptions.dat
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

os2-gcc:
	@echo ':' > di.env;chmod a+rx di.env
	$(MAKE) MKCONFIG_TYPE=perl \
		CC=gcc LD=gcc EXE_EXT=".exe" OBJ_EXT=".o" \
		CFLAGS="$(CFLAGS) -g -O2 \
		LDFLAGS="-g -O2 -Zexe" $(PROG).exe

bsd43-cc:
	@$(MAKE) dioptions.dat
	@echo ':' > di.env;chmod a+rx di.env
	@-$(RM) -f config.h mkconfig.cache mkconfig_*.vars tests.done
	$(CP) config.bsd43.cc config.h
	@touch $(MKC_REQLIB)
	$(MAKE) _MKCONFIG_SHELL=$(MKCONFIG_SHELL) \
		CC=cc LD=cc EXE_EXT="" OBJ_EXT=".o" \
		CFLAGS="$(CFLAGS) -O" \
		LDFLAGS="-O" di-programs

bsd43-gcc:
	@$(MAKE) dioptions.dat
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
#   config.h, di.env, mkconfig.reqlibs
#   mkconfig.cache, mkconfig_*.vars, 
#   dioptions.dat, pretests.done, test_di
clean:
	@-$(RM) -rf $(PROG) $(MPROG) \
		mingw-$(PROG).exe $(PROG).exe $(MPROG).exe \
		*.o *.obj mkconfig.log \
		tests.done _tmp_mkconfig \
		>/dev/null 2>&1

# leaves:
#   _mkconfig_runtests, _tmp_mkconfig, dioptions.dat
#   pretests.done, test_di
realclean:
	@$(MAKE) clean
	@-$(RM) -rf config.h di.env mkconfig.reqlibs \
		mkconfig.cache mkconfig_*.vars \

distclean:
	@$(MAKE) realclean
	@-$(RM) -rf pretests.done test_di \
		_mkconfig_runtests _tmp_mkconfig dioptions.dat
		>/dev/null 2>&1

###
# installation

install:	di.env
	. ./di.env; \
		$(MAKE) -e install-prog install-man

build-po:	di.env
	-. ./di.env; \
		(cd po;for i in *.po; do \
		j=`echo $$i | $(SED) 's,\\.po$$,,'`; \
		$${XMSGFMT} -o $$j.mo $$i; \
	done)

install-po: 	build-po
	-$(TEST) -d $(INST_LOCALEDIR) || $(MKDIR) -p $(INST_LOCALEDIR)
	-(cd po;for i in *.po; do \
		j=`echo $$i | $(SED) 's,\\.po$$,,'`; \
		$(TEST) -d $(INST_LOCALEDIR)/$$j || \
			$(MKDIR) $(INST_LOCALEDIR)/$$j; \
		$(TEST) -d $(INST_LOCALEDIR)/$$j/LC_MESSAGES || \
			$(MKDIR) $(INST_LOCALEDIR)/$$j/LC_MESSAGES; \
		$(CP) -f $$j.mo $(INST_LOCALEDIR)/$$j/LC_MESSAGES/di.mo; \
		$(RM) -f $$j.mo; \
		done)

install-prog:	di.env di-programs
	-$(TEST) -d $(INSTALL_DIR) || $(MKDIR) $(INSTALL_DIR)
	-$(TEST) -d $(INSTALL_BIN_DIR) || $(MKDIR) $(INSTALL_BIN_DIR)
	-$(TEST) -f $(TARGET) && $(MV) -f $(TARGET) $(TARGET).old
	-$(TEST) -f $(MTARGET) && $(MV) -f $(MTARGET) $(MTARGET).old
	$(CP) -f $(PROG)$(EXE_EXT) $(TARGET)
	-$(LN) -s $(PROG)$(EXE_EXT) $(MTARGET)
	-$(RM) -f $(TARGET).old $(MTARGET).old > /dev/null 2>&1
	-grep '^#define _enable_nls 1' config.h && \
		(. ./di.env; $(MAKE) -e INST_LOCALEDIR="$(INST_LOCALEDIR)" \
		install-po)

install-man:
	-$(TEST) -d $(DI_MANINSTDIR) || $(MKDIR) -p $(DI_MANINSTDIR)
	-$(TEST) -d $(DI_MANDIR) || $(MKDIR) -p $(DI_MANDIR)
	$(CP) -f di.1 $(DI_MANDIR)/$(MAN_TARGET)
	$(CHMOD) $(MANPERM) $(DI_MANDIR)/$(MAN_TARGET)

installperms:
	$(CHOWN) $(USER) $(TARGET)
	$(CHGRP) $(GROUP) $(TARGET)
	$(CHMOD) $(INSTPERM) $(TARGET)

###
# programs

di-programs:	$(PROG)$(EXE_EXT)

###
# configuration file

dioptions.dat:	features/dioptions.dat
	@cp features/dioptions.dat dioptions.dat
	@chmod u+w dioptions.dat
	
config.h:	di.env dioptions.dat features/mkconfig.dat
	@-$(RM) -f config.h tests.done
	@if [ "$(DI_NO_NLS)" != "" ]; then \
		echo "*** User requested no NLS"; \
		$(MKCONFIGPATH)/mkcsetopt.sh -o dioptions.dat NLS F; fi
	@if [ "$(MKCONFIG_TYPE)" = "sh" -o "$(MKCONFIG_TYPE)" = "" ]; then \
		. ./di.env;$(_MKCONFIG_SHELL) \
		$(MKCONFIGPATH)/mkconfig.sh \
		features/mkconfig.dat; fi
	@if [ "$(MKCONFIG_TYPE)" = "perl" ]; then \
		. ./di.env;perl \
		$(MKCONFIGPATH)/mkconfig.pl \
		features/mkconfig.dat; fi

$(MKC_REQLIB)	di.env config.h
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
		$(CFLAGS) $(LDFLAGS) $(OBJECTS) $(LIBS) \
		`$(CAT) $(MKC_REQLIB)`

mingw-$(PROG)$(EXE_EXT):	$(OBJECTS)
	$(CC) -o mingw-$(PROG)$(EXE_EXT) \
		$(CFLAGS) $(LDFLAGS) $(OBJECTS) $(LIBS)

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

di-all-test:	pretests.done tests.done

pretests.done:
	cd $(MKCONFIGPATH); $(MAKE) CC=$(CC) tests.done

tests.done: $(MKCONFIGPATH)/runtests.sh
	@echo "## running tests"
	CC=$(CC) $(_MKCONFIG_SHELL) $(MKCONFIGPATH)/runtests.sh tests.d
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

