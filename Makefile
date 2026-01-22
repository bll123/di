#
#  di makefile - C
#
#  Copyright 2001-2026 Brad Lanam Walnut Creek CA, USA
#  Copyright 2023-2026 Brad Lanam, Pleasant Hill, CA
#

# does not work on many systems
# MAKEFLAGS += --no-print-directory

# for checking cmake
CMAKE_REQ_MAJ_VERSION = 3
CMAKE_REQ_MIN_VERSION = 18
BUILDDIR = build
DI_BUILD = Release

# need to turn off for testing
PMODE=--parallel

# DI_USE_MATH = DI_GMP
# DI_USE_MATH = DI_MPDECIMAL
# DI_USE_MATH = DI_TOMMATH
# DI_USE_MATH = DI_INTERNAL

# for mkconfig
MKC_CONFDIR = mkc_config
MKC_FILES = mkc_files
MKC_DI_ENV = di.env
MKC_LIBDI_ENV = libdi.env
MKC_DI_CONF = $(MKC_CONFDIR)/di.mkc
MKC_LIBDI_CONF = $(MKC_CONFDIR)/libdi.mkc
MKC_DI_ENV_CONF = $(MKC_CONFDIR)/di-env.mkc
MKC_LIBDI_ENV_CONF = $(MKC_CONFDIR)/libdi-env.mkc
#
#    To turn off stack fortification:
#
#      make -e DI_FORTIFY=N PREFIX=$HOME/local
#      make -e DI_FORTIFY=N PREFIX=$HOME/local install
#
DI_FORTIFY = Y

OBJ_EXT = .o
EXE_EXT =

###
# installation locations
#  the cmake install only uses PREFIX
#
PREFIX =
BINDIR = $(PREFIX)/bin
LIBDIR = $(PREFIX)/$(_MKCONFIG_LIBNAME)
INCDIR = $(PREFIX)/include
PKGCDIR = $(LIBDIR)/pkgconfig
SHAREDIR = $(PREFIX)/share
MANDIR = $(SHAREDIR)/man
LOCALEDIR = $(SHAREDIR)/locale

INST_DIR = $(DESTDIR)$(PREFIX)
INST_INCDIR = $(DESTDIR)$(INCDIR)
INST_BINDIR = $(DESTDIR)$(BINDIR)
INST_LIBDIR = $(DESTDIR)$(LIBDIR)
INST_SHAREDIR = $(DESTDIR)$(SHAREDIR)
INST_PKGCDIR = $(DESTDIR)$(PKGCDIR)
INST_MANDIR = $(DESTDIR)$(MANDIR)
INST_LOCALEDIR = $(DESTDIR)$(LOCALEDIR)

###
# mkconfig variables

MKC_DIR = mkconfig
MKCONFIG_TYPE = sh
MKC_DI_REQLIB = di.reqlibs
MKC_LIBDI_REQLIB = libdi.reqlibs
MKC_ECHO =
#MKC_ECHO = -e

###
# generic targets

.PHONY: all
all:
	@$(MAKE) -e TARGET=$@ switcher

.PHONY: install
install:
	@$(MAKE) -e TARGET=$@ switcher

.PHONY: test
test:
	@$(MAKE) -e TARGET=$@ switcher

.PHONY: chkswitcher
chkswitcher:
	@$(MAKE) -e TARGET=$@ switcher

# checks the cmake version, and builds using either cmake or mkconfig
.PHONY: switcher
switcher:
	@if [ "$(PREFIX)" = "" ]; then echo "No prefix set"; exit 1; fi
	@bldval=`./utils/chkcmake.sh \
	    $(CMAKE_REQ_MAJ_VERSION) $(CMAKE_REQ_MIN_VERSION)` ; \
	if [ $$bldval = cmake ]; then \
	  $(MAKE) -e cmake-$(TARGET) ; \
	else \
	  $(MAKE) -e mkc-$(TARGET) ; \
	fi

###
# cleaning

# clean temporary files
.PHONY: tclean
tclean:
	@-rm -f w ww asan.* *.orig comparemath.log \
		dep-*.txt examples/diex \
		>/dev/null 2>&1; exit 0
	@-find . -name '*~' -print0 | xargs -0 rm > /dev/null 2>&1; exit 0
	@-find . -name '*.orig' -print0 | xargs -0 rm > /dev/null 2>&1; exit 0

# leaves config.h diconfig.h
.PHONY: clean
clean:
	@$(MAKE) tclean
	@-rm -f \
		di libdi.* dimathtest getoptn_test \
		di.exe dimathtest.exe getoptn_test.exe \
		*.o *.obj \
		$(MKC_FILES)/mkc_compile.log \
		tests.d/chksh* \
		>/dev/null 2>&1; exit 0
	@-test -d $(BUILDDIR) && cmake --build $(BUILDDIR) --target clean

.PHONY: realclean
realclean:
	@$(MAKE) clean >/dev/null 2>&1
	@-rm -rf config.h diconfig.h \
		$(MKC_DI_ENV) $(MKC_LIBDI_ENV) \
                $(MKC_DI_REQLIB) $(MKC_LIBDI_REQLIB) \
		_mkconfig_runtests \
		>/dev/null 2>&1; exit 0

.PHONY: distclean
distclean:
	@$(MAKE) realclean >/dev/null 2>&1
	@-rm -rf test_di _mkconfig_runtests \
		results x x.m x.g x.t x.i \
		$(MKC_FILES) \
		$(BUILDDIR) \
		>/dev/null 2>&1; exit 0

###
# utility

.PHONY: tar
tar:
	./utils/mktar.sh

.PHONY: betatar
betatar:
	./utils/mktar.sh beta

###
# cmake

.PHONY: cmake-debug
cmake-debug:
	$(MAKE) -e DI_BUILD=Debug cmake-all

.PHONY: cmake-sanitize
cmake-sanitize:
	$(MAKE) -e DI_BUILD=SanitizeAddress cmake-all

# parallel doesn't seem to work under msys2
# cmake doesn't seem to support parallel under *BSD
#   (passing -j w/o arguments, and *BSD complains)
.PHONY: cmake-all
cmake-all:
	echo "## Building with cmake"
	@case `uname -s` in \
	  CYGWIN*) \
	    COMP=$(CC) \
	    $(MAKE) -e cmake-unix; \
	    $(MAKE) -e cmake-build; \
	    ;; \
	  MSYS*|MINGW*) \
	    COMP=$(CC) \
	    $(MAKE) -e cmake-windows; \
	    $(MAKE) -e cmake-build; \
	    ;; \
	  *BSD*) \
	    COMP=$(CC) \
	    $(MAKE) -e cmake-unix; \
	    $(MAKE) -e cmake-build; \
	    ;; \
	  *) \
	    COMP=$(CC) \
	    $(MAKE) -e cmake-unix; \
	    pmode=$(PMODE) $(MAKE) -e cmake-build; \
	    ;; \
	esac

.PHONY: cmakeclang
cmakeclang:
	case `uname -s` in \
	  *BSD*) \
	    COMP=$(CC) \
	    $(MAKE) -e cmake-unix; \
	    $(MAKE) -e cmake-build; \
	    ;; \
	  MSYS*|MINGW*) \
	    COMP=/ucrt64/bin/clang.exe \
	    $(MAKE) -e cmake-windows; \
	    $(MAKE) -e cmake-build; \
	    ;; \
	  *) \
	    $(MAKE) -e cmake-unix; \
	    pmode=$(PMODE) $(MAKE) -e cmake-build; \
	    ;; \
	esac

# --debug-trycompile
# internal use
.PHONY: cmake-unix
cmake-unix:
	@if [ "$(PREFIX)" = "" ]; then echo "No prefix set"; exit 1; fi
	@test -d $(BUILDDIR) || mkdir $(BUILDDIR)
	cmake \
		-DCMAKE_C_COMPILER=$(COMP) \
		-DCMAKE_INSTALL_PREFIX="$(PREFIX)" \
		-DDI_BUILD:STATIC=$(DI_BUILD) \
		-DDI_BUILD_SYS:STATIC=make-cmake \
		-S . -B $(BUILDDIR) -Werror=deprecated

# internal use
.PHONY: cmake-windows
cmake-windows:
	@if [ "$(PREFIX)" = "" ]; then echo "No prefix set"; exit 1; fi
	@test -d $(BUILDDIR) || mkdir $(BUILDDIR)
	cmake \
		-DCMAKE_C_COMPILER=$(COMP) \
		-DCMAKE_INSTALL_PREFIX="$(PREFIX)" \
		-DDI_BUILD:STATIC=$(DI_BUILD) \
		-DDI_BUILD_SYS:STATIC=make-cmake \
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
	cmake --build $(BUILDDIR) --target ditest

.PHONY: cmake-chkswitcher
cmake-chkswitcher:
	@echo "cmake"

###
# main

# have to get various environment variables set up.

# don't know any good way to determine if lib64 is preferred
.PHONY: mkc-all
mkc-all:
	echo "## Building with mkconfig"
	@. ./VERSION.txt ; \
	$(MAKE) -e mkc-sh

.PHONY: mkc-sh
mkc-sh:
	$(MAKE) -e MKCONFIG_TYPE=sh \
		DI_PREFIX=$(PREFIX) \
		mkc-di-all

.PHONY: mkc-perl
mkc-perl:
	DI_PREFIX=$(PREFIX) \
		DI_VERSION=$(DI_VERSION) \
		DI_LIBVERSION=$(DI_LIBVERSION) \
		DI_SOVERSION=$(DI_SOVERSION) \
		DI_RELEASE_STATUS=$(DI_RELEASE_STATUS) \
		$(MAKE) -e MKCONFIG_TYPE=perl \
		mkc-di-all

.PHONY: mkc-test
mkc-test:
	./tests/localtest.sh mkc

.PHONY: mkc-chkswitcher
mkc-chkswitcher:
	@echo "mkc"

.PHONY: mkc-install
mkc-install: $(MKC_DI_ENV)
	@. ./VERSION.txt ; \
	. ./$(MKC_DI_ENV);$(MAKE) -e \
	      PREFIX=$(PREFIX) mkc-install-all

###
# installation

.PHONY: mkc-install-all
mkc-install-all:
	$(MAKE) -e mkc-install-di
	$(MAKE) -e mkc-install-include
	$(MAKE) -e mkc-install-libdi
	-$(MAKE) -e mkc-install-po
	$(MAKE) -e mkc-install-man
	$(MAKE) -e mkc-install-pc
	$(MAKE) -e mkc-install-examples

.PHONY: mkc-install-examples
mkc-install-examples:
	test -d $(INST_SHAREDIR)/di || mkdir -p $(INST_SHAREDIR)/di
	cp -rf examples $(INST_SHAREDIR)/di

.PHONY: mkc-install-po
mkc-install-po:
	-./utils/instpo.sh "`pwd`/po" $(INST_LOCALEDIR) "`pwd`/tmp"

# -lintl and -liconv are removed, as those libraries are only needed
# for the main di program, not to link with the library.
# just use a modern version of runpath for gcc/clang
.PHONY: mkc-install-pc
mkc-install-pc: $(MKC_LIBDI_REQLIB)
	-./utils/instpc.sh "$(INST_PKGCDIR)" "$(PREFIX)" "$(INCDIR)" "$(LIBDIR)"

.PHONY: mkc-install-di
mkc-install-di:
	test -d $(INST_BINDIR) || mkdir -p $(INST_BINDIR)
	cp -f di$(EXE_EXT) $(INST_BINDIR)

.PHONY: mkc-install-include
mkc-install-include:
	test -d $(INST_INCDIR) || mkdir -p $(INST_INCDIR)
	cp -f di.h $(INST_INCDIR)

# not sure about how the naming works on aix
# OpenBSD only has the main version, no plain .so
.PHONY: mkc-install-libdi
mkc-install-libdi:
	test -d $(INST_LIBDIR) || mkdir -p $(INST_LIBDIR)
	./utils/instlibdi.sh "$(INST_LIBDIR)" "$(INST_BINDIR)" $(DI_LIBVERSION) $(DI_SOVERSION)

# incorrect for netbsd
.PHONY: mkc-install-man
mkc-install-man:
	./utils/instman.sh "$(PREFIX)" "$(MANDIR)"

###
# mkc environment

$(MKC_DI_ENV):	$(MKC_DI_ENV_CONF)
	@-rm -f $(MKC_DI_ENV)
	CC=$(CC) DI_FORTIFY=$(DI_FORTIFY) \
	    $(_MKCONFIG_SHELL) $(MKC_DIR)/mkconfig.sh $(MKC_DI_ENV_CONF)

$(MKC_LIBDI_ENV):	$(MKC_LIBDI_ENV_CONF)
	@-rm -f $(MKC_LIBDI_ENV)
	CC=$(CC) DI_FORTIFY=$(DI_FORTIFY) \
	    $(_MKCONFIG_SHELL) $(MKC_DIR)/mkconfig.sh $(MKC_LIBDI_ENV_CONF)

###
# programs

.PHONY: mkc-di-all
mkc-di-all: $(MKC_LIBDI_ENV) $(MKC_DI_ENV)
	@. ./$(MKC_DI_ENV);. ./$(MKC_LIBDI_ENV);$(MAKE) -e mkc-di-lib
	@. ./$(MKC_DI_ENV);$(MAKE) -e mkc-di-programs

.PHONY: mkc-di-programs
mkc-di-programs:	di$(EXE_EXT) getoptn_test$(EXE_EXT)

.PHONY: mkc-di-lib
mkc-di-lib: 	libdi$(SHLIB_EXT) dimathtest$(EXE_EXT)

###
# configuration file

config.h:	$(MKC_DI_ENV) $(MKC_LIBDI_ENV) $(MKC_LIBDI_CONF)
	@-rm -f config.h
	@if [ "$(MKCONFIG_TYPE)" = "sh" -o "$(MKCONFIG_TYPE)" = "" ]; then \
		. ./$(MKC_DI_ENV);. ./$(MKC_LIBDI_ENV);$(_MKCONFIG_SHELL) \
		$(MKC_DIR)/mkconfig.sh \
		$(MKC_LIBDI_CONF); fi
	@if [ "$(MKCONFIG_TYPE)" = "perl" ]; then \
		. ./$(MKC_DI_ENV);. ./$(MKC_LIBDI_ENV);perl \
		$(MKC_DIR)/mkconfig.pl \
		$(MKC_LIBDI_CONF); fi

diconfig.h:	$(MKC_DI_ENV) $(MKC_DI_CONF)
	@-rm -f diconfig.h
	@if [ "$(MKCONFIG_TYPE)" = "sh" -o "$(MKCONFIG_TYPE)" = "" ]; then \
		. ./$(MKC_DI_ENV);$(_MKCONFIG_SHELL) \
		$(MKC_DIR)/mkconfig.sh \
		$(MKC_DI_CONF); fi
	@if [ "$(MKCONFIG_TYPE)" = "perl" ]; then \
		. ./$(MKC_DI_ENV);perl \
		$(MKC_DIR)/mkconfig.pl \
		$(MKC_DI_CONF); fi

$(MKC_DI_REQLIB):	diconfig.h
	@$(_MKCONFIG_SHELL) $(MKC_DIR)/mkc.sh -reqlib \
		-o $(MKC_DI_REQLIB) diconfig.h

$(MKC_LIBDI_REQLIB):	config.h
	@$(_MKCONFIG_SHELL) $(MKC_DIR)/mkc.sh -reqlib \
		-o $(MKC_LIBDI_REQLIB) config.h

###
# executables

LIBOBJECTS = dilib$(OBJ_EXT) didiskutil$(OBJ_EXT) \
		digetentries$(OBJ_EXT) digetinfo$(OBJ_EXT) dimath$(OBJ_EXT) \
		diquota$(OBJ_EXT) dizone$(OBJ_EXT) getoptn$(OBJ_EXT) \
		dioptions$(OBJ_EXT) distrutils$(OBJ_EXT)

MAINOBJECTS = di$(OBJ_EXT)

libdi$(SHLIB_EXT):	$(MKC_LIBDI_REQLIB) $(LIBOBJECTS)
	@$(_MKCONFIG_SHELL) $(MKC_DIR)/mkc.sh \
		-link -shared $(MKC_ECHO) \
		-r $(MKC_LIBDI_REQLIB) \
		-o libdi$(SHLIB_EXT) \
		$(LIBOBJECTS) \
		-R $(LIBDIR)

di$(EXE_EXT):	$(MKC_DI_REQLIB) $(MAINOBJECTS) libdi$(SHLIB_EXT)
	@$(_MKCONFIG_SHELL) $(MKC_DIR)/mkc.sh \
		-link -exec $(MKC_ECHO) \
		-r $(MKC_DI_REQLIB) \
		-o di$(EXE_EXT) \
		$(MAINOBJECTS) \
		-L . -ldi \
		-R $(LIBDIR)

dimathtest$(EXE_EXT):	dimathtest$(OBJ_EXT) dimath$(OBJ_EXT)
	@$(_MKCONFIG_SHELL) $(MKC_DIR)/mkc.sh \
		-link -exec $(MKC_ECHO) \
		-r $(MKC_LIBDI_REQLIB) \
		-o dimathtest$(EXE_EXT) \
		dimathtest$(OBJ_EXT) dimath$(OBJ_EXT) \
		-R $(LIBDIR)

getoptn_test$(EXE_EXT):	getoptn_test$(OBJ_EXT) distrutils$(OBJ_EXT)
	@$(_MKCONFIG_SHELL) $(MKC_DIR)/mkc.sh \
		-link -exec $(MKC_ECHO) \
		-o getoptn_test$(EXE_EXT) \
		getoptn_test$(OBJ_EXT) \
		distrutils$(OBJ_EXT)

###
# objects

.c$(OBJ_EXT):
	@$(_MKCONFIG_SHELL) $(MKC_DIR)/mkc.sh -compile -shared $(MKC_ECHO) $<

di$(OBJ_EXT):		di.c

didiskutil$(OBJ_EXT):	didiskutil.c

digetentries$(OBJ_EXT):	digetentries.c

digetinfo$(OBJ_EXT):	digetinfo.c

dilib$(OBJ_EXT):	dilib.c

dimath$(OBJ_EXT):	dimath.c

dimathtest$(OBJ_EXT):	dimathtest.c

dioptions$(OBJ_EXT):	dioptions.c

diquota$(OBJ_EXT):	diquota.c

distrutils$(OBJ_EXT):	distrutils.c

dizone$(OBJ_EXT):	dizone.c

getoptn$(OBJ_EXT):	getoptn.c

getoptn_test$(OBJ_EXT):	getoptn.c
	@$(_MKCONFIG_SHELL) $(MKC_DIR)/mkc.sh \
		-compile $(MKC_ECHO) \
		-DTEST_GETOPTN=1 \
		-o getoptn_test$(OBJ_EXT) getoptn.c

# DO NOT DELETE

di.o: diconfig.h
didiskutil.o: config.h
didiskutil.o: di.h disystem.h
didiskutil.o: diinternal.h
didiskutil.o: dimath_mp.h
didiskutil.o: dimath_mpdec.h dimath_gmp.h dimath_tommath.h dimath_internal.h
didiskutil.o: dimath.h distrutils.h dimntopt.h
digetentries.o: config.h
digetentries.o:  di.h disystem.h
digetentries.o:  diinternal.h dimath_mp.h
digetentries.o: dimath_mpdec.h dimath_gmp.h dimath_tommath.h dimath_internal.h
digetentries.o: dimath.h distrutils.h dimntopt.h dioptions.h getoptn.h
digetinfo.o: config.h
digetinfo.o:   di.h disystem.h
digetinfo.o:   diinternal.h
digetinfo.o: dimath_mp.h
digetinfo.o: dimath_mpdec.h dimath_gmp.h dimath_tommath.h dimath_internal.h
digetinfo.o: dimath.h dimntopt.h
digetinfo.o:   distrutils.h
digetinfo.o: dioptions.h getoptn.h
dilib.o: config.h
dilib.o:  di.h disystem.h
dilib.o:  dimath.h dimath_mp.h
dilib.o: dimath_mpdec.h dimath_gmp.h dimath_tommath.h dimath_internal.h
dilib.o: diinternal.h dizone.h dioptions.h getoptn.h diquota.h distrutils.h
dimath.o: config.h  dimath.h
dimathtest.o: config.h
dimathtest.o:   dimath.h dimath_mp.h
dimathtest.o: dimath_mpdec.h dimath_gmp.h dimath_tommath.h dimath_internal.h
dioptions.o: config.h
dioptions.o:   di.h disystem.h
dioptions.o:   diinternal.h
dioptions.o: dimath_mp.h dimath_mpdec.h dimath_gmp.h dimath_tommath.h dimath_internal.h
dioptions.o: dimath.h distrutils.h getoptn.h
dioptions.o: dioptions.h
diquota.o: config.h
diquota.o:  di.h disystem.h
diquota.o:  dimath.h dimath_mp.h
diquota.o: dimath_mpdec.h dimath_gmp.h dimath_tommath.h dimath_internal.h
diquota.o: diquota.h diinternal.h distrutils.h dioptions.h getoptn.h
distrutils.o: config.h
distrutils.o:  distrutils.h
dizone.o: config.h
dizone.o: di.h dizone.h disystem.h
dizone.o:  dioptions.h getoptn.h distrutils.h
getoptn.o: config.h
getoptn.o:  disystem.h
getoptn.o:  distrutils.h getoptn.h
