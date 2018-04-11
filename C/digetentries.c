/*
 * $Id$
 * $Source$
 * Copyright 1994-2011 Brad Lanam, Walnut Creek, CA
 */

/********************************************************/
/*

    di_getDiskEntries ()
        Get a list of mounted filesystems.
        In many cases, this also does the work of di_getDiskInfo ().

*/
/********************************************************/

#include "config.h"
#include "di.h"
#include "dimntopt.h"

#if _hdr_stdio
# include <stdio.h>
#endif
#if _hdr_stdlib
# include <stdlib.h>
#endif
#if _hdr_dirent
# include <dirent.h>
#endif
#if _sys_types \
    && ! defined (DI_INC_SYS_TYPES_H) /* xenix */
# define DI_INC_SYS_TYPES_H
# include <sys/types.h>
#endif
#if _sys_param
# include <sys/param.h>
#endif
#if _sys_ftype                      /* QNX */
# include <sys/ftype.h>
#endif
#if _sys_dcmd_blk                   /* QNX */
# include <sys/dcmd_blk.h>
#endif
#if _sys_io                         /* QNX */
# include <sys/io.h>
#endif
#if _hdr_errno
# include <errno.h>
#endif
#if _hdr_string
# include <string.h>
#endif
#if _hdr_strings
# include <strings.h>
#endif
#if _hdr_memory
# include <memory.h>
#endif
#if _hdr_malloc
# include <malloc.h>
#endif

#if _hdr_mntent \
  && ! defined (DI_INC_MNTENT)        /* Linux, kFreeBSD, HP-UX */
# define DI_INC_MNTENT 1
# include <mntent.h>            /* hasmntopt(); _PATH_MNTTAB */
#endif                          /* HP-UX: set/get/endmntent(); hasmntopt() */

#if _sys_mount \
  && ! defined (DI_INC_SYS_MOUNT) /* FreeBSD, OpenBSD, NetBSD, HP-UX */
# define DI_INC_SYS_MOUNT 1
# include <sys/mount.h>         /* getmntinfo(); struct statfs */
#endif
#if _sys_fstypes                /* NetBSD */
# include <sys/fstypes.h>
#endif
#if _sys_fs_types               /* OSF/1, AROS */
# include <sys/fs_types.h>
#endif
#if _sys_mnttab                 /* Solaris, SCO_SV, UnixWare */
# include <sys/mnttab.h>        /* getmntent(); MNTTAB */
#endif

#if _sys_statfs && ! _sys_statvfs /* Linux, FreeBSD, SysV.3 */
# include <sys/statfs.h>                    /* struct statfs; statfs() */
#endif
#if _sys_statvfs                    /* NetBSD, Solaris */
# include <sys/statvfs.h>           /* struct statvfs; statvfs() */
#endif
#if _sys_vfs                    /* BSD 4.3 */
# include <sys/vfs.h>           /* struct statfs */
#endif
#if _sys_mntctl                     /* AIX */
# include <sys/mntctl.h>
#endif
#if _sys_vmount                     /* AIX */
# include <sys/vmount.h>
#endif
#if _hdr_fshelp                     /* AIX */
# include <fshelp.h>
#endif
#if _hdr_windows                    /* windows */
# include <windows.h>
#endif
#if _hdr_winioctl                   /* windows */
# include <winioctl.h>
#endif
#if _hdr_kernel_fs_info             /* BeOS */
# include <kernel/fs_info.h>
#endif
#if _hdr_storage_Directory          /* BeOS */
# include <storage/Directory.h>
#endif
#if _hdr_storage_Entry              /* BeOS */
# include <storage/Entry.h>
#endif
#if _hdr_storage_Path               /* BeOS */
# include <storage/Path.h>
#endif
 /* bozo syllable volumes header requires gui/window */
#if _hdr_gui_window                 /* Syllable */
# include <gui/window.h>            /* gack! */
#endif
#if _hdr_storage_volumes            /* Syllable */
# include <storage/volumes.h>       /* class os::Volumes */
#endif
#if _hdr_util_string                /* Syllable */
# include <util/string.h>           /* os::String - to get mount name */
#endif
#if _hdr_starlet                    /* VMS */
# include <starlet.h>
#endif
#if _hdr_descrip                    /* VMS */
# include <descrip.h>
#endif
#if _hdr_dcdef                      /* VMS */
# include <dcdef.h>
#endif
#if _hdr_dvsdef                     /* VMS */
# include <dvsdef.h>
#endif
#if _hdr_ssdef                      /* VMS */
# include <ssdef.h>
#endif
#if _hdr_dvidef                     /* VMS */
# include <dvidef.h>
#endif


/********************************************************/

#if defined (__cplusplus) || defined (c_plusplus)
  extern "C" {
#endif

/* workaround for AIX - mntctl not declared */
# if _lib_mntctl && _npt_mntctl
  extern int mntctl _((int, Size_t, char *));
# endif

#if (_lib_getmntent \
    || _args_statfs > 0) \
    && ! _lib_getmntinfo \
    && ! _lib_getfsstat \
    && ! _lib_getvfsstat \
    && ! _lib_mntctl \
    && ! _lib_getmnt \
    && ! _class_os__Volumes
# if defined (_PATH_MOUNTED)
#  define DI_MOUNT_FILE        _PATH_MOUNTED
# else
#  if defined (_PATH_MNTTAB)
#   define DI_MOUNT_FILE        _PATH_MNTTAB
#  else
#   if defined (MOUNTED)
#    define DI_MOUNT_FILE       MOUNTED
#   else
#    if defined (MNTTAB)
#     define DI_MOUNT_FILE      MNTTAB
#    else
#     if (USE_ETC_FILESYSTEMS)
#      define DI_MOUNT_FILE     "/etc/filesystems" /* AIX 4.x or /etc/mntent? */
#     else
#      define DI_MOUNT_FILE     "/etc/mnttab"      /* SysV.3 default */
#     endif
#    endif
#   endif
#  endif
# endif
#endif

#if defined (__QNX__)
static int di_getQNXDiskEntries _((char *ipath, diDiskInfo_t **diskInfo, int *diCount));
#endif
extern int debug;

#if defined (__cplusplus) || defined (c_plusplus)
  }
#endif

/********************************************************/

#if _lib_getmntent \
    && ! _lib_setmntent \
    && ! _lib_mntctl \
    && ! _class_os__Volumes

#if defined (__cplusplus) || defined (c_plusplus)
  extern "C" {
#endif
static char *checkMountOptions      _((struct mnttab *, char *));
#if defined (__cplusplus) || defined (c_plusplus)
  }
#endif

/*
 * di_getDiskEntries
 *
 * For SysV.4, we open the file and call getmntent () repeatedly.
 *
 */

int
# if _proto_stdc
di_getDiskEntries (diDiskInfo_t **diskInfo, int *diCount)
# else
di_getDiskEntries (diskInfo, diCount)
    diDiskInfo_t **diskInfo;
    int *diCount;
# endif
{
    diDiskInfo_t     *diptr;
    FILE            *f;
    int             idx;
    struct mnttab   mntEntry;
    char            *devp;   /* local ptr to dev entry */


    if (debug > 0) { printf ("# di_getDiskEntries: getmntent\n"); }
    if ((f = fopen (DI_MOUNT_FILE, "r")) == (FILE *) NULL)
    {
        fprintf (stderr, "Unable to open: %s errno %d\n", DI_MOUNT_FILE, errno);
        return -1;
    }

    while (getmntent (f, &mntEntry) == 0)
    {
        idx = *diCount;
        ++*diCount;
        *diskInfo = (diDiskInfo_t *) di_realloc ((char *) *diskInfo,
                sizeof (diDiskInfo_t) * (Size_t) *diCount);
        if (*diskInfo == (diDiskInfo_t *) NULL) {
          fprintf (stderr, "malloc failed for diskInfo. errno %d\n", errno);
          return -1;
        }
        diptr = *diskInfo + idx;
        di_initDiskInfo (diptr);

        strncpy (diptr->special, mntEntry.mnt_special, DI_SPEC_NAME_LEN);
        strncpy (diptr->name, mntEntry.mnt_mountp, DI_NAME_LEN);
        if (checkMountOptions (&mntEntry, DI_MNTOPT_IGNORE) != (char *) NULL)
        {
            diptr->printFlag = DI_PRNT_IGNORE;
            if (debug > 2)
            {
                printf ("mnt: ignore: mntopt 'ignore': %s\n",
                        diptr->name);
            }
        }
        if ((devp = checkMountOptions (&mntEntry, DI_MNTOPT_DEV)) !=
                (char *) NULL)
        {
            if (devp != mntEntry.mnt_mntopts)
            {
                --devp;
            }
            *devp = 0;   /* point to preceeding comma and cut off */
        }
        if (checkMountOptions (&mntEntry, DI_MNTOPT_RO) != (char *) NULL)
        {
            diptr->isReadOnly = TRUE;
        }
        strncpy (diptr->options, mntEntry.mnt_mntopts, DI_OPT_LEN);

            /* get the file system type now... */
        strncpy (diptr->fsType, mntEntry.mnt_fstype, DI_TYPE_LEN);
        if (debug > 2)
        {
            printf ("mnt:%s - %s\n", diptr->name, diptr->fsType);
        }

        if (debug > 1)
        {
            printf ("mnt:%s - %s\n", diptr->name, diptr->special);
        }
    }

    fclose (f);
    return 0;
}

static char *
# if _proto_stdc
checkMountOptions (struct mnttab *mntEntry, char *str)
# else
checkMountOptions (mntEntry, str)
    struct mnttab *mntEntry;
    char          *str;
# endif
{
# if _lib_hasmntopt
    return hasmntopt (mntEntry, str);
# else
    return chkMountOptions (mntEntry->mnt_mntopts, str);
# endif
}

#endif

#if _lib_getmntent \
    && _lib_setmntent \
    && _lib_endmntent \
    && ! _lib_getmntinfo \
    && ! _lib_getfsstat \
    && ! _lib_getvfsstat \
    && ! _lib_mntctl \
    && ! _lib_GetDriveType \
    && ! _lib_GetLogicalDriveStrings \
    && ! _class_os__Volumes

/*
 * di_getDiskEntries
 *
 * SunOS supplies an open and close routine for the mount table.
 *
 */

#if ! defined (MNTTYPE_IGNORE)
# define MNTTYPE_IGNORE "ignore"
#endif

int
# if _proto_stdc
di_getDiskEntries (diDiskInfo_t **diskInfo, int *diCount)
# else
di_getDiskEntries (diskInfo, diCount)
    diDiskInfo_t **diskInfo;
    int *diCount;
# endif
{
    diDiskInfo_t    *diptr;
    FILE            *f;
    int             idx;
    struct mntent   *mntEntry;
    char            *devp;   /* local ptr to dev entry */


    if (debug > 0) { printf ("# getDiskEntries: set/get/endmntent\n"); }
/* if both are set not an ansi compiler... */
#if _args_setmntent == 1
    if ((f = setmntent (DI_MOUNT_FILE)) == (FILE *) NULL)
#else
    if ((f = setmntent (DI_MOUNT_FILE, "r")) == (FILE *) NULL)
#endif
    {
        fprintf (stderr, "Unable to open: %s errno %d\n", DI_MOUNT_FILE, errno);
        return -1;
    }

    while ((mntEntry = getmntent (f)) != (struct mntent *) NULL)
    {
        idx = *diCount;
        ++*diCount;
        *diskInfo = (diDiskInfo_t *) di_realloc ((char *) *diskInfo,
                sizeof (diDiskInfo_t) * (Size_t) *diCount);
        if (*diskInfo == (diDiskInfo_t *) NULL) {
          fprintf (stderr, "malloc failed for diskInfo. errno %d\n", errno);
          return -1;
        }
        diptr = *diskInfo + idx;
        di_initDiskInfo (diptr);

        strncpy (diptr->special, mntEntry->mnt_fsname,
            (Size_t) DI_SPEC_NAME_LEN);
        strncpy (diptr->name, mntEntry->mnt_dir, (Size_t) DI_NAME_LEN);
        strncpy (diptr->fsType, mntEntry->mnt_type, (Size_t) DI_TYPE_LEN);

        if (strcmp (mntEntry->mnt_fsname, "none") == 0) {
          diptr->printFlag = DI_PRNT_IGNORE;
          if (debug > 2) {
            printf ("mnt: ignore: special 'none': %s\n", diptr->name);
          }
        }

        if (strcmp (mntEntry->mnt_type, MNTTYPE_IGNORE) == 0)
        {
            diptr->printFlag = DI_PRNT_IGNORE;
            if (debug > 2)
            {
                printf ("mnt: ignore: mntopt 'ignore': %s\n",
                        diptr->name);
            }
        }

        if ((devp = strstr (mntEntry->mnt_opts, "dev=")) != (char *) NULL)
        {
            if (devp != mntEntry->mnt_opts)
            {
                --devp;
            }
            *devp = 0;   /* point to preceeding comma and cut off */
        }
        if (chkMountOptions (mntEntry->mnt_opts, DI_MNTOPT_RO) != (char *) NULL)
        {
            diptr->isReadOnly = TRUE;
        }
        strncpy (diptr->options, mntEntry->mnt_opts, (Size_t) DI_OPT_LEN);

        if (debug > 1)
        {
            printf ("mnt:%s - %s : %s\n", diptr->name,
                    diptr->special, diptr->fsType);
        }
    }

    endmntent (f);
    return 0;
}

#endif /* _lib_getmntent && _lib_setmntent && _lib_endmntent */

/* QNX */
#if ! _lib_getmntent \
    && ! _lib_mntctl \
    && ! _lib_getmntinfo \
    && ! _lib_getfsstat \
    && ! _lib_getvfsstat \
    && ! _lib_getmnt \
    && ! _lib_GetDriveType \
    && ! _lib_GetLogicalDriveStrings \
    && ! _lib_fs_stat_dev \
    && ! _class_os__Volumes \
    && ! _lib_sys_dollar_device_scan \
    && defined (__QNX__)

/*
 * di_getDiskEntries
 *
 * QNX
 *
 * This is bloody slow.
 * It would be nice to have a way to short-circuit some of
 * the directory subtrees.
 * /proc/mount/dev is not processed...hopefully that won't affect much.
 *
 */

int
# if _proto_stdc
di_getDiskEntries (diDiskInfo_t **diskInfo, int *diCount)
# else
di_getDiskEntries (diskInfo, diCount)
    diDiskInfo_t **diskInfo;
    int *diCount;
# endif
{
    if (debug > 0) { printf ("# getDiskEntries: QNX\n"); }
    return di_getQNXDiskEntries ("/proc/mount", diskInfo, diCount);
}

static int
# if _proto_stdc
di_getQNXDiskEntries (char *ipath, diDiskInfo_t **diskInfo, int *diCount)
# else
di_getQNXDiskEntries (ipath, diskInfo, diCount)
    char *ipath;
    diDiskInfo_t **diskInfo;
    int *diCount;
# endif
{
    diDiskInfo_t    *diptr;
    int             idx;
    char            path [MAXPATHLEN + 1];
    int             len;   /* current length of path */
    DIR             *dirp;
    struct dirent   *dent;
    int             ret;
    int             nodeid;
    int             pid;
    int             chid;
    int             handle;
    int             ftype;
    struct stat     statinfo;
    int             fd;
    char            tspecial [DI_SPEC_NAME_LEN];

    if (strcmp (ipath, "/proc/mount/dev") == 0) {
      return 0;
    }

    strncpy (path, ipath, MAXPATHLEN);
    len = strlen (path);

    if (!(dirp = opendir(path))) {
      return 0;
    }
    while ((dent = readdir(dirp))) {
      if (strcmp (dent->d_name, ".") == 0 || strcmp (dent->d_name, "..") == 0) {
        continue;
      }

      path[len] = '\0';
      ret = sscanf(dent->d_name, "%d,%d,%d,%d,%d",
          &nodeid, &pid, &chid, &handle, &ftype);

      if (len + strlen(dent->d_name) + 1 > MAXPATHLEN) {
        continue;
      }

      path[len] = '/';
      strncpy (&path[len+1], dent->d_name, MAXPATHLEN - len - 1);
      if (debug > 4) { printf ("check: %s\n", path); }

      memset(&statinfo, 0, sizeof(statinfo));

      if (stat(path, &statinfo) == -1) {
        continue;
      }

      if (ret != 5) {
        if (S_ISDIR (statinfo.st_mode)) {
          if (debug > 4) { printf ("into: %s\n", path); }
          di_getQNXDiskEntries (path, diskInfo, diCount);
        }
        continue;
      }

      if (ftype != _FTYPE_ANY) {
        continue;
      }

      *tspecial = '\0';
      if (S_ISDIR(statinfo.st_mode) && ftype == _FTYPE_ANY) {
        if ((fd = open (path, /* O_ACCMODE */ O_RDONLY | O_NOCTTY)) != -1) {
          devctl (fd, DCMD_FSYS_MOUNTED_ON, tspecial, DI_SPEC_NAME_LEN, 0);
          close (fd);
          if (*tspecial == '\0') {
            /* unfortunately, this cuts out /proc, /dev/sem, etc. */
            /* but it also removes strange duplicate stuff        */
            continue;
          }
        } else {
          continue;
        }
      } else {
        continue;
      }

      idx = *diCount;
      ++*diCount;
      *diskInfo = (diDiskInfo_t *) di_realloc ((char *) *diskInfo,
              sizeof (diDiskInfo_t) * (Size_t) *diCount);
      if (*diskInfo == (diDiskInfo_t *) NULL) {
        fprintf (stderr, "malloc failed for diskInfo. errno %d\n", errno);
        return -1;
      }
      diptr = *diskInfo + idx;
      di_initDiskInfo (diptr);

      path[len] = '\0';
      strncpy (diptr->special, tspecial, DI_SPEC_NAME_LEN);
      strncpy (diptr->name, path + 11, DI_NAME_LEN);
      if (*diptr->name == '\0') {
        strncpy (diptr->name, "/", DI_NAME_LEN);
      }
      if (debug > 4) { printf ("found: %s %s\n", diptr->special, diptr->name); }
    }

    closedir (dirp);
    return 0;
}

#endif /* QNX */

/* if nothing matches, assume a SysV.3 /etc/mnttab or similar */
#if ! _lib_getmntent \
    && ! _lib_mntctl \
    && ! _lib_getmntinfo \
    && ! _lib_getfsstat \
    && ! _lib_getvfsstat \
    && ! _lib_getmnt \
    && ! _lib_GetDriveType \
    && ! _lib_GetLogicalDriveStrings \
    && ! _lib_fs_stat_dev \
    && ! _class_os__Volumes \
    && ! _lib_sys_dollar_device_scan \
    && ! defined (__QNX__)

/*
 * di_getDiskEntries
 *
 * For SysV.3 we open the file and read it ourselves.
 *
 */

int
# if _proto_stdc
di_getDiskEntries (diDiskInfo_t **diskInfo, int *diCount)
# else
di_getDiskEntries (diskInfo, diCount)
    diDiskInfo_t **diskInfo;
    int *diCount;
# endif
{
    diDiskInfo_t    *diptr;
    FILE             *f;
    int              idx;
    struct mnttab    mntEntry;


    if (debug > 0) { printf ("# getDiskEntries: not anything; sys v.3\n"); }
    if ((f = fopen (DI_MOUNT_FILE, "r")) == (FILE *) NULL)
    {
        fprintf (stderr, "Unable to open: %s errno %d\n", DI_MOUNT_FILE, errno);
        return -1;
    }

    while (fread ((char *) &mntEntry, sizeof (struct mnttab), 1, f) == 1)
    {
            /* xenix allows null mount table entries */
            /* sco nfs background mounts are marked as "nothing" */
        if (mntEntry.mt_filsys [0] &&
                strcmp (mntEntry.mt_filsys, "nothing") != 0)
        {
            idx = *diCount;
            ++*diCount;
            *diskInfo = (diDiskInfo_t *) di_realloc ((char *) *diskInfo,
                    sizeof (diDiskInfo_t) * *diCount);
            if (*diskInfo == (diDiskInfo_t *) NULL) {
              fprintf (stderr, "malloc failed for diskInfo. errno %d\n", errno);
              return -1;
            }
            diptr = *diskInfo + idx;
            di_initDiskInfo (diptr);

# if defined (COHERENT)
                /* Coherent seems to have these fields reversed. oh well. */
            strncpy (diptr->name, mntEntry.mt_dev, DI_NAME_LEN);
            strncpy (diptr->special, mntEntry.mt_filsys, DI_SPEC_NAME_LEN);
# else
            strncpy (diptr->special, mntEntry.mt_dev, DI_SPEC_NAME_LEN);
            strncpy (diptr->name, mntEntry.mt_filsys, DI_NAME_LEN);
# endif
# if _mem_struct_mnttab_mntopts
            strncpy (diptr->options, mntEntry.mt_mntopts, DI_OPT_LEN);
# endif
        }

        if (debug > 1)
        {
            printf ("mnt:%s - %s\n", diptr->name,
                    diptr->special);
        }
    }

    fclose (f);
    return 0;
}

#endif /* Sys V.3 */

/*
 * All of the following routines also replace di_getDiskInfo()
 */


#if _lib_getfsstat \
    && ! (_lib_getvfsstat && _args_getvfsstat == 3)

/*
 * di_getDiskEntries
 *
 * OSF/1 / Digital Unix / Compaq Tru64 / FreeBSD / NetBSD 2.x / OpenBSD
 *
 */

# if _dcl_mnt_names
#  if ! defined (MNT_NUMTYPES)
#   define MNT_NUMTYPES (sizeof(mnt_names)/sizeof(char *))
#  endif
# endif

int
# if _proto_stdc
di_getDiskEntries (diDiskInfo_t **diskInfo, int *diCount)
# else
di_getDiskEntries (diskInfo, diCount)
    diDiskInfo_t **diskInfo;
    int *diCount;
# endif
{
    diDiskInfo_t     *diptr;
    int             count;
    int             idx;
# if _dcl_mnt_names && _mem_struct_statfs_f_type
    short           fstype;
# endif
    _c_arg_2_getfsstat bufsize;
    struct statfs   *mntbufp;
    struct statfs   *sp;

    if (debug > 0) { printf ("# getDiskEntries: getfsstat\n"); }
    count = getfsstat ((struct statfs *) NULL, (_c_arg_2_getfsstat) 0, MNT_NOWAIT);
    if (count < 1)
    {
        fprintf (stderr, "Unable to do getfsstat () errno %d\n", errno);
        return -1;
    }
    bufsize = (_c_arg_2_getfsstat) (sizeof (struct statfs) * (Size_t) count);
    mntbufp = malloc ((Size_t) bufsize);
    if (mntbufp == (struct statfs *) NULL) {
      fprintf (stderr, "malloc failed for mntbufp. errno %d\n", errno);
      return -1;
    }
    memset ((char *) mntbufp, '\0', sizeof (struct statfs) * (Size_t) count);
    count = getfsstat (mntbufp, bufsize, MNT_NOWAIT);

    *diCount = count;
    *diskInfo = (diDiskInfo_t *) malloc (sizeof (diDiskInfo_t) * (Size_t) count);
    if (*diskInfo == (diDiskInfo_t *) NULL)
    {
        fprintf (stderr, "malloc failed for diskInfo. errno %d\n", errno);
        return -1;
    }
    memset ((char *) *diskInfo, '\0', sizeof (diDiskInfo_t) * (Size_t) count);

    for (idx = 0; idx < count; idx++)
    {
        _fs_size_t          tblocksz;

        diptr = *diskInfo + idx;
        di_initDiskInfo (diptr);

        sp = mntbufp + idx;
# if defined (MNT_RDONLY)
        if ((sp->f_flags & MNT_RDONLY) == MNT_RDONLY)
        {
            diptr->isReadOnly = TRUE;
        }
# endif
# if defined (MNT_LOCAL)
        if ((sp->f_flags & MNT_LOCAL) != MNT_LOCAL)
        {
            diptr->isLocal = FALSE;
        }
# endif
        convertMountOptions ((unsigned long) sp->f_flags, diptr);
# if _mem_struct_statfs_f_type
#  if defined (MOUNT_NFS3)
        if (sp->f_type == MOUNT_NFS3)
        {
            strncat (diptr->options, "v3,",
                    DI_OPT_LEN - strlen (diptr->options) - 1);
        }
#  endif
# endif
# if _mem_struct_statfs_mount_info \
        && defined (MOUNT_NFS) \
        && (_mem_struct_statfs_f_type || _mem_struct_statfs_f_fstypename)
#  if _mem_struct_statfs_f_type
        if (sp->f_type == MOUNT_NFS
#  endif
#  if _mem_struct_statfs_f_fstypename
        if (strcmp (sp->f_fstypename, MOUNT_NFS) == 0
#  endif
#  if _mem_struct_statfs_f_fstypename && defined (MOUNT_NFS3)
                || strcmp (sp->f_fstypename, MOUNT_NFS3) == 0
#  endif
#  if _mem_struct_statfs_f_type && defined (MOUNT_NFS3)
                || sp->f_type == MOUNT_NFS3
#  endif
           )
        {
            struct nfs_args *na;
            na = &sp->mount_info.nfs_args;
            convertNFSMountOptions (na->flags, na->wsize, na->rsize, diptr);
        }
# endif
        trimChar (diptr->options, ',');

        strncpy (diptr->special, sp->f_mntfromname, (Size_t) DI_SPEC_NAME_LEN);
        strncpy (diptr->name, sp->f_mntonname, (Size_t) DI_NAME_LEN);
# if _mem_struct_statfs_f_fsize
        tblocksz = (_fs_size_t) sp->f_fsize;
# endif
# if _mem_struct_statfs_f_bsize && ! _mem_struct_statfs_f_fsize
        tblocksz = (_fs_size_t) sp->f_bsize;
# endif
        di_saveBlockSizes (diptr, tblocksz,
            (_fs_size_t) sp->f_blocks,
            (_fs_size_t) sp->f_bfree,
            (_fs_size_t) sp->f_bavail);
        di_saveInodeSizes (diptr,
            (_fs_size_t) sp->f_files,
            (_fs_size_t) sp->f_ffree,
            (_fs_size_t) sp->f_ffree);
# if _mem_struct_statfs_f_fstypename
        strncpy (diptr->fsType, sp->f_fstypename, (Size_t) DI_TYPE_LEN);
# else
#  if _lib_sysfs && _mem_struct_statfs_f_type
        sysfs (GETFSTYP, sp->f_type, diptr->fsType);
#  else
#   if _dcl_mnt_names && _mem_struct_statfs_f_type
#    define DI_UNKNOWN_FSTYPE       "(%.2d)?"
        fstype = sp->f_type;
        if ((fstype >= 0) && (fstype < MNT_NUMTYPES))
        {
            strncpy (diptr->fsType, mnt_names [fstype], DI_TYPE_LEN);
        }
        else
        {
            Snprintf1 (diptr->fsType, sizeof (diptr->fsType),
                      DI_UNKNOWN_FSTYPE, fstype);
        }
#   endif
#  endif
# endif
    }

    free ((char *) mntbufp);
    return 0;
}

#endif /* _lib_getfsstat */

#if _lib_getmntinfo \
    && ! _lib_getfsstat \
    && ! _lib_getvfsstat

/*
 * di_getDiskEntries
 *
 * Old OSF/1 system call.
 * OSF/1 does this with a system call and library routine.
 *                  [mogul@wrl.dec.com (Jeffrey Mogul)]
 *
 */

# if defined (INITMOUNTNAMES) && ! _dcl_mnt_names
 static char *mnt_names [] = INITMOUNTNAMES;
#  if ! defined (MNT_NUMTYPES)
#   define MNT_NUMTYPES (MOUNT_MAXTYPE + 1)
#  endif
# endif

int
# if _proto_stdc
di_getDiskEntries (diDiskInfo_t **diskInfo, int *diCount)
# else
di_getDiskEntries (diskInfo, diCount)
    diDiskInfo_t **diskInfo;
    int *diCount;
# endif
{
    diDiskInfo_t     *diptr;
    int             count;
    int             idx;
    short           fstype;
    struct statfs   *mntbufp;

    if (debug > 0) { printf ("# getDiskEntries: getmntinfo\n"); }
    count = getmntinfo (&mntbufp, MNT_WAIT);
    if (count < 1)
    {
        fprintf (stderr, "Unable to do getmntinfo() errno %d\n", errno);
        return -1;
    }

    *diCount = count;
    *diskInfo = (diDiskInfo_t *) malloc (sizeof (diDiskInfo_t) * (Size_t) count);
    if (*diskInfo == (diDiskInfo_t *) NULL)
    {
        fprintf (stderr, "malloc failed for diskInfo. errno %d\n", errno);
        return -1;
    }
    memset ((char *) *diskInfo, '\0', sizeof (diDiskInfo_t) * (Size_t) count);

    if (debug > 1)
    {
        printf ("type_len %d name_len %d spec_name_len %d\n", DI_TYPE_LEN,
                DI_NAME_LEN, DI_SPEC_NAME_LEN);
    }

    for (idx = 0; idx < count; idx++)
    {
        _fs_size_t      tblocksz;

        diptr = *diskInfo + idx;
        di_initDiskInfo (diptr);
# if defined (MNT_LOCAL)
        if ((mntbufp [idx].f_flags & MNT_LOCAL) != MNT_LOCAL)
        {
            diptr->isLocal = FALSE;
        }
# endif

        strncpy (diptr->special, mntbufp [idx].f_mntfromname,
                DI_SPEC_NAME_LEN);
        strncpy (diptr->name, mntbufp [idx].f_mntonname, DI_NAME_LEN);

        tblocksz = (_fs_size_t) 1024;

# if _mem_struct_statfs_f_fsize /* OSF 1.x */
        tblocksz = (_fs_size_t) mntbufp [idx].f_fsize;
# endif
# if _mem_struct_statfs_f_bsize /* OSF 2.x */
        tblocksz = (_fs_size_t) mntbufp [idx].f_bsize;
# endif
        di_saveBlockSizes (diptr, tblocksz,
            (_fs_size_t) mntbufp [idx].f_blocks,
            (_fs_size_t) mntbufp [idx].f_bfree,
            (_fs_size_t) mntbufp [idx].f_bavail);
        di_saveInodeSizes (diptr,
            (_fs_size_t) mntbufp [idx].f_files,
            (_fs_size_t) mntbufp [idx].f_ffree,
            (_fs_size_t) mntbufp [idx].f_ffree);

        fstype = mntbufp [idx].f_type;
# if ! _sys_fs_types && ! defined (INITMOUNTNAMES) \
    && ! _mem_struct_statfs_f_fstypename
        if ((fstype >= 0) && (fstype <= MOUNT_MAXTYPE))
        {
            switch (fstype)
            {
#  if defined (MOUNT_NONE)
                case MOUNT_NONE:         /* No Filesystem */
                {
                    strncpy (diptr->fsType, "none", DI_TYPE_LEN);
                    break;
                }
#  endif
#  if defined (MOUNT_UFS)
                case MOUNT_UFS:         /* UNIX "Fast" Filesystem */
                {
                    strncpy (diptr->fsType, "ufs", DI_TYPE_LEN);
                    break;
                }
#  endif
#  if defined (MOUNT_NFS)
                case MOUNT_NFS:         /* Network Filesystem */
                {
                    strncpy (diptr->fsType, "nfs", DI_TYPE_LEN);
                    break;
                }
#  endif
#  if defined (MOUNT_MFS)
                case MOUNT_MFS:         /* Memory Filesystem */
                {
                    strncpy (diptr->fsType, "mfs", DI_TYPE_LEN);
                    break;
                }
#  endif
#  if defined (MOUNT_MSDOS)
                case MOUNT_MSDOS:       /* MSDOS Filesystem */
                {
                    strncpy (diptr->fsType, "msdos", DI_TYPE_LEN);
                    break;
                }
#  endif
#  if defined (MOUNT_LFS)
                case MOUNT_LFS:
                {
                    strncpy (diptr->fsType, "lfs", DI_TYPE_LEN);
                    break;
                }
#  endif
#  if defined (MOUNT_LOFS)
                case MOUNT_LOFS:
                {
                    strncpy (diptr->fsType, "lofs", DI_TYPE_LEN);
                    break;
                }
#  endif
#  if defined (MOUNT_FDESC)
                case MOUNT_FDESC:
                {
                    strncpy (diptr->fsType, "fdesc", DI_TYPE_LEN);
                    break;
                }
#  endif
#  if defined (MOUNT_PORTAL)
                case MOUNT_PORTAL:
                {
                    strncpy (diptr->fsType, "portal", DI_TYPE_LEN);
                    break;
                }
#  endif
#  if defined (MOUNT_NULL)
                case MOUNT_NULL:
                {
                    strncpy (diptr->fsType, "null", DI_TYPE_LEN);
                    break;
                }
#  endif
#  if defined (MOUNT_UMAP)
                case MOUNT_UMAP:
                {
                    strncpy (diptr->fsType, "umap", DI_TYPE_LEN);
                    break;
                }
#  endif
#  if defined (MOUNT_KERNFS)
                case MOUNT_KERNFS:
                {
                    strncpy (diptr->fsType, "kernfs", DI_TYPE_LEN);
                    break;
                }
#  endif
#  if defined (MOUNT_PROCFS)
                case MOUNT_PROCFS:      /* proc filesystem */
                {
                    strncpy (diptr->fsType, "pfs", DI_TYPE_LEN);
                    break;
                }
#  endif
#  if defined (MOUNT_AFS)
                case MOUNT_AFS:
                {
                    strncpy (diptr->fsType, "afs", DI_TYPE_LEN);
                    break;
                }
#  endif
#  if defined (MOUNT_ISOFS)
                case MOUNT_ISOFS:       /* iso9660 cdrom */
                {
                    strncpy (diptr->fsType, "iso9660fs", DI_TYPE_LEN);
                    break;
                }
#  endif
#  if defined (MOUNT_ISO9660) && ! defined (MOUNT_CD9660)
                case MOUNT_ISO9660:       /* iso9660 cdrom */
                {
                    strncpy (diptr->fsType, "iso9660", DI_TYPE_LEN);
                    break;
                }
#  endif
#  if defined (MOUNT_CD9660)
                case MOUNT_CD9660:       /* iso9660 cdrom */
                {
                    strncpy (diptr->fsType, "cd9660", DI_TYPE_LEN);
                    break;
                }
#  endif
#  if defined (MOUNT_UNION)
                case MOUNT_UNION:
                {
                    strncpy (diptr->fsType, "union", DI_TYPE_LEN);
                    break;
                }
#  endif
            } /* switch on mount type */
        }
# else
#  if _mem_struct_statfs_f_fstypename
        strncpy (diptr->fsType, mntbufp [idx].f_fstypename, DI_TYPE_LEN);
#  else
#   define DI_UNKNOWN_FSTYPE       "(%.2d)?"

            /* could use getvfsbytype here... */
        if ((fstype >= 0) && (fstype < MNT_NUMTYPES))
        {
            strncpy (diptr->fsType, mnt_names [fstype], DI_TYPE_LEN);
        }
        else
        {
            Snprintf1 (diptr->fsType, sizeof (diptr->fsType),
                      DI_UNKNOWN_FSTYPE, fstype);
        }
#  endif
# endif /* else has sys/fs_types.h */

        diptr->isReadOnly = FALSE;
# if defined (MNT_RDONLY)
        if ((mntbufp [idx].f_flags & MNT_RDONLY) == MNT_RDONLY)
        {
            diptr->isReadOnly = TRUE;
        }
# endif
        convertMountOptions ((unsigned long) mntbufp [idx].f_flags, diptr);
        trimChar (diptr->options, ',');

        if (debug > 1)
        {
            printf ("%s: %s\n", diptr->name, diptr->fsType);
            printf ("\tblocks: tot:%ld free:%ld avail:%ld\n",
                    (long) mntbufp [idx].f_blocks,
                    (long) mntbufp [idx].f_bfree,
                    (long) mntbufp [idx].f_bavail);
# if _mem_struct_statfs_f_fsize
            printf ("\tfsize:%ld \n", (long) mntbufp [idx].f_fsize);
# endif
# if _mem_struct_statfs_f_bsize
            printf ("\tbsize:%ld \n", (long) mntbufp [idx].f_bsize);
# endif
# if _mem_struct_statfs_f_iosize
            printf ("\tiosize:%ld \n", (long) mntbufp [idx].f_iosize);
# endif
            printf ("\tinodes: tot:%ld free:%ld\n",
                    (long) mntbufp [idx].f_files,
                    (long) mntbufp [idx].f_ffree);
        }
    }

    free ((char *) mntbufp);  /* man page says this can't be freed. */
                              /* is it ok to try?                   */
    return 0;
}

#endif /* _lib_getmntinfo */

#if _lib_getvfsstat && _args_getvfsstat == 3

/*
 * di_getDiskEntries
 *
 * NetBSD
 *
 */

int
# if _proto_stdc
di_getDiskEntries (diDiskInfo_t **diskInfo, int *diCount)
# else
di_getDiskEntries (diskInfo, diCount)
    diDiskInfo_t **diskInfo;
    int *diCount;
# endif
{
    diDiskInfo_t     *diptr;
    int             count;
    int             idx;
    Size_t          bufsize;
    struct statvfs  *mntbufp;
    struct statvfs  *sp;

    if (debug > 0) { printf ("# getDiskEntries: getvfsstat\n"); }
    count = getvfsstat ((struct statvfs *) NULL, 0, ST_NOWAIT);
    if (count < 1)
    {
        fprintf (stderr, "Unable to do getvfsstat () errno %d\n", errno);
        return -1;
    }
    bufsize = sizeof (struct statvfs) * (Size_t) count;
    mntbufp = malloc ((Size_t) bufsize);
    if (mntbufp == (struct statvfs *) NULL) {
      fprintf (stderr, "malloc failed for mntbufp. errno %d\n", errno);
      return -1;
    }
    memset ((char *) mntbufp, '\0', sizeof (struct statvfs) * (Size_t) count);
    count = getvfsstat (mntbufp, (Size_t) bufsize, ST_NOWAIT);

    *diCount = count;
    *diskInfo = (diDiskInfo_t *) malloc (sizeof (diDiskInfo_t) * (Size_t) count);
    if (*diskInfo == (diDiskInfo_t *) NULL)
    {
        fprintf (stderr, "malloc failed for diskInfo. errno %d\n", errno);
        return -1;
    }
    memset ((char *) *diskInfo, '\0', sizeof (diDiskInfo_t) * (Size_t) count);

    for (idx = 0; idx < count; idx++)
    {
        _fs_size_t      tblocksz;

        diptr = *diskInfo + idx;
        di_initDiskInfo (diptr);

        sp = mntbufp + idx;

# if defined (MNT_RDONLY)
        if ((sp->f_flag & MNT_RDONLY) == MNT_RDONLY)
        {
            diptr->isReadOnly = TRUE;
        }
# endif
# if defined (MNT_LOCAL)
        if ((sp->f_flag & MNT_LOCAL) != MNT_LOCAL)
        {
            diptr->isLocal = FALSE;
        }
# endif
        convertMountOptions ((unsigned long) sp->f_flag, diptr);
        trimChar (diptr->options, ',');

        if (sp->f_frsize == 0 && sp->f_bsize != 0)
        {
            tblocksz = (_fs_size_t) sp->f_bsize;
        }
        else
        {
            tblocksz = (_fs_size_t) sp->f_frsize;
        }

        di_saveBlockSizes (diptr, tblocksz,
            (_fs_size_t) sp->f_blocks,
            (_fs_size_t) sp->f_bfree,
            (_fs_size_t) sp->f_bavail);
        di_saveInodeSizes (diptr,
            (_fs_size_t) sp->f_files,
            (_fs_size_t) sp->f_ffree,
            (_fs_size_t) sp->f_ffree);

        strncpy (diptr->special, sp->f_mntfromname, DI_SPEC_NAME_LEN);
        strncpy (diptr->name, sp->f_mntonname, DI_NAME_LEN);
        strncpy (diptr->fsType, sp->f_fstypename, DI_TYPE_LEN);

        if (debug > 1)
        {
            printf ("%s: %s\n", diptr->name, diptr->fsType);
            printf ("\tbsize:%ld  frsize:%ld\n", (long) sp->f_bsize,
                    (long) sp->f_frsize);
#if _siz_long_long >= 8
            printf ("\tblocks: tot:%llu free:%lld avail:%llu\n",
                   sp->f_blocks, sp->f_bfree, sp->f_bavail);
            printf ("\tinodes: tot:%llu free:%llu avail:%llu\n",
                    sp->f_files, sp->f_ffree, sp->f_favail);
#else
            printf ("\tblocks: tot:%lu free:%lu avail:%lu\n",
                   sp->f_blocks, sp->f_bfree, sp->f_bavail);
            printf ("\tinodes: tot:%lu free:%lu avail:%lu\n",
                    sp->f_files, sp->f_ffree, sp->f_favail);
#endif
        }
    }

    free ((char *) mntbufp);
    return 0;
}

#endif /* _lib_getvfsstat */

#if _lib_getmnt

# if _npt_getmnt
  int getmnt _((int *, struct fs_data *, int, int, char *));
# endif

/*
 * di_getDiskEntries
 *
 * ULTRIX does this with a system call.  The system call allows one
 * to retrieve the information in a series of calls, but coding that
 * looks a little tricky; I just allocate a huge buffer and do it in
 * one shot.
 *
 *                  [mogul@wrl.dec.com (Jeffrey Mogul)]
 */

int
# if _proto_stdc
di_getDiskEntries (diDiskInfo_t **diskInfo, int *diCount)
# else
di_getDiskEntries (diskInfo, diCount)
    diDiskInfo_t **diskInfo;
    int *diCount;
# endif
{
    diDiskInfo_t     *diptr;
    int             count;
    int             bufsize;
    int             idx;
    short           fstype;
    struct fs_data  *fsdbuf;
    int             start;
    int             len;


    if (debug > 0) { printf ("# getDiskEntries: getmnt\n"); }
    bufsize = NMOUNT * sizeof (struct fs_data);  /* enough for max # mounts */
    fsdbuf = (struct fs_data *) malloc ((Size_t) bufsize);
    if (fsdbuf == (struct fs_data *) NULL)
    {
        fprintf (stderr, "malloc (%d) for getmnt () failed errno %d\n",
                 bufsize, errno);
        return -1;
    }

    start = 0;
    count = getmnt (&start, fsdbuf, bufsize, STAT_MANY, 0);
    if (count < 1)
    {
        fprintf (stderr, "Unable to do getmnt () [= %d] errno %d\n",
                 count, errno);
        free ((char *) fsdbuf);
        return -1;
    }

    *diCount = count;
    *diskInfo = (diDiskInfo_t *) malloc (sizeof (diDiskInfo_t) * (Size_t) count);
    if (*diskInfo == (diDiskInfo_t *) NULL)
    {
        fprintf (stderr, "malloc failed for diskInfo. errno %d\n", errno);
        free ((char *) fsdbuf);
        return -1;
    }
    memset ((char *) *diskInfo, '\0', sizeof (diDiskInfo_t) * count);

    for (idx = 0; idx < count; idx++)
    {
        diptr = *diskInfo + idx;
        di_initDiskInfo (diptr);

        if ((fsdbuf [idx].fd_req.flags & MNT_LOCAL) != MNT_LOCAL)
        {
            diptr->isLocal = FALSE;
        }

        strncpy (diptr->special, fsdbuf [idx].fd_devname, DI_SPEC_NAME_LEN);
        strncpy (diptr->name, fsdbuf [idx].fd_path, DI_NAME_LEN);

            /* ULTRIX keeps these fields in units of 1K byte */
        di_saveBlockSizes (diptr, (_fs_size_t) 1024,
            (_fs_size_t) fsdbuf [idx].fd_btot,
            (_fs_size_t) fsdbuf [idx].fd_bfree,
            (_fs_size_t) fsdbuf [idx].fd_bfreen);
        di_saveInodeSizes (diptr,
            (_fs_size_t) fsdbuf [idx].fd_gtot,
            (_fs_size_t) fsdbuf [idx].fd_gfree,
            (_fs_size_t) fsdbuf [idx].fd_gfree);

        fstype = fsdbuf [idx].fd_fstype;
        if (fstype == GT_UNKWN)
        {
            diptr->printFlag = DI_PRNT_IGNORE;
            if (debug > 2)
            {
                printf ("mnt: ignore: disk type unknown: %s\n",
                        diptr->name);
            }
        }
        else if ((fstype > 0) && (fstype < GT_NUMTYPES))
        {
            strncpy (diptr->fsType, gt_names [fstype], DI_TYPE_LEN);
        }
        else
        {
            Snprintf1 (diptr->fsType, sizeof (diptr->fsType),
                          "Unknown fstyp %.2d", fstype);
        }

        if ((fsdbuf [idx].fd_req.flags & MNT_RDONLY) == MNT_RDONLY)
        {
            diptr->isReadOnly = TRUE;
        }
        else
        {
            diptr->isReadOnly = FALSE;
        }
        convertMountOptions ((unsigned long) fsdbuf [idx].fd_req.flags, diptr);
        trimChar (diptr->options, ',');

        if (debug > 1)
        {
            printf ("%s: %s\n", diptr->name, diptr->fsType);
            printf ("\tblocks: tot:%ld free:%ld avail:%ld\n",
                    fsdbuf [idx].fd_btot, fsdbuf [idx].fd_bfree,
                    (int) fsdbuf [idx].fd_bfreen);
            printf ("\tinodes: tot:%ld free:%ld\n",
                    fsdbuf [idx].fd_gtot, fsdbuf [idx].fd_gfree);
        }
    }

    free ((char *) fsdbuf);
    return 0;
}

#endif /* _lib_getmnt */


#if _lib_mntctl

/*
 * di_getDiskEntries
 *
 * AIX uses mntctl to find out about mounted file systems
 * This seems to be better than set/get/end, as we get the
 * remote filesystem flag.
 *
 */

# define DI_FSMAGIC 10    /* base AIX configuration has 5 file systems */
# define NUM_AIX_FSTYPES         6
static char *AIX_fsType [NUM_AIX_FSTYPES] =
    { "oaix", "", "nfs", "jfs", "", "cdrom" };

/*
 * from xfsm-1.80:
 *
 * MNT_AIX - "aix"
 * MNT_NFS - "nfs"
 * MNT_JFS - "jfs"
 * MNT_CDROM - "cdrom"
 * other - "user defined"
 *
 */

#define DI_RETRY_COUNT         5

int
# if _proto_stdc
di_getDiskEntries (diDiskInfo_t **diskInfo, int *diCount)
# else
di_getDiskEntries (diskInfo, diCount)
    diDiskInfo_t **diskInfo;
    int *diCount;
# endif
{
    diDiskInfo_t     *diptr;
    int             num;        /* number of vmount structs returned    */
    char            *vmbuf;     /* buffer for vmount structs returned   */
    Size_t          vmbufsz;    /* size in bytes of vmbuf               */
    int             i;          /* index for looping and stuff          */
    char            *bufp;      /* pointer into vmbuf                   */
    struct vmount   *vmtp;      /* pointer into vmbuf                   */
    struct vfs_ent  *ve;        /* pointer for file system type entry   */


    if (debug > 0) { printf ("# getDiskEntries: mntctl\n"); }
    i = 0;
    vmbufsz = sizeof (struct vmount) * DI_FSMAGIC; /* initial vmount buffer */

    do
    {
        if ((vmbuf = (char *) malloc (vmbufsz)) == (char *) NULL)
        {
            fprintf (stderr, "malloc (%d) for mntctl() failed errno %d\n",
                    (int) vmbufsz, errno);
            return -1;
        }

        num = mntctl (MCTL_QUERY, vmbufsz, vmbuf);
            /*
             * vmbuf is too small, could happen for
             * following reasons:
             * - inital buffer is too small
             * - newly mounted file system
             */
        if (num == 0)
        {
            memcpy (&vmbufsz, vmbuf, sizeof (vmbufsz)); /* see mntctl(2) */
            if (debug > 0)
            {
                printf ("vmbufsz too small, new size: %d\n", (int) vmbufsz);
            }
            free ((char *) vmbuf); /* free this last, it's still being used! */
            ++i;
        }
    } while (num == 0 && i < DI_RETRY_COUNT);

    if (i >= DI_RETRY_COUNT)
    {
        free ((char *) vmbuf);
        fprintf (stderr, "unable to allocate adequate buffer for mntctl\n");
        return -1;
    }

    if (num == -1)
    {
        free ((char *) vmbuf);
        fprintf (stderr, "%s errno %d\n", strerror (errno), errno);
        return -1;
    }

        /* <num> vmount structs returned in vmbuf */
    *diCount = num;
    *diskInfo = (diDiskInfo_t *) malloc (sizeof (diDiskInfo_t) *
        (Size_t) *diCount);
    if (*diskInfo == (diDiskInfo_t *) NULL)
    {
        fprintf (stderr, "malloc failed for diskInfo. %s errno %d\n",
                 strerror (errno), errno);
        return -1;
    }

    bufp = vmbuf;
    for (i = 0; i < num; i++)
    {
        diptr = *diskInfo + i;
        di_initDiskInfo (diptr);

        vmtp = (struct vmount *) bufp;
        if ((vmtp->vmt_flags & MNT_REMOTE) == MNT_REMOTE)
        {
            diptr->isLocal = FALSE;
        }
        if ((vmtp->vmt_flags & MNT_RDONLY) == MNT_RDONLY)
        {
            diptr->isReadOnly = TRUE;
        }

        *diptr->special = '\0';
        if (diptr->isLocal == FALSE)
        {
            strncpy (diptr->special,
                    (char *) vmt2dataptr (vmtp, VMT_HOSTNAME),
                    DI_SPEC_NAME_LEN);
            strncat (diptr->special, ":",
                    DI_SPEC_NAME_LEN - strlen (diptr->special) - 1);
        }
        strncat (diptr->special,
                (char *) vmt2dataptr (vmtp, VMT_OBJECT),
                DI_SPEC_NAME_LEN - strlen (diptr->special) - 1);
        strncpy (diptr->name,
                (char *) vmt2dataptr (vmtp, VMT_STUB), DI_NAME_LEN);

        ve = getvfsbytype (vmtp->vmt_gfstype);
        if (ve == (struct vfs_ent *) NULL || *ve->vfsent_name == '\0')
        {
            if (vmtp->vmt_gfstype >= 0 &&
                    (vmtp->vmt_gfstype < NUM_AIX_FSTYPES))
            {
                strncpy (diptr->fsType,
                        AIX_fsType [vmtp->vmt_gfstype], DI_TYPE_LEN);
            }
        }
        else
        {
            strncpy (diptr->fsType, ve->vfsent_name, DI_TYPE_LEN);
        }

        strncpy (diptr->options, (char *) vmt2dataptr (vmtp, VMT_ARGS),
                 DI_OPT_LEN);
        trimChar (diptr->options, ',');
        bufp += vmtp->vmt_length;

        if (debug > 1)
        {
            printf ("mnt:%s - %s : %s\n", diptr->name,
                    diptr->special, diptr->fsType);
            printf ("\t%s\n", (char *) vmt2dataptr (vmtp, VMT_ARGS));
        }
    }
    return 0;
}

#endif  /* _lib_mntctl */


#if _lib_GetDriveType \
    && _lib_GetLogicalDriveStrings

/*
 * di_getDiskInfo
 *
 * Windows
 *
 */

# define MSDOS_BUFFER_SIZE          256
# define BYTES_PER_LOGICAL_DRIVE    4

int
# if _proto_stdc
di_getDiskEntries (diDiskInfo_t **diskInfo, int *diCount)
# else
di_getDiskEntries (diskInfo, diCount)
    diDiskInfo_t **diskInfo;
    int *diCount;
# endif
{
    diDiskInfo_t     *diptr;
    int             i;
    char            diskflag;
    int             rc;
    char            *p;
    char            buff [MSDOS_BUFFER_SIZE];


    if (debug > 0) { printf ("# getDiskInfo: GetLogicalDriveStrings GetDriveType\n"); }
    diskflag = DI_PRNT_SKIP;
    rc = (int) GetLogicalDriveStrings (MSDOS_BUFFER_SIZE, buff);
    *diCount = rc / BYTES_PER_LOGICAL_DRIVE;

    *diskInfo = (diDiskInfo_t *)
        malloc (sizeof (diDiskInfo_t) * (Size_t) *diCount);
    if (*diskInfo == (diDiskInfo_t *) NULL)
    {
        fprintf (stderr, "malloc failed for diskInfo. errno %d\n", errno);
        return -1;
    }

    for (i = 0; i < *diCount; ++i)
    {
        diptr = *diskInfo + i;
        p = buff + (BYTES_PER_LOGICAL_DRIVE * i);
        strncpy (diptr->name, p, DI_NAME_LEN);
        rc = (int) GetDriveType (p);
        diptr->printFlag = DI_PRNT_OK;

        if (rc == DRIVE_NO_ROOT_DIR)
        {
            diptr->printFlag = DI_PRNT_BAD;
        }

            /* assume that any removable drives before the  */
            /* first non-removable disk are floppies...     */
        else if (rc == DRIVE_REMOVABLE)
        {
          DWORD br;
          BOOL bSuccess;
          char handleName [MSDOS_BUFFER_SIZE + 1];

          diptr->printFlag = diskflag;
          bSuccess = 1;
          strncpy (handleName, "\\\\.\\", MSDOS_BUFFER_SIZE);
          strncat (handleName, p, MSDOS_BUFFER_SIZE);
          handleName[strlen(handleName)-1] = '\0';

# if _define_IOCTL_STORAGE_CHECK_VERIFY2
          HANDLE hDevice = CreateFile (handleName,
              FILE_READ_ATTRIBUTES, FILE_SHARE_READ | FILE_SHARE_WRITE,
              NULL, OPEN_EXISTING, 0, NULL);
          bSuccess = DeviceIoControl (hDevice,
              IOCTL_STORAGE_CHECK_VERIFY2,
              NULL, 0, NULL, 0, &br, (LPOVERLAPPED) NULL);
# else
          HANDLE hDevice = CreateFile (handleName,
              GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
              NULL, OPEN_EXISTING, 0, NULL);
          bSuccess = DeviceIoControl (hDevice,
              IOCTL_STORAGE_CHECK_VERIFY,
              NULL, 0, NULL, 0, &br, (LPOVERLAPPED) NULL);
# endif
          CloseHandle (hDevice);

          if (! bSuccess)
          {
            diptr->printFlag = DI_PRNT_BAD;
          }
        }
        else
        {
            diskflag = DI_PRNT_OK;
        }

        if (rc != DRIVE_REMOTE)
        {
            diptr->isLocal = TRUE;
        }
    } /* for each mounted drive */

    return *diCount;
}

#endif  /* _lib_GetDiskFreeSpace || _lib_GetDiskFreeSpaceEx */

#if _lib_fs_stat_dev \
    && _lib_next_dev

/*
 * di_getDiskEntries
 *
 * For BeOS.
 *
 */

int
# if _proto_stdc
di_getDiskEntries (diDiskInfo_t **diskInfo, int *diCount)
# else
di_getDiskEntries (diskInfo, diCount)
    diDiskInfo_t **diskInfo;
    int *diCount;
# endif
{
    diDiskInfo_t     *diptr;
    status_t        stat;
    int             idx;
    int32           count;
    dev_t           dev;
    char            buff [B_FILE_NAME_LENGTH];
    fs_info         fsinfo;
    node_ref        nref;
    BDirectory      *dir;
    BEntry          entry;
    BPath           path;

    if (debug > 0) { printf ("# getDiskEntries: fs_stat_dev\n"); }
    count = 0;
    while ((dev = next_dev (&count)) != B_BAD_VALUE)
    {
        if ((stat = fs_stat_dev (dev, &fsinfo)) == B_BAD_VALUE)
        {
            break;
        }

        idx = *diCount;
        ++*diCount;
        *diskInfo = (diDiskInfo_t *) di_realloc ((char *) *diskInfo,
                sizeof (diDiskInfo_t) * *diCount);
        if (*diskInfo == (diDiskInfo_t *) NULL) {
          fprintf (stderr, "malloc failed for diskInfo. errno %d\n", errno);
          return -1;
        }
        diptr = *diskInfo + idx;
        di_initDiskInfo (diptr);

        *buff = '\0';
        nref.device = dev;
        nref.node = fsinfo.root;
        dir = new BDirectory (&nref);
        stat = dir->GetEntry (&entry);
        stat = entry.GetPath (&path);
        strncpy (diptr->name, path.Path(), DI_NAME_LEN);
        strncpy (diptr->special, fsinfo.device_name, DI_SPEC_NAME_LEN);
        strncpy (diptr->fsType, fsinfo.fsh_name, DI_TYPE_LEN);
        di_saveBlockSizes (diptr, (_fs_size_t) fsinfo.block_size,
            (_fs_size_t) fsinfo.total_blocks,
            (_fs_size_t) fsinfo.free_blocks,
            (_fs_size_t) fsinfo.free_blocks);
        di_saveInodeSizes (diptr, (_fs_size_t) fsinfo.total_nodes,
            (_fs_size_t) fsinfo.free_nodes,
            (_fs_size_t) fsinfo.free_nodes);
# if defined (MNT_RDONLY)
        if ((fsinfo.flags & MNT_RDONLY) == MNT_RDONLY)
        {
           diptr->isReadOnly = TRUE;
        }
# endif
# if defined (MNT_PERSISTENT)
        if ((fsinfo.flags & MNT_PERSISTENT) != MNT_PERSISTENT)
        {
           diptr->printFlag = DI_PRNT_IGNORE;
        }
# endif
        convertMountOptions ((unsigned long) fsinfo.flags, diptr);
        trimChar (diptr->options, ',');

        if (debug > 1)
        {
            printf ("mnt:%s - %s\n", diptr->name, diptr->special);
            printf ("dev:%d fs:%s\n", dev, diptr->fsType);
        }
        if (debug > 1)
        {
            printf ("%s: %s\n", diptr->name, diptr->fsType);
            printf ("\tblocks: tot:%ld free:%ld\n",
                    fsinfo.total_blocks, fsinfo.free_blocks);
            printf ("\tinodes: tot:%ld free:%ld\n",
                    fsinfo.total_nodes, fsinfo.free_nodes);
        }
    }
    return 0;
}

#endif

#if _class_os__Volumes

/*
 * di_getDiskEntries
 *
 * For Syllable.
 *
 */

int
# if _proto_stdc
di_getDiskEntries (diDiskInfo_t **diskInfo, int *diCount)
# else
di_getDiskEntries (diskInfo, diCount)
    diDiskInfo_t **diskInfo;
    int *diCount;
# endif
{
    diDiskInfo_t    *diptr;
    int             idx;
    Uint_t          i;
    status_t        stat;
    uint32          count;
    fs_info         fsinfo;
    os::Volumes     *vols;
    os::String      mpString;

    if (debug > 0) { printf ("# getDiskEntries: os::Volumes\n"); }
    vols = new os::Volumes;
    count = vols->GetMountPointCount ();
    for (i = 0; i < count; ++i)
    {
        if ((stat = vols->GetFSInfo (i, &fsinfo)) != 0)
        {
           break;
        }

        idx = *diCount;
        ++*diCount;
        *diskInfo = (diDiskInfo_t *) di_realloc ((char *) *diskInfo,
                sizeof (diDiskInfo_t) * *diCount);
        if (*diskInfo == (diDiskInfo_t *) NULL) {
          fprintf (stderr, "malloc failed for diskInfo. errno %d\n", errno);
          return -1;
        }
        diptr = *diskInfo + idx;
        di_initDiskInfo (diptr);
        stat = vols->GetMountPoint (i, &mpString);
        strncpy (diptr->name, mpString.c_str(), DI_NAME_LEN);
        strncpy (diptr->special, fsinfo.fi_device_path, DI_SPEC_NAME_LEN);
        strncpy (diptr->fsType, fsinfo.fi_driver_name, DI_TYPE_LEN);
        di_saveBlockSizes (diptr, (_fs_size_t) fsinfo.fi_block_size,
            (_fs_size_t) fsinfo.fi_total_blocks,
            (_fs_size_t) fsinfo.fi_free_blocks,
            (_fs_size_t) fsinfo.fi_free_user_blocks);
        di_saveInodeSizes (diptr, (_fs_size_t) fsinfo.fi_total_inodes,
            (_fs_size_t) fsinfo.fi_free_inodes,
            (_fs_size_t) fsinfo.fi_free_inodes);
# if defined (MNT_RDONLY)
        if ((fsinfo.fi_flags & MNT_RDONLY) == MNT_RDONLY)
        {
           diptr->isReadOnly = TRUE;
        }
# endif
# if defined (MNT_PERSISTENT)
        if ((fsinfo.fi_flags & MNT_PERSISTENT) != MNT_PERSISTENT)
        {
           diptr->printFlag = DI_PRNT_IGNORE;
        }
# endif
        convertMountOptions ((unsigned long) fsinfo.fi_flags, diptr);
        trimChar (diptr->options, ',');

        if (debug > 1)
        {
            printf ("mnt:%s - %s\n", diptr->name, diptr->special);
            printf ("dev:%d fs:%s\n", fsinfo.fi_dev, diptr->fsType);
        }
        if (debug > 1)
        {
            printf ("mount_args: %s\n", fsinfo.fi_mount_args);
            printf ("%s: %s\n", diptr->name, diptr->fsType);
            printf ("\tblocks: tot:%ld free:%ld avail:%ld\n",
                    (long int) fsinfo.fi_total_blocks,
                    (long int) fsinfo.fi_free_blocks,
                    (long int) fsinfo.fi_free_user_blocks);
            printf ("\tinodes: tot:%ld free:%ld\n",
                    (long int) fsinfo.fi_total_inodes,
                    (long int) fsinfo.fi_free_inodes);
        }
    }
    return 0;
}

#endif

/*
 * VMS
 */

#if _lib_sys_dollar_device_scan && _lib_sys_dollar_getdviw

/* Thanks to Craig Berry's VMS::Device for the VMS code. */

char *monthNames[12] = {
  "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep",
  "Oct", "Nov", "Dec" };

genericID_t devInfoList[] =
{
#ifdef DVI$_DISPLAY_DEVNAM
  DVI_ENT(DISPLAY_DEVNAM, 256, DVI_IS_STRING),
#endif
#ifdef DVI$_DEVBUFSIZ
  DVI_ENT(DEVBUFSIZ, 4, DVI_IS_LONGWORD),
#endif
#ifdef DVI$_MAXBLOCK
  DVI_ENT(MAXBLOCK, 4, DVI_IS_LONGWORD),
#endif
#ifdef DVI$_FREEBLOCKS
  DVI_ENT(FREEBLOCKS, 4, DVI_IS_LONGWORD),
#endif
#ifdef DVI$_MAXFILES
  DVI_ENT(MAXFILES, 4, DVI_IS_LONGWORD),
#endif
#ifdef DVI$_REMOTE_DEVICE
  DVI_ENT(REMOTE_DEVICE, 4, DVI_IS_LONGWORD),
#endif
#ifdef DVI$_VOLNAM
  DVI_ENT(VOLNAM, 12, DVI_IS_STRING),
#endif
#ifdef DVI$_AVL
  DVI_ENT(AVL, 4, DVI_IS_LONGWORD),
#endif
#ifdef DVI$_MNT
  DVI_ENT(MNT, 4, DVI_IS_LONGWORD),
#endif
#ifdef DVI$_SHR
  DVI_ENT(SHR, 4, DVI_IS_LONGWORD),
#endif
#ifdef DVI$_ODS2_SUBSET0
  DVI_ENT(ODS2_SUBSET0, 4, DVI_IS_LONGWORD),
#endif
#ifdef DVI$_ODS5
  DVI_ENT(ODS5, 4, DVI_IS_LONGWORD),
#endif
#ifdef DVI$_NET
  DVI_ENT(NET, 4, DVI_IS_LONGWORD),
#endif
#ifdef DVI$_DEVICE_TYPE_NAME
  DVI_ENT(DEVICE_TYPE_NAME, 64, DVI_IS_STRING),
#endif
#ifdef DVI$_SHDW_MASTER
  DVI_ENT(SHDW_MASTER, 4, DVI_IS_LONGWORD),
#endif
#ifdef DVI$_SHDW_MEMBER
  DVI_ENT(SHDW_MEMBER, 4, DVI_IS_LONGWORD),
#endif
#ifdef DVI$_SHDW_MASTER_NAME
  DVI_ENT(SHDW_MASTER_NAME, 64, DVI_IS_STRING),
#endif
  {NULL, 0, 0, 0}
};
#define devInfoListCount ((sizeof(devInfoList)/sizeof (genericID_t)) - 1)


int
# if _proto_stdc
di_getDiskEntries (diDiskInfo_t **diskInfo, int *diCount)
# else
di_getDiskEntries (diskInfo, diCount)
    diDiskInfo_t **diskInfo;
    int *diCount;
# endif
{
  diDiskInfo_t    *diptr;
  int             idx;
  unsigned short  devNameLen;
  unsigned short  devReturnLen;
  unsigned int    deviceClass;
  char            context[8] = {0,0,0,0,0,0,0,0};
  struct dsc$descriptor_s devNameDesc;
  struct dsc$descriptor_s devSearchNameDesc;
  VMS_ITMLST      scanItemList;
  VMS_ITMLST      *itemList;
  unsigned short  *returnLengths;
  char            **returnBuffers;
  long            *tempLong;
  char            returnedDevName [65];
  unsigned short  returnedTime[7];
  char            displayTime[100];
  int             i;
  int             status;
# ifdef __ALPHA
  __int64         *tempLongLong;
# endif
  _fs_size_t      freeBlocks;
  _fs_size_t      totBlocks;
  _fs_size_t      maxFiles;
  _fs_size_t      tblocksz;


  if (debug > 0) { printf ("# getDiskEntries: sys$device_scan/sys$getdviw\n"); }

  devNameDesc.dsc$a_pointer = returnedDevName;
  devNameDesc.dsc$w_length = 64;
  devNameDesc.dsc$b_dtype = DSC$K_DTYPE_T;
  devNameDesc.dsc$b_class = DSC$K_CLASS_S;

  devSearchNameDesc.dsc$a_pointer = "*";
  devSearchNameDesc.dsc$w_length = 1;
  devSearchNameDesc.dsc$b_dtype = DSC$K_DTYPE_T;
  devSearchNameDesc.dsc$b_class = DSC$K_CLASS_S;

  memset (&scanItemList, 0, sizeof(VMS_ITMLST));
  itemList = malloc (sizeof(VMS_ITMLST) * (devInfoListCount));
  if (itemList == (VMS_ITMLST *) NULL) {
    fprintf (stderr, "malloc failed for itemList. errno %d\n", errno);
    return -1;
  }
  memset (itemList, 0, sizeof(VMS_ITMLST) * (devInfoListCount));
  returnBuffers = (char **) malloc (sizeof (char *) * devInfoListCount);
  if (returnBuffers == (char **) NULL) {
    fprintf (stderr, "malloc failed for returnBuffers. errno %d\n", errno);
    free ((char *) itemList);
    return -1;
  }
  returnLengths = malloc (sizeof(short) * devInfoListCount);
  if (returnLengths == (short *) NULL) {
    fprintf (stderr, "malloc failed for returnLengths. errno %d\n", errno);
    free ((char *) itemList);
    free ((char *) returnBuffers);
    return -1;
  }

  for (i = 0; i < devInfoListCount; i++) {
    returnBuffers[i] = malloc (devInfoList[i].bufferLen + 1);
    if (returnBuffers[i] == (char *) NULL) {
      fprintf (stderr, "malloc failed for returnBuffers[i]. errno %d\n", errno);
      free ((char *) itemList);
      free ((char *) returnBuffers);
      free ((char *) returnLengths);
      return -1;
    }
    memset (returnBuffers[i], 0, devInfoList[i].bufferLen + 1);
    init_itemlist (&itemList[i], devInfoList[i].bufferLen,
          devInfoList[i].syscallValue, returnBuffers[i],
          &returnLengths[i]);
  }

  deviceClass = DC$_DISK;
  init_itemlist (&scanItemList, sizeof(deviceClass),
     DVS$_DEVCLASS, &deviceClass, NULL);

  while (sys$device_scan (&devNameDesc, &devReturnLen,
          &devSearchNameDesc, &scanItemList, context) == SS$_NORMAL) {
    idx = *diCount;
    ++*diCount;
    *diskInfo = (diDiskInfo_t *) di_realloc ((char *) *diskInfo,
        sizeof (diDiskInfo_t) * (Size_t) *diCount);
    if (*diskInfo == (di_DiskInfo_t *) NULL)
      fprintf (stderr, "malloc failed for diskInfo. errno %d\n", errno);
      return -1;
    }
    diptr = *diskInfo + idx;
    di_initDiskInfo (diptr);

    returnedDevName [devReturnLen] = '\0';

    if (debug > 4)
    {
      printf ("%s\n", returnedDevName);
    }

    devNameDesc.dsc$a_pointer = returnedDevName;
    devNameDesc.dsc$w_length = devReturnLen;
    devNameDesc.dsc$b_dtype = DSC$K_DTYPE_T;
    devNameDesc.dsc$b_class = DSC$K_CLASS_S;

    strncpy (diptr->fsType, "ODS-2", DI_TYPE_LEN);
    /* _$9$LDA2: */
    if (strlen (returnedDevName) > 4 && returnedDevName[4] == 'L')
    {
      strncpy (diptr->fsType, "LD", DI_TYPE_LEN);
    }

    status = sys$getdviw(0, 0, &devNameDesc, itemList,
            NULL, NULL, NULL, NULL);
    if (status == SS$_NORMAL) {
      for (i = 0; i < devInfoListCount; i++) {
        switch (devInfoList[i].returnType) {
          case DVI_IS_STRING:
            if (debug > 4)
            {
              printf ("  string:");
              printf ("%s:", devInfoList[i].name);
              printf ("%*s:\n", (int) returnLengths[i],
                      (char *) returnBuffers[i]);
            }
            if (strcmp (devInfoList[i].name, "DISPLAY_DEVNAM") == 0)
            {
                strncpy (diptr->special, returnBuffers[i], returnLengths[i]);
                diptr->special[returnLengths[i]] = '\0';
            }
            if (strcmp (devInfoList[i].name, "VOLNAM") == 0)
            {
                strncpy (diptr->name, returnBuffers[i], returnLengths[i]);
                diptr->name[returnLengths[i]] = '\0';
            }
            if (strcmp (devInfoList[i].name, "DEVICE_TYPE_NAME") == 0)
            {
              returnBuffers[i][returnLengths[i]] = '\0';
              strncat (diptr->options, "dev=",
                  DI_OPT_LEN - strlen (diptr->options) - 1);
              strncat (diptr->options, returnBuffers[i],
                  DI_OPT_LEN - strlen (diptr->options) - 1);
              strncat (diptr->options, ",",
                  DI_OPT_LEN - strlen (diptr->options) - 1);
            }
            if (strcmp (devInfoList[i].name, "SHDW_MASTER_NAME") == 0)
            {
              if (returnLengths[i] > 0) {
                returnBuffers[i][returnLengths[i]] = '\0';
                strncat (diptr->options, returnBuffers[i],
                    DI_OPT_LEN - strlen (diptr->options) - 1);
                strncat (diptr->options, "),",
                    DI_OPT_LEN - strlen (diptr->options) - 1);
              }
            }
            break;
          case DVI_IS_VMSDATE:
            sys$numtim (returnedTime, returnBuffers[i]);
            sprintf (displayTime, "%02hi-%s-%hi %02hi:%02hi:%02hi.%02hi",
                    returnedTime[2], monthNames[returnedTime[1] - 1],
                    returnedTime[0], returnedTime[3], returnedTime[4],
                    returnedTime[5], returnedTime[6]);
            if (debug > 4)
            {
              printf ("  date:");
              printf ("%s:", devInfoList[i].name);
              printf ("%s:\n", displayTime);
            }
            break;
          case DVI_IS_ENUM:
            tempLong = (long *) returnBuffers[i];
            if (debug > 4)
            {
              printf ("  enum:");
              printf ("%s:", devInfoList[i].name);
              printf ("%ld:\n", *tempLong);
            }
            break;
          case DVI_IS_BITMAP:
          case DVI_IS_LONGWORD:
            tempLong = (long *) returnBuffers[i];
            if (debug > 4)
            {
              printf ("  long:");
              printf ("%s:", devInfoList[i].name);
              printf ("%ld:\n", *tempLong);
            }
            if (strcmp (devInfoList[i].name, "MAXBLOCK") == 0)
            {
                totBlocks = *tempLong;
            }
            if (strcmp (devInfoList[i].name, "FREEBLOCKS") == 0)
            {
                freeBlocks = *tempLong;
            }
            if (strcmp (devInfoList[i].name, "MAXFILES") == 0)
            {
                maxFiles = *tempLong;
            }
            if (strcmp (devInfoList[i].name, "REMOTE_DEVICE") == 0)
            {
                if (*tempLong == 1L)
                {
                    diptr->isLocal = FALSE;
                }
            }
            if (strcmp (devInfoList[i].name, "NET") == 0)
            {
                if (*tempLong == 1L)
                {
                    diptr->isLocal = FALSE;
                }
            }
            if (strcmp (devInfoList[i].name, "DEVBUFSIZ") == 0)
            {
                tblocksz = *tempLong;
            }
            if (strcmp (devInfoList[i].name, "AVL") == 0)
            {
              if (*tempLong == 0L)
              {
                diptr->printFlag = DI_PRNT_IGNORE;
                strncat (diptr->options, "Offline,",
                    DI_OPT_LEN - strlen (diptr->options) - 1);
              }
              if (*tempLong == 1L)
              {
                strncat (diptr->options, "Online,",
                    DI_OPT_LEN - strlen (diptr->options) - 1);
              }
            }
            if (strcmp (devInfoList[i].name, "MNT") == 0)
            {
              if (*tempLong == 1L)
              {
                strncat (diptr->options, "Mounted,",
                    DI_OPT_LEN - strlen (diptr->options) - 1);
              }
              else
              {
                diptr->printFlag = DI_PRNT_IGNORE;
              }
            }
            if (strcmp (devInfoList[i].name, "SHR") == 0)
            {
              if (*tempLong == 1L)
              {
                strncat (diptr->options, "Shareable,",
                    DI_OPT_LEN - strlen (diptr->options) - 1);
              }
            }
            if (strcmp (devInfoList[i].name, "SHDW_MASTER") == 0)
            {
              if (*tempLong == 1L)
              {
                strncat (diptr->options, "Shadow Master,",
                    DI_OPT_LEN - strlen (diptr->options) - 1);
              }
            }
            if (strcmp (devInfoList[i].name, "SHDW_MEMBER") == 0)
            {
              if (*tempLong == 1L)
              {
                strncat (diptr->options, "Shadow Member(",
                    DI_OPT_LEN - strlen (diptr->options) - 1);
              }
            }
            if (strcmp (devInfoList[i].name, "ODS2_SUBSET0") == 0)
            {
              if (*tempLong == 1L)
              {
                strncpy (diptr->fsType, "ODS-2/0", DI_TYPE_LEN);
              }
            }
            if (strcmp (devInfoList[i].name, "ODS5") == 0)
            {
              if (*tempLong == 1L)
              {
                strncpy (diptr->fsType, "ODS-5", DI_TYPE_LEN);
              }
            }
            break;
# ifdef __ALPHA
          case DVI_IS_QUADWORD:
            tempLongLong = (__int64 *) returnBuffers[i];
            if (debug > 4)
            {
              printf ("  quad:");
              printf ("%s:", devInfoList[i].name);
              printf ("%llu:\n", (long long) *tempLongLong);
            }
            break;
# endif
          }
      }
    } else {
      /* fail */
      ;
    }

    di_saveBlockSizes (diptr, tblocksz, totBlocks, freeBlocks, freeBlocks);
    di_saveInodeSizes (diptr, maxFiles, 0, 0);

    /* reset this so there's room during the next syscall */
    devNameDesc.dsc$w_length = 64;
    trimChar (diptr->options, ',');
  } /* while there are entries */

  for (i = 0; i < devInfoListCount; i++) {
    free (returnBuffers[i]);
  }
  free (returnBuffers);
  free (returnLengths);
  free (itemList);

  return 0;
}

#endif


