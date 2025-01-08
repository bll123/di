#
# Copyright 2024 Brad Lanam Pleasant Hill CA
#
cmake_minimum_required (VERSION 3.19)

# avoid msys2/windows issue
set (CMAKE_C_COMPILER_WORKS 1)
set (CMAKE_CXX_COMPILER_WORKS 1)

set (DI_BUILD_VERS "5.0.0")
project (DI VERSION ${DI_BUILD_VERS})

option (BUILD_SHARED_LIBS "Build dynamic library" ON)

set (default_build_type "Release")

include (GNUInstallDirs)

set (DI_LIBNAME libdi)
if (WIN32)
  set (CMAKE_SHARED_LIBRARY_PREFIX "lib")
  set (CMAKE_STATIC_LIBRARY_PREFIX "lib")
endif()

include_directories (
  "${CMAKE_BINARY_DIR}"
)

SET (CMAKE_SKIP_BUILD_RPATH FALSE)
SET (CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)

#### compile options

add_compile_options (-fPIC)

add_compile_options (-Wall)
add_compile_options (-Wextra)
add_compile_options (-Wno-unused-parameter)
add_compile_options (-Wno-unknown-pragmas)
add_compile_options (-Wno-float-equal)
add_compile_options (-Wdeclaration-after-statement)
add_compile_options (-Wmissing-prototypes)
add_compile_options (-Wformat)
add_compile_options (-Wformat-security)
add_compile_options (-Werror=format-security)
add_compile_options (-Wdeprecated-declarations)

#### compiler-specific compile options

if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
  add_compile_options (-Wmaybe-uninitialized)
  add_compile_options (-Wno-unused-but-set-variable)
  add_compile_options (-Wno-stringop-overflow)
  add_compile_options (-Wno-stringop-truncation)
  add_compile_options (-Wno-format-truncation)
endif()
if (CMAKE_C_COMPILER_ID STREQUAL "Clang")
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
endif()

#### build compile options

if (DI_BUILD STREQUAL "Release")
  add_compile_options (-O2)
endif()

if (DI_BUILD STREQUAL "Debug")
  add_compile_options (-O0)
endif()

add_compile_options (-g)
add_link_options (-g)
if (NOT WIN32)
  add_link_options (-rdynamic)
endif()

#### more compile options: fortification/address sanitizer

set (DI_FORTIFY T)

# address sanitizer
if (DI_BUILD STREQUAL "SanitizeAddress")
  set (DI_FORTIFY F)
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

if (DI_FORTIFY STREQUAL T)
  # hardening
  add_compile_options (-fstack-protector-strong)
  add_compile_options (-fstack-protector-all)
  add_compile_options (-fstack-protector-strong)
  add_compile_options (-fstack-protector-all)
  add_compile_options (-D_FORTIFY_SOURCE=2)
else()
  if (CMAKE_C_COMPILER_ID STREQUAL "Clang")
    add_compile_options (-Wno-macro-redefined)
  endif()
  add_compile_options (-U_FORTIFY_SOURCE)
  add_compile_options (-D_FORTIFY_SOURCE=0)
endif()

#### system specific compile options

if (NOT WIN32)
  if (NOT APPLE)
#    SET (CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_FULL_LIBDIR} "\${ORIGIN}")
  endif()
  if (APPLE)
#    SET (CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_FULL_LIBDIR} "@loader_path")
    # 10.12 = Sierra, 10.13 = High Sierra
    # 10.14 = Mojave, 10.15 = Catalina
    # 11 = Big Sur, 12 = Monterey, 13 = Ventura, 14 = Sonoma
    # 15 = Sequoia
    add_compile_options (-mmacosx-version-min=10.12)
    add_link_options (-mmacosx-version-min=10.12)
    # universal binary
    set (CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
  endif()
else()
  add_link_options (-static-libgcc)
  add_link_options (-static-libstdc++)
endif()

#### configuration

find_package (PkgConfig)
find_package (Intl)
find_package (Iconv)

pkg_check_modules (GMP gmp)
pkg_check_modules (TOMMATH libtommath)

include (CheckFunctionExists)
include (CheckIncludeFile)
include (CheckLibraryExists)
include (CheckLinkerFlag)
include (CheckSourceCompiles)
include (CheckStructHasMember)
include (CheckSymbolExists)
include (CheckTypeSize)
include (CheckVariableExists)
include (CheckPrototypeDefinition)

macro (checkPrototype_quotactl hdr)
  check_prototype_definition (quotactl
    "int quotactl (const char *, int,  int, caddr_t)"
    0
    ${hdr}
    _quotactl_pos_1
    )
  check_prototype_definition (quotactl
    "int quotactl (int, const char *, int, caddr_t)"
    0
    ${hdr}
    _quotactl_pos_2
    )
endmacro()

macro (checkPrototype_statfs hdr)
  check_prototype_definition (statfs
    "int statfs (const char *, struct statfs *)"
    0
    ${hdr}
    _args_statfs_2
    )
  check_prototype_definition (statfs
    "int statfs (const char *, struct statfs *, int)"
    0
    ${hdr}
    _args_statfs_3
    )
  check_prototype_definition (statfs
    "int statfs (const char *, struct statfs *, int, int)"
    0
    ${hdr}
    _args_statfs_4
    )
endmacro()

macro (checkPrototype_setmntent hdr)
  check_prototype_definition (setmntent
    "FILE * setmntent (const char *)"
    "NULL"
    "stdio.h;stddef.h;${hdr}"
    _args_setmntent_1
    )
  check_prototype_definition (setmntent
    "FILE * setmntent (const char *, const char *)"
    "NULL"
    "stdio.h;stddef.h;${hdr}"
    _args_setmntent_2
    )
endmacro()

macro (checkdqblk hdr)
  if (_hdr_linux_dqblk_xfs)
    set (CMAKE_EXTRA_INCLUDE_FILES <linux/dqblk_xfs.h>)
  endif()
  check_type_size ("struct dqblk" _typ_struct_dqblk)
  check_type_size ("struct quotaval" _typ_struct_quotaval)
  check_type_size ("struct ufs_dqblk" _typ_struct_ufs_dqblk)
  check_type_size ("fs_disk_quota_t" _typ_fs_disk_quota_t)
  check_struct_has_member ("struct dqblk" dqb_curspace
      ${hdr} _mem_struct_dqblk_dqb_curspace)
  check_struct_has_member ("struct dqblk" dqb_curblocks
      ${hdr} _mem_struct_dqblk_dqb_curblocks)
  check_struct_has_member ("struct dqblk" dqb_fhardlimit
      ${hdr} _mem_struct_dqblk_dqb_fhardlimit)
  check_struct_has_member ("struct dqblk" dqb_fsoftlimit
      ${hdr} _mem_struct_dqblk_dqb_fsoftlimit)
  check_struct_has_member ("struct dqblk" dqb_curfiles
      ${hdr} _mem_struct_dqblk_dqb_curfiles)
  unset (CMAKE_EXTRA_INCLUDE_FILES)
endmacro ()

macro (addIntlLibrary name)
  if (Intl_LIBRARY)
    target_link_libraries (${name} PRIVATE
      ${Intl_LIBRARY}
      ${Iconv_LIBRARY}
    )
  endif()
endmacro()

check_include_file (ctype.h _hdr_ctype)
check_include_file (dirent.h _hdr_dirent)
check_include_file (errno.h _hdr_errno)
check_include_file (fcntl.h _hdr_fcntl)
check_include_file (gmp.h _hdr_gmp)
check_include_file (fshelp.h _hdr_fshelp)
check_include_file (gui/window.h _hdr_gui_window)
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
check_include_file (memory.h _hdr_memory)
check_include_file (mntent.h _hdr_mntent)
check_include_file (mnttab.h _hdr_mnttab)
# NetBSD
check_include_file (quota.h _hdr_quota)
check_include_file (rpc/rpc.h _hdr_rpc_rpc)
check_include_file (rpcsvc/rquota.h _hdr_rpcsvc_rquota)
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
check_include_file (tommath.h _hdr_tommath)
check_include_file (ufs/quota.h _hdr_ufs_quota)
check_include_file (ufs/ufs/quota.h _hdr_ufs_ufs_quota)
check_include_file (unistd.h _hdr_unistd)
check_include_file (util/string.h _hdr_util_string)
check_include_file (wchar.h _hdr_wchar)
check_include_file (windows.h _hdr_windows)
check_include_file (winioctl.h _hdr_winioctl)
check_include_file (zone.h _hdr_zone)

check_include_file (sys/dcmd_blk.h _sys_dcmd_blk)
check_include_file (sys/file.h _sys_file)
check_include_file (sys/fs_types.h _sys_fs_types)
check_include_file (sys/fs/ufs_quota.h _sys_fs_ufs_quota)
check_include_file (sys/fstyp.h _sys_fstyp)
check_include_file (sys/fstypes.h _sys_fstypes)
check_include_file (sys/ftype.h _sys_ftype)
check_include_file (sys/io.h _sys_io)
check_include_file (sys/mntctl.h _sys_mntctl)
check_include_file (sys/param.h _sys_param)
check_include_file (sys/types.h _sys_types)

# SCO OpenServer/UnixWare require sys/mnttab.h for struct mnttab declaration.
check_include_file (sys/mnttab.h _sys_mnttab)
if (_sys_mnttab)
  set (CMAKE_EXTRA_INCLUDE_FILES <sys/mnttab.h>)
endif()
check_include_file (sys/mntent.h _sys_mntent)
unset (CMAKE_EXTRA_INCLUDE_FILES)

check_include_file (sys/mount.h _sys_mount)
check_include_file (sys/quota.h _sys_quota)
check_include_file (sys/stat.h _sys_stat)
check_include_file (sys/statfs.h _sys_statfs)
check_include_file (sys/statvfs.h _sys_statvfs)
check_include_file (sys/time.h _sys_time)
check_include_file (sys/vfs.h _sys_vfs)
check_include_file (sys/vfs_quota.h _sys_vfs_quota)
check_include_file (sys/vfstab.h _sys_vfstab)
check_include_file (sys/vmount.h _sys_vmount)

check_function_exists (bcopy _lib_bcopy)
check_function_exists (bzero _lib_bzero)
# -lsun, -lseq
check_function_exists (endmntent _lib_endmntent)
check_function_exists (fs_stat_dev _lib_fs_stat_dev)
check_function_exists (fshelp _lib_fshelp)
check_function_exists (GetDiskFreeSpace _lib_GetDiskFreeSpace)
check_function_exists (GetDiskFreeSpaceEx _lib_GetDiskFreeSpaceEx)
check_function_exists (GetDriveType _lib_GetDriveType)
check_function_exists (getfsstat _lib_getfsstat)
check_function_exists (GetLogicalDriveStrings _lib_GetLogicalDriveStrings)
check_function_exists (GetVolumeInformation _lib_GetVolumeInformation)
check_function_exists (getmnt _lib_getmnt)
# unixware put getmntent into libgen for some reason
check_function_exists (getmntent _lib_getmntent)
set (LIBGEN_REQUIRED 0)
if (NOT _lib_getmntent)
  set (CMAKE_REQUIRED_LIBRARIES -lgen)
  check_function_exists (getmntent _lib_getmntent)
  if (_lib_getmntent)
    set (LIBGEN_REQUIRED 1)
  endif()
  unset (CMAKE_REQUIRED_LIBRARIES)
endif()
check_function_exists (getmntinfo _lib_getmntinfo)
check_function_exists (getvfsstat _lib_getvfsstat)
check_function_exists (getzoneid _lib_getzoneid)
check_function_exists (hasmntopt _lib_hasmntopt)
check_function_exists (lstat _lib_lstat)
check_function_exists (mbrlen _lib_mbrlen)
check_function_exists (memcpy _lib_memcpy)
check_function_exists (memset _lib_memset)

# AIX doesn't declare this :(
# Look for MCTL_QUERY (see below)
check_function_exists (mntctl _lib_mntctl)

check_function_exists (next_dev _lib_next_dev)

# dragonflybsd; need this to get the library
set (CMAKE_REQUIRED_LIBRARIES -lprop)
check_function_exists (prop_dictionary_create _lib_prop_dictionary_create)
unset (CMAKE_REQUIRED_LIBRARIES)

# quota_open is a new interface from NetBSD
set (CMAKE_REQUIRED_LIBRARIES -lquota -lrpcsvc)
check_function_exists (quota_open _lib_quota_open)
unset (CMAKE_REQUIRED_LIBRARIES)

check_function_exists (quotactl _lib_quotactl)
check_function_exists (realpath _lib_realpath)
# unknown if -lsun, -lseq are needed (old irix, sequent)
check_function_exists (setmntent _lib_setmntent)

check_function_exists (snprintf _lib_snprintf)
set (LIBSNPRINTF_REQUIRED 0)
if (NOT _lib_snprintf)
  set (CMAKE_REQUIRED_LIBRARIES -lsnprintf)
  check_function_exists (snprintf _lib_snprintf)
  if (_lib_snprintf)
    set (LIBSNPRINTF_REQUIRED 1)
  endif()
  unset (CMAKE_REQUIRED_LIBRARIES)
endif()

check_function_exists (statfs _lib_statfs)
check_function_exists (statvfs _lib_statvfs)
check_function_exists (strcoll _lib_strcoll)
check_function_exists (strdup _lib_strdup)
check_function_exists (strstr _lib_strstr)
check_function_exists (sysfs _lib_sysfs)
# dragonflybsd
check_function_exists (vquotactl _lib_vquotactl)

check_function_exists (xdr_int _lib_xdr_int)
set (LIBNSL_REQUIRED 1)
if (NOT _lib_xdr_int)
  set (CMAKE_REQUIRED_LIBRARIES -lnsl)
  check_function_exists (xdr_int _lib_xdr_int)
  if (_lib_xdr_int)
    set (LIBNSL_REQUIRED 1)
  endif()
  unset (CMAKE_REQUIRED_LIBRARIES)
endif()

# solaris
check_function_exists (zone_getattr _lib_zone_getattr)
check_function_exists (zone_list _lib_zone_list)

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

if (_sys_statfs)
  set (CMAKE_EXTRA_INCLUDE_FILES <sys/statfs.h>)
  check_struct_has_member ("struct statfs" f_bsize
      sys/statfs.h _mem_struct_statfs_f_bsize)
  check_struct_has_member ("struct statfs" f_fsize
      sys/statfs.h _mem_struct_statfs_f_fsize)
  check_struct_has_member ("struct statfs" f_fstyp
      sys/statfs.h _mem_struct_statfs_f_fstyp)
  check_struct_has_member ("struct statfs" f_iosize
      sys/statfs.h _mem_struct_statfs_f_iosize)
  check_struct_has_member ("struct statfs" f_frsize
      sys/statfs.h _mem_struct_statfs_f_frsize)
  check_struct_has_member ("struct statfs" f_fstypename
      sys/statfs.h _mem_struct_statfs_f_fstypename)
  check_struct_has_member ("struct statfs" mount_info
      sys/statfs.h _mem_struct_statfs_mount_info)
  check_struct_has_member ("struct statfs" f_type
      sys/statfs.h _mem_struct_statfs_f_type)
  unset (CMAKE_EXTRA_INCLUDE_FILES)
endif()
if (_sys_statvfs)
  set (CMAKE_EXTRA_INCLUDE_FILES <sys/statvfs.h>)
  check_type_size ("statvfs_t" _typ_statvfs_t)
  check_struct_has_member ("struct statvfs" f_basetype
      sys/statvfs.h _mem_struct_statvfs_f_basetype)
  unset (CMAKE_EXTRA_INCLUDE_FILES)
endif()

check_type_size ("gid_t" _typ_gid_t)
check_type_size ("size_t" _typ_size_t)
check_type_size ("uint_t" _typ_uint_t)
check_type_size ("uid_t" _typ_uid_t)
check_type_size ("uint64_t" _siz_uint64_t)
check_type_size ("long" _siz_long)
check_type_size ("long long" _siz_long_long)

if (_hdr_quota)
  set (QUOTA_HDR "quota.h")
endif()
if (_sys_quota)
  set (QUOTA_HDR "sys/quota.h")
endif()
if (_hdr_ufs_quota)
  set (QUOTA_HDR "ufs/quota.h")
endif()
if (_hdr_ufs_ufs_quota)
  set (QUOTA_HDR "ufs/ufs/quota.h")
endif()
if (_sys_fs_ufs_quota)
  set (QUOTA_HDR "sys/fs/ufs_quota.h")
endif()
if (DEFINED QUOTA_HDR)
  message ("quota header: ${QUOTA_HDR}")
  check_struct_has_member ("struct getquota_rslt" gqr_status
      ${QUOTA_HDR} _mem_struct_getquota_rslt_gqr_status)
  check_struct_has_member ("struct getquota_rslt" gqr_rquota
      ${QUOTA_HDR} _mem_struct_getquota_rslt_gqr_rquota)
  checkdqblk (${QUOTA_HDR})
  checkPrototype_quotactl (${QUOTA_HDR})
  check_symbol_exists (QCMD ${QUOTA_HDR} _define_QCMD)
endif()

if (_sys_mnttab)
 set (SETMNTENT_HDR sys/mnttab.h)
endif()
if (_hdr_mntent)
 set (SETMNTENT_HDR mntent.h)
endif()
if (DEFINED SETMNTENT_HDR)
  message ("setmntent header: ${SETMNTENT_HDR}")
  checkPrototype_setmntent (${SETMNTENT_HDR})
endif()

if (_sys_statfs)
  set (STATFS_HDR "sys/statfs.h")
endif()
if (_sys_vfs)
  set (STATFS_HDR "sys/vfs.h")
endif()
if (DEFINED STATFS_HDR)
  message ("statfs header: ${STATFS_HDR}")
  checkPrototype_statfs (${STATFS_HDR})
endif()

check_source_compiles (C
"#include <time.h>
#include <sys/time.h>
int main (int argc, char *argv []) { return 0; }"
  _inc_conflict__hdr_time__sys_time)

check_source_compiles (C
"#include <sys/quota.h>
#include <linux/quota.h>
int main (int argc, char *argv []) { return 0; }"
  _inc_conflict__sys_quota__hdr_linux_quota)

check_symbol_exists (O_NOCTTY fcntl.h _const_O_NOCTTY)
#if (_hdr_fcntl)
#  set (CMAKE_EXTRA_INCLUDE_FILES <fcntl.h>)
#endif()
#const   O_NOCTTY
#unset (CMAKE_EXTRA_INCLUDE_FILES)

check_symbol_exists (IOCTL_STORAGE_CHECK_VERIFY2 winioctl.h
    _define_IOCTL_STORAGE_CHECK_VERIFY2)
check_symbol_exists (MCTL_QUERY sys/mntctl.h _define_MCTL_QUERY)
check_symbol_exists (S_ISLNK sys/stat.h _define_S_ISLNK)

if (DEFINED DI_USE_MATH AND NOT DI_USE_MATH STREQUAL "")
  # must be one of DI_UINT64, DI_GMP, DI_TOMMATH
  set (_use_math ${DI_USE_MATH})
endif()
if (NOT DEFINED _use_math)
  if (_hdr_gmp AND NOT _hdr_tommath)
    set (_use_math DI_GMP)
  endif()
  if (_hdr_tommath AND NOT _hdr_gmp)
    set (_use_math DI_TOMMATH)
  endif()
  if (_hdr_gmp AND _hdr_tommath)
    # use the smaller library if both are present
    set (_use_math DI_TOMMATH)
  endif()
endif()
if (NOT DEFINED _use_math)
  set (_use_math DI_UINT64)
endif()
message ("math-library: ${_use_math}")

configure_file (config.h.in config.h)

#### libraries

add_library (${DI_LIBNAME}
  didiskutil.c
  digetentries.c
  digetinfo.c
  dilib.c
  diquota.c
  getoptn.c
  options.c
  realloc.c
  strdup.c
  strstr.c
  trimchar.c
)
addIntlLibrary (${DI_LIBNAME})

# I don't know if this is needed.  windows works fine for me.
if (WIN32)
  set_target_properties (${DI_LIBNAME} PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()
set_target_properties (${DI_LIBNAME} PROPERTIES
  VERSION ${CMAKE_PROJECT_VERSION}
  SOVERSION ${CMAKE_PROJECT_VERSION}
)

#### executables

add_executable (di
  di.c
  display.c
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

# di.pc

configure_file (${CMAKE_SOURCE_DIR}/di.pc.in di.pc @ONLY)

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

install (FILES man/di.1
  DESTINATION "${CMAKE_INSTALL_MANDIR}/man1"
)
install (FILES man/libdi.3
  DESTINATION "${CMAKE_INSTALL_MANDIR}/man3"
)

install (FILES
  ${CMAKE_BINARY_DIR}/di.pc
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig"
)
