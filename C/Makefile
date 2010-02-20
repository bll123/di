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
CFLAGS = -O
LDFLAGS =

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
TARGET = $(INSTALL_BIN_DIR)/$(PROG)
MTARGET = $(INSTALL_BIN_DIR)/$(MPROG)

#
# additional flags/libraries
#
OTHERFLAGS = -DPROG=\"$(PROG)\" -DMPROG=\"$(MPROG)\" \
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
MANPATH = $(INSTALL_DIR)/share/man
MANDIR = $(MANPATH)/man1
MANTARGET = $(PROG).1
MANPERM = 644

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
MSGFMT = msgfmt
MV = mv
RM = rm
RPMBUILD = rpmbuild
SED = sed
TEST = test

###

# have to get various environment variables set up.
all:
	$(SHELL) ./Build

di-all:	di-prog

di-prog:	config.h $(PROG)$(EXE_EXT) hasnls$(EXE_EXT)

# for regression testing
di-all-test:	pretests.done tests.done

pretests.done:
	echo "## running pretests"
	$(SHELL) ./runtests.sh pretests
	touch pretests.done

tests.done:
	echo "## running tests"
	$(SHELL) ./runtests.sh tests
	touch tests.done

config.h:	features/cflags.sh mkconfig.pl mkconfig.sh features/mkconfig.dat
	@-$(RM) -f config.h mkconfig.cache tests.done
	$(SHELL) ./Build config.h

wconfig.h:	config.win
	@-$(RM) -f config.h mkconfig.cache tests.done
	$(CP) -f config.win config.h

# This was tested using vc++ 5.0.
windows:	wconfig.h
	$(MAKE) -f Makefile \
		CC=cl LD=cl EXE_EXT=".exe" OBJ_EXT=".obj" \
		CFLAGS="$(CFLAGS) -nologo -O2 \
		-DDEFAULT_FORMAT=\\\"sMbuvpT\\\"" \
		LDFLAGS="-nologo -O2" $(PROG).exe

# This was tested using cygwin
windows-gcc:	wconfig.h
	$(MAKE) -f Makefile \
		CC=gcc LD=gcc EXE_EXT=".exe" OBJ_EXT=".o" \
		CFLAGS="$(CFLAGS) -g -O2 \
		-DDEFAULT_FORMAT=\\\"sMbuvpT\\\"" \
		LDFLAGS="-g -O2" $(PROG).exe

os2-gcc:
	$(MAKE) -f Makefile \
		CC=gcc LD=gcc EXE_EXT=".exe" OBJ_EXT=".o" \
		CFLAGS="$(CFLAGS) -g -O2 \
		-DDEFAULT_FORMAT=\\\"sMbuvpT\\\"" \
		LDFLAGS="-g -O2 -Zexe" $(PROG).exe

####################

clean:
	-$(RM) -rf $(PROG)$(EXE_EXT) $(MPROG)$(EXE_EXT) \
		*.o *.obj hasnls$(EXE_EXT) mkconfig.log \
		tests.done _tmp_mkconfig \
		>/dev/null 2>&1

distclean:
	$(MAKE) -f Makefile clean
	-$(RM) -f config.h mkconfig.cache object_ext \
		pretests.done reqlibs.txt \
		>/dev/null 2>&1

realclean:
	$(MAKE) -f Makefile distclean

install:	install-prog install-man

build-po:
	-(cd po;for i in *.po; do \
		j=`echo $$i | $(SED) 's,\\.po$$,,'`; \
		$(MSGFMT) -o $$j.mo $$i; \
	done)

install-po: build-po
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

check-nls:	hasnls$(EXE_EXT)
	@-./hasnls$(EXE_EXT) && echo "# NLS enabled"
	@-./hasnls$(EXE_EXT) || echo "# NLS not enabled"

install-prog:	$(PROG)$(EXE_EXT) hasnls$(EXE_EXT)
	-$(TEST) -d $(INSTALL_DIR) || $(MKDIR) $(INSTALL_DIR)
	-$(TEST) -d $(INSTALL_BIN_DIR) || $(MKDIR) $(INSTALL_BIN_DIR)
	-$(TEST) -f $(TARGET) && $(MV) -f $(TARGET) $(TARGET).old
	-$(TEST) -f $(MTARGET) && $(MV) -f $(MTARGET) $(MTARGET).old
	$(CP) -f $(PROG)$(EXE_EXT) $(TARGET)
	-$(LN) -s $(TARGET) $(MTARGET)
	-$(RM) -f $(TARGET).old $(MTARGET).old > /dev/null 2>&1
	@-./hasnls$(EXE_EXT) && echo "# NLS enabled"
	@-./hasnls$(EXE_EXT) || echo "# NLS not enabled"
	-./hasnls$(EXE_EXT) && \
		$(MAKE) INST_LOCALEDIR="$(INST_LOCALEDIR)"\
		MSGFMT="$(MSGFMT)" install-po

install-man:
	-$(TEST) -d $(MANPATH) || $(MKDIR) -p $(MANPATH)
	-$(TEST) -d $(MANDIR) || $(MKDIR) -p $(MANDIR)
	$(CP) -f di.1 $(MANDIR)/$(MANTARGET)
	$(CHMOD) $(MANPERM) $(MANDIR)/$(MANTARGET)

installperms:
	$(CHOWN) $(USER) $(TARGET)
	$(CHGRP) $(GROUP) $(TARGET)
	$(CHMOD) $(INSTPERM) $(TARGET)

###

current-files:
	@-$(RM) -f features/* > /dev/null 2>&1
	@-$(RM) -f po/* > /dev/null 2>&1
	@-$(RM) -f tests/* > /dev/null 2>&1
	@-$(RM) -f pretests/* > /dev/null 2>&1
	@-$(CP) -f po.base/*.po po > /dev/null 2>&1
	@-$(CP) -f features.base/*.sh features.base/*.dat features \
		> /dev/null 2>&1
	@-$(CP) -f pretests.base/* pretests > /dev/null 2>&1
	@-$(CP) -f tests.base/* tests > /dev/null 2>&1
	@-$(CP) -f mkconfig.units.base/* mkconfig.units > /dev/null 2>&1

tar:	Build LICENSE MANIFEST \
		Makefile README config.bsd43.cc config.bsd43.gcc config.win \
		di.1 di.c di.h dimntopt.h didiskutil.c digetentries.c \
		digetinfo.c realloc.c strdup.c strstr.c trimchar.c \
		features getopt.c hasnls.c \
		mkconfig.pl mkconfig.sh mkconfig.units pretests po \
		runtests.sh strdup.c strstr.c tests version.h
	$(MAKE) -f Makefile current-files
	$(SHELL) mktar.sh $?

testrpmbuild:
	-$(TEST) -d ./rpmbuild && rm -rf ./rpmbuild
	$(MKDIR) -p ./rpmbuild/SOURCES
	$(CP) -f $(DI_DIR)/di-$(DI_VERSION).tar.gz ./rpmbuild/SOURCES
	$(RPMBUILD) --define="_topdir `pwd`/rpmbuild" -ba di.spec
	$(TEST) -f ./rpmbuild/RPMS/i386/di-$(DI_VERSION)-1.i386.rpm || exit 1
	$(TEST) -f ./rpmbuild/SRPMS/di-$(DI_VERSION)-1.src.rpm || exit 1
	$(TEST) -d ./rpmbuild && rm -rf ./rpmbuild

###

$(PROG)$(EXE_EXT):	di$(OBJ_EXT) didiskutil$(OBJ_EXT) \
		digetentries$(OBJ_EXT) digetinfo$(OBJ_EXT) \
		realloc$(OBJ_EXT) strdup$(OBJ_EXT) \
		strstr$(OBJ_EXT) trimchar$(OBJ_EXT) getopt$(OBJ_EXT)
	@$(TEST) ! -f reqlibs.txt || echo "*** Using libs: `$(CAT) reqlibs.txt`"
	$(CC) -o $(PROG)$(EXE_EXT) \
		$(CFLAGS) $(LDFLAGS) \
		di$(OBJ_EXT) didiskutil$(OBJ_EXT) digetinfo$(OBJ_EXT) \
		digetentries$(OBJ_EXT) realloc$(OBJ_EXT) \
		strdup$(OBJ_EXT) strstr$(OBJ_EXT) \
		trimchar$(OBJ_EXT) getopt$(OBJ_EXT) \
		$(LIBS) \
		`$(TEST) ! -f reqlibs.txt || $(CAT) reqlibs.txt`

hasnls$(EXE_EXT):	hasnls$(OBJ_EXT)
	$(CC) -o hasnls$(EXE_EXT) \
		$(CFLAGS) $(LDFLAGS) \
		hasnls$(OBJ_EXT)

###

di$(OBJ_EXT):		di.c config.h di.h version.h
	$(CC) -c $(CFLAGS) $(OTHERFLAGS) di.c

digetinfo$(OBJ_EXT):	digetinfo.c config.h di.h dimntopt.h
	$(CC) -c $(CFLAGS) digetinfo.c

didiskutil$(OBJ_EXT):	didiskutil.c config.h di.h dimntopt.h
	$(CC) -c $(CFLAGS) didiskutil.c

digetentries$(OBJ_EXT):	digetentries.c config.h di.h dimntopt.h
	$(CC) -c $(CFLAGS) digetentries.c

getopt$(OBJ_EXT):	getopt.c config.h
	$(CC) -c $(CFLAGS) getopt.c

hasnls$(OBJ_EXT):	hasnls.c config.h
	$(CC) -c $(CFLAGS) $(OTHERFLAGS) hasnls.c

realloc$(OBJ_EXT):	realloc.c config.h di.h
	$(CC) -c $(CFLAGS) realloc.c

strdup$(OBJ_EXT):	strdup.c config.h di.h
	$(CC) -c $(CFLAGS) strdup.c

strstr$(OBJ_EXT):	strstr.c config.h di.h
	$(CC) -c $(CFLAGS) strstr.c

trimchar$(OBJ_EXT):	trimchar.c config.h di.h
	$(CC) -c $(CFLAGS) trimchar.c


