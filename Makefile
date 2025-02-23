#
#  di makefile - C
#
#  Copyright 2001-2018 Brad Lanam Walnut Creek CA, USA
#  Copyright 2023-2025 Brad Lanam, Pleasant Hill, CA
#

# for checking cmake
CMAKE_REQ_MAJ_VERSION = 3
CMAKE_REQ_MIN_VERSION = 13
BUILDDIR = build
DI_BUILD = Release

# DI_USE_MATH = DI_GMP
# DI_USE_MATH = DI_TOMMATH
# DI_USE_MATH = DI_INTERNAL

# for mkconfig
MKC_CONFDIR = mkc_config
MKC_FILES = mkc_files
MKC_ENV = di.env
MKC_CONF = $(MKC_CONFDIR)/di.mkc
MKC_ENV_CONF = $(MKC_CONFDIR)/di-env.mkc
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
LIBNM = lib
LIBDIR = $(PREFIX)/$(LIBNM)
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
MKC_REQLIB = di.reqlibs
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
	@-rm -f w ww asan.* *.orig \
		dep-*.txt >/dev/null 2>&1; exit 0
	@-find . -name '*~' -print0 | xargs -0 rm > /dev/null 2>&1; exit 0
	@-find . -name '*.orig' -print0 | xargs -0 rm > /dev/null 2>&1; exit 0

# leaves config.h
.PHONY: clean
clean:
	@$(MAKE) tclean
	@-rm -f \
		di libdi.* dimathtest getoptn_test \
		di.exe libdi.dll dimathtest.exe getoptn_test.exe \
		*.o *.obj \
		$(MKC_FILES)/mkc_compile.log \
		tests.d/chksh* \
		>/dev/null 2>&1; exit 0
	@-test -d $(BUILDDIR) && cmake --build $(BUILDDIR) --target clean

.PHONY: realclean
realclean:
	@$(MAKE) clean >/dev/null 2>&1
	@-rm -rf config.h \
		$(MKC_ENV) $(MKC_REQLIB) \
		_mkconfig_runtests \
		>/dev/null 2>&1; exit 0

.PHONY: distclean
distclean:
	@$(MAKE) realclean >/dev/null 2>&1
	@-rm -rf test_di _mkconfig_runtests \
		test_results x \
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
	    pmode=--parallel $(MAKE) -e cmake-build; \
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
	    pmode=--parallel $(MAKE) -e cmake-build; \
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
	./build/dimathtest
	./build/getoptn_test

.PHONY: cmake-chkswitcher
cmake-chkswitcher:
	@echo "cmake"

###
# main

# have to get various environment variables set up.

# don't know any good way to determine if lib64 is preferred
.PHONY: mkc-all
mkc-all:
	@. ./VERSION.txt ; \
	libnm=`./utils/chklibnm.sh` ; \
	$(MAKE) -e LIBNM=$$libnm mkc-sh

.PHONY: mkc-sh
mkc-sh:	$(MKC_ENV)
	. ./$(MKC_ENV);$(MAKE) -e MKCONFIG_TYPE=sh \
		DI_PREFIX=$(PREFIX) \
		mkc-di-programs

.PHONY: mkc-perl
mkc-perl:	$(MKC_ENV)
	. ./$(MKC_ENV) ; \
		DI_PREFIX=$(PREFIX) \
		DI_VERSION=$(DI_VERSION) \
		DI_LIBVERSION=$(DI_LIBVERSION) \
		DI_SOVERSION=$(DI_SOVERSION) \
		DI_RELEASE_STATUS=$(DI_RELEASE_STATUS) \
		$(MAKE) -e MKCONFIG_TYPE=perl \
		mkc-di-programs

.PHONY: mkc-test
mkc-test:
	. ./$(MKC_ENV); \
	    ./dimathtest$(EXE_EXT); \
	    ./getoptn_test$(EXE_EXT)

.PHONY: mkc-chkswitcher
mkc-chkswitcher:
	@echo "mkc"

.PHONY: mkc-install
mkc-install: $(MKC_ENV) mkc-all
	@. ./VERSION.txt ; \
	libnm=`./utils/chklibnm.sh` ; \
	. ./$(MKC_ENV);$(MAKE) -e \
	      PREFIX=$(PREFIX) LIBNM=$$libnm mkc-install-all

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

.PHONY: mkc-install-po
mkc-install-po:
	-./utils/instpo.sh "`pwd`/po" $(INST_LOCALEDIR) "`pwd`/tmp"

# -lintl and -liconv are removed, as those libraries are only needed
# for the main di program, not to link with the library.
.PHONY: mkc-install-pc
mkc-install-pc: $(MKC_REQLIB)
	test -d $(INST_PKGCDIR) || mkdir -p $(INST_PKGCDIR)
	cat di.pc.in | \
	  sed -e 's,@CMAKE_INSTALL_PREFIX@,$(PREFIX),g' \
	      -e 's,@CMAKE_INSTALL_FULL_INCLUDEDIR@,$(INCDIR),g' \
	      -e 's,@CMAKE_INSTALL_FULL_LIBDIR@,$(LIBDIR),g' \
	      -e 's,@DI_VERSION@,$(DI_VERSION),g' \
	  > $(INST_PKGCDIR)/di.pc

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

$(MKC_ENV):	$(MKC_ENV_CONF)
	@-rm -f $(MKC_ENV)
	CC=$(CC) DI_FORTIFY=$(DI_FORTIFY) \
	    $(_MKCONFIG_SHELL) $(MKC_DIR)/mkconfig.sh $(MKC_ENV_CONF)

###
# programs

.PHONY: mkc-di-programs
mkc-di-programs:	di$(EXE_EXT) dimathtest$(EXE_EXT) getoptn_test$(EXE_EXT)

###
# configuration file

config.h:	$(MKC_ENV) $(MKC_CONF)
	@-rm -f config.h
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
		-L . -ldi \
		-R $(LIBDIR)

dimathtest$(EXE_EXT):	dimathtest$(OBJ_EXT)
	@$(_MKCONFIG_SHELL) $(MKC_DIR)/mkc.sh \
		-link -exec $(MKC_ECHO) \
		-o dimathtest$(EXE_EXT) \
		dimathtest$(OBJ_EXT) \
		-l m

getoptn_test$(EXE_EXT):	getoptn_test$(OBJ_EXT) distrutils$(OBJ_EXT)
	@$(_MKCONFIG_SHELL) $(MKC_DIR)/mkc.sh \
		-link -exec $(MKC_ECHO) \
		-o getoptn_test$(EXE_EXT) \
		getoptn_test$(OBJ_EXT) \
		distrutils$(OBJ_EXT) \
		-l m

###
# objects

.c$(OBJ_EXT):
	@$(_MKCONFIG_SHELL) $(MKC_DIR)/mkc.sh -compile -shared $(MKC_ECHO) $<

di$(OBJ_EXT):		di.c

didiskutil$(OBJ_EXT):	didiskutil.c

digetentries$(OBJ_EXT):	digetentries.c

digetinfo$(OBJ_EXT):	digetinfo.c

dilib$(OBJ_EXT):	dilib.c

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

di.o: config.h
di.o: di.h
di.o: disystem.h distrutils.h
didiskutil.o: config.h
didiskutil.o: di.h disystem.h
didiskutil.o: diinternal.h
didiskutil.o: dimath.h dioptions.h getoptn.h
didiskutil.o: distrutils.h dimntopt.h
digetentries.o: config.h
digetentries.o: di.h disystem.h
digetentries.o: diinternal.h dimath.h
digetentries.o: dioptions.h getoptn.h distrutils.h
digetentries.o: dimntopt.h
digetinfo.o: config.h
digetinfo.o: di.h disystem.h
digetinfo.o: diinternal.h
digetinfo.o: dimath.h dioptions.h getoptn.h
digetinfo.o: dimntopt.h
digetinfo.o: distrutils.h
dilib.o: config.h
dilib.o: di.h disystem.h
dilib.o: dimath.h diinternal.h
dilib.o: dioptions.h getoptn.h dizone.h diquota.h distrutils.h
dimathtest.o: config.h
dimathtest.o: dimath.h
dioptions.o: config.h
dioptions.o: di.h disystem.h
dioptions.o: diinternal.h
dioptions.o: dimath.h dioptions.h getoptn.h
dioptions.o: distrutils.h
diquota.o: config.h
diquota.o: di.h
diquota.o: disystem.h dimath.h
diquota.o: diquota.h diinternal.h dioptions.h
diquota.o: getoptn.h distrutils.h
distrutils.o: config.h
distrutils.o: distrutils.h
dizone.o: config.h
dizone.o: di.h dizone.h disystem.h
dizone.o: dioptions.h getoptn.h distrutils.h
getoptn.o: config.h
getoptn.o: disystem.h
getoptn.o: distrutils.h getoptn.h
