#ifndef __INC_CONFIG_H
#define __INC_CONFIG_H 1

#define _hdr_stdio 1
#define _hdr_stdlib 0
#define _sys_types 1
#define _sys_param 1
#define _key_void 1
#define _key_const 0
#define _param_void_star 0
#define _proto_stdc 0
#define _hdr_ctype 1
#define _hdr_dcdef 0
#define _hdr_descrip 0
#define _hdr_dirent 0
#define _hdr_dvidef 0
#define _hdr_dvsdef 0
#define _hdr_errno 1
#define _hdr_fcntl 1
#define _hdr_fshelp 0
#define _hdr_getopt 0
#define _hdr_gui_window 0
#define _hdr_kernel_fs_info 0
#define _hdr_limits 0
#define _hdr_linux_dqblk_xfs 0
#define _hdr_linux_quota 0
#define _hdr_libintl 0
#define _hdr_locale 0
#define _hdr_malloc 0
#define _hdr_memory 1
#define _hdr_mntent 1
#define _hdr_mnttab 0
#define _hdr_rpc_rpc 1
#define _hdr_rpcsvc_rquota 0
#define _hdr_ssdef 0
#define _hdr_starlet 0
#define _hdr_storage_Directory 0
#define _hdr_storage_Entry 0
#define _hdr_storage_Path 0
#define _hdr_storage_volumes 0
#define _hdr_string 1
#define _hdr_strings 1
#define _hdr_time 1
#define _hdr_ufs_quota 1
#define _hdr_ufs_ufs_quota 0
#define _hdr_unistd 0
#define _hdr_util_string 0
#define _hdr_wchar 0
#define _hdr_windows 0
#define _hdr_zone 0
#define _sys_dcmd_blk 0
#define _sys_file 1
#define _sys_fs_types 0
#define _sys_fs_ufs_quota 0
#define _sys_fstyp 0
#define _sys_fstypes 0
#define _sys_ftype 0
#define _sys_io 0
#define _sys_mntctl 0
#define _sys_mntent 0
#define _sys_mnttab 0
#define _sys_mount 1
#define _sys_quota 0
#define _sys_stat 1
#define _sys_statfs 0
#define _sys_statvfs 0
#define _sys_time 1
#define _sys_vfs 1
#define _sys_vfstab 0
#define _sys_vmount 0
#define _inc_conflict__hdr_time__sys_time 0
#define _inc_conflict__sys_quota__hdr_linux_quota 1
#define _command_msgfmt 0
#define _command_gmsgfmt 0
#define _command_rpmbuild 0
#define _const_O_NOCTTY 0
#define _define_MCTL_QUERY 0
#define _typ_struct_dqblk 1
#define _typ_fs_disk_quota_t 0
#define _typ_gid_t 1
#define _typ_statvfs_t 0
#define _typ_size_t 1
#define _typ_uint_t 0
#define _typ_uid_t 1
#define _lib_bcopy 1
#define _lib_bindtextdomain 0
#define _lib_bzero 1
#define _lib_endmntent 1
#define _lib_fs_stat_dev 0
#define _lib_fshelp 0
#define _lib_GetDiskFreeSpace 0
#define _lib_GetDiskFreeSpaceEx 0
#define _lib_GetDriveType 0
#define _lib_getfsstat 0
#define _lib_GetLogicalDriveStrings 0
#define _lib_GetVolumeInformation 0
#define _lib_getmnt 0
#define _lib_getmntent 1
#define _lib_getmntinfo 0
#define _lib_getopt 1
#define _lib_gettext 0
#define _lib_getvfsstat 0
#define _lib_getzoneid 0
#define _lib_hasmntopt 1
#define _lib_mbrlen 0
#define _lib_memcpy 1
#define _lib_memset 1
#define _lib_mntctl 0
#define _lib_next_dev 0
#define _lib_quotactl 1
#define _lib_setlocale 0
#define _lib_setmntent 1
#define _lib_snprintf 0
#define _lib_statfs 1
#define _lib_statvfs 0
#define _lib_strcoll 0
#define _lib_strdup 0
#define _lib_strstr 0
#define _lib_sys_dollar_device_scan 0
#define _lib_sys_dollar_getdviw 0
#define _lib_sysfs 0
#define _lib_textdomain 0
#define _lib_xdr_int 1
#define _lib_zone_getattr 0
#define _lib_zone_list 0
#define _quotactl_pos 2
#define _setmntent_args 2
#define _statfs_args 2
#define _class_os__Volumes 0
#define _npt_getenv 1
#define _npt_getopt 1
#define _npt_statfs 1
#define _dcl_errno 0
#define _dcl_mnt_names 0
#define _dcl_optind 0
#define _dcl_optarg 0
#define _mem_struct_dqblk_dqb_curspace 0
#define _mem_struct_dqblk_dqb_curblocks 1
#define _mem_struct_dqblk_dqb_fhardlimit 1
#define _mem_struct_dqblk_dqb_fsoftlimit 1
#define _mem_struct_dqblk_dqb_curfiles 1
#define _mem_struct_getquota_rslt_gqr_status 0
#define _mem_struct_getquota_rslt_gqr_rquota 0
#define _mem_struct_statfs_f_bsize 1
#define _mem_struct_statfs_f_fsize 0
#define _mem_struct_statfs_f_fstyp 0
#define _mem_struct_statfs_f_iosize 0
#define _mem_struct_statfs_f_frsize 0
#define _mem_struct_statfs_f_fstypename 0
#define _mem_struct_statfs_mount_info 0
#define _mem_struct_statfs_f_type 1
#define _mem_struct_statvfs_f_basetype 0
#define _siz_long_long 0
#define DI_DEFAULT_FORMAT "smbuvpT"
#define DI_DEFAULT_DISP_SIZE "H"
#define _has_std_quotas 1
#define _enable_nls 0

#if ! _key_void
# define void int
#endif
#if ! _key_void || ! _param_void_star
  typedef char *_pvoid;
#else
  typedef void *_pvoid;
#endif
#if ! _key_const
# define const
#endif

#ifndef _
# if _proto_stdc
#  define _(args) args
# else
#  define _(args) ()
# endif
#endif

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

#if _lib_snprintf
# define Snprintf snprintf
# define DI_SPF(a2,a3)         a2,a3
#else
# define Snprintf sprintf
# define DI_SPF(a2,a3)         a3
#endif

#if ! _lib_strcoll
# define strcoll strcmp
#endif


#endif /* __INC_CONFIG_H */
