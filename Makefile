#
#  di makefile - C
#
#  Copyright 2001-2018 Brad Lanam Walnut Creek CA, USA
#  Copyright 2023 Brad Lanam, Pleasant Hill, CA
#

# for cmake
CMAKE_REQ_MAJ_VERSION=3
CMAKE_REQ_MIN_VERSION=10
BUILDDIR = build

# DI_USE_MATH = DI_GMP
# DI_USE_MATH = DI_TOMMATH
# DI_USE_MATH = DI_INTERNAL

# for mkconfig
MKC_PREFIX = di
MKC_CONFDIR = mkc_config
MKC_FILES = mkc_files
MKC_OUTPUT = config.h
MKC_ENV = $(MKC_PREFIX).env

MKC_CONF = $(MKC_CONFDIR)/$(MKC_PREFIX).mkc
MKC_ENV_CONF = $(MKC_CONFDIR)/$(MKC_PREFIX)-env.mkc

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
SED = sed
TEST = test
MSGFMT = msgfmt

###
# installation options
#
PREFIX = /usr/local
BINDIR = $(PREFIX)/bin
LIBDIR = $(PREFIX)/lib
DATADIR = $(PREFIX)/share
MANDIR = $(DATADIR)/man
LOCALEDIR = $(DATADIR)/locale
DI_VERSION = 5.0.0-beta-1

INST_DIR = $(DESTDIR)$(PREFIX)
INST_BINDIR = $(DESTDIR)$(BINDIR)
INST_LIBDIR = $(DESTDIR)$(LIBDIR)
INST_DATADIR = $(DESTDIR)$(DATADIR)
INST_MANDIR = $(DESTDIR)$(MANDIR)
INST_LOCALEDIR = $(DESTDIR)$(LOCALEDIR)

###
# additional flags/libraries
#
DI_SHARED =
DI_CFLAGS =

###
# mkconfig variables

MKC_DIR = mkconfig
MKCONFIG_TYPE = sh
MKC_REQLIB = di.reqlibs
MKC_ECHO =
#MKC_ECHO = -e

###
# generic targets

# checks the cmake version, and runs the
.PHONY: all
all:
	@$(MAKE) TARGET=$@ switcher

.PHONY: install
install:
	@$(MAKE) TARGET=$@ switcher

.PHONY: test
test:
	@$(MAKE) TARGET=$@ switcher

.PHONY: switcher
switcher:
	@cmvers=`cmake --version 2>/dev/null`; \
	cmmajv=`echo $${cmvers} | \
	  $(SED) -n -e '/version/ s,[^0-9]*\([0-9]*\)\..*,\1, p'` ; \
	cmminv=`echo $${cmvers} | \
	  $(SED) -n -e '/version/ s,[^0-9]*3\.\([0-9]*\).*,\1, p'` ; \
	if [ "$${cmmajv}" -ge $(CMAKE_REQ_MAJ_VERSION) -a \
	    "$${cmminv}" -ge $(CMAKE_REQ_MIN_VERSION) ]; then \
	  $(MAKE) cmake-$(TARGET); \
	else \
	  $(MAKE) mkc-$(TARGET); \
	fi

###
# cleaning

# clean temporary files
.PHONY: clean
tclean:
	@-$(RM) -f w ww asan.* *.orig >/dev/null 2>&1; exit 0
	@-find . -name '*~' -print0 | xargs -0 rm > /dev/null 2>&1; exit 0

# leaves config.h
.PHONY: clean
clean:
	@$(MAKE) tclean
	@-$(RM) -f \
		di libdi.* dimathtest getoptn_test \
		di.exe libdi.dll dimathtest.exe getoptn_test.exe \
		*.o *.obj \
		$(MKC_FILES)/mkc_compile.log \
		tests.done tests.d/chksh* \
		tests.d/test_order.tmp >/dev/null 2>&1; exit 0
	@-test -d build && cmake --build build --target clean

# mkc tests use this
.PHONY: realclean
realclean:
	@$(MAKE) clean >/dev/null 2>&1
	@-$(RM) -rf config.h \
		$(MKC_ENV) $(MKC_REQLIB) \
		$(MKC_FILES) tests.d/chksh* \
		>/dev/null 2>&1; exit 0

.PHONY: distclean
distclean:
	@$(MAKE) clean >/dev/null 2>&1
	@-$(RM) -rf tests.done test_di _mkconfig_runtests \
		$(MKC_FILES) \
		build \
		>/dev/null 2>&1; exit 0

###
# cmake

.PHONY: cmake-release
cmake-release:
	$(MAKE) DI_BUILD=Release cmake-all

.PHONY: cmake-debug
cmake-debug:
	$(MAKE) DI_BUILD=Debug cmake-all

.PHONY: cmake-sanitize
cmake-sanitize:
	$(MAKE) DI_BUILD=SanitizeAddress cmake-all

# parallel doesn't seem to work under msys2
# cmake doesn't seem to support parallel under *BSD
.PHONY: cmake-all
cmake-all:
	@case $$(uname -s) in \
	  CYGWIN*) \
	    COMP=$(CC) \
	    $(MAKE) cmake-windows; \
	    $(MAKE) cmake-build; \
            ;; \
	  MINGW*) \
	    COMP=$(CC) \
	    $(MAKE) cmake-windows; \
	    $(MAKE) cmake-build; \
            ;; \
	  BSD*) \
	    COMP=$(CC) \
	    $(MAKE) cmake-unix; \
	    $(MAKE) cmake-build; \
            ;; \
	  *) \
	    COMP=$(CC) \
	    $(MAKE) cmake-unix; \
	    pmode=--parallel $(MAKE) cmake-build; \
            ;; \
	esac

.PHONY: cmakeclang
cmakeclang:
	case $$(uname -s) in \
	  BSD*) \
	    COMP=$(CC) \
	    $(MAKE) cmake-unix; \
	    $(MAKE) cmake-build; \
            ;; \
	  MINGW*) \
	    COMP=/ucrt64/bin/clang.exe \
	    $(MAKE) cmake-windows; \
	    $(MAKE) cmake-build; \
            ;; \
	  *) \
	    $(MAKE) cmake-unix; \
	    pmode=--parallel $(MAKE) cmake-build; \
            ;; \
	esac

# --debug-trycompile
# internal use
.PHONY: cmake-unix
cmake-unix:
	@if [ "$(PREFIX)" = "" ]; then echo "No prefix set"; exit 1; fi
	cmake \
		-DCMAKE_C_COMPILER=$(COMP) \
		-DCMAKE_INSTALL_PREFIX="$(PREFIX)" \
		-DDI_BUILD:STATIC=$(DI_BUILD) \
		-DPREFIX:STATIC=$(PREFIX) \
		-DDI_USE_MATH:STATIC=$(DI_USE_MATH) \
		-S . -B $(BUILDDIR) -Werror=deprecated

# internal use
.PHONY: cmake-windows
cmake-windows:
	@if [ "$(PREFIX)" = "" ]; then echo "No prefix set"; exit 1; fi
	cmake \
		-DCMAKE_C_COMPILER=$(COMP) \
		-DCMAKE_INSTALL_PREFIX="$(PREFIX)" \
		-DDI_BUILD:STATIC=$(DI_BUILD) \
		-DPREFIX:STATIC=$(PREFIX) \
		-DDI_USE_MATH:STATIC=$(DI_USE_MATH) \
		-G "MSYS Makefiles" \
		-S . -B $(BUILDDIR) -Werror=deprecated

# cmake on windows installs extra unneeded crap
# --parallel does not work correctly on msys2
# --parallel also seems to not work on *BSD
.PHONY: cmake-build
cmake-build:
	cmake --build $(BUILDDIR) $(pmode)

.PHONY: cmake-install
cmake-install:
	cmake --install $(BUILDDIR)

.PHONY: cmake-test
cmake-test:
	./build/dimathtest
	./build/getoptn_test

###
# main

# have to get various environment variables set up.

.PHONY: mkc-all
mkc-all: mkc-sh

.PHONY: mkc-sh
mkc-sh:	$(MKC_ENV)
	. ./$(MKC_ENV);$(MAKE) -e MKCONFIG_TYPE=sh di-programs

.PHONY: mkc-perl
mkc-perl:	$(MKC_ENV)
	. ./$(MKC_ENV);$(MAKE) -e MKCONFIG_TYPE=perl di-programs

.PHONY: test
mkc-test:		tests.done
	./dimathtest
	./getoptn_test

.PHONY: mkc-install
mkc-install:
	$(MAKE) mkc-all
	. ./$(MKC_ENV);$(MAKE) -e PREFIX=$(PREFIX) install-prog install-man

###
# installation

.PHONY: build.po
build-po:
	-. ./$(MKC_ENV); \
		(cd po >/dev/null && for i in *.po; do \
		j=`echo $$i | $(SED) 's,\\.po$$,,'`; \
		$(MSGFMT) -o $$j.mo $$i; \
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
	$(TEST) -d $(INST_BINDIR) || $(MKDIR) -p $(INST_BINDIR)
	$(TEST) -d $(INST_LIBDIR) || $(MKDIR) -p $(INST_LIBDIR)
	$(CP) -f di$(EXE_EXT) $(INST_BINDIR)
	$(CP) -f libdi$(SHLIB_EXT) $(INST_LIBDIR)
	-$(MAKE) install-po

.PHONY: install-man
install-man:
	-$(TEST) -d $(INST_MANDIR)/man1 || $(MKDIR) -p $(INST_MANDIR)/man1
	$(CP) -f man/di.1 $(INST_MANDIR)/man1/$(MAN_TARGET)

###
# mkc environment

$(MKC_ENV):	$(MKC_ENV_CONF)
	@-$(RM) -f $(MKC_ENV) tests.done
	CC=$(CC) $(_MKCONFIG_SHELL) $(MKC_DIR)/mkconfig.sh $(MKC_ENV_CONF)

###
# specific builds

.PHONY: os2-gcc
os2-gcc:
	@echo ':' > $(MKC_ENV);chmod a+rx $(MKC_ENV)
	$(MAKE) MKCONFIG_TYPE=perl \
		CC=gcc LD=gcc EXE_EXT=".exe" OBJ_EXT=".o" \
		DI_CFLAGS="$(DI_CFLAGS) -g -O2" \
		LDFLAGS="-g -O2 -Zexe" di.exe

###
# programs

.PHONY: di-programs
di-programs:	di$(EXE_EXT) dimathtest$(EXE_EXT) getoptn_test$(EXE_EXT)

###
# configuration file

config.h:	$(MKC_ENV) $(MKC_CONF)
	@-$(RM) -f config.h tests.done
	@if [ "$(MKCONFIG_TYPE)" = "sh" -o "$(MKCONFIG_TYPE)" = "" ]; then \
		. ./$(MKC_ENV);$(_MKCONFIG_SHELL) \
		$(MKC_DIR)/mkconfig.sh \
		$(MKC_CONF); fi
	@if [ "$(MKCONFIG_TYPE)" = "perl" ]; then \
		. ./$(MKC_ENV);perl \
		$(MKC_DIR)/mkconfig.pl \
		$(MKC_CONF); fi

$(MKC_REQLIB):	config.h
	@$(_MKCONFIG_SHELL) $(MKC_DIR)/mkc.sh -reqlib \
		-o $(MKC_REQLIB) config.h

###
# executables

LIBOBJECTS = dilib$(OBJ_EXT) didiskutil$(OBJ_EXT) \
		digetentries$(OBJ_EXT) digetinfo$(OBJ_EXT) \
		diquota$(OBJ_EXT) dizone$(OBJ_EXT) getoptn$(OBJ_EXT) \
		dioptions$(OBJ_EXT) distrutils$(OBJ_EXT)

MAINOBJECTS = di$(OBJ_EXT)

libdi$(SHLIB_EXT):	$(MKC_REQLIB) $(LIBOBJECTS)
	@$(_MKCONFIG_SHELL) $(MKC_DIR)/mkc.sh \
		-link -shared $(MKC_ECHO) \
                -r $(MKC_REQLIB) \
		-o libdi$(SHLIB_EXT) \
		$(LIBOBJECTS)

di$(EXE_EXT):	$(MKC_REQLIB) $(MAINOBJECTS) libdi$(SHLIB_EXT)
	@$(_MKCONFIG_SHELL) $(MKC_DIR)/mkc.sh \
		-link -exec $(MKC_ECHO) \
		-r $(MKC_REQLIB) \
		-o di$(EXE_EXT) \
		$(MAINOBJECTS) \
		libdi$(SHLIB_EXT) \
		-R $(LIBDIR)

dimathtest$(EXE_EXT):	dimathtest$(OBJ_EXT)
	@$(_MKCONFIG_SHELL) $(MKC_DIR)/mkc.sh \
		-link -exec $(MKC_ECHO) \
		-r $(MKC_REQLIB) \
		-o dimathtest$(EXE_EXT) \
		dimathtest$(OBJ_EXT)

getoptn_test$(EXE_EXT):	getoptn_test$(OBJ_EXT) distrutils$(OBJ_EXT)
	$(_MKCONFIG_SHELL) $(MKC_DIR)/mkc.sh \
		-link -exec $(MKC_ECHO) \
		-o getoptn_test$(EXE_EXT) \
		getoptn_test$(OBJ_EXT) \
		distrutils$(OBJ_EXT)

# for ms cl
#di$(EXE_EXT):	$(MAINOBJECTS) $(LIBOBJECTS)
#	$(LD) -Fedi$(EXE_EXT) $(MAINOBJECTS) $(LIBOBJECTS)

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

# diinternal.h: config.h disystem.h di.h dimath.h dioptions.h
# dioptions.h: disystem.h
# diquota.h: disystem.h dimath.h
# dizone.h: disystem.h

di$(OBJ_EXT):		di.c config.h di.h disystem.h distrutils.h version.h

didiskutil$(OBJ_EXT):	didiskutil.c config.h di.h disystem.h diinternal.h \
				distrutils.h dimath.h dimntopt.h \
				dioptions.h

digetentries$(OBJ_EXT):	digetentries.c config.h di.h disystem.h diinternal.h \
				distrutils.h dimntopt.h \
				dimath.h dioptions.h

digetinfo$(OBJ_EXT):	digetinfo.c config.h di.h disystem.h diinternal.h \
				dimntopt.h \
				dimath.h dioptions.h

dilib$(OBJ_EXT):	dilib.c config.h di.h disystem.h diinternal.h \
				dizone.h diquota.h dioptions.h version.h \
                                dimath.h

dimathtest$(OBJ_EXT):	dimathtest.c config.h dimath.h

dioptions$(OBJ_EXT):	dioptions.c config.h di.h diinternal.h distrutils.h \
				getoptn.h dioptions.h version.h \
				disystem.h dimath.h

diquota$(OBJ_EXT):	diquota.c config.h di.h diquota.h \
				dimath.h diinternal.h \
				disystem.h dioptions.h

distrutils$(OBJ_EXT):	distrutils.c config.h distrutils.h

dizone$(OBJ_EXT):	dizone.c config.h di.h dizone.h \
				disystem.h

getoptn$(OBJ_EXT):	getoptn.c config.h distrutils.h getoptn.h

getoptn_test$(OBJ_EXT):	getoptn.c config.h distrutils.h getoptn.h
	@$(_MKCONFIG_SHELL) $(MKC_DIR)/mkc.sh \
		-compile $(MKC_ECHO) \
		-DTEST_GETOPTN=1 $(DI_CFLAGS) \
		-o getoptn_test$(OBJ_EXT) getoptn.c

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

# DO NOT DELETE
