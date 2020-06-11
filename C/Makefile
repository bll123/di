#
#  di makefile - C
#
#  Copyright 2001-2018 Brad Lanam Walnut Creek CA, USA
#

MKC_PREFIX = di
MKC_CONFDIR = mkc_config
MKC_FILES = mkc_files
MKC_OUTPUT = config.h
MKC_ENV = $(MKC_PREFIX).env
MKC_ENV_SHR = $(MKC_PREFIX)-shared.env

MKC_CONF = $(MKC_CONFDIR)/$(MKC_PREFIX).mkc
MKC_ENV_CONF = $(MKC_CONFDIR)/$(MKC_PREFIX)-env.mkc
MKC_ENV_SHR_CONF = $(MKC_CONFDIR)/$(MKC_PREFIX)-env-shared.mkc
MKC_CONF_TCL = $(MKC_CONFDIR)/$(MKC_PREFIX)-tcl.mkc
MKC_CONF_GETOPTN = $(MKC_CONFDIR)/$(MKC_PREFIX)-getoptn.mkc

# for tests.done
DC = gdc

OBJ_EXT = .o
EXE_EXT =

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

###
# additional flags/libraries
#
DI_SHARED =
DI_CFLAGS = -DDI_LOCALE_DIR=\\\"$(LOCALEDIR)\\\"

###
# mkconfig variables

MKC_DIR = ../mkconfig
MKCONFIG_TYPE = sh
MKC_REQLIB = di.reqlibs
MKC_TCLREQLIB = di-tcl.reqlibs
MKC_ECHO =
#MKC_ECHO = -e

###
# main

# have to get various environment variables set up.

.PHONY: all
all:	all-sh

.PHONY: all-sh
all-sh:	$(MKC_ENV)
	. ./$(MKC_ENV);$(MAKE) -e MKCONFIG_TYPE=sh di-programs

.PHONY: all-perl
all-perl:	$(MKC_ENV)
	. ./$(MKC_ENV);$(MAKE) -e MKCONFIG_TYPE=perl di-programs

# tcl interface
.PHONY: tcl-sh
tcl-sh:	$(MKC_ENV_SHR)
	. ./$(MKC_ENV_SHR);$(MAKE) -e MKCONFIG_TYPE=sh tcl-programs

# this is not working...need to research
#tcl-perl:	$(MKC_ENV_SHR)
#	. ./$(MKC_ENV_SHR);$(MAKE) -e MKCONFIG_TYPE=perl tcl-programs

# perl interface
.PHONY: perl-sh
perl-sh:	$(MKC_ENV_SHR)
	. ./$(MKC_ENV_SHR);$(MAKE) -e MKCONFIG_TYPE=sh perl-programs

.PHONY: perl-perl
perl-perl:	$(MKC_ENV_SHR)
	. ./$(MKC_ENV_SHR);$(MAKE) -e MKCONFIG_TYPE=perl perl-programs

.PHONY: test
test:		tests.done

###
# environment

$(MKC_ENV):	$(MKC_ENV_CONF) ../dioptions.dat
	@-$(RM) -f $(MKC_ENV) tests.done
	CC=$(CC) $(_MKCONFIG_SHELL) $(MKC_DIR)/mkconfig.sh $(MKC_ENV_CONF)

$(MKC_ENV_SHR):	$(MKC_ENV_SHR_CONF)
	@-$(RM) -f $(MKC_ENV_SHR) tests.done
	CC=$(CC) $(_MKCONFIG_SHELL) $(MKC_DIR)/mkconfig.sh \
		$(MKC_ENV_SHR_CONF)

###
# specific builds

# This was tested using vc
# Use:
#     nmake MAKE=nmake windows
#
.PHONY: windows
windows:
	@$(MAKE) ../dioptions.dat
	copy /y NUL: $(MKC_ENV)
	-del config.h
	copy /y /b NUL:+configs\config.ms.cl config.h
	copy /y NUL: $(MKC_REQLIB)
	$(MAKE) \
		MAKE=nmake CC=cl LD=cl EXE_EXT=".exe" OBJ_EXT=".obj" \
		DI_CFLAGS="$(DI_CFLAGS) -nologo -O2" \
		LDFLAGS="-nologo -O2" di-programs

# This was tested using cygwin
.PHONY: windows-gcc
windows-gcc:
	@$(MAKE) ../dioptions.dat
	@echo ':' > $(MKC_ENV);chmod a+rx $(MKC_ENV)
	@-$(RM) -f config.h mkconfig.cache mkc*.vars tests.done
	$(CP) -f configs/config.cygwin.gcc config.h
	@echo '-lintl' > $(MKC_REQLIB)
	$(MAKE) \
		CC=gcc LD=gcc EXE_EXT=".exe" OBJ_EXT=".o" \
		DI_CFLAGS="$(DI_CFLAGS) -g -O2" \
		LDFLAGS="-g -O2" di-programs

.PHONY: windows-msys
windows-msys:
	MAKE=mingw32-make
	cp ../features/dioptions.dat ../dioptions.dat
	> $(MKC_ENV)
	-rm config.h
	cp configs/config.mingw config.h
	> $(MKC_REQLIB)
	$(MAKE) \
		MAKE=$(MAKE) \
		CC=gcc \
		EXE_EXT=".exe" OBJ_EXT=".o" \
		DI_CFLAGS="$(DI_CFLAGS) -g -O2" \
		LDFLAGS="-g -O2" di-programs

.PHONY: windows-mingw
windows-mingw:
	MAKE=mingw32-make
	copy /y /b NUL:+..\features\dioptions.dat ..\dioptions.dat
	copy /y NUL: $(MKC_ENV)
	-del config.h
	copy /y /b NUL:+configs\config.mingw config.h
	copy /y NUL: $(MKC_REQLIB)
	$(MAKE) \
		MAKE=$(MAKE) \
		CC=gcc LD=gcc \
		EXE_EXT=".exe" OBJ_EXT=".o" \
		DI_CFLAGS="$(DI_CFLAGS) -g -O2" \
		LDFLAGS="-g -O2" mingw-di.exe

.PHONY: os2-gcc
os2-gcc:
	@echo ':' > $(MKC_ENV);chmod a+rx $(MKC_ENV)
	$(MAKE) MKCONFIG_TYPE=perl \
		CC=gcc LD=gcc EXE_EXT=".exe" OBJ_EXT=".o" \
		DI_CFLAGS="$(DI_CFLAGS) -g -O2" \
		LDFLAGS="-g -O2 -Zexe" di.exe

.PHONY: bsd43-cc
bsd43-cc:
	@$(MAKE) ../dioptions.dat
	@echo ':' > $(MKC_ENV);chmod a+rx $(MKC_ENV)
	@-$(RM) -f config.h mkconfig.cache mkc*.vars tests.done
	sleep 1
	$(CP) configs/config.bsd43.cc config.h
	sleep 1
	@touch $(MKC_REQLIB)
	$(MAKE) _MKCONFIG_SHELL=$(MKCONFIG_SHELL) \
		CC=cc LD=cc EXE_EXT="" OBJ_EXT=".o" \
		DI_CFLAGS="$(DI_CFLAGS) -O" \
		LDFLAGS="-O" di-programs

.PHONY: bsd43-gcc
bsd43-gcc:
	@$(MAKE) ../dioptions.dat
	@echo ':' > $(MKC_ENV);chmod a+rx $(MKC_ENV)
	@-$(RM) -f config.h mkconfig.cache mkc*.vars tests.done
	sleep 1
	$(CP) configs/config.bsd43.gcc config.h
	sleep 1
	@touch $(MKC_REQLIB)
	$(MAKE) _MKCONFIG_SHELL=$(MKCONFIG_SHELL) \
		CC=gcc LD=gcc EXE_EXT="" OBJ_EXT=".o" \
		DI_CFLAGS="$(DI_CFLAGS) -O2" \
		LDFLAGS="-O2" di-programs

.PHONY: ultrix-cc
ultrix-cc:
	@$(MAKE) ../dioptions.dat
	@echo ':' > $(MKC_ENV);chmod a+rx $(MKC_ENV)
	@-$(RM) -f config.h mkconfig.cache mkc*.vars tests.done
	sleep 1
	$(CP) configs/config.ultrix.cc config.h
	sleep 1
	@touch $(MKC_REQLIB)
	$(MAKE) _MKCONFIG_SHELL=$(MKCONFIG_SHELL) \
		CC=cc LD=cc EXE_EXT="" OBJ_EXT=".o" \
		DI_CFLAGS="$(DI_CFLAGS) -O" \
		LDFLAGS="-O" di-programs

.PHONY: ultrix-gcc
ultrix-gcc:
	@$(MAKE) ../dioptions.dat
	@echo ':' > $(MKC_ENV);chmod a+rx $(MKC_ENV)
	@-$(RM) -f config.h mkconfig.cache mkc*.vars tests.done
	sleep 1
	$(CP) configs/config.ultrix.gcc config.h
	sleep 1
	@echo '-lintl' > $(MKC_REQLIB)
	$(MAKE) _MKCONFIG_SHELL=$(MKCONFIG_SHELL) \
		CC=gcc LD=gcc EXE_EXT="" OBJ_EXT=".o" \
		DI_CFLAGS="$(DI_CFLAGS) -O2" \
		LDFLAGS="-O2" di-programs

.PHONY: hpux-cc
hpux-cc:
	@$(MAKE) -e ../dioptions.dat
	@$(MAKE) CC=/usr/ccs/bin/cc -e $(MKC_ENV)
	@-$(RM) -f config.h mkconfig.cache mkc*.vars tests.done
	sleep 1
	$(CP) configs/config.hpux.bcc config.h
	sleep 1;touch $(MKC_REQLIB)
	. ./$(MKC_ENV);$(MAKE) -e CC=/usr/ccs/bin/cc \
		_MKCONFIG_SHELL=$(MKCONFIG_SHELL) di-programs

.PHONY: xenix-cc
xenix-cc:
	@$(MAKE) ../dioptions.dat
	@echo ':' > $(MKC_ENV);chmod a+rx $(MKC_ENV)
	@-$(RM) -f config.h mkconfig.cache mkc*.vars tests.done
	sleep 1
	$(CP) configs/conf.xenix.cc config.h
	sleep 1
	@touch $(MKC_REQLIB)
	$(MAKE) _MKCONFIG_SHELL=$(MKCONFIG_SHELL) \
		CC=cc LD=cc EXE_EXT="" OBJ_EXT=".o" \
		DI_CFLAGS="$(DI_CFLAGS) -O" \
		LDFLAGS="-O" di-programs

###
# cleaning

# leaves:
#   config.h configtcl.h di.reqlibs
#   ../dioptions.dat, tests.done, test_di, $(MKC_ENV), $(MKC_ENV_SHR)
.PHONY: clean
clean:
	@-cd Perl; \
		if [ -f Makefile ]; then \
		$(MAKE) clean; \
		fi
	@-$(RM) -rf di mi \
		di.exe mingw-di.exe mi.exe \
		tcldiskspace.bld diskspace.so diskspace.dylib diskspace.dll \
		perlfilesysdi.bld libdiperl.a \
		*.o *.obj $(MKC_FILES)/mkconfig.log \
		tests.done $(MKC_FILES)/_tmp_mkconfig tests.d/chksh* \
		$(MKC_FILES)/mkconfig.cache mkc*.vars \
		getoptn_test* gconfig.h $(MKC_FILES)/getoptn.reqlibs \
		$(MKC_FILES)/mkconfig.reqlibs $(MKC_FILES)/mkc_compile.log \
		Perl/Filesys-di-*.tar.gz \
		Perl/Makefile.old \
		tests.d/test_order.tmp >/dev/null 2>&1; exit 0
	@-find . -name '*~' -print | xargs rm > /dev/null 2>&1; exit 0

# leaves:
#   _mkconfig_runtests, mkc_files, ../dioptions.dat
#   tests.done, test_di
.PHONY: realclean
realclean:
	@$(MAKE) clean >/dev/null 2>&1
	@-$(RM) -rf config.h configtcl.h gconfig.h \
		$(MKC_ENV) $(MKC_ENV_SHR) $(MKC_REQLIB) $(MKC_TCLREQLIB) \
		>/dev/null 2>&1; exit 0

# leaves:
#   ../dioptions.dat
.PHONY: distclean
distclean:
	@-cd Perl; \
		if [ -f Makefile ]; then \
		$(MAKE) distclean; \
		fi
	@$(MAKE) realclean >/dev/null 2>&1
	@-$(RM) -rf tests.done test_di \
		_mkconfig_runtests \
		$(MKC_FILES) \
		*~ *.orig */*.orig \
		>/dev/null 2>&1; exit 0

###
# installation

.PHONY: install
install:	$(MKC_ENV) di-programs
	. ./$(MKC_ENV);cd ..;$(MAKE) -e prefix=$(prefix) \
		LOCALEDIR=$(LOCALEDIR) FROMDIR=C install

###
# programs

.PHONY: di-programs
di-programs:	di$(EXE_EXT)
.PHONY: tcl-programs
tcl-programs:	tcldiskspace.bld
.PHONY: perl-programs
perl-programs:	perlfilesysdi.bld

###
# configuration file

../dioptions.dat:	../features/dioptions.dat
	cd ../;$(MAKE) dioptions.dat

config.h:	$(MKC_ENV) ../dioptions.dat $(MKC_CONF)
	@-$(RM) -f config.h tests.done
	@if [ "$(DI_NO_NLS)" != "" ]; then \
		echo "*** User requested no NLS"; \
		$(MKC_DIR)/mkc.sh -setopt -o ../dioptions.dat NLS F; fi
	@if [ "$(MKCONFIG_TYPE)" = "sh" -o "$(MKCONFIG_TYPE)" = "" ]; then \
		. ./$(MKC_ENV);$(_MKCONFIG_SHELL) \
		$(MKC_DIR)/mkconfig.sh \
		$(MKC_CONF); fi
	@if [ "$(MKCONFIG_TYPE)" = "perl" ]; then \
		. ./$(MKC_ENV);perl \
		$(MKC_DIR)/mkconfig.pl \
		$(MKC_CONF); fi

configtcl.h:	$(MKC_ENV_SHR) ../dioptions.dat $(MKC_CONF_TCL)
	@-$(RM) -f configtcl.h
	@if [ "$(MKCONFIG_TYPE)" = "sh" -o "$(MKCONFIG_TYPE)" = "" ]; then \
		. ./$(MKC_ENV_SHR);$(_MKCONFIG_SHELL) \
		$(MKC_DIR)/mkconfig.sh \
		$(MKC_CONF_TCL); fi
	@if [ "$(MKCONFIG_TYPE)" = "perl" ]; then \
		. ./$(MKC_ENV_SHR);perl \
		$(MKC_DIR)/mkconfig.pl \
		$(MKC_CONF_TCL); fi

$(MKC_REQLIB):	config.h
	@$(_MKCONFIG_SHELL) $(MKC_DIR)/mkc.sh -reqlib \
		-o $(MKC_REQLIB) config.h

$(MKC_TCLREQLIB):	configtcl.h
	@$(_MKCONFIG_SHELL) $(MKC_DIR)/mkc.sh -reqlib \
		-o $(MKC_TCLREQLIB) configtcl.h


###
# executables

LIBOBJECTS = dimain$(OBJ_EXT) didiskutil$(OBJ_EXT) \
		digetentries$(OBJ_EXT) digetinfo$(OBJ_EXT) \
		diquota$(OBJ_EXT)  display$(OBJ_EXT) getoptn$(OBJ_EXT) \
		options$(OBJ_EXT) realloc$(OBJ_EXT) strdup$(OBJ_EXT) \
		strstr$(OBJ_EXT) trimchar$(OBJ_EXT)

MAINOBJECTS = di$(OBJ_EXT)

TCLOBJECTS = tcldiskspace$(OBJ_EXT)

di$(EXE_EXT):	$(MKC_REQLIB) $(MAINOBJECTS) $(LIBOBJECTS)
	@$(_MKCONFIG_SHELL) $(MKC_DIR)/mkc.sh \
		-link -exec $(MKC_ECHO) \
		-r $(MKC_REQLIB) -o di$(EXE_EXT) \
		$(MAINOBJECTS) $(LIBOBJECTS)

# for ms cl
#di$(EXE_EXT):	$(MAINOBJECTS) $(LIBOBJECTS)
#	$(LD) -Fedi$(EXE_EXT) $(MAINOBJECTS) $(LIBOBJECTS)

tcldiskspace.bld:	$(MKC_REQLIB) $(MKC_TCLREQLIB) \
		$(TCLOBJECTS) $(LIBOBJECTS)
	@$(_MKCONFIG_SHELL) $(MKC_DIR)/mkc.sh -link -shared $(MKC_ECHO) \
		-lib -o diskspace$(SHLIB_EXT) \
		-r $(MKC_REQLIB) -r $(MKC_TCLREQLIB) \
		$(TCLOBJECTS) $(LIBOBJECTS) $(LIBS)
	@touch tcldiskspace.bld

perlfilesysdi.bld:	$(MKC_REQLIB) $(LIBOBJECTS)
	@echo "*** Using libs: `$(CAT) $(MKC_REQLIB)`"
	@$(_MKCONFIG_SHELL) $(MKC_DIR)/mkc.sh -staticlib $(MKC_ECHO) \
		-o libdiperl.a $(LIBOBJECTS) $(LIBS) \
		`$(CAT) $(MKC_REQLIB)`
	# Don't know how to pass additional libs to Makefile.PL
	# so pass all the information.
	( \
	cd Perl ; \
	perl Makefile.PL LIBS="-L.. -ldiperl `$(CAT) ../$(MKC_REQLIB)`" ; \
	make ; \
	make test ; \
	)
	@touch perlfilesysdi.bld

mingw-di$(EXE_EXT):	$(MAINOBJECTS) $(LIBOBJECTS)
	$(CC) -o mingw-di$(EXE_EXT) \
		$(DI_CFLAGS) $(LDFLAGS) $(LIBOBJECTS) $(LIBS)

###
# objects

.c$(OBJ_EXT):
	@$(_MKCONFIG_SHELL) $(MKC_DIR)/mkc.sh -compile $(MKC_ECHO) \
		$(DI_SHARED) $(DI_CFLAGS) $<

# for ms cl
#.c$(OBJ_EXT):
#	$(CC) -c $(DI_SHARED) $(DI_CFLAGS) $<

di$(OBJ_EXT):		di.c config.h di.h dimain.h getoptn.h \
				options.h version.h

tcldiskspace$(OBJ_EXT):		tcldiskspace.c config.h di.h \
				dimain.h getoptn.h \
				options.h

dimain$(OBJ_EXT):	dimain.c config.h di.h dimain.h getoptn.h \
				options.h

digetinfo$(OBJ_EXT):	digetinfo.c config.h di.h dimntopt.h

didiskutil$(OBJ_EXT):	didiskutil.c config.h di.h dimntopt.h

digetentries$(OBJ_EXT):	digetentries.c config.h di.h dimntopt.h

diquota$(OBJ_EXT):	diquota.c config.h di.h

display$(OBJ_EXT):	display.c config.h di.h display.h options.h version.h

getoptn$(OBJ_EXT):	getoptn.c config.h getoptn.h

options$(OBJ_EXT):	options.c config.h di.h options.h

realloc$(OBJ_EXT):	realloc.c config.h di.h

strdup$(OBJ_EXT):	strdup.c config.h di.h

strstr$(OBJ_EXT):	strstr.c config.h di.h

trimchar$(OBJ_EXT):	trimchar.c config.h di.h

###
# regression testing

.PHONY: all-test
all-test:	tests.done

tests.done: $(MKC_DIR)/runtests.sh
	@echo "## running tests"
	CC=$(CC) DC=$(DC) $(_MKCONFIG_SHELL) \
		$(MKC_DIR)/runtests.sh ./tests.d
	touch tests.done

# needs environment
.PHONY: testrpmbuild
testrpmbuild:
	-$(TEST) -d ./rpmbuild && rm -rf ./rpmbuild
	$(MKDIR) -p ./rpmbuild/SOURCES
	$(MKDIR) -p ./rpmbuild/BUILD
	$(CP) -f $(DI_DIR)/di-$(DI_VERSION).tar.gz ./rpmbuild/SOURCES
	$(LN) -f ./rpmbuild/SOURCES/di-$(DI_VERSION).tar.gz ./rpmbuild/SOURCES/download
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
	@echo "$(CFLAGS_OPTIMIZE)"
	@echo "$(CFLAGS_COMPILER)"
	@echo "$(LDFLAGS_COMPILER)"
	@echo "$(OBJ_EXT)"
	@echo "$(EXE_EXT)"
	@echo "$(XMSGFMT)"

.PHONY: test-env
test-env:
	@echo "cc: $(CC)"
	@echo "make: $(MAKE)"

gconfig.h:	$(MKC_ENV) features/getoptn.dat
	@-$(RM) -f gconfig.h
	@if [ "$(MKCONFIG_TYPE)" = "sh" -o "$(MKCONFIG_TYPE)" = "" ]; then \
		. ./$(MKC_ENV); \
		$(_MKCONFIG_SHELL) $(MKC_DIR)/mkconfig.sh \
		$(MKC_CONF_GETOPTN); fi
	@if [ "$(MKCONFIG_TYPE)" = "perl" ]; then \
		. ./$(MKC_ENV); \
		perl $(MKC_DIR)/mkconfig.pl \
		$(MKC_CONF_GETOPTN); fi

getoptn_test$(OBJ_EXT):	getoptn.c gconfig.h getoptn.h
	@. ./$(MKC_ENV);$(_MKCONFIG_SHELL) $(MKC_DIR)/mkc.sh -compile \
		-DTEST_GETOPTN=1 \
		-o getoptn_test$(OBJ_EXT) getoptn.c

getoptn.reqlibs:	$(MKC_ENV) gconfig.h
	@. ./$(MKC_ENV);$(_MKCONFIG_SHELL) $(MKC_DIR)/mkc.sh -reqlib \
		-o getoptn.reqlibs gconfig.h

getoptn_test.exe:	getoptn_test$(OBJ_EXT) getoptn.reqlibs
	@. ./$(MKC_ENV);$(_MKCONFIG_SHELL) $(MKC_DIR)/mkc.sh -link -exec \
		$(MKC_ECHO) -r getoptn.reqlibs \
		-o getoptn_test.exe $(LDFLAGS) \
		getoptn_test$(OBJ_EXT)
