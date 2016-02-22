#
#  di makefile - top level
#
#  Copyright 2001-2011 Brad Lanam Walnut Creek CA, USA
#
#  $Id$
#

SHELL = /bin/sh
MAKE = make
FROMDIR = C

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

MKC_DIR = ./mkconfig

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

.PHONY: all
all:
	$(MAKE) checkbuild
	cd C >/dev/null && $(MAKE) CC=$(CC) -e all

.PHONY: all-c
all-c:
	$(MAKE) checkbuild
	cd C >/dev/null && $(MAKE) CC=$(CC) -e all

.PHONY: tcl-sh
tcl-sh:
	$(MAKE) checkbuild
	cd C >/dev/null && $(MAKE) CC=$(CC) -e tcl-sh

.PHONY: all-d
all-d:
	$(MAKE) checkbuild
	cd D >/dev/null && $(MAKE) -e all

.PHONY: all-perl
all-perl:
	$(MAKE) checkperlbuild
	cd C >/dev/null && $(MAKE) CC=$(CC) -e all-perl

.PHONY: windows-gcc
windows-gcc:
	cd C >/dev/null && $(MAKE) -e windows-gcc

.PHONY: windows-clang
windows-clang:
	cd C >/dev/null && $(MAKE) -e windows-clang

.PHONY: windows-msys
windows-msys:
	cd C >/dev/null && $(MAKE) -e windows-msys

.PHONY: windows-mingw
windows-mingw:
	cd C >/dev/null && $(MAKE) -e windows-mingw

.PHONY: test
test:
	cd C >/dev/null && $(MAKE) -e test
	$(MAKE) -e tests.done

tests.done: $(MKC_DIR)/runtests.sh
	@echo "## running tests"
	CC=$(CC) DC="skip" $(_MKCONFIG_SHELL) \
		$(MKC_DIR)/runtests.sh ./tests.d
	touch tests.done


###

###
# installation

.PHONY: install
install:
	$(MAKE) all-c
	$(MAKE) checkinstall
	. ./C/di.env; $(MAKE) -e FROMDIR=$(FROMDIR) install-prog install-man

.PHONY: build.po
build-po:
	-. ./C/di.env; \
		(cd po >/dev/null && for i in *.po; do \
		j=`echo $$i | $(SED) 's,\\.po$$,,'`; \
		$${XMSGFMT} -o $$j.mo $$i; \
	done)

.PHONY: install-po
install-po: 	build-po
	-$(TEST) -d $(INST_LOCALEDIR) || $(MKDIR) -p $(INST_LOCALEDIR)
	-(cd po >/dev/null && for i in *.po; do \
		j=`echo $$i | $(SED) 's,\\.po$$,,'`; \
		$(TEST) -d $(INST_LOCALEDIR)/$$j || \
			$(MKDIR) $(INST_LOCALEDIR)/$$j; \
		$(TEST) -d $(INST_LOCALEDIR)/$$j/LC_MESSAGES || \
			$(MKDIR) $(INST_LOCALEDIR)/$$j/LC_MESSAGES; \
		$(CP) -f $$j.mo $(INST_LOCALEDIR)/$$j/LC_MESSAGES/di.mo; \
		$(RM) -f $$j.mo; \
		done)

.PHONY: install-prog
install-prog:
	$(TEST) -d $(INSTALL_DIR) || $(MKDIR) -p $(INSTALL_DIR)
	$(TEST) -d $(INSTALL_BIN_DIR) || $(MKDIR) $(INSTALL_BIN_DIR)
	$(CP) -f ./$(FROMDIR)/$(PROG)$(EXE_EXT) $(TARGET)
	-$(RM) -f $(MTARGET) > /dev/null 2>&1
	-$(LN) -s $(PROG)$(EXE_EXT) $(MTARGET)
	@-test -f $(FROMDIR)/config.h && \
		grep '^#define _enable_nls 1' $(FROMDIR)/config.h >/dev/null 2>&1 && \
		(. ./$(FROMDIR)/di.env; $(MAKE) -e INST_LOCALEDIR="$(INST_LOCALEDIR)" \
		install-po)
	@-test -f $(FROMDIR)/config.d && \
		grep '^enum int _enable_nls = 1;' $(FROMDIR)/config.d >/dev/null 2>&1 && \
		(. ./$(FROMDIR)/di.env; $(MAKE) -e INST_LOCALEDIR="$(INST_LOCALEDIR)" \
		install-po)

.PHONY: install-man
install-man:
	-$(TEST) -d $(DI_MANINSTDIR) || $(MKDIR) -p $(DI_MANINSTDIR)
	-$(TEST) -d $(DI_MANDIR) || $(MKDIR) -p $(DI_MANDIR)
	$(CP) -f di.1 $(DI_MANDIR)/$(MAN_TARGET)
	$(CHMOD) $(MANPERM) $(DI_MANDIR)/$(MAN_TARGET)

.PHONY: installperms
installperms:
	$(CHOWN) $(USER) $(TARGET)
	$(CHGRP) $(GROUP) $(TARGET)
	$(CHMOD) $(INSTPERM) $(TARGET)

###
# packaging

.PHONY: tar
tar:
	@-rm -f $(MKCONFIGPATH)/*.tar.gz \
		di-[0-9].[0-9][0-9][a-z].tar.gz > /dev/null 2>&1
	cd $(MKCONFIGPATH) && $(MAKE) tar
	./mktar.sh

###
# cleaning

# Leaves:
#  _tmp_mkconfig/, _mkconfig_runtests/
.PHONY: clean
clean:
	@-rm -rf mkconfig.cache mkc*.vars mkconfig.log \
		checkbuild checkperlbuild checkinstall \
		tests.done > /dev/null 2>&1
	@-find . -name '*~' -print | xargs rm 
	@-(cd C >/dev/null && $(MAKE) clean > /dev/null 2>&1)
	@-(cd mkconfig >/dev/null && $(MAKE) clean > /dev/null 2>&1)
	@-(cd D >/dev/null && $(MAKE) clean > /dev/null 2>&1)

# Leaves:
#  _tmp_mkconfig/, _mkconfig_runtests/
.PHONY: realclean
realclean:
	@-rm -rf mkconfig.cache mkc*.vars mkconfig.log \
		checkbuild checkperlbuild checkinstall \
		tests.done *~ */*~ */*/*~ > /dev/null 2>&1
	@-(cd C >/dev/null && $(MAKE) realclean > /dev/null 2>&1)
	@-(cd mkconfig >/dev/null && $(MAKE) realclean > /dev/null 2>&1)
	@-(cd D >/dev/null && $(MAKE) realclean > /dev/null 2>&1)

# leaves:
#   dioptions.dat
.PHONY: distclean
distclean:
	@-rm -rf mkconfig.cache mkc*.vars mkconfig.log \
		_mkconfig_runtests checkbuild checkperlbuild checkinstall \
		tests.done _tmp_mkconfig *~ */*~ */*/*~ *.orig > /dev/null 2>&1
	@-(cd C >/dev/null && $(MAKE) distclean > /dev/null 2>&1)
	@-(cd mkconfig >/dev/null && $(MAKE) distclean > /dev/null 2>&1)
	@-(cd D >/dev/null && $(MAKE) distclean > /dev/null 2>&1)


###
# dioptions.dat

dioptions.dat:	features/dioptions.dat
	@$(CP) features/dioptions.dat dioptions.dat
	@$(CHMOD) u+w dioptions.dat

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

