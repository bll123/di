#
#  di makefile
#
#  Copyright 2001-2004 Brad Lanam
#
#  $Id$
#  $Source$
#  @(#)Makefile	1.25
#
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
INSTDIR = $(prefix)/bin
LOCALEDIR = $(prefix)/lib/di
PROG = di
MPROG = mi
TARGET = $(INSTDIR)/$(PROG)
MTARGET = $(INSTDIR)/$(MPROG)

#
# additional flags/libraries
#
OTHERFLAGS = -DPROG=\"$(PROG)\" -DMPROG=\"$(MPROG)\" \
	-DDI_LOCALE_DIR=\"$(LOCALEDIR)\"

# if you need permissions other than the default,
# edit these, and do a "make installperm".
USER = root
GROUP = bin
#INSTPERM = 111   # change USER to bin also...
INSTPERM = 4111   # install suid if your system has a mount table only root
#                   can read.  For SysV.4 and Solaris, the mount command may
#                   reset the permissions of /etc/mnttab.

#
# simple man page installation
#
MANPATH = $(prefix)/man
MANDIR = $(MANPATH)/man1
MANTARGET = $(PROG).1
MANPERM = 644

#
# common programs
#
LN = ln
RM = rm
CP = cp
MV = mv
TEST = test
CHOWN = chown
CHGRP = chgrp
CHMOD = chmod
TOUCH = touch
MSGFMT = msgfmt
MKDIR = mkdir
SED = sed

###

all:	config.h $(PROG)$(EXE_EXT)

config.h:	features/di.iffe features/cflags.sh
	@-$(RM) -f config.h
	$(SHELL) ./bin/iffe -c "$(CC) $(CFLAGS)" -u -v -o config.h \
		run features/di.iffe

#config.h:	features/di.iffe features/cflags.sh
#	$(SHELL) ./bin/iffe -d 2 -c "$(CC) $(CFLAGS)" -u -v -o config.h \
#		run features/di.iffe 2>&1 | tee w

wconfig.h:	config.win95
	$(CP) -f config.win95 config.h

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

####################

all:	$(PROG)$(EXE_EXT)
	-$(LN) -f $(PROG)$(EXE_EXT) $(MPROG)$(EXE_EXT)

objclean:
	-$(RM) -rf *.o *.obj

clean:
	-$(RM) -f $(PROG)$(EXE_EXT) $(MPROG)$(EXE_EXT) \
		*.o *.obj a.out \
		>/dev/null 2>&1

distclean:
	$(MAKE) -f Makefile clean
	-$(RM) -f config.h object_ext \
		di$(EXE_EXT) diskinfo$(EXE_EXT) \
		>/dev/null 2>&1

realclean:
	$(MAKE) -f Makefile \
		distclean

install:	install-prog install-man

build-po:
	-(cd po;for i in *.po; do \
		j=`echo $$i | $(SED) 's,\\.po$$,,'`; \
		$(MSGFMT) -o $$j.mo $$i; \
	done)

install-po: build-po
	-$(TEST) -d $(LOCALEDIR) || $(MKDIR) -p $(LOCALEDIR)
	-$(TEST) -d $(LOCALEDIR)/locale || $(MKDIR) -p $(LOCALEDIR)/locale
	-(cd po;for i in *.po; do \
		j=`echo $$i | $(SED) 's,\\.po$$,,'`; \
		$(TEST) -d $(LOCALEDIR)/locale/$$j || \
			$(MKDIR) $(LOCALEDIR)/locale/$$j; \
		$(TEST) -d $(LOCALEDIR)/locale/$$j/LC_MESSAGES || \
			$(MKDIR) $(LOCALEDIR)/locale/$$j/LC_MESSAGES; \
		$(CP) -f $$j.mo $(LOCALEDIR)/locale/$$j/LC_MESSAGES/di.mo; \
        $(RM) -f $$j.mo; \
	done)

install-prog:	$(PROG)$(EXE_EXT)
	-$(TEST) -d $(prefix) || $(MKDIR) $(prefix)
	-$(TEST) -d $(INSTDIR) || $(MKDIR) $(INSTDIR)
	-$(TEST) -f $(TARGET) && $(MV) -f $(TARGET) $(TARGET).old
	-$(TEST) -f $(MTARGET) && $(MV) -f $(MTARGET) $(MTARGET).old
	$(CP) -f $(PROG)$(EXE_EXT) $(TARGET)
	-$(LN) -f $(TARGET) $(MTARGET)
	-$(RM) -f $(TARGET).old $(MTARGET).old

install-man:
	-$(TEST) -d $(MANPATH) || $(MKDIR) $(MANPATH)
	-$(TEST) -d $(MANDIR) || $(MKDIR) $(MANDIR)
	$(CP) -f di.1 $(MANDIR)/$(MANTARGET)
	$(CHMOD) $(MANPERM) $(MANDIR)/$(MANTARGET)

installperms:
	$(CHOWN) $(USER) $(TARGET)
	$(CHGRP) $(GROUP) $(TARGET)
	$(CHMOD) $(INSTPERM) $(TARGET)

###

current-files:
	-$(RM) -f features/* > /dev/null 2>&1
	$(CP) -f po.base/*.po po
	$(CP) -f features.base/*.sh features.base/*.iffe features

tar:	Build LICENSE LICENSE.iffe MANIFEST \
		Makefile README bin config.win95 \
		di.1 di.c \
		di.h dilib.c features getopt.c \
		po version.h
	$(MAKE) -f Makefile current-files
	$(SHELL) mktar.sh $?

testtar:	Build LICENSE LICENSE.iffe MANIFEST \
		Makefile README bin config.win95 \
		di.1 di.c \
		di.h dilib.c features getopt.c \
		po version.h
	$(MAKE) -f Makefile current-files
	$(SHELL) mktar.sh $?

###

$(PROG)$(EXE_EXT):	di$(OBJ_EXT) dilib$(OBJ_EXT) \
		getopt$(OBJ_EXT)
	$(CC) -o $(PROG)$(EXE_EXT) \
		$(CFLAGS) $(LDFLAGS) \
		di$(OBJ_EXT) dilib$(OBJ_EXT) \
		getopt$(OBJ_EXT) $(LIBS)

###

di$(OBJ_EXT):		di.c config.h di.h version.h
	$(CC) -c $(CFLAGS) $(OTHERFLAGS) di.c

dilib$(OBJ_EXT):	dilib.c config.h di.h
	$(CC) -c $(CFLAGS) dilib.c

getopt$(OBJ_EXT):	getopt.c config.h di.h
	$(CC) -c $(CFLAGS) getopt.c
