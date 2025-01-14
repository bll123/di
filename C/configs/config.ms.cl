/* Created on: Fri Jan 22 10:41:21 PST 2016
    From: ../features/mkconfig.dat
    Using: mkconfig-1.27 */

#ifndef __INC_CONFIG_H
#define __INC_CONFIG_H 1

#define _hdr_stdio 1
#define _hdr_stdlib 1
#define _sys_types 1
#define _sys_param 0
#define _key_void 1
#define _key_const 1
#define _param_void_star 1
#define _proto_stdc 1
#define _hdr_ctype 1
#define _hdr_dcdef 0
#define _hdr_descrip 0
#define _hdr_dirent 0
#define _hdr_dvidef 0
#define _hdr_dvsdef 0
#define _hdr_errno 1
#define _hdr_fcntl 1
#define _hdr_fshelp 0
#define _hdr_gui_window 0
#define _hdr_jfs_quota 0
#define _hdr_kernel_fs_info 0
#define _hdr_limits 1
#define _hdr_linux_dqblk_xfs 0
#define _hdr_linux_quota 0
#define _hdr_libintl 0
#define _hdr_libprop_proplib 0
#define _hdr_locale 1
#define _hdr_malloc 1
#define _hdr_mcheck 0
#define _hdr_memory 1
#define _hdr_mntent 0
#define _hdr_mnttab 0
#define _hdr_quota 0
#define _hdr_rpc_rpc 0
#define _hdr_rpcsvc_rquota 0
#define _hdr_ssdef 0
#define _hdr_starlet 0
#define _hdr_storage_Directory 0
#define _hdr_storage_Entry 0
#define _hdr_storage_Path 0
#define _hdr_storage_volumes 0
#define _hdr_string 1
#define _hdr_strings 0
#define _hdr_time 1
#define _hdr_ufs_quota 0
#define _hdr_ufs_ufs_quota 0
#define _hdr_unistd 0
#define _hdr_util_string 0
#define _hdr_wchar 1
#define _hdr_windows 1
#define _hdr_winioctl 1
#define _hdr_zone 0
#define _sys_dcmd_blk 0
#define _sys_file 0
#define _sys_fs_types 0
#define _sys_fs_ufs_quota 0
#define _sys_fstyp 0
#define _sys_fstypes 0
#define _sys_ftype 0
#define _sys_io 0
#define _sys_mntctl 0
#define _sys_mntent 0
#define _sys_mnttab 0
#define _sys_mount 0
#define _sys_quota 0
#define _sys_stat 1
#define _sys_statfs 0
#define _sys_statvfs 0
#define _sys_time 0
#define _sys_vfs 0
#define _sys_vfs_quota 0
#define _sys_vfstab 0
#define _sys_vmount 0
#define _inc_conflict__hdr_time__sys_time 1
#define _inc_conflict__sys_quota__hdr_linux_quota 1
#define _command_msgfmt 1
#define _cmd_loc_msgfmt "/usr/bin/msgfmt"
#define _command_rpmbuild 0
#define _const_O_NOCTTY 1
#define _define_bcopy 0
#define _define_bzero 0
#define _define_MCTL_QUERY 0
#define _define_memcpy 0
#define _define_memset 0
#define _define_QCMD 1
#define _define_S_ISLNK 1
#define _typ_struct_dqblk 0
#define _typ_struct_quotaval 0
#define _typ_struct_ufs_dqblk 0
#define _typ_fs_disk_quota_t 0
#define _typ_gid_t 0
#define _typ_statvfs_t 0
#define _typ_size_t 0
#define _typ_uint_t 0
#define _typ_uid_t 0
#define _lib_bcopy 1
#define _lib_bindtextdomain 1
#define _lib_bzero 1
#define _lib_CreateFile 1
#define _lib_DeviceIoControl 1
#define _lib_endmntent 1
#define _lib_fs_stat_dev 0
#define _lib_fshelp 0
#define _lib_GetDiskFreeSpace 1
#define _lib_GetDiskFreeSpaceEx 1
#define _lib_GetDriveType 1
#define _lib_getfsstat 0
#define _lib_GetLogicalDriveStrings 1
#define _lib_GetVolumeInformation 1
#define _lib_getmnt 0
#define _lib_getmntent 0
#define _lib_getmntinfo 0
#define _lib_gettext 1
#define _lib_getvfsstat 0
#define _lib_getzoneid 0
#define _lib_hasmntopt 0
#define _lib_lstat 1
#define _lib_mbrlen 1
#define _lib_mcheck_pedantic 0
#define _lib_memcpy 1
#define _lib_memset 1
#define _lib_mntctl 0
#define _lib_next_dev 0
#define _lib_prop_dictionary_create 0
#define _lib_quota_open 0
#define _lib_quotactl 0
#define _lib_realpath 0
#define _lib_setlocale 1
#define _lib_setmntent 0
#define _lib_snprintf 1
#define _lib_statfs 0
#define _lib_statvfs 0
#define _lib_strcoll 1
#define _lib_strdup 1
#define _lib_strstr 1
#define _lib_sys_dollar_device_scan 0
#define _lib_sys_dollar_getdviw 0
#define _lib_sysfs 0
#define _lib_textdomain 1
#define _lib_vquotactl 0
#define _lib_xdr_int 0
#define _lib_zone_getattr 0
#define _lib_zone_list 0
#define _memberxdr_rquota_rq_bhardlimit 0
#define _memberxdr_rquota_rq_bsoftlimit 0
#define _memberxdr_rquota_rq_curblocks 0
#define _memberxdr_rquota_rq_fhardlimit 0
#define _memberxdr_rquota_rq_fsoftlimit 0
#define _memberxdr_rquota_rq_curfiles 0
#define _memberxdr_getquota_args_gqa_uid 0
#define _args_getfsstat 0
#define _args_getvfsstat 0
#define _c_arg_1_quotactl int
#define _c_arg_2_quotactl char *
#define _c_arg_3_quotactl int
#define _c_arg_4_quotactl caddr_t
#define _c_type_quotactl int
#define _args_quotactl 4
#define _quotactl_pos_1 0
#define _quotactl_pos_2 1
#define _c_arg_1_setmntent char *
#define _c_arg_2_setmntent char *
#define _c_type_setmntent FILE *
#define _args_setmntent 2
#define _c_arg_1_statfs char *
#define _c_arg_2_statfs struct statfs *
#define _c_type_statfs int
#define _args_statfs 2
#define _class_os__Volumes 0
#define _npt_getenv 0
#define _npt_getmnt 1
#define _npt_mntctl 1
#define _npt_quotactl 0
#define _npt_statfs 0
#define _dcl_errno 1
#define _dcl_mnt_names 0
#define _mem_struct_dqblk_dqb_curspace 0
#define _mem_struct_dqblk_dqb_curblocks 0
#define _mem_struct_dqblk_dqb_fhardlimit 0
#define _mem_struct_dqblk_dqb_fsoftlimit 0
#define _mem_struct_dqblk_dqb_curfiles 0
#define _mem_struct_getquota_rslt_gqr_status 0
#define _mem_struct_getquota_rslt_gqr_rquota 0
#define _mem_struct_mnttab_mt_mntopts 0
#define _mem_struct_statfs_f_bsize 0
#define _mem_struct_statfs_f_fsize 0
#define _mem_struct_statfs_f_fstyp 0
#define _mem_struct_statfs_f_iosize 0
#define _mem_struct_statfs_f_frsize 0
#define _mem_struct_statfs_f_fstypename 0
#define _mem_struct_statfs_mount_info 0
#define _mem_struct_statfs_f_type 0
#define _mem_struct_statvfs_f_basetype 0
#define _siz_long_long 8
#define _siz_long_double 16
#define _printf_long_double 1
#define _has_std_quotas 0
#define _has_std_nfs_quotas 0
#define _use_mcheck 0
#define _enable_nls 0
#define DI_DEFAULT_FORMAT "MbuvpT"
#define DI_DEFAULT_DISP_SIZE "H"

#ifndef __MKC_STANDARD_DEFS
# define __MKC_STANDARD_DEFS 1
# if ! _key_void
#  define void int
# endif
# if ! _key_void || ! _param_void_star
   typedef char *_pvoid;
# else
   typedef void *_pvoid;
# endif
# if ! _key_const
#  define const
# endif

# ifndef _
#  if _proto_stdc
#   define _(args) args
#  else
#   define _(args) ()
#  endif
# endif
#endif /* __MKC_STANDARD_DEFS */


#if _typ_statvfs_t
# define Statvfs_t statvfs_t
#else
# define Statvfs_t struct statvfs
#endif

#if _typ_size_t
# define Size_t size_t
#else
# define Size_t unsigned int
#endif

#if _typ_uint_t
# define Uint_t uint_t
#else
# define Uint_t unsigned int
#endif

#if _typ_uid_t
# define Uid_t uid_t
#else
# define Uid_t int
#endif

#if _typ_gid_t
# define Gid_t gid_t
#else
# define Gid_t int
#endif

/* Do this the old-fashioned way for old compilers */
/* Have to work around MacOSX's snprintf macro.    */
#if _lib_snprintf
# define Snprintf1 snprintf
# define Snprintf2 snprintf
# define Snprintf3 snprintf
# define Snprintf4 snprintf
#else
# define Snprintf1(a1,a2,a3,a4) sprintf(a1,a3,a4)
# define Snprintf2(a1,a2,a3,a4,a5) sprintf(a1,a3,a4,a5)
# define Snprintf3(a1,a2,a3,a4,a5,a6) sprintf(a1,a3,a4,a5,a6)
# define Snprintf4(a1,a2,a3,a4,a5,a6,a7) sprintf(a1,a3,a4,a5,a6,a7)
#endif

#if ! _lib_strcoll
# define strcoll strcmp
#endif


#endif /* __INC_CONFIG_H */
