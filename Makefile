#
#  di makefile - C
#
#  Copyright 2001-2018 Brad Lanam Walnut Creek CA, USA
#  Copyright 2023-2025 Brad Lanam, Pleasant Hill, CA
#

DI_VERSION = 4.99.5
DI_LIBVERSION = 4.99.5
DI_SOVERSION = 4
DI_RELEASE_STATUS = beta

# for cmake
CMAKE_REQ_MAJ_VERSION = 3
CMAKE_REQ_MIN_VERSION = 13
BUILDDIR = build

# DI_USE_MATH = DI_GMP
# DI_USE_MATH = DI_TOMMATH
# DI_USE_MATH = DI_INTERNAL

# for mkconfig
MKC_CONFDIR = mkc_config
MKC_FILES = mkc_files
MKC_ENV = di.env
MKC_CONF = $(MKC_CONFDIR)/di.mkc
MKC_ENV_CONF = $(MKC_CONFDIR)/di-env.mkc

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
# installation locations
#  the cmake install only uses PREFIX
#
PREFIX = /usr/local
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

# checks the cmake version, and builds using either cmake or mkconfig
.PHONY: switcher
switcher:
	@if [ "$(PREFIX)" = "" ]; then echo "No prefix set"; exit 1; fi
	@-./utils/chkcmake.sh \
	    $(CMAKE_REQ_MAJ_VERSION) $(CMAKE_REQ_MIN_VERSION) ; \
	rc=$$? ; \
	if [ $$rc -eq 0 ]; then \
	  $(MAKE) cmake-$(TARGET) ; \
	else \
	  $(MAKE) -e mkc-$(TARGET) ; \
	fi

###
# cleaning

# clean temporary files
.PHONY: clean
tclean:
	@-$(RM) -f w ww asan.* *.orig \
		dep-*.txt >/dev/null 2>&1; exit 0
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
	@-test -d $(BUILDDIR) && cmake --build $(BUILDDIR) --target clean

# mkc tests use this
.PHONY: realclean
realclean:
	@$(MAKE) clean >/dev/null 2>&1
	@-$(RM) -rf config.h \
		$(MKC_ENV) $(MKC_REQLIB) \
                _mkconfig_runtests \
		>/dev/null 2>&1; exit 0

.PHONY: distclean
distclean:
	@$(MAKE) realclean >/dev/null 2>&1
	@-$(RM) -rf tests.done test_di _mkconfig_runtests \
		test_results \
		$(MKC_FILES) \
		$(BUILDDIR) \
		>/dev/null 2>&1; exit 0

###
# utility

.PHONY: tar
tar:
	./utils/mktar.sh

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
#   (passing -j w/o arguments, and *BSD complains)
.PHONY: cmake-all
cmake-all:
	@case $$(uname -s) in \
	  CYGWIN*|MSYS*|MINGW*) \
	    COMP=$(CC) \
	    $(MAKE) cmake-windows; \
	    $(MAKE) cmake-build; \
            ;; \
	  *BSD*) \
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
	  *BSD*) \
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
		-DDI_VERSION:STATIC=$(DI_VERSION) \
		-DDI_LIBVERSION:STATIC=$(DI_LIBVERSION) \
		-DDI_SOVERSION:STATIC=$(DI_SOVERSION) \
		-DDI_RELEASE_STATUS:STATIC=$(DI_RELEASE_STATUS) \
		-DPREFIX:STATIC=$(PREFIX) \
		-DDI_USE_MATH:STATIC=$(DI_USE_MATH) \
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
		-DDI_VERSION:STATIC=$(DI_VERSION) \
		-DDI_LIBVERSION:STATIC=$(DI_LIBVERSION) \
		-DDI_SOVERSION:STATIC=$(DI_SOVERSION) \
		-DDI_RELEASE_STATUS:STATIC=$(DI_RELEASE_STATUS) \
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
	. ./$(MKC_ENV);$(MAKE) -e MKCONFIG_TYPE=sh \
		DI_PREFIX=$(PREFIX) \
		DI_VERSION=$(DI_VERSION) \
		DI_LIBVERSION=$(DI_LIBVERSION) \
		DI_SOVERSION=$(DI_SOVERSION) \
		DI_RELEASE_STATUS=$(DI_RELEASE_STATUS) \
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

.PHONY: test
mkc-test:		tests.done
	. ./$(MKC_ENV); \
	    ./dimathtest$(EXE_EXT); \
	    ./getoptn_test$(EXE_EXT)

.PHONY: mkc-install
mkc-install:
	$(MAKE) -e mkc-all
	. ./$(MKC_ENV);$(MAKE) -e \
		PREFIX=$(PREFIX) \
		mkc-install-di mkc-install-man

###
# installation

.PHONY: mkc-install-po
mkc-install-po:
	./utils/instpo.sh $(INST_LOCALEDIR)

.PHONY: mkc-install-pc
mkc-install-pc:
	$(TEST) -d $(INST_PKGCDIR) || $(MKDIR) -p $(INST_PKGCDIR)
	$(CAT) di.pc.in | \
	  $(SED) -e 's,@CMAKE_INSTALL_PREFIX@,$(PREFIX),g' \
	      -e 's,@CMAKE_INSTALL_FULL_INCLUDEDIR@,$(INCDIR),g' \
	      -e 's,@CMAKE_INSTALL_FULL_LIBDIR@,$(LIBDIR),g' \
	      -e 's,@DI_VERSION@,$(DI_VERSION),g' \
	  > $(INST_PKGCDIR)/di.pc

.PHONY: mkc-install-di
mkc-install-di:
	$(TEST) -d $(INST_BINDIR) || $(MKDIR) -p $(INST_BINDIR)
	$(TEST) -d $(INST_LIBDIR) || $(MKDIR) -p $(INST_LIBDIR)
	$(TEST) -d $(INST_INCDIR) || $(MKDIR) -p $(INST_INCDIR)
	$(CP) -f di$(EXE_EXT) $(INST_BINDIR)
	$(CP) -f di.h $(INST_INCDIR)
	-$(MAKE) mkc-install-po
	$(MAKE) mkc-install-man
	$(MAKE) mkc-install-pc
	@sym=T ; \
	case `uname -s` in \
	  Darwin) \
            libnm=libdi.$(DI_LIBVERSION)$(SHLIB_EXT) ; \
            ;; \
          CYGWIN*|MSYS*|MINGW*) \
	    libnm=libdi$(SHLIB_EXT) ; \
	    sym=F ; \
	    ;; \
	  *) \
            libnm=libdi$(SHLIB_EXT).$(DI_LIBVERSION) ; \
            ;; \
	esac ; \
	$(CP) -f libdi$(SHLIB_EXT) $(INST_LIBDIR)/$${libnm} ; \
	if [ $$sym = T ]; then \
	  (cd $(INST_LIBDIR); $(LN) -sf $${libnm} libdi$(SHLIB_EXT)) ; \
	fi

.PHONY: mkc-install-man
mkc-install-man:
	-$(TEST) -d $(INST_MANDIR)/man1 || $(MKDIR) -p $(INST_MANDIR)/man1
	$(CP) -f man/di.1 $(INST_MANDIR)/man1/$(MAN_TARGET)
	-$(TEST) -d $(INST_MANDIR)/man3 || $(MKDIR) -p $(INST_MANDIR)/man3
	$(CP) -f man/libdi.3 $(INST_MANDIR)/man3/$(MAN_TARGET)

.PHONY: mkc-install-ditest
mkc-install-ditest:
	$(TEST) -d $(INST_BINDIR) || $(MKDIR) -p $(INST_BINDIR)
	$(CP) -f dimathtest$(EXE_EXT) $(INST_BINDIR)
	$(CP) -f getoptn_test$(EXE_EXT) $(INST_BINDIR)

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
		CFLAGS="$(CFLAGS) -g -O2" \
		LDFLAGS="-g -O2 -Zexe" di.exe

###
# programs

.PHONY: mkc-di-programs
mkc-di-programs:	di$(EXE_EXT) dimathtest$(EXE_EXT) getoptn_test$(EXE_EXT)

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
		-L . -ldi \
		-R $(LIBDIR)

dimathtest$(EXE_EXT):	dimathtest$(OBJ_EXT)
	@$(_MKCONFIG_SHELL) $(MKC_DIR)/mkc.sh \
		-link -exec $(MKC_ECHO) \
		-r $(MKC_REQLIB) \
		-o dimathtest$(EXE_EXT) \
		dimathtest$(OBJ_EXT) \
                -l m

getoptn_test$(EXE_EXT):	getoptn_test$(OBJ_EXT) distrutils$(OBJ_EXT)
	$(_MKCONFIG_SHELL) $(MKC_DIR)/mkc.sh \
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

###
# regression testing

.PHONY: mkc-test-all
mkc-test-all:	tests.done

tests.done: $(MKC_DIR)/runtests.sh
	@echo "## running tests"
	CC=$(CC) DC=$(DC) $(_MKCONFIG_SHELL) \
		$(MKC_DIR)/runtests.sh ./tests.d
	touch tests.done

# needs environment
.PHONY: mkc-r-test-env
mkc-r-test-env:
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

.PHONY: mkc-test-env
mkc-test-env:
	@echo "cc: $(CC)"
	@echo "make: $(MAKE)"

# DO NOT DELETE

di$(OBJ_EXT): config.h
di$(OBJ_EXT):   di.h disystem.h
di$(OBJ_EXT):   distrutils.h
didiskutil$(OBJ_EXT): config.h
didiskutil$(OBJ_EXT):   di.h disystem.h
didiskutil$(OBJ_EXT):   diinternal.h
didiskutil$(OBJ_EXT): dimath.h  dioptions.h distrutils.h
didiskutil$(OBJ_EXT): dimntopt.h
digetentries$(OBJ_EXT): config.h
digetentries$(OBJ_EXT):  di.h disystem.h
digetentries$(OBJ_EXT):  diinternal.h dimath.h
digetentries$(OBJ_EXT):  dioptions.h distrutils.h dimntopt.h
digetinfo$(OBJ_EXT): config.h
digetinfo$(OBJ_EXT):   di.h disystem.h
digetinfo$(OBJ_EXT):   diinternal.h
digetinfo$(OBJ_EXT): dimath.h  dioptions.h dimntopt.h
digetinfo$(OBJ_EXT):   distrutils.h
dilib$(OBJ_EXT): config.h
dilib$(OBJ_EXT): di.h disystem.h   dimath.h
dilib$(OBJ_EXT):  diinternal.h dioptions.h dizone.h diquota.h
dilib$(OBJ_EXT): distrutils.h
dimathtest$(OBJ_EXT): config.h
dimathtest$(OBJ_EXT):   dimath.h
dioptions$(OBJ_EXT): config.h
dioptions$(OBJ_EXT):   di.h diinternal.h
dioptions$(OBJ_EXT): disystem.h   dimath.h
dioptions$(OBJ_EXT):  dioptions.h distrutils.h getoptn.h
diquota$(OBJ_EXT): config.h
diquota$(OBJ_EXT):   di.h
diquota$(OBJ_EXT): dimath.h   diquota.h
diquota$(OBJ_EXT): disystem.h  diinternal.h dioptions.h
diquota$(OBJ_EXT): distrutils.h
distrutils$(OBJ_EXT): config.h
distrutils$(OBJ_EXT):  distrutils.h
dizone$(OBJ_EXT): config.h di.h dizone.h disystem.h
dizone$(OBJ_EXT):  distrutils.h
getoptn$(OBJ_EXT): config.h
getoptn$(OBJ_EXT):  distrutils.h getoptn.h
