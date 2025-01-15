#
#  di makefile - C
#
#  Copyright 2001-2018 Brad Lanam Walnut Creek CA, USA
#  Copyright 2023 Brad Lanam, Pleasant Hill, CA
#

# cmake
# one less than the required version
# 3.${CMAKE_REQ_VERSION}
CMAKE_REQ_VERSION=10

BUILDDIR = build
# DI_USE_MATH = DI_GMP
# DI_USE_MATH = DI_TOMMATH
# DI_USE_MATH = DI_INTERNAL

# mkconfig
MKC_PREFIX = di
MKC_CONFDIR = mkc_config
MKC_FILES = mkc_files
MKC_OUTPUT = config.h
MKC_ENV = $(MKC_PREFIX).env
MKC_ENV_SHR = $(MKC_PREFIX)-shared.env

MKC_CONF = $(MKC_CONFDIR)/$(MKC_PREFIX).mkc
MKC_ENV_CONF = $(MKC_CONFDIR)/$(MKC_PREFIX)-env.mkc
MKC_ENV_SHR_CONF = $(MKC_CONFDIR)/$(MKC_PREFIX)-env-shared.mkc
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
PREFIX = $(prefix)
BINDIR = $(PREFIX)/bin
DATADIR = $(PREFIX)/share
MANDIR = $(DATADIR)/man
LOCALEDIR = $(DATADIR)/locale

###
# additional flags/libraries
#
DI_SHARED =
DI_CFLAGS = -DDI_LOCALE_DIR=\\\"$(LOCALEDIR)\\\"

###
# mkconfig variables

MKC_DIR = mkconfig
MKCONFIG_TYPE = sh
MKC_REQLIB = di.reqlibs
MKC_ECHO =
#MKC_ECHO = -e

###
# cmake

.PHONY: all
all:
	@cmv=0 ; \
	cmv=`cmake --version | \
	  sed -n -e '/version/ s,[^0-9]*3\.\([0-9]*\).*,\1, p'` ; \
	if [ "$${cmv}" -ge $(CMAKE_REQ_VERSION) ]; then \
	  $(MAKE) cmake-all; \
	else \
	  $(MAKE) mkc-all; \
	fi

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
		-DDI_LOCALE_DIR:STATIC=$(LOCALEDIR) \
		-DDI_USE_MATH:STATIC=$(DI_USE_MATH) \
		-S . -B $(BUILDDIR) -Werror=deprecated

# internal use
.PHONY: cmake-windows
cmake-windows:
	@if [ "$(PREFIX)" = "" ]; then echo "No prefix set"; exit 1; fi
	cmake \
		-DCMAKE_C_COMPILER=$(COMP) \
		-DCMAKE_INSTALL_PREFIX="$(PREFIX)" \
		-DDI_LOCALE_DIR:STATIC=$(LOCALEDIR) \
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

###
# environment

$(MKC_ENV):	$(MKC_ENV_CONF) dioptions.dat
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
	@$(MAKE) dioptions.dat
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
	@$(MAKE) dioptions.dat
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
	cp features/dioptions.dat dioptions.dat
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
	copy /y /b NUL:+features\dioptions.dat dioptions.dat
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

###
# cleaning

# leaves:
#   config.h di.reqlibs
#   dioptions.dat, tests.done, test_di, $(MKC_ENV), $(MKC_ENV_SHR)
.PHONY: clean
clean:
	@-$(RM) -f w ww di mi libdi.* \
		di.exe mingw-di.exe mi.exe \
		diskspace.so diskspace.dylib diskspace.dll \
		*.o *.obj $(MKC_FILES)/mkconfig.log \
		tests.done $(MKC_FILES)/_tmp_mkconfig tests.d/chksh* \
		$(MKC_FILES)/mkconfig.cache mkc*.vars \
		getoptn_test* gconfig.h getoptn.reqlibs \
		$(MKC_FILES)/mkconfig.reqlibs $(MKC_FILES)/mkc_compile.log \
		tests.d/test_order.tmp >/dev/null 2>&1; exit 0
	@-find . -name '*~' -print0 | xargs -0 rm > /dev/null 2>&1; exit 0

# leaves:
#   _mkconfig_runtests, mkc_files, dioptions.dat
#   tests.done, test_di
.PHONY: realclean
realclean:
	@$(MAKE) clean >/dev/null 2>&1
	@-$(RM) -rf config.h gconfig.h \
		$(MKC_ENV) $(MKC_ENV_SHR) $(MKC_REQLIB) \
		>/dev/null 2>&1; exit 0

# leaves:
#   dioptions.dat
.PHONY: distclean
distclean:
	@$(MAKE) realclean >/dev/null 2>&1
	@-$(RM) -rf tests.done test_di \
		_mkconfig_runtests \
		$(MKC_FILES) \
		*~ *.orig */*.orig \
		build \
		>/dev/null 2>&1; exit 0

###
# installation

.PHONY: install
install-mkc:
	$(MAKE) all
#	. ./$(MKC_ENV);$(MAKE) -e PREFIX=$(PREFIX) \
#		LOCALEDIR=$(LOCALEDIR) install

###
# programs

.PHONY: di-programs
di-programs:	di$(EXE_EXT) dimathtest$(EXE_EXT) getoptn_test$(EXE_EXT)

###
# configuration file

dioptions.dat:	features/dioptions.dat
	$(MAKE) dioptions.dat

config.h:	$(MKC_ENV) dioptions.dat $(MKC_CONF)
	@-$(RM) -f config.h tests.done
	@if [ "$(DI_NO_NLS)" != "" ]; then \
		echo "*** User requested no NLS"; \
		$(MKC_DIR)/mkc.sh -setopt -o dioptions.dat NLS F; fi
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
		diquota$(OBJ_EXT) getoptn$(OBJ_EXT) \
		options$(OBJ_EXT) strutils$(OBJ_EXT)

MAINOBJECTS = di$(OBJ_EXT) display$(OBJ_EXT)

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
		libdi$(SHLIB_EXT)

dimathtest$(EXE_EXT):	dimathtest$(OBJ_EXT)
	@$(_MKCONFIG_SHELL) $(MKC_DIR)/mkc.sh \
		-link -exec $(MKC_ECHO) \
		-r $(MKC_REQLIB) \
		-o dimathtest$(EXE_EXT) \
		dimathtest$(OBJ_EXT) \
		-lm

getoptn_test$(EXE_EXT):	getoptn_test$(OBJ_EXT)
	$(_MKCONFIG_SHELL) $(MKC_DIR)/mkc.sh \
		-link -exec $(MKC_ECHO) \
		-o getoptn_test$(EXE_EXT) \
		getoptn_test$(OBJ_EXT) \
		-lm

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

di$(OBJ_EXT):		di.c config.h di.h dimath.h dilib.h version.h

dilib$(OBJ_EXT):	dilib.c config.h di.h dimath.h dilib.h \
				options.h version.h

digetinfo$(OBJ_EXT):	digetinfo.c config.h di.h dimath.h dimntopt.h

didiskutil$(OBJ_EXT):	didiskutil.c config.h di.h dimath.h strutils.h \
				dimntopt.h

digetentries$(OBJ_EXT):	digetentries.c config.h di.h dimath.h strutils.h \
				dimntopt.h

diquota$(OBJ_EXT):	diquota.c config.h di.h dimath.h

display$(OBJ_EXT):	display.c config.h di.h dimath.h strutils.h \
				display.h options.h version.h

getoptn$(OBJ_EXT):	getoptn.c config.h strutils.h getoptn.h

options$(OBJ_EXT):	options.c config.h di.h dimath.h strutils.h options.h

strutils$(OBJ_EXT):	strutils.c config.h strutils.h

dimathtest$(OBJ_EXT):	dimathtest.c config.h dimath.h

getoptn_test$(OBJ_EXT):	getoptn.c config.h getoptn.h
	@$(_MKCONFIG_SHELL) $(MKC_DIR)/mkc.sh -compile $(MKC_ECHO) \
		-DTEST_GETOPTN=1 $(DI_CFLAGS) -o getoptn_test$(OBJ_EXT) $<

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

