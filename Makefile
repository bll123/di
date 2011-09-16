#
#  di makefile
#
#  Copyright 2001-2010 Brad Lanam Walnut Creek CA, USA
#
#  $Id$
#

SHELL = /bin/sh
MAKE = make

###
# mkconfig variables

MKCONFIGPATH = mkconfig

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
#
INSTALL_DIR = $(prefix)
INSTALL_BIN_DIR = $(INSTALL_DIR)/bin
INST_LOCALEDIR = $(INSTALL_DIR)/share/locale
TARGET = $(INSTALL_BIN_DIR)/$(PROG)$(EXE_EXT)
MTARGET = $(INSTALL_BIN_DIR)/$(MPROG)$(EXE_EXT)

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
# all

all:
	$(MAKE) checkbuild
	cd C >/dev/null;$(MAKE) -e all

all-c:
	$(MAKE) checkbuild
	cd C >/dev/null;$(MAKE) -e all

all-perl:
	$(MAKE) checkperlbuild
	cd C >/dev/null;$(MAKE) -e all-perl

windows-gcc:
	cd C >/dev/null;$(MAKE) -e windows-gcc

all-test:
	$(MAKE) checkbuild
	cd C >/dev/null;$(MAKE) -e all-test

all-d:
	$(MAKE) checkbuild
	cd D >/dev/null;$(MAKE) -e all

###
# installation

install:
	$(MAKE) checkinstall
	. ./C/di.env; \
		$(MAKE) -e install-prog install-man

build-po:
	-. ./C/di.env; \
		(cd po >/dev/null;for i in *.po; do \
		j=`echo $$i | $(SED) 's,\\.po$$,,'`; \
		$${XMSGFMT} -o $$j.mo $$i; \
	done)

install-po: 	build-po
	-$(TEST) -d $(INST_LOCALEDIR) || $(MKDIR) -p $(INST_LOCALEDIR)
	-(cd po >/dev/null;for i in *.po; do \
		j=`echo $$i | $(SED) 's,\\.po$$,,'`; \
		$(TEST) -d $(INST_LOCALEDIR)/$$j || \
			$(MKDIR) $(INST_LOCALEDIR)/$$j; \
		$(TEST) -d $(INST_LOCALEDIR)/$$j/LC_MESSAGES || \
			$(MKDIR) $(INST_LOCALEDIR)/$$j/LC_MESSAGES; \
		$(CP) -f $$j.mo $(INST_LOCALEDIR)/$$j/LC_MESSAGES/di.mo; \
		$(RM) -f $$j.mo; \
		done)

install-prog:
	$(TEST) -d $(INSTALL_DIR) || $(MKDIR) $(INSTALL_DIR)
	$(TEST) -d $(INSTALL_BIN_DIR) || $(MKDIR) $(INSTALL_BIN_DIR)
	$(TEST) ! -f $(TARGET) || $(MV) -f $(TARGET) $(TARGET).old
	$(CP) -f ./C/$(PROG)$(EXE_EXT) $(TARGET)
	-$(RM) -f $(MTARGET) > /dev/null 2>&1
	-$(LN) -s $(PROG)$(EXE_EXT) $(MTARGET)
	-$(RM) -f $(TARGET).old > /dev/null 2>&1
	-grep '^#define _enable_nls 1' C/config.h >/dev/null 2>&1 && \
		(. ./C/di.env; $(MAKE) -e INST_LOCALEDIR="$(INST_LOCALEDIR)" \
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
# packaging

tar:
	@-rm -f $(MKCONFIGPATH)/*.tar.gz \
		di-[0-9].[0-9][0-9][a-z].tar.gz > /dev/null 2>&1
	cd $(MKCONFIGPATH);$(MAKE) tar
	./mktar.sh

###
# cleaning

clean:
	@-rm -rf mkconfig.cache mkconfig_mkc.vars mkconfig.log _tmp_mkconfig \
		checkbuild checkperlbuild checkinstall > /dev/null 2>&1
	@-(cd C >/dev/null;$(MAKE) clean > /dev/null 2>&1)
	@-(cd mkconfig >/dev/null;$(MAKE) clean > /dev/null 2>&1)
	@-(cd D >/dev/null;$(MAKE) clean > /dev/null 2>&1)

# leaves:
#   */_mkconfig_runtests, */_tmp_mkconfig, dioptions.dat
#   pretests.done, */test_di
realclean:
	@-rm -rf mkconfig.cache mkconfig_mkc.vars mkconfig.log _tmp_mkconfig \
		checkbuild checkperlbuild checkinstall > /dev/null 2>&1
	@-(cd C >/dev/null;$(MAKE) realclean > /dev/null 2>&1)
	@-(cd mkconfig >/dev/null;$(MAKE) realclean > /dev/null 2>&1)
	@-(cd D >/dev/null;$(MAKE) realclean > /dev/null 2>&1)

# leaves:
#   dioptions.dat
distclean:
	@-rm -rf mkconfig.cache mkconfig_mkc.vars mkconfig.log _tmp_mkconfig \
		checkbuild checkperlbuild checkinstall > /dev/null 2>&1
	@-(cd C >/dev/null;$(MAKE) distclean > /dev/null 2>&1)
	@-(cd mkconfig >/dev/null;$(MAKE) distclean > /dev/null 2>&1)
	@-(cd D >/dev/null;$(MAKE) distclean > /dev/null 2>&1)


###
# dioptions.dat

dioptions.dat:	features/dioptions.dat
	@cp features/dioptions.dat dioptions.dat
	@chmod u+w dioptions.dat

###
# pre-checks

checkbuild:	features/checkbuild.dat
	$(_MKCONFIG_SHELL) \
		$(MKCONFIGPATH)/mkconfig.sh features/checkbuild.dat
	touch checkbuild

checkperlbuild:	features/checkperlbuild.dat
	$(_MKCONFIG_SHELL) \
		$(MKCONFIGPATH)/mkconfig.sh features/checkperlbuild.dat
	touch checkperlbuild

checkinstall:	features/checkinstall.dat
	$(_MKCONFIG_SHELL) \
		$(MKCONFIGPATH)/mkconfig.sh features/checkinstall.dat
	touch checkinstall
	
