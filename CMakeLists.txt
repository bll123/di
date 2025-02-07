#
# Copyright 2025 Brad Lanam Pleasant Hill CA
#
cmake_minimum_required (VERSION 3.10)

# avoid msys2/windows issue
set (CMAKE_C_COMPILER_WORKS 1)

project (DI
  VERSION ${DI_VERSION}
  DESCRIPTION "di - disk information utility"
  HOMEPAGE_URL "https://diskinfo-di.sourceforge.io"
  LANGUAGES C
)
option (BUILD_SHARED_LIBS "Build dynamic library" ON)

set (default_build_type "Release")

include (GNUInstallDirs)

set (DI_LIBNAME libdi)

SET (CMAKE_SKIP_BUILD_RPATH FALSE)
SET (CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)

if (DEFINED ENV{CMAKE_SKIP_RPATH} AND NOT ENV{CMAKE_SKIP_RPATH} STREQUAL "")
  message ("-- Skip RPATH: $ENV{CMAKE_SKIP_RPATH}")
  set (CMAKE_SKIP_RPATH "$ENV{CMAKE_SKIP_RPATH}")
endif()

#### compile options

add_compile_options (-fPIC)
add_link_options (-fPIC)

add_compile_options (-Wall)
add_compile_options (-Wextra)
add_compile_options (-Wconversion)
add_compile_options (-Wno-unused-but-set-variable)
add_compile_options (-Wno-unused-parameter)
add_compile_options (-Wno-unknown-pragmas)
add_compile_options (-Wno-float-equal)
add_compile_options (-Wformat)
add_compile_options (-Wformat-security)
add_compile_options (-Werror=format-security)
add_compile_options (-Werror=return-type)
add_compile_options (-Wdeprecated-declarations)
add_compile_options (-Wunreachable-code)

if (NOT CMAKE_C_COMPILER MATCHES "c\\+\\+")
  add_compile_options (-Wdeclaration-after-statement)
  add_compile_options (-Wmissing-prototypes)
endif()

#### compiler-specific compile options

if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
  add_compile_options (-Wmaybe-uninitialized)
  add_compile_options (-Wno-unused-but-set-variable)
  add_compile_options (-Wno-stringop-overflow)
  add_compile_options (-Wno-stringop-truncation)
  add_compile_options (-Wno-format-truncation)
endif()
if (CMAKE_C_COMPILER_ID STREQUAL "Clang")
  add_compile_options (-Wno-extra-semi-stmt)
  add_compile_options (-Wno-poison-system-directories)
  add_compile_options (-Wno-shift-sign-overflow)
  add_compile_options (-Wno-pragma-pack)
  add_compile_options (-Wno-ignored-attributes)
  if (APPLE)
    add_compile_options (-Wno-reserved-macro-identifier)
  endif()
  add_compile_options (-Wno-reserved-id-macro)
  add_compile_options (-Wno-implicit-int-conversion)
  add_compile_options (-Wno-switch-enum)
  add_compile_options (-Wno-gnu-zero-variadic-macro-arguments)
  add_compile_options (-Wno-documentation-deprecated-sync)
  add_compile_options (-Wno-documentation-unknown-command)
  add_compile_options (-Wno-documentation)
  if (CMAKE_CLANG_COMPILER_VERSION VERSION_GREATER_EQUAL "16.0.0")
    add_compile_options (-Wno-unsafe-buffer-usage)
  endif()
endif()

#### build compile options

if (DI_BUILD STREQUAL "")
  set (DI_BUILD "Release")
endif()

if (DI_BUILD STREQUAL "Release")
  add_compile_options (-O2)
endif()

if (DI_BUILD STREQUAL "Debug")
  add_compile_options (-O0)
endif()

add_compile_options (-g)
add_link_options (-g)

#### more compile options: fortification/address sanitizer

set (DI_FORTIFY Y)
if (DEFINED ENV{DI_FORTIFY} AND NOT ENV{DI_FORTIFY} STREQUAL "")
  set (DI_FORTIFY $ENV{DI_FORTIFY})
endif()

# address sanitizer
if (DI_BUILD STREQUAL "SanitizeAddress")
  set (DI_FORTIFY N)
  add_compile_options (-O0)
  add_compile_options (-ggdb)
  add_link_options (-g)
  add_compile_options (-fsanitize=address)
  add_link_options (-fsanitize=address)
  add_compile_options (-fsanitize-address-use-after-scope)
  add_link_options (-fsanitize-address-use-after-scope)
  add_compile_options (-fsanitize-recover=address)
  add_link_options (-fsanitize-recover=address)
  add_compile_options (-fno-omit-frame-pointer)
  add_compile_options (-fno-common)
  add_compile_options (-fno-inline)
  if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
    add_link_options (-lrt)
  endif()
endif()

# solaris may not be using the gnu/clang loader
if (DI_FORTIFY STREQUAL Y AND NOT CMAKE_HOST_SOLARIS)
message ("== fortify is on")
  # hardening
  add_compile_options (-fstack-protector-strong)
  add_compile_options (-fstack-protector-all)
  add_compile_options (-D_FORTIFY_SOURCE=2)
else()
message ("== fortify is OFF")
  if (CMAKE_C_COMPILER_ID STREQUAL "Clang")
    add_compile_options (-Wno-macro-redefined)
  endif()
  add_compile_options (-U_FORTIFY_SOURCE)
  add_compile_options (-D_FORTIFY_SOURCE=0)
endif()

#### system specific compile options

if (NOT WIN32)
  SET (CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_FULL_LIBDIR})
  if (NOT APPLE)
#    SET (CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_FULL_LIBDIR} "\${ORIGIN}")
  endif()
  if (APPLE)
#    SET (CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_FULL_LIBDIR} "@loader_path")
    # 10.6 = Snow Leopard, 10.7 = Lion
    # 10.8 = Mountain Lion, 10.9 = Mavericks
    # 10.10 = Yosemite, 10.11 = El Capitan
    # 10.12 = Sierra, 10.13 = High Sierra
    # 10.14 = Mojave, 10.15 = Catalina
    # 11 = Big Sur, 12 = Monterey,
    # 13 = Ventura, 14 = Sonoma
    # 15 = Sequoia
    # set (CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
  endif()
else()
  add_link_options (-static-libgcc)
  add_link_options (-static-libstdc++)
endif()

#### include paths

include_directories (
  SYSTEM
  # for macos
  /opt/local/include
  # for freebsd
  /usr/local/include
  "${CMAKE_BINARY_DIR}"
  "${PROJECT_SOURCE_DIR}/include"
)

set (required_includes
  # for macos
  /opt/local/include
  # for freebsd
  /usr/local/include
)

set (CMAKE_REQUIRED_INCLUDES ${required_includes})
add_compile_options (-DWIN32_LEAN_AND_MEAN=1)

#### configuration

find_package (PkgConfig)
find_package (Intl)
find_package (Iconv)

pkg_check_modules (GMP gmp)
pkg_check_modules (TOMMATH libtommath)
pkg_check_modules (TIRPC libtirpc)

include (CheckIncludeFile)
include (CheckIncludeFiles)
include (CheckLibraryExists)
include (CheckLinkerFlag)
include (CheckStructHasMember)
include (CheckSymbolExists)
include (CheckTypeSize)
include (CheckVariableExists)
include (CheckPrototypeDefinition)

###
# macros for capability testing

macro (checkPrototype_quotactl hdr)
  # this fails when using the c++ compiler.
  # the c++ compiler is only needed on Haiku,
  # but it would be nice to be able to test it elsewhere
  # the mkconfig suite can be used to test c++.
  check_prototype_definition (quotactl
    "int quotactl (const char * a, int b, int c, void * d)"
    0
    "${hdr}"
    _quotactl_pos_1a
    )
  # OpenBSD has char *, but caddr_t works...
  #  "int quotactl (const char *path, int cmd, int id, char *addr)"
  check_prototype_definition (quotactl
    "int quotactl (const char * a, int b, int c, caddr_t d)"
    0
    "${hdr}"
    _quotactl_pos_1b
    )
  check_prototype_definition (quotactl
    "int quotactl (int a, const char * b, int c, caddr_t d)"
    0
    "${hdr}"
    _quotactl_pos_2a
    )
  # alpine linux
  check_prototype_definition (quotactl
    "int quotactl (int a, const char * b, int c, char *d)"
    0
    "${hdr}"
    _quotactl_pos_2b
    )
  if (_quotactl_pos_1a OR _quotactl_pos_1b)
    set (_quotactl_pos_1 1)
  endif()
  if (_quotactl_pos_2a OR _quotactl_pos_2b)
    set (_quotactl_pos_2 1)
  endif()
  if (_quotactl_pos_1)
    set (_c_arg_2_quotactl "int")
    set (_args_quotactl 4)
  endif()
  if (_quotactl_pos_2)
    set (_c_arg_2_quotactl "const char *")
    set (_args_quotactl 4)
  endif()
endmacro()

macro (checkPrototype_statfs hdr)
  check_prototype_definition (statfs
    "int statfs (const char * a, struct statfs * b)"
    0
    "${hdr}"
    _args_statfs_2
    )
  check_prototype_definition (statfs
    "int statfs (const char * a, struct statfs * b, int c)"
    0
    "${hdr}"
    _args_statfs_3
    )
  check_prototype_definition (statfs
    "int statfs (const char * a, struct statfs * b, int c, int d)"
    0
    "${hdr}"
    _args_statfs_4
    )
  if (_args_statfs_2)
    set (_args_statfs 2)
  endif()
  if (_args_statfs_3)
    set (_args_statfs 3)
  endif()
  if (_args_statfs_4)
    set (_args_statfs 4)
  endif()
endmacro()

macro (checkMember_statfs hdr)
  check_struct_has_member ("struct statfs" f_bsize
      "${hdr}" _mem_struct_statfs_f_bsize)
  check_struct_has_member ("struct statfs" f_fsize
      "${hdr}" _mem_struct_statfs_f_fsize)
  check_struct_has_member ("struct statfs" f_fstyp
      "${hdr}" _mem_struct_statfs_f_fstyp)
  check_struct_has_member ("struct statfs" f_iosize
      "${hdr}" _mem_struct_statfs_f_iosize)
  check_struct_has_member ("struct statfs" f_frsize
      "${hdr}" _mem_struct_statfs_f_frsize)
  check_struct_has_member ("struct statfs" f_fstypename
      "${hdr}" _mem_struct_statfs_f_fstypename)
  check_struct_has_member ("struct statfs" mount_info
      "${hdr}" _mem_struct_statfs_mount_info)
  check_struct_has_member ("struct statfs" f_type
      "${hdr}" _mem_struct_statfs_f_type)
endmacro()

macro (checkPrototype_setmntent hdr)
  check_prototype_definition (setmntent
    "FILE * setmntent (const char * a)"
    "NULL"
    "stdio.h;stddef.h;${hdr}"
    _args_setmntent_1
    )
  check_prototype_definition (setmntent
    "FILE * setmntent (const char * a, const char * b)"
    "NULL"
    "stdio.h;stddef.h;${hdr}"
    _args_setmntent_2
    )
  if (_args_setmntent_1)
    set (_args_setmntent 1)
  endif()
  if (_args_setmntent_2)
    set (_args_setmntent 2)
  endif()
endmacro()

macro (checkPrototype_getfsstat hdr)
  check_prototype_definition (getfsstat
    "int getfsstat (struct statfs * a, int b, int c)"
    0
    "stdio.h;stddef.h;${hdr}"
    _getfsstat_type_int
    )
  check_prototype_definition (getfsstat
    "int getfsstat (struct statfs * a, long b, int c)"
    0
    "stdio.h;stddef.h;${hdr}"
    _getfsstat_type_long
    )
  check_prototype_definition (getfsstat
    "int getfsstat(struct statfs *buf, size_t bufsize, int flags)"
    0
    "stdio.h;stddef.h;${hdr}"
    _getfsstat_type_size_t
    )
  if (_getfsstat_type_int)
    set (_c_arg_2_getfsstat int)
  endif()
  if (_getfsstat_type_long)
    set (_c_arg_2_getfsstat long)
  endif()
  if (_getfsstat_type_size_t)
    set (_c_arg_2_getfsstat size_t)
  endif()
endmacro()

macro (checkPrototype_getvfsstat hdr)
  check_prototype_definition (getvfsstat
    "int getvfsstat (struct statvfs *buf, size_t bufsize, int flags)"
    0
    "stdio.h;stddef.h;${hdr}"
    _args_getvfsstat_3a
    )
  check_prototype_definition (getvfsstat
    "int getvfsstat (struct statvfs *buf, int bufsize, int flags)"
    0
    "stdio.h;stddef.h;${hdr}"
    _args_getvfsstat_3b
    )
  if (_args_getvfsstat_3a OR _args_getvfsstat_3b)
    set (_args_getvfsstat 3)
  endif()
endmacro()

macro (checkdqblk hdr)
  # the calls to checkdqblk do not include the angle brackets
  set (CMAKE_EXTRA_INCLUDE_FILES "${hdr}")
  check_type_size ("struct dqblk" _typ_struct_dqblk)
  check_type_size ("struct quotaval" _typ_struct_quotaval)
  check_type_size ("struct ufs_dqblk" _typ_struct_ufs_dqblk)
  check_struct_has_member ("struct dqblk" dqb_curspace
      "${hdr}" _mem_struct_dqblk_dqb_curspace)
  check_struct_has_member ("struct dqblk" dqb_curblocks
      "${hdr}" _mem_struct_dqblk_dqb_curblocks)
  check_struct_has_member ("struct dqblk" dqb_fhardlimit
      "${hdr}" _mem_struct_dqblk_dqb_fhardlimit)
  check_struct_has_member ("struct dqblk" dqb_fsoftlimit
      "${hdr}" _mem_struct_dqblk_dqb_fsoftlimit)
  check_struct_has_member ("struct dqblk" dqb_curfiles
      "${hdr}" _mem_struct_dqblk_dqb_curfiles)
  unset (CMAKE_EXTRA_INCLUDE_FILES)
endmacro ()

macro (memberxdr structnm membernm)
  message ("-- Determining ${structnm} ${membernm} xdr type")
  file (WRITE ${membernm}.c "#include <rpcsvc/rquota.h>
int main (int argc, char *argv []) { return 0; }
")
  set (tinc ".")
  if (NOT ${TIRPC_INCLUDE_DIRS} STREQUAL "")
    set (tinc ${TIRPC_INCLUDE_DIRS})
  endif()
  execute_process (
    OUTPUT_FILE ${membernm}.out
    ERROR_QUIET
    COMMAND ${CMAKE_C_COMPILER} -I ${tinc} -E ${membernm}.c
  )
  unset (tinc)
  if (EXISTS ${membernm}.out)
    execute_process (
      OUTPUT_VARIABLE xdr_out
      # unfortunately, the sed command will find other matches
      # try this and see how it works out.
      # possibly will switch to awk in the future.
      # Some headers have tabs, use the [^a-z0-9_] to avoid putting
      # them in here.
      COMMAND sed -n -e "/struct *${structnm}/,/^ *};/ p" ${membernm}.out
      COMMAND grep "^[^a-z0-9_]*[a-z0-9_][a-z0-9_]*[^a-z0-9_]*${membernm};"
      COMMAND sed -e "s,^[^a-z0-9_]*,," -e "s,[^a-z0-9_].*,,"
      COMMAND tr -d "\n"
    )
    set (xdr_${membernm} xdr_${xdr_out})
  endif()
  file (REMOVE ${membernm}.c)
  if (EXISTS ${membernm}.out)
    file (REMOVE ${membernm}.out)
  endif()
  set (CMAKE_REQUIRED_INCLUDES ${required_includes})
endmacro()

macro (checkIncludeConflict hdrvara hdra hdrvarb hdrb varnm)
  message ("-- Check for include conflict ${hdra} ${hdrb}")
  if (NOT ${hdrvara} OR NOT ${hdrvarb})
    set (${varnm} 1)
  else()
    check_include_files ("${hdra};${hdrb}" ${varnm})
  endif()
endmacro()

macro (checkvquotactlenabled)
  message ("-- Check if vquotactl is enabled")
  set (tvquotactl 0)
  if (_lib_vquotactl)
    execute_process (
      OUTPUT_QUIET
      ERROR_QUIET
      RESULT_VARIABLE tvquotactl
      COMMAND grep vfs.quota_enabled=.*1 /boot/loader.conf
    )
  endif()
  if (NOT tvquotactl)
    set (_lib_vquotactl 0)
    set (_lib_prop_dictionary_create 0)
  endif()
endmacro()

check_include_file (ctype.h _hdr_ctype)
check_include_file (dirent.h _hdr_dirent)
check_include_file (errno.h _hdr_errno)
check_include_file (fcntl.h _hdr_fcntl)
set (CMAKE_REQUIRED_INCLUDES ${required_includes} ${GMP_INCLUDE_DIRS})
check_include_file (gmp.h _hdr_gmp)
set (CMAKE_REQUIRED_INCLUDES ${required_includes})
check_include_file (fshelp.h _hdr_fshelp)
check_include_file (inttypes.h _hdr_inttypes)
check_include_file (jfs/quota.h _hdr_jfs_quota)
check_include_file (kernel/fs_info.h _hdr_kernel_fs_info)
check_include_file (limits.h _hdr_limits)
check_include_file (linux/dqblk_xfs.h _hdr_linux_dqblk_xfs)
check_include_file (linux/quota.h _hdr_linux_quota)
check_include_file (libintl.h _hdr_libintl)
check_include_file (libprop/proplib.h _hdr_libprop_proplib)
check_include_file (locale.h _hdr_locale)
check_include_file (malloc.h _hdr_malloc)
check_include_file (math.h _hdr_math)
check_include_file (memory.h _hdr_memory)
check_include_file (mntent.h _hdr_mntent)
check_include_file (mnttab.h _hdr_mnttab)
# NetBSD
check_include_file (quota.h _hdr_quota)

set (CMAKE_REQUIRED_INCLUDES ${required_includes} ${TIRPC_INCLUDE_DIRS})
# OpenBSD rpc/auth requires rpc/rpc
check_include_files ("rpc/rpc.h;rpc/auth.h" _hdr_rpc_auth)
check_include_file (rpc/rpc.h _hdr_rpc_rpc)
# Linux: rquota.h includes rpc/rpc.h
check_include_file (rpcsvc/rquota.h _hdr_rpcsvc_rquota)
set (CMAKE_REQUIRED_INCLUDES ${required_includes})

check_include_file (storage/Directory.h _hdr_storage_Directory)
check_include_file (storage/Entry.h _hdr_storage_Entry)
check_include_file (storage/Path.h _hdr_storage_Path)
check_include_file (stdbool.h _hdr_stdbool)
check_include_file (stddef.h _hdr_stddef)
check_include_file (stdio.h _hdr_stdio)
check_include_file (stdint.h _hdr_stdint)
check_include_file (stdlib.h _hdr_stdlib)
check_include_file (string.h _hdr_string)
check_include_file (strings.h _hdr_strings)
check_include_file (time.h _hdr_time)
set (CMAKE_REQUIRED_INCLUDES ${required_includes} ${TOMMATH_INCLUDE_DIRS})
check_include_file (tommath.h _hdr_tommath)
check_include_file (libtommath/tommath.h _hdr_libtommath_tommath)
set (CMAKE_REQUIRED_INCLUDES ${required_includes})
check_include_file (ufs/quota.h _hdr_ufs_quota)
# FreeBSD/OpenBSD ufs/ufs/quota.h does not include its dependencies
check_include_files ("sys/types.h;ufs/ufs/quota.h" _hdr_ufs_ufs_quota)
check_include_file (unistd.h _hdr_unistd)
check_include_file (wchar.h _hdr_wchar)
check_include_file (windows.h _hdr_windows)
check_include_file (winioctl.h _hdr_winioctl)
check_include_file (zone.h _hdr_zone)

check_include_file (sys/dcmd_blk.h _sys_dcmd_blk)
check_include_file (sys/file.h _sys_file)
check_include_file (sys/fs_types.h _sys_fs_types)
check_include_files ("sys/types.h;sys/fs/ufs_quota.h" _sys_fs_ufs_quota)
check_include_file (sys/fstyp.h _sys_fstyp)  # solaris
# NetBSD requires sys/types.h
check_include_files ("sys/types.h;sys/fstypes.h" _sys_fstypes)
check_include_file (sys/ftype.h _sys_ftype)
check_include_file (sys/io.h _sys_io)
check_include_file (sys/mntctl.h _sys_mntctl)
check_include_file (sys/param.h _sys_param)
check_include_file (sys/types.h _sys_types)

# SCO OpenServer/UnixWare require sys/mnttab.h for struct mnttab declaration.
# solaris
check_include_file (sys/mnttab.h _sys_mnttab)
if (_sys_mnttab)
  check_include_files ("sys/mnttab.h;sys/mntent.h" _sys_mntent)
else()
  check_include_file ("sys/mntent.h" _sys_mntent)
endif()

# openbsd, dragonflybsd, freebsd
check_include_files ("sys/types.h;sys/mount.h" _sys_mount)
check_include_files ("sys/types.h;sys/quota.h" _sys_quota)
check_include_file (sys/stat.h _sys_stat)
check_include_file (sys/statfs.h _sys_statfs)
check_include_file (sys/statvfs.h _sys_statvfs)
check_include_file (sys/time.h _sys_time)
check_include_file (sys/vfs.h _sys_vfs)
check_include_file (sys/vfs_quota.h _sys_vfs_quota)  # dfly-bsd
# SCO OpenServer/UnixWare require stdio.h for sys/vfstab.h
check_include_files ("stdio.h;sys/vfstab.h" _sys_vfstab)
check_include_file (sys/vmount.h _sys_vmount)   # AIX

check_symbol_exists (bcopy strings.h _lib_bcopy)
check_symbol_exists (bzero strings.h _lib_bzero)
# -lsun, -lseq
check_symbol_exists (endmntent mntent.h  _lib_endmntent)
check_symbol_exists (fs_stat_dev kernel/fs_info.h _lib_fs_stat_dev)  # haiku
check_symbol_exists (fshelp fshelp.h _lib_fshelp)  # AIX
# macos, freebsd, openbsd
check_symbol_exists (getfsstat "sys/stat.h;sys/mount.h" _lib_getfsstat)
# ultrix, .h file may be wrong
check_symbol_exists (getmnt mntent.h _lib_getmnt)

set (_lib_getmntent 0)
check_symbol_exists (getmntent mntent.h  _lib_getmntent_a)
check_symbol_exists (getmntent sys/mnttab.h _lib_getmntent_b)
set (LIBGEN_REQUIRED 0)
if (NOT _lib_getmntent_a)
  # unixware put getmntent into libgen for some reason
  set (CMAKE_REQUIRED_LIBRARIES -lgen)
  check_symbol_exists (getmntent mntent.h  _lib_getmntent_c)
  if (_lib_getmntent)
    set (LIBGEN_REQUIRED 1)
  endif()
  unset (CMAKE_REQUIRED_LIBRARIES)
endif()
if (_lib_getmntent_a OR _lib_getmntent_b OR _lib_getmntent_c)
  set (_lib_getmntent 1)
endif()

set (_lib_getmntinfo 0)
check_symbol_exists (getmntinfo sys/statvfs.h _lib_getmntinfo_a)  # netbsd
# macos, openbsd (req sys/types.h)
check_symbol_exists (getmntinfo "sys/types.h;sys/mount.h" _lib_getmntinfo_b)
if (_lib_getmntinfo_a OR _lib_getmntinfo_b)
  set (_lib_getmntinfo 1)
endif()

check_symbol_exists (getvfsstat sys/statvfs.h  _lib_getvfsstat)
check_symbol_exists (getzoneid zone.h _lib_getzoneid)

set (_lib_hasmntopt 0)
check_symbol_exists (hasmntopt mntent.h  _lib_hasmntopt_a)
check_symbol_exists (hasmntopt sys/mnttab.h _lib_hasmntopt_b)  # solaris
if (_lib_hasmntopt_a OR _lib_hasmntopt_b)
  set (_lib_hasmntopt 1)
endif()

check_symbol_exists (lstat sys/stat.h _lib_lstat)
check_symbol_exists (mbrlen wchar.h _lib_mbrlen)
check_symbol_exists (memcpy string.h _lib_memcpy)
check_symbol_exists (memset string.h _lib_memset)

# AIX doesn't declare this :(
# Look for MCTL_QUERY (see below)
check_symbol_exists (mntctl sys/mntctl.h _lib_mntctl)

check_symbol_exists (next_dev kernel/fs_info.h _lib_next_dev)  # haiku

# dragonflybsd; need this to get the library
set (CMAKE_REQUIRED_LIBRARIES -lprop)
check_symbol_exists (prop_dictionary_create libprop/proplib.h _lib_prop_dictionary_create)
unset (CMAKE_REQUIRED_LIBRARIES)

# quota_open is a new interface from NetBSD
set (LIBQUOTACTL_REQUIRED 0)
set (CMAKE_REQUIRED_LIBRARIES -lquota)
check_symbol_exists (quota_open quota.h _lib_quota_open)
unset (CMAKE_REQUIRED_LIBRARIES)
if (_lib_quota_open)
  set (LIBQUOTACTL_REQUIRED 1)
endif()

set (_lib_quotactl 0)
check_symbol_exists (quotactl "unistd.h" _lib_quotactl_a)
check_symbol_exists (quotactl "sys/types.h;sys/quota.h" _lib_quotactl_b)
check_symbol_exists (quotactl "sys/types.h;ufs/ufs/quota.h" _lib_quotactl_c)
if (_lib_quotactl_a OR _lib_quotactl_b OR _lib_quotactl_c)
  set (_lib_quotactl 1)
endif()

check_symbol_exists (realpath stdlib.h _lib_realpath)

# unknown if -lsun, -lseq are needed (old irix, sequent)
check_symbol_exists (setmntent mntent.h _lib_setmntent)

check_symbol_exists (snprintf stdio.h _lib_snprintf_a)
set (LIBSNPRINTF_REQUIRED 0)
if (NOT _lib_snprintf_a)
  set (CMAKE_REQUIRED_LIBRARIES -lsnprintf)
  check_symbol_exists (snprintf stdio.h _lib_snprintf_b)
  if (_lib_snprintf_b)
    set (LIBSNPRINTF_REQUIRED 1)
  endif()
  unset (CMAKE_REQUIRED_LIBRARIES)
endif()
if (_lib_snprintf_a OR _lib_snprintf_b)
  set (_lib_snprintf 1)
endif()

check_symbol_exists (setlocale locale.h _lib_setlocale)

set (_lib_statfs 0)
check_symbol_exists (statfs "sys/statfs.h;sys/vfs.h" _lib_statfs_a)
# openbsd (requires sys/types)
check_symbol_exists (statfs "sys/types.h;sys/mount.h" _lib_statfs_b)
if (_lib_statfs_a OR _lib_statfs_b)
  set (_lib_statfs 1)
endif()

check_symbol_exists (statvfs sys/statvfs.h _lib_statvfs)
check_symbol_exists (stpecpy string.h _lib_stpecpy)
check_symbol_exists (strcoll string.h _lib_strcoll)
check_symbol_exists (strdup string.h _lib_strdup)
check_symbol_exists (strstr string.h _lib_strstr)
check_symbol_exists (sysfs sys/fstyp.h _lib_sysfs)  # solaris
# dragonflybsd
check_symbol_exists (vquotactl sys/vfs_quota.h _lib_vquotactl)

set (CMAKE_REQUIRED_INCLUDES ${required_includes} ${TIRPC_INCLUDE_DIRS})
check_symbol_exists (xdr_int rpc/rpc.h _lib_xdr_int)
set (LIBNSL_REQUIRED 0)
set (LIBTIRPC_REQUIRED 0)
if (NOT _lib_xdr_int)
  # check with nsl library (solaris 10)
  set (CMAKE_REQUIRED_LIBRARIES nsl)
  check_symbol_exists (xdr_int rpc/rpc.h _lib_xdr_int_nsl)
  if (_lib_xdr_int_nsl)
    set (_lib_xdr_int 1)
    set (LIBNSL_REQUIRED 1)
  endif()
  unset (CMAKE_REQUIRED_LIBRARIES)
endif()
if (NOT _lib_xdr_int)
  # check with tirpc library
  set (CMAKE_REQUIRED_LIBRARIES tirpc)
  check_symbol_exists (xdr_int rpc/rpc.h _lib_xdr_int_tirpc)
  if (_lib_xdr_int_tirpc)
    set (_lib_xdr_int 1)
    set (LIBTIRPC_REQUIRED 1)
  endif()
  unset (CMAKE_REQUIRED_LIBRARIES)
endif()
set (CMAKE_REQUIRED_INCLUDES ${required_includes})

# solaris
check_symbol_exists (zone_getattr zone.h _lib_zone_getattr)
check_symbol_exists (zone_list zone.h _lib_zone_list)

# windows routines
check_symbol_exists (GetDiskFreeSpace windows.h _lib_GetDiskFreeSpace)
check_symbol_exists (GetDiskFreeSpaceEx windows.h _lib_GetDiskFreeSpaceEx)
check_symbol_exists (GetDriveType windows.h _lib_GetDriveType)
check_symbol_exists (GetLogicalDriveStrings windows.h _lib_GetLogicalDriveStrings)
check_symbol_exists (GetVolumeInformation windows.h _lib_GetVolumeInformation)

if (Intl_LIBRARY AND NOT Intl_LIBRARY STREQUAL "NOTFOUND")
  set (CMAKE_REQUIRED_LIBRARIES ${Intl_LIBRARY})
endif()
if (Iconv_LIBRARY AND NOT Iconv_LIBRARY STREQUAL "NOTFOUND")
 set (CMAKE_REQUIRED_LIBRARIES "${CMAKE_REQUIRED_LIBRARIES};${Iconv_LIBRARY}")
endif()
check_symbol_exists (bindtextdomain libintl.h _lib_bindtextdomain)
check_symbol_exists (gettext libintl.h _lib_gettext)
check_symbol_exists (textdomain libintl.h _lib_textdomain)
unset (CMAKE_REQUIRED_LIBRARIES)

if (_sys_mnttab)
  set (MNTTAB_HDR sys/mnttab.h)
endif()
if (_hdr_mnttab)
  set (MNTTAB_HDR mnttab.h)
endif()
if (DEFINED MNTTAB_HDR)
  message ("mnttab header: ${MNTTAB_HDR}")
  check_struct_has_member ("struct mnttab" mt_mntopts
      ${MNTTAB_HDR} _mem_struct_mnttab_mt_mntopts)
endif()

if (_sys_statvfs)
  # cmake: check_type_size and extra-include files issue
  set (CMAKE_EXTRA_INCLUDE_FILES sys/statvfs.h)
  check_type_size ("statvfs_t" _typ_statvfs_t)
  set (CMAKE_EXTRA_INCLUDE_FILES <sys/statvfs.h>)
  check_struct_has_member ("struct statvfs" f_basetype
      sys/statvfs.h _mem_struct_statvfs_f_basetype)
  unset (CMAKE_EXTRA_INCLUDE_FILES)
endif()

check_type_size ("gid_t" _typ_gid_t)
check_type_size ("size_t" _typ_size_t)
check_type_size ("uid_t" _typ_uid_t)
check_type_size ("double" _siz_double)
check_type_size ("long double" _siz_long_double)
check_type_size ("uint64_t" _siz_uint64_t)
check_type_size ("long" _siz_long)
check_type_size ("long long" _siz_long_long)

if (_hdr_ufs_ufs_quota)
  # FreeBSD ufs/ufs/quota.h does not include its dependencies
  # OpenBSD declares quotactl in unistd.h
  set (QUOTA_HDR "sys/types.h;unistd.h;ufs/ufs/quota.h")
endif()
if (_sys_quota)
  set (QUOTA_HDR "sys/types.h;sys/quota.h")
endif()
if (_hdr_ufs_quota)
  set (QUOTA_HDR "ufs/quota.h")
endif()
if (_sys_fs_ufs_quota)
  # solaris
  set (QUOTA_HDR "sys/types.h;sys/fs/ufs_quota.h")
endif()
if (_hdr_quota)
  # NetBSD : put this after ufs/ufs/quota.h and sys/quota.h
  set (QUOTA_HDR "quota.h")
endif()
set (_args_quotactl 0)
if (DEFINED QUOTA_HDR)
  message ("quota header: ${QUOTA_HDR}")
  checkdqblk ("${QUOTA_HDR}")
  checkPrototype_quotactl ("${QUOTA_HDR}")
  check_symbol_exists (QCMD "${QUOTA_HDR}" _define_QCMD)

  if (_hdr_linux_dqblk_xfs)
    # cmake doesn't properly handle the angle brackets when
    # using check_type_size
    set (CMAKE_EXTRA_INCLUDE_FILES linux/dqblk_xfs.h)
  endif()
  check_type_size ("fs_disk_quota_t" _typ_fs_disk_quota_t)
  unset (CMAKE_EXTRA_INCLUDE_FILES)
endif()

if (_sys_mnttab)
 set (SETMNTENT_HDR sys/mnttab.h)
endif()
if (_hdr_mntent)
 set (SETMNTENT_HDR mntent.h)
endif()
set (_args_setmntent 0)
if (DEFINED SETMNTENT_HDR)
  message ("setmntent header: ${SETMNTENT_HDR}")
  checkPrototype_setmntent ("${SETMNTENT_HDR}")
endif()

if (_hdr_rpcsvc_rquota)
  # linux rpcsvc/rquota.h includes rpc/rpc.h
  set (CMAKE_REQUIRED_INCLUDES ${required_includes} ${TIRPC_INCLUDE_DIRS})
  check_struct_has_member ("struct getquota_rslt" gqr_status
      rpcsvc/rquota.h _mem_struct_getquota_rslt_gqr_status)
  check_struct_has_member ("struct getquota_rslt" gqr_rquota
      rpcsvc/rquota.h _mem_struct_getquota_rslt_gqr_rquota)
  check_struct_has_member ("struct rquota" rq_bhardlimit
      rpcsvc/rquota.h _mem_struct_rquota_rq_bhardlimit)
  check_struct_has_member ("struct getquota_args" gqa_uid
      rpcsvc/rquota.h _mem_struct_getquota_args_gqa_uid)
  set (CMAKE_REQUIRED_INCLUDES ${required_includes})

  memberxdr (rquota rq_bhardlimit)
  memberxdr (rquota rq_bsoftlimit)
  memberxdr (rquota rq_curblocks)
  memberxdr (rquota rq_fhardlimit)
  memberxdr (rquota rq_fsoftlimit)
  memberxdr (rquota rq_curfiles)
  memberxdr (getquota_args gqa_uid)
endif()

if ((_lib_statfs OR _lib_getfsstat) AND _sys_mount)
  # OpenBSD has broken headers
  set (STATFS_HDR "sys/types.h;sys/mount.h")
endif()
if ((_lib_statfs OR _lib_getfsstat) AND _sys_vfs)
  set (STATFS_HDR "sys/vfs.h")
endif()
if ((_lib_statfs OR _lib_getfsstat) AND _sys_statfs)
  set (STATFS_HDR "sys/statfs.h")
endif()
set (_args_statfs 0)
if (DEFINED STATFS_HDR)
  message ("statfs header: ${STATFS_HDR}")
  checkPrototype_statfs ("${STATFS_HDR}")
  checkMember_statfs ("${STATFS_HDR}")
endif()

if (DEFINED STATFS_HDR AND _lib_getfsstat)
  message ("getfsstat header: ${STATFS_HDR}")
  checkPrototype_getfsstat ("${STATFS_HDR}")
endif()

set (_args_getvfsstat 0)
set (GETVFSSTAT_HDR "sys/mount.h")
if (DEFINED GETVFSSTAT_HDR AND _lib_getvfsstat)
  message ("getvfsstat header: ${GETVFSSTAT_HDR}")
  checkPrototype_getvfsstat ("${GETVFSSTAT_HDR}")
endif()

if (_lib_getenv)
  check_prototype_definition (getenv
      "char * getenv (const char * a)"
      "NULL"
      stdlib.h
      _npt_getenv
      )
  if (_npt_getenv)
    set (_npt_getenv 0)
  else()
    set (_npt_getenv 1)
  endif()
else()
  set (_npt_getenv 0)
endif()

if (_lib_mntctl)
  # AIX
  check_prototype_definition (mntctl
      "int mntctl (int a, size_t b, char * c)"
      0
      sys/mntctl.h
      _npt_mntctl
      )
  if (_npt_mntctl)
    set (_npt_mntctl 0)
  else()
    set (_npt_mntctl 1)
  endif()
else()
  set (_npt_mntctl 0)
endif()

# HP-UX
if (_lib_quotactl AND NOT _lib_vquotactl AND
    NOT _quotactl_pos_1 AND NOT _quotactl_pos_2)
  check_prototype_definition (quotactl
      "int quotactl (int a, const char * b, uid_t c, void * d)"
      0
      "${QUOTA_HDR}"
      _npt_quotactl
      )
  if (_npt_quotactl)
    set (_npt_quotactl 0)
  else()
    set (_npt_quotactl 1)
  endif()
else()
  set (_npt_quotactl 0)
endif()

if (_lib_statfs AND _args_statfs STREQUAL "0")
  set (_npt_statfs 1)
  set (_args_statfs 2)    # no idea which one...
else()
  set (_npt_statfs 0)
endif()

check_symbol_exists (errno errno.h _dcl_errno)
# *BSD
# which include files?
# check_symbol_exists (mnt_names ${STATFS_HDR} _dcl_mnt_names)

checkvquotactlenabled ()

set (_has_std_quotas 0)
if (_lib_quotactl OR _lib_quota_open OR _lib_vquotactl OR
    _typ_struct_dqblk OR _typ_struct_ufs_dqblk)
  set (_has_std_quotas 1)
endif()

set (_has_std_nfs_quotas 0)
if (_hdr_rpc_rpc AND
    _hdr_rpcsvc_rquota AND
    _lib_xdr_int AND
    _mem_struct_rquota_rq_bhardlimit AND
    _mem_struct_getquota_args_gqa_uid)
  set (_has_std_nfs_quotas 1)
endif()

set (_enable_nls 0)
if (_lib_bindtextdomain AND _lib_gettext AND _lib_setlocale AND
    _lib_textdomain AND _hdr_libintl AND _hdr_locale)
  set (_enable_nls 1)
endif()

checkIncludeConflict (_hdr_time time.h _sys_time sys/time.h
    _inc_conflict__hdr_time__sys_time)
checkIncludeConflict (_sys_quota sys/quota.h _linux_quota linux/quota.h
    _inc_conflict__sys_quota__hdr_linux_quota)

check_symbol_exists (O_NOCTTY fcntl.h _const_O_NOCTTY)

check_symbol_exists (IOCTL_STORAGE_CHECK_VERIFY2 winioctl.h
    _define_IOCTL_STORAGE_CHECK_VERIFY2)
check_symbol_exists (MCTL_QUERY sys/mntctl.h _define_MCTL_QUERY)
check_symbol_exists (S_ISLNK sys/stat.h _define_S_ISLNK)

if (NOT DEFINED DI_USE_MATH OR DI_USE_MATH STREQUAL "")
  if (_hdr_gmp AND GMP_LDFLAGS AND
      NOT (_hdr_tommath OR _hdr_libtommath_tommath))
    set (DI_USE_MATH DI_GMP)
  endif()
  # at the moment, libtommath in macports has a broken pkg-config
  if ((_hdr_tommath OR _hdr_libtommath_tommath) AND
      TOMMATH_LDFLAGS AND
      NOT _hdr_gmp AND
      NOT APPLE)
    set (DI_USE_MATH DI_TOMMATH)
  endif()
  if (_hdr_gmp AND GMP_LDFLAGS AND
      (_hdr_tommath OR _hdr_libtommath_tommath))
    # use the most common library
    set (DI_USE_MATH DI_GMP)
  endif()
endif()
if (NOT DEFINED DI_USE_MATH OR DI_USE_MATH STREQUAL "")
  set (DI_USE_MATH DI_INTERNAL)
endif()
set (_use_math ${DI_USE_MATH})
message ("math-library: ${_use_math}")

configure_file (config.h.in config.h)

###
# macros for building

macro (addIntlLibrary name)
  if (Intl_LIBRARY AND NOT Intl_LIBRARY STREQUAL "NOTFOUND")
    target_include_directories (${name} PRIVATE
      ${Intl_INCLUDE_DIRS}
    )
    target_link_libraries (${name} PRIVATE
      ${Intl_LIBRARY}
    )
  endif()
  if (Iconv_LIBRARY AND NOT Iconv_LIBRARY STREQUAL "NOTFOUND")
    target_include_directories (${name} PRIVATE
      ${Iconv_INCLUDE_DIRS}
    )
    target_link_libraries (${name} PRIVATE
      ${Iconv_LIBRARY}
    )
  endif()
endmacro()

#### libraries

add_library (objdistrutils OBJECT
  distrutils.c
)

add_library (${DI_LIBNAME}
  didiskutil.c
  digetentries.c
  digetinfo.c
  dilib.c
  diquota.c
  dizone.c
  getoptn.c
  dioptions.c
)
target_include_directories (${DI_LIBNAME} PRIVATE
  ${TIRPC_INCLUDE_DIRS}
  ${GMP_INCLUDE_DIRS}
  ${TOMMATH_INCLUDE_DIRS}
)

target_link_libraries (${DI_LIBNAME} PRIVATE
  objdistrutils
)
# cmake's auto prefixing is annoying
set_target_properties (${DI_LIBNAME} PROPERTIES PREFIX "")
if (WIN32)
  set (CMAKE_SHARED_LIBRARY_PREFIX "lib")
  set (CMAKE_STATIC_LIBRARY_PREFIX "lib")
endif()
if (LIBQUOTACTL_REQUIRED)
  # NetBSD
  target_link_libraries (${DI_LIBNAME} PRIVATE
    -lquota
  )
endif()
if (LIBGEN_REQUIRED)
  # Unixware
  target_link_libraries (${DI_LIBNAME} PRIVATE
    -lgen
  )
endif()
if (LIBNSL_REQUIRED)
  target_link_libraries (${DI_LIBNAME} PRIVATE
    -lnsl
  )
endif()
if (LIBTIRPC_REQUIRED)
  target_link_libraries (${DI_LIBNAME} PRIVATE
    ${TIRPC_LDFLAGS}
  )
endif()
if (LIBSNPRINTF_REQUIRED)
  target_link_libraries (${DI_LIBNAME} PRIVATE
    -lsnprintf
  )
endif()
if (_use_math STREQUAL "DI_GMP")
  target_link_libraries (${DI_LIBNAME} PRIVATE
    ${GMP_LDFLAGS}
  )
endif()
if (_use_math STREQUAL "DI_TOMMATH")
  target_link_libraries (${DI_LIBNAME} PRIVATE
    ${TOMMATH_LDFLAGS}
  )
endif()
set_target_properties (${DI_LIBNAME} PROPERTIES
  VERSION ${DI_LIBVERSION}
  SOVERSION ${DI_SOVERSION}
)

# I don't know if this is needed.  windows works fine for me.
if (WIN32)
  set_target_properties (${DI_LIBNAME} PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

#### executables

add_executable (di
  di.c
)
addIntlLibrary (di)
target_include_directories (di PRIVATE
  ${TIRPC_INCLUDE_DIRS}
)
if (CMAKE_HOST_SOLARIS)
  target_link_options (di PRIVATE
    "-Wl,-R${CMAKE_INSTALL_FULL_LIBDIR}"
  )
endif()
target_link_libraries (di PRIVATE
  ${TIRPC_LDFLAGS}
)
target_link_libraries (di PRIVATE
  ${DI_LIBNAME}
)
if (_use_math STREQUAL "DI_GMP")
  target_link_libraries (di PRIVATE
    ${GMP_LDFLAGS}
  )
endif()
if (_use_math STREQUAL "DI_TOMMATH")
  target_link_libraries (di PRIVATE
    ${TOMMATH_LDFLAGS}
  )
endif()

add_executable (dimathtest
  dimathtest.c
)
target_include_directories (dimathtest PRIVATE
  ${GMP_INCLUDE_DIRS}
  ${TOMMATH_INCLUDE_DIRS}
)
if (_use_math STREQUAL "DI_GMP")
  target_link_libraries (dimathtest PRIVATE
    ${GMP_LDFLAGS}
  )
endif()
if (_use_math STREQUAL "DI_TOMMATH")
  target_link_libraries (dimathtest PRIVATE
    ${TOMMATH_LDFLAGS}
  )
endif()

add_executable (getoptn_test
  getoptn.c
)
target_compile_options (getoptn_test PRIVATE
  -DTEST_GETOPTN
)
target_link_libraries (getoptn_test PRIVATE
  objdistrutils
)

# di.pc

# cmake puts 'general' in front of every item, and semicolons
# are treated specially... weird
# list join does not work
set (DI_REQUIRED_LIBS ${${DI_LIBNAME}_LIB_DEPENDS})
if (NOT DEFINED DI_REQUIRED_LIBS OR DI_REQUIRED_LIBS STREQUAL "")
  set (DI_REQUIRED_LIBS " ")
endif()
string (REPLACE "general" " " DI_REQUIRED_LIBS ${DI_REQUIRED_LIBS})

configure_file (${CMAKE_SOURCE_DIR}/di.pc.in di.pc @ONLY)

# need to build the .mo files

#### install

install (TARGETS ${DI_LIBNAME}
  ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  # windows seems to need this
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
)

install (TARGETS di
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
)

install (FILES di.h
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

# gnu-install-dirs has an incorrect path for netbsd10
install (FILES man/di.1
  DESTINATION "${CMAKE_INSTALL_DATADIR}/man/man1"
)
install (FILES man/libdi.3
  DESTINATION "${CMAKE_INSTALL_DATADIR}/man/man3"
)

install (FILES
  ${CMAKE_BINARY_DIR}/di.pc
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig"
)

install (CODE
  "execute_process (
    COMMAND utils/instpo.sh \"${CMAKE_INSTALL_FULL_LOCALEDIR}\"
  )"
)
