#
# Copyright 2024 Brad Lanam Pleasant Hill CA
#
cmake_minimum_required (VERSION 3.10)

# avoid msys2/windows issue
set (CMAKE_C_COMPILER_WORKS 1)
set (CMAKE_CXX_COMPILER_WORKS 1)

project (DI VERSION ${DI_VERSION})
option (BUILD_SHARED_LIBS "Build dynamic library" ON)

set (default_build_type "Release")

include (GNUInstallDirs)

set (DI_LIBNAME libdi)

SET (CMAKE_SKIP_BUILD_RPATH FALSE)
SET (CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)

#### compile options

add_compile_options (-fPIC)

add_compile_options (-Wall)
add_compile_options (-Wextra)
add_compile_options (-Wconversion)
add_compile_options (-Wno-unused-but-set-variable)
add_compile_options (-Wno-unused-parameter)
add_compile_options (-Wno-unknown-pragmas)
add_compile_options (-Wno-float-equal)
add_compile_options (-Wdeclaration-after-statement)
add_compile_options (-Wmissing-prototypes)
add_compile_options (-Wformat)
add_compile_options (-Wformat-security)
add_compile_options (-Werror=format-security)
add_compile_options (-Werror=return-type)
add_compile_options (-Wdeprecated-declarations)
add_compile_options (-Wunreachable-code)

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
  SET (CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_FULL_LIBDIR} ${CMAKE_INSTLAL_LIBDIR})
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
    # not universal binary, I don't have universal libintl installed.
    # set (CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
  endif()
else()
  add_link_options (-static-libgcc)
  add_link_options (-static-libstdc++)
endif()

#### include paths

include_directories (
  SYSTEM
  /opt/local/include
  "${CMAKE_BINARY_DIR}"
  "${PROJECT_SOURCE_DIR}/include"
)

set (CMAKE_REQUIRED_INCLUDES
  /opt/local/include
)

#### configuration

find_package (PkgConfig)
find_package (Intl)
find_package (Iconv)

pkg_check_modules (GMP gmp)
pkg_check_modules (TOMMATH libtommath)
pkg_check_modules (TIRPC libtirpc)

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
    "int quotactl (const char * a, int b, int c, caddr_t d)"
    0
    ${hdr}
    _quotactl_pos_1
    )
  check_prototype_definition (quotactl
    "int quotactl (int a, const char * b, int c, caddr_t d)"
    0
    ${hdr}
    _quotactl_pos_2
    )
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
    ${hdr}
    _args_statfs_2
    )
  check_prototype_definition (statfs
    "int statfs (const char * a, struct statfs * b, int c)"
    0
    ${hdr}
    _args_statfs_3
    )
  check_prototype_definition (statfs
    "int statfs (const char * a, struct statfs * b, int c, int d)"
    0
    ${hdr}
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
      ${hdr} _mem_struct_statfs_f_bsize)
  check_struct_has_member ("struct statfs" f_fsize
      ${hdr} _mem_struct_statfs_f_fsize)
  check_struct_has_member ("struct statfs" f_fstyp
      ${hdr} _mem_struct_statfs_f_fstyp)
  check_struct_has_member ("struct statfs" f_iosize
      ${hdr} _mem_struct_statfs_f_iosize)
  check_struct_has_member ("struct statfs" f_frsize
      ${hdr} _mem_struct_statfs_f_frsize)
  check_struct_has_member ("struct statfs" f_fstypename
      ${hdr} _mem_struct_statfs_f_fstypename)
  check_struct_has_member ("struct statfs" mount_info
      ${hdr} _mem_struct_statfs_mount_info)
  check_struct_has_member ("struct statfs" f_type
      ${hdr} _mem_struct_statfs_f_type)
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
  if (_getfsstat_type_int)
    set (_c_arg_2_getfsstat int)
  endif()
  if (_getfsstat_type_long)
    set (_c_arg_2_getfsstat long)
  endif()
endmacro()

macro (checkPrototype_getvfsstat hdr)
  check_prototype_definition (getvfsstat
    "int getvfsstat (struct statvfs * a, int b, int c)"
    0
    "stdio.h;stddef.h;${hdr}"
    _args_getvfsstat_3
    )
  if (_args_getvfsstat_3)
    set (_args_getvfsstat 3)
  endif()
endmacro()

macro (checkdqblk hdr)
  # the calls to checkdqblk do not include the angle brackets
  set (CMAKE_EXTRA_INCLUDE_FILES ${hdr})
  check_type_size ("struct dqblk" _typ_struct_dqblk)
  check_type_size ("struct quotaval" _typ_struct_quotaval)
  check_type_size ("struct ufs_dqblk" _typ_struct_ufs_dqblk)
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

macro (memberxdr structnm membernm)
  message ("-- Determining ${structnm} ${membernm} xdr type")
  file (WRITE txdr.c "#include <rpcsvc/rquota.h>")
  set (tinc ".")
  if (NOT ${TIRPC_INCLUDE_DIRS} STREQUAL "")
    set (tinc ${TIRPC_INCLUDE_DIRS})
  endif()
  execute_process (
    OUTPUT_FILE txdr.out
    ERROR_QUIET
    COMMAND ${CMAKE_C_COMPILER} -I ${tinc} -E txdr.c
  )
  unset (tinc)
  if (EXISTS txdr.out)
    execute_process (
      OUTPUT_VARIABLE xdr_out
      COMMAND grep "^ *[a-z_][a-z_]* *${membernm};" txdr.out
      COMMAND sed -e "s,^ *,," -e "s, .*,,"
      COMMAND tr -d "\n"
    )
    set (xdr_${membernm} xdr_${xdr_out})
  endif()
  file (REMOVE txdr.c)
  if (EXISTS txdr.out)
    file (REMOVE txdr.out)
  endif()
  unset (CMAKE_REQUIRED_INCLUDES)
endmacro()

macro (checkIncludeConflict hdra hdrb varnm)
  message ("-- Check for include conflict ${hdra} ${hdrb}")
  # use execute_process rather than check_source_compiles
  # so that earlier versions of cmake can be used.
  file (WRITE tinc.c "#include <${hdra}>
#include <${hdrb}>
int main (int argc, char *argv []) {
  return 0;
}"
  )
  execute_process (
    OUTPUT_QUIET
    ERROR_QUIET
    RESULT_VARIABLE tincres
    COMMAND ${CMAKE_C_COMPILER} -c tinc.c
  )
  set (${varnm} 0)
  if (tincres EQUAL 0)
    set (${varnm} 1)
  endif()
  file (REMOVE tinc.c)
  if (EXISTS tinc.o)
    file (REMOVE tinc.o)
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
check_include_file (math.h _hdr_math)
check_include_file (memory.h _hdr_memory)
check_include_file (mntent.h _hdr_mntent)
check_include_file (mnttab.h _hdr_mnttab)
# NetBSD
check_include_file (quota.h _hdr_quota)

set (CMAKE_REQUIRED_INCLUDES ${TIRPC_INCLUDE_DIRS})
check_include_file (rpc/auth.h _hdr_rpc_auth)
check_include_file (rpc/rpc.h _hdr_rpc_rpc)
# Linux: rquota.h includes rpc/rpc.h
check_include_file (rpcsvc/rquota.h _hdr_rpcsvc_rquota)
unset (CMAKE_REQUIRED_INCLUDES)

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
check_include_file (libtommath/tommath.h _hdr_libtommath_tommath)
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

check_function_exists (setlocale _lib_setlocale)
check_function_exists (statfs _lib_statfs)
check_function_exists (statvfs _lib_statvfs)
check_function_exists (stpecpy _lib_stpecpy)
check_function_exists (strcoll _lib_strcoll)
check_function_exists (strdup _lib_strdup)
check_function_exists (strstr _lib_strstr)
check_function_exists (sysfs _lib_sysfs)
# dragonflybsd
check_function_exists (vquotactl _lib_vquotactl)

check_function_exists (xdr_int _lib_xdr_int)
set (LIBNSL_REQUIRED 0)
set (LIBTIRPC_REQUIRED 0)
if (NOT _lib_xdr_int)
  # check with nsl library
  set (CMAKE_REQUIRED_LIBRARIES nsl)
  check_function_exists (xdr_int _lib_xdr_int_nsl)
  if (_lib_xdr_int_nsl)
    set (_lib_xdr_int 1)
    set (LIBNSL_REQUIRED 1)
  endif()
endif()
if (NOT _lib_xdr_int)
  # check with tirpc library
  set (CMAKE_REQUIRED_LIBRARIES tirpc)
  check_function_exists (xdr_int _lib_xdr_int_tirpc)
  if (_lib_xdr_int_tirpc)
    set (_lib_xdr_int 1)
    set (LIBTIRPC_REQUIRED 1)
  endif()
endif()

# solaris
check_function_exists (zone_getattr _lib_zone_getattr)
check_function_exists (zone_list _lib_zone_list)

set (CMAKE_REQUIRED_LIBRARIES ${Intl_LIBRARY} ${Iconv_LIBRARY})
check_function_exists (bindtextdomain _lib_bindtextdomain)
check_function_exists (gettext _lib_gettext)
check_function_exists (textdomain _lib_textdomain)
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
  checkdqblk (${QUOTA_HDR})
  checkPrototype_quotactl (${QUOTA_HDR})
  check_symbol_exists (QCMD ${QUOTA_HDR} _define_QCMD)

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
  checkPrototype_setmntent (${SETMNTENT_HDR})
endif()

if (_hdr_rpcsvc_rquota)
  # linux rpcsvc/rquota.h includes rpc/rpc.h
  set (CMAKE_REQUIRED_INCLUDES ${TIRPC_INCLUDE_DIRS})
  check_struct_has_member ("struct getquota_rslt" gqr_status
      rpcsvc/rquota.h _mem_struct_getquota_rslt_gqr_status)
  check_struct_has_member ("struct getquota_rslt" gqr_rquota
      rpcsvc/rquota.h _mem_struct_getquota_rslt_gqr_rquota)
  check_struct_has_member ("struct rquota" rq_bhardlimit
      rpcsvc/rquota.h _mem_struct_rquota_rq_bhardlimit)
  check_struct_has_member ("struct getquota_args" gqa_uid
      rpcsvc/rquota.h _mem_struct_getquota_args_gqa_uid)
  unset (CMAKE_REQUIRED_INCLUDES)

  memberxdr (rquota rq_bhardlimit)
  memberxdr (rquota rq_bsoftlimit)
  memberxdr (rquota rq_curblocks)
  memberxdr (rquota rq_fhardlimit)
  memberxdr (rquota rq_fsoftlimit)
  memberxdr (rquota rq_curfiles)
  memberxdr (getquota_args gqa_uid)
endif()

if ((_lib_statfs OR _lib_getfsstat) AND _sys_mount)
  set (STATFS_HDR "sys/mount.h")
endif()
if ((_lib_statfs OR _lib_getfsstat) AND _sys_statfs)
  set (STATFS_HDR "sys/statfs.h")
endif()
if ((_lib_statfs OR _lib_getfsstat) AND _sys_vfs)
  set (STATFS_HDR "sys/vfs.h")
endif()
set (_args_statfs 0)
if (DEFINED STATFS_HDR)
  message ("statfs header: ${STATFS_HDR}")
  checkPrototype_statfs (${STATFS_HDR})
  checkMember_statfs (${STATFS_HDR})
endif()

if (DEFINED STATFS_HDR AND _lib_getfsstat)
  message ("getfsstat header: ${STATFS_HDR}")
  checkPrototype_getfsstat (${STATFS_HDR})
endif()

set (_args_getvfsstat 0)
# ### needs fixing
set (GETVFSSTAT_HDR "sys/mount.h")
if (DEFINED GETVFSSTAT_HDR AND _lib_getvfsstat)
  message ("getvfsstat header: ${GETVFSSTAT_HDR}")
  checkPrototype_getvfsstat (${GETVFSSTAT_HDR})
endif()

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

# ultrix, which header should be checked?
# this is so old, it can probably be removed.
check_prototype_definition (getmnt
    "int getmnt (int * a, struct fs_data * b, int c, int d, char * e)"
    0
    stdio.h
    _npt_getmnt
    )
if (_npt_getmnt)
  set (_npt_getmnt 0)
else()
  set (_npt_getmnt 1)
endif()

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

# HP-UX
if (NOT _quotactl_pos_1 AND NOT _quotactl_pos_2)
  check_prototype_definition (quotactl
      "int quotactl (int a, const char * b, uid_t c, void * d)"
      0
      ${QUOTA_HDR}
      _npt_quotactl
      )
  if (_npt_quotactl)
    set (_npt_quotactl 0)
  else()
    set (_npt_quotactl 1)
  endif()
endif()

check_symbol_exists (errno errno.h _dcl_errno)
# *BSD
# which include files?
check_symbol_exists (mnt_names ${STATFS_HDR} _dcl_mnt_names)

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

find_program (_command_msgfmt msgfmt)
find_program (_command_gmsgfmt gmsgfmt)
if (_command_gmsgfmt)
  set (_command_msgfmt ${_command_gmsgfmt})
endif()

set (_enable_nls 0)
if (_lib_bindtextdomain AND _lib_gettext AND _lib_setlocale AND
    _lib_textdomain AND _hdr_libintl AND _hdr_locale)
  set (_enable_nls 1)
endif()

# check_source_compiles requires a fairly recent version of cmake (3.19).
# instead, assume that anything with cmake doesn't have broken headers
set (_inc_conflict__hdr_time__sys_time 1)
set (_inc_conflict__sys_quota__hdr_linux_quota 1)

checkIncludeConflict (time.h sys/time.h _inc_conflict__hdr_time__sys_time)
checkIncludeConflict (sys/quota.h linux/quota.h _inc_conflict__sys_quota__hdr_linux_quota)

check_symbol_exists (O_NOCTTY fcntl.h _const_O_NOCTTY)

check_symbol_exists (IOCTL_STORAGE_CHECK_VERIFY2 winioctl.h
    _define_IOCTL_STORAGE_CHECK_VERIFY2)
check_symbol_exists (MCTL_QUERY sys/mntctl.h _define_MCTL_QUERY)
check_symbol_exists (S_ISLNK sys/stat.h _define_S_ISLNK)

if (DEFINED DI_USE_MATH AND NOT DI_USE_MATH STREQUAL "")
  # must be one of DI_INTERNAL, DI_GMP, DI_TOMMATH
  set (_use_math ${DI_USE_MATH})
endif()
if (NOT DEFINED _use_math)
  if (_hdr_gmp AND NOT (_hdr_tommath OR _hdr_libtommath_tommath))
    set (_use_math DI_GMP)
  endif()
  if ((_hdr_tommath OR _hdr_libtommath_tommath) AND NOT _hdr_gmp)
    set (_use_math DI_TOMMATH)
  endif()
  if (_hdr_gmp AND (_hdr_tommath OR _hdr_libtommath_tommath))
    # use the more popular library if both are present
    set (_use_math DI_GMP)
  endif()
endif()
if (NOT DEFINED _use_math)
  set (_use_math DI_INTERNAL)
endif()
message ("math-library: ${_use_math}")

configure_file (config.h.in config.h)

macro (addIntlLibrary name)
  if (Intl_LIBRARY)
    target_link_libraries (${name} PRIVATE
      ${Intl_LIBRARY}
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
target_link_libraries (${DI_LIBNAME} PRIVATE
  objdistrutils
)
# cmake's auto prefixing is annoying
set_target_properties (${DI_LIBNAME} PROPERTIES PREFIX "")
if (WIN32)
  set (CMAKE_SHARED_LIBRARY_PREFIX "lib")
  set (CMAKE_STATIC_LIBRARY_PREFIX "lib")
endif()
target_include_directories (${DI_LIBNAME} PRIVATE
  ${TIRPC_INCLUDE_DIRS}
)
target_link_libraries (${DI_LIBNAME} PRIVATE
  ${TIRPC_LDFLAGS}
)
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

install (FILES man/di.1
  DESTINATION "${CMAKE_INSTALL_MANDIR}/man1"
)
# does not yet exist
#install (FILES man/libdi.3
#  DESTINATION "${CMAKE_INSTALL_MANDIR}/man3"
#)

install (FILES
  ${CMAKE_BINARY_DIR}/di.pc
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig"
)

# need to install the .mo files
