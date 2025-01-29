/*
 * Copyright 1994-2018 Brad Lanam, Walnut Creek, CA
 * Copyright 2023-2025 Brad Lanam, Pleasant Hill, CA
 */

/*
 *
 *    di_get_disk_entries ()
 *        Get a list of mounted filesystems.
 *        In many cases, this also does the work of di_get_disk_info ().
 */

#include "config.h"

#if _hdr_stdio
# include <stdio.h>
#endif
#if _hdr_stdlib
# include <stdlib.h>
#endif
#if _hdr_stdbool
# include <stdbool.h>
#endif
#if _hdr_dirent
# include <dirent.h>
#endif
#if _sys_types && ! defined (DI_INC_SYS_TYPES_H) /* xenix */
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
# include <mntent.h>            /* hasmntopt (); _PATH_MNTTAB */
#endif                          /* HP-UX: set/get/endmntent (); hasmntopt () */

/* FreeBSD, OpenBSD, old NetBSD, HP-UX, MacOS */
#if _sys_mount && ! defined (DI_INC_SYS_MOUNT)
# define DI_INC_SYS_MOUNT 1
# include <sys/mount.h>         /* getmntinfo (); struct statfs */
#endif
#if _sys_fstypes                /* NetBSD */
# include <sys/fstypes.h>
#endif
#if _sys_fs_types               /* OSF/1, AROS */
# include <sys/fs_types.h>
#endif
#if _sys_mnttab                 /* Solaris, SCO_SV, UnixWare */
# include <sys/mnttab.h>        /* getmntent (); MNTTAB */
#endif

#if _sys_statfs && ! _sys_statvfs /* Linux, FreeBSD, SysV.3 */
# include <sys/statfs.h>                    /* struct statfs; statfs () */
#endif
#if _sys_statvfs                    /* NetBSD, Solaris */
# include <sys/statvfs.h>           /* struct statvfs; statvfs () */
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
#if _hdr_util_string                /* Syllable */
# include <util/string.h>           /* os::String - to get mount name */
#endif

#include "di.h"
#include "disystem.h"
#include "diinternal.h"
#include "distrutils.h"
#include "dimntopt.h"

/********************************************************/

#if defined (__cplusplus) || defined (c_plusplus)
extern "C" {
#endif
/* workaround for AIX - mntctl not declared */
# if _lib_mntctl && _npt_mntctl
  extern int mntctl (int, Size_t, char *);
# endif
#if defined (__cplusplus) || defined (c_plusplus)
}
#endif

#if (_lib_getmntent \
    || _args_statfs > 0) \
    && ! _lib_getmntinfo \
    && ! _lib_getfsstat \
    && ! _lib_getvfsstat \
    && ! _lib_mntctl \
    && ! _lib_getmnt
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
static int di_getQNXDiskEntries (di_data_t *di_data, char *ipath, int *diCount);
#endif

/********************************************************/

#if _lib_getmntent \
    && ! _lib_setmntent \
    && ! _lib_mntctl

static char *checkMountOptions      (struct mnttab *, char *);

/*
 * di_get_disk_entries
 *
 * For SysV.4, we open the file and call getmntent () repeatedly.
 *
 */

int
di_get_disk_entries (di_data_t *di_data, int *diCount)
{
  di_disk_info_t     *diptr;
  FILE            *f;
  int             idx;
  struct mnttab   mntEntry;
  char            *devp;   /* local ptr to dev entry */
  di_opt_t        *diopts;

  diopts = (di_opt_t *) di_data->options;

  if (diopts->optval [DI_OPT_DEBUG] > 0) { printf ("# di_get_disk_entries: getmntent\n"); }
  if ( (f = fopen (DI_MOUNT_FILE, "r")) == (FILE *) NULL) {
    fprintf (stderr, "Unable to open: %s errno %d\n", DI_MOUNT_FILE, errno);
    return -1;
  }

  while (getmntent (f, &mntEntry) == 0) {
    idx = *diCount;
    ++*diCount;
    di_data->diskInfo = (di_disk_info_t *) di_realloc (
        (char *) di_data->diskInfo,
        sizeof (di_disk_info_t) * (Size_t) (*diCount + 1));
    if (di_data->diskInfo == (di_disk_info_t *) NULL) {
      fprintf (stderr, "malloc failed for diskInfo. errno %d\n", errno);
      return -1;
    }
    diptr = di_data->diskInfo + idx;
    di_initialize_disk_info (diptr, idx);

    stpecpy (diptr->strdata [DI_DISP_FILESYSTEM],
        diptr->strdata [DI_DISP_FILESYSTEM] + DI_FILESYSTEM_LEN,
        mntEntry.mnt_special);
    stpecpy (diptr->strdata [DI_DISP_MOUNTPT],
             diptr->strdata [DI_DISP_MOUNTPT] + DI_MOUNTPT_LEN,
             mntEntry.mnt_mountp);
    if (checkMountOptions (&mntEntry, DI_MNTOPT_IGNORE) != (char *) NULL) {
      diptr->printFlag = DI_PRNT_IGNORE;
      if (diopts->optval [DI_OPT_DEBUG] > 2) {
        printf ("mnt: ignore: mntopt 'ignore': %s\n",
            diptr->strdata [DI_DISP_MOUNTPT]);
      }
    }
    if ( (devp = checkMountOptions (&mntEntry, DI_MNTOPT_DEV)) !=
            (char *) NULL) {
      if (devp != mntEntry.mnt_mntopts) {
        --devp;
      }
      *devp = 0;   /* point to preceeding comma and cut off */
    }
    if (checkMountOptions (&mntEntry, DI_MNTOPT_RO) != (char *) NULL) {
      diptr->isReadOnly = true;
    }
    stpecpy (diptr->strdata [DI_DISP_MOUNTOPT],
        diptr->strdata [DI_DISP_MOUNTOPT] + DI_MOUNTOPT_LEN,
        mntEntry.mnt_mntopts);

    /* get the file system type now... */
    stpecpy (diptr->strdata [DI_DISP_FSTYPE],
        diptr->strdata [DI_DISP_FSTYPE] + DI_FSTYPE_LEN,
        mntEntry.mnt_fstype);
    if (diopts->optval [DI_OPT_DEBUG] > 2) {
      printf ("mnt:%s - %s\n", diptr->strdata [DI_DISP_MOUNTPT], diptr->strdata [DI_DISP_FSTYPE]);
    }

    if (diopts->optval [DI_OPT_DEBUG] > 1) {
      printf ("mnt:%s - %s\n", diptr->strdata [DI_DISP_MOUNTPT], diptr->strdata [DI_DISP_FILESYSTEM]);
    }
  }

  fclose (f);
  return 0;
}

static char *
checkMountOptions (struct mnttab *mntEntry, char *str)
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
    && ! _lib_GetLogicalDriveStrings

/*
 * di_get_disk_entries
 *
 * SunOS supplies an open and close routine for the mount table.
 *
 */

#if ! defined (MNTTYPE_IGNORE)
# define MNTTYPE_IGNORE "ignore"
#endif

int
di_get_disk_entries (di_data_t *di_data, int *diCount)
{
  di_disk_info_t    *diptr;
  FILE            *f;
  int             idx;
  struct mntent   *mntEntry;
  char            *devp;   /* local ptr to dev entry */
  di_opt_t        *diopts;

  diopts = (di_opt_t *) di_data->options;

  if (diopts->optval [DI_OPT_DEBUG] > 0) { printf ("# di_get_disk_entries: set/get/endmntent\n"); }
/* if both are set not an ansi compiler... */
#if _args_setmntent == 1
  if ( (f = setmntent (DI_MOUNT_FILE)) == (FILE *) NULL) {
#else
  if ( (f = setmntent (DI_MOUNT_FILE, "r")) == (FILE *) NULL) {
#endif
    fprintf (stderr, "Unable to open: %s errno %d\n", DI_MOUNT_FILE, errno);
    return -1;
  }

  while ( (mntEntry = getmntent (f)) != (struct mntent *) NULL) {
    idx = *diCount;
    ++*diCount;
    di_data->diskInfo = (di_disk_info_t *) di_realloc (
        (char *) di_data->diskInfo,
        sizeof (di_disk_info_t) * (Size_t) (*diCount + 1));
    if (di_data->diskInfo == (di_disk_info_t *) NULL) {
      fprintf (stderr, "malloc failed for diskInfo. errno %d\n", errno);
      return -1;
    }
    diptr = di_data->diskInfo + idx;
    di_initialize_disk_info (diptr, idx);

    stpecpy (diptr->strdata [DI_DISP_FILESYSTEM],
        diptr->strdata [DI_DISP_FILESYSTEM] + DI_FILESYSTEM_LEN,
        mntEntry->mnt_fsname);
    stpecpy (diptr->strdata [DI_DISP_MOUNTPT],
        diptr->strdata [DI_DISP_MOUNTPT] + DI_MOUNTPT_LEN,
        mntEntry->mnt_dir);
    stpecpy (diptr->strdata [DI_DISP_FSTYPE],
        diptr->strdata [DI_DISP_FSTYPE] + DI_FSTYPE_LEN,
        mntEntry->mnt_type);

    if (strcmp (mntEntry->mnt_fsname, "none") == 0) {
      diptr->printFlag = DI_PRNT_IGNORE;
      if (diopts->optval [DI_OPT_DEBUG] > 2) {
        printf ("mnt: ignore: special 'none': %s\n", diptr->strdata [DI_DISP_MOUNTPT]);
      }
    }

    if (strcmp (mntEntry->mnt_type, MNTTYPE_IGNORE) == 0) {
      diptr->printFlag = DI_PRNT_IGNORE;
      if (diopts->optval [DI_OPT_DEBUG] > 2) {
        printf ("mnt: ignore: mntopt 'ignore': %s\n",
            diptr->strdata [DI_DISP_MOUNTPT]);
      }
    }

    if ( (devp = strstr (mntEntry->mnt_opts, "dev=")) != (char *) NULL) {
      if (devp != mntEntry->mnt_opts) {
        --devp;
      }
      *devp = 0;   /* point to preceeding comma and cut off */
    }
    if (chkMountOptions (mntEntry->mnt_opts, DI_MNTOPT_RO) != (char *) NULL) {
      diptr->isReadOnly = true;
    }
    stpecpy (diptr->strdata [DI_DISP_MOUNTOPT],
        diptr->strdata [DI_DISP_MOUNTOPT] + DI_MOUNTOPT_LEN,
        mntEntry->mnt_opts);

    if (diopts->optval [DI_OPT_DEBUG] > 1) {
      printf ("mnt:%s - %s : %s\n", diptr->strdata [DI_DISP_MOUNTPT],
          diptr->strdata [DI_DISP_FILESYSTEM], diptr->strdata [DI_DISP_FSTYPE]);
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
    && defined (__QNX__)

/*
 * di_get_disk_entries
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
di_get_disk_entries (di_data_t *di_data, int *diCount)
{
  di_opt_t        *diopts;

  diopts = (di_opt_t *) di_data->options;

  if (diopts->optval [DI_OPT_DEBUG] > 0) { printf ("# di_get_disk_entries: QNX\n"); }
  return di_getQNXDiskEntries (di_data, "/proc/mount", diCount);
}

static int
di_getQNXDiskEntries (di_data_t *di_data, char *ipath, int *diCount)
{
  di_disk_info_t  *diptr;
  int             idx;
  char            path [MAXPATHLEN];
  char            *p;
  char            *pathend;
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
  char            tfilesystem [DI_FILESYSTEM_LEN];
  di_opt_t        *diopts;

  diopts = (di_opt_t *) di_data->options;

  if (strcmp (ipath, "/proc/mount/dev") == 0) {
    return 0;
  }

  p = path;
  pathend = path + MAXPATHLEN;
  p = stpecpy (p, pathend, ipath);
  len = (int) strlen (path);

  if (! (dirp = opendir (path))) {
    return 0;
  }
  while ( (dent = readdir (dirp))) {
    if (strcmp (dent->d_name, ".") == 0 || strcmp (dent->d_name, "..") == 0) {
      continue;
    }

    ret = sscanf (dent->d_name, "%d,%d,%d,%d,%d",
        &nodeid, &pid, &chid, &handle, &ftype);

    if (len + (int) strlen (dent->d_name) + 1 > MAXPATHLEN) {
      continue;
    }

    p = stpecpy (p, pathend, "/");
    p = stpecpy (p, pathend, dent->d_name);
    if (diopts->optval [DI_OPT_DEBUG] > 4) { printf ("check: %s\n", path); }

    memset (&statinfo, 0, sizeof (statinfo));

    if (stat (path, &statinfo) == -1) {
      continue;
    }

    if (ret != 5) {
      if (S_ISDIR (statinfo.st_mode)) {
        if (diopts->optval [DI_OPT_DEBUG] > 4) { printf ("into: %s\n", path); }
        di_getQNXDiskEntries (di_data, path, diCount);
      }
      continue;
    }

    if (ftype != _FTYPE_ANY) {
      continue;
    }

    *tfilesystem = '\0';
    if (S_ISDIR (statinfo.st_mode) && ftype == _FTYPE_ANY) {
      if ( (fd = open (path, /* O_ACCMODE */ O_RDONLY | O_NOCTTY)) != -1) {
        devctl (fd, DCMD_FSYS_MOUNTED_ON, tfilesystem, DI_FILESYSTEM_LEN, 0);
        close (fd);
        if (*tfilesystem == '\0') {
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
    di_data->diskInfo = (di_disk_info_t *) di_realloc (
        (char *) di_data->diskInfo,
        sizeof (di_disk_info_t) * (Size_t) (*diCount + 1));
    if (di_data->diskInfo == (di_disk_info_t *) NULL) {
      fprintf (stderr, "malloc failed for diskInfo. errno %d\n", errno);
      return -1;
    }
    diptr = di_data->diskInfo + idx;
    di_initialize_disk_info (diptr, idx);

    path [len] = '\0';
    stpecpy (diptr->strdata [DI_DISP_FILESYSTEM],
        diptr->strdata [DI_DISP_FILESYSTEM] + DI_FILESYSTEM_LEN,
        tfilesystem);
    stpecpy (diptr->strdata [DI_DISP_MOUNTPT],
        diptr->strdata [DI_DISP_MOUNTPT] + DI_MOUNTPT_LEN,
        path + 11);
    if (*diptr->strdata [DI_DISP_MOUNTPT] == '\0') {
      stpecpy (diptr->strdata [DI_DISP_MOUNTPT],
          diptr->strdata [DI_DISP_MOUNTPT] + DI_MOUNTPT_LEN,
          "/");
    }
    if (diopts->optval [DI_OPT_DEBUG] > 4) { printf ("found: %s %s\n", diptr->strdata [DI_DISP_FILESYSTEM], diptr->strdata [DI_DISP_MOUNTPT]); }
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
    && ! defined (__QNX__)

/*
 * di_get_disk_entries
 *
 * For SysV.3 we open the file and read it ourselves.
 *
 */

int
di_get_disk_entries (di_data_t *di_data, int *diCount)
{
  di_disk_info_t    *diptr;
  FILE             *f;
  int              idx;
  struct mnttab    mntEntry;
  di_opt_t        *diopts;

  diopts = (di_opt_t *) di_data->options;

  if (diopts->optval [DI_OPT_DEBUG] > 0) { printf ("# di_get_disk_entries: not anything; sys v.3\n"); }
  if ( (f = fopen (DI_MOUNT_FILE, "r")) == (FILE *) NULL) {
    fprintf (stderr, "Unable to open: %s errno %d\n", DI_MOUNT_FILE, errno);
    return -1;
  }

  while (fread ( (char *) &mntEntry, sizeof (struct mnttab), 1, f) == 1) {
        /* xenix allows null mount table entries */
        /* sco nfs background mounts are marked as "nothing" */
    if (mntEntry.mt_filsys [0] &&
            strcmp (mntEntry.mt_filsys, "nothing") != 0) {
      idx = *diCount;
      ++*diCount;
      di_data->diskInfo = (di_disk_info_t *) di_realloc (
          (char *) di_data->diskInfo,
          sizeof (di_disk_info_t) * (*diCount + 1));
      if (di_data->diskInfo == (di_disk_info_t *) NULL) {
        fprintf (stderr, "malloc failed for diskInfo. errno %d\n", errno);
        return -1;
      }
      diptr = di_data->diskInfo + idx;
      di_initialize_disk_info (diptr, idx);

# if defined (COHERENT)
      /* Coherent seems to have these fields reversed. oh well. */
      stpecpy (diptr->strdata [DI_DISP_MOUNTPT],
          diptr->strdata [DI_DISP_MOUNTPT] + DI_MOUNTPT_LEN,
          mntEntry.mt_dev);
      stpecpy (diptr->strdata [DI_DISP_FILESYSTEM],
          diptr->strdata [DI_DISP_FILESYSTEM] + DI_FILESYSTEM_LEN,
          mntEntry.mt_filsys);
# else
      stpecpy (diptr->strdata [DI_DISP_MOUNTPT],
          diptr->strdata [DI_DISP_MOUNTPT] + DI_MOUNTPT_LEN,
          mntEntry.mt_filsys);
      stpecpy (diptr->strdata [DI_DISP_FILESYSTEM],
          diptr->strdata [DI_DISP_FILESYSTEM] + DI_FILESYSTEM_LEN,
          mntEntry.mt_dev);
# endif
# if _mem_struct_mnttab_mntopts
      stpecpy (diptr->strdata [DI_DISP_MOUNTOPT],
          diptr->strdata [DI_DISP_MOUNTOPT] + DI_MOUNTOPT_LEN,
          mntEntry.mt_mntopts);
# endif
    }

    if (diopts->optval [DI_OPT_DEBUG] > 1)
    {
        printf ("mnt:%s - %s\n", diptr->strdata [DI_DISP_MOUNTPT],
                diptr->strdata [DI_DISP_FILESYSTEM]);
    }
  }

  fclose (f);
  return 0;
}

#endif /* Sys V.3 */

/*
 * All of the following routines also replace di_get_disk_info ()
 */

#if _lib_getfsstat \
    && ! (_lib_getvfsstat && _args_getvfsstat == 3)

/*
 * di_get_disk_entries
 *
 * OSF/1 / Digital Unix / Compaq Tru64 / FreeBSD / NetBSD 2.x / OpenBSD
 *  MacOS
 *
 */

# if _dcl_mnt_names
#  if ! defined (MNT_NUMTYPES)
#   define MNT_NUMTYPES (sizeof (mnt_names)/sizeof (char *))
#  endif
# endif

int
di_get_disk_entries (di_data_t *di_data, int *diCount)
{
  di_disk_info_t  *diptr;
  int             count;
  int             idx;
# if _dcl_mnt_names && _mem_struct_statfs_f_type
  short           fstype;
# endif
  _c_arg_2_getfsstat bufsize;
  struct statfs   *mntbufp;
  struct statfs   *sp;
  di_opt_t        *diopts;

  diopts = (di_opt_t *) di_data->options;

  if (diopts->optval [DI_OPT_DEBUG] > 0) { printf ("# di_get_disk_entries: getfsstat\n"); }
  count = getfsstat ( (struct statfs *) NULL, (_c_arg_2_getfsstat) 0, MNT_NOWAIT);
  if (count < 1) {
    fprintf (stderr, "Unable to do getfsstat () errno %d\n", errno);
    return -1;
  }
  bufsize = (_c_arg_2_getfsstat) (sizeof (struct statfs) * (Size_t) count);
  mntbufp = (struct statfs *) malloc ( (Size_t) bufsize);
  if (mntbufp == (struct statfs *) NULL) {
    fprintf (stderr, "malloc failed for mntbufp. errno %d\n", errno);
    return -1;
  }
  memset ( (char *) mntbufp, '\0', sizeof (struct statfs) * (Size_t) count);
  count = getfsstat (mntbufp, bufsize, MNT_NOWAIT);

  *diCount = count;
  di_data->diskInfo = (di_disk_info_t *) malloc (sizeof (di_disk_info_t) * (Size_t) (count + 1));
  if (di_data->diskInfo == (di_disk_info_t *) NULL) {
    fprintf (stderr, "malloc failed for diskInfo. errno %d\n", errno);
    return -1;
  }
  memset ((char *) di_data->diskInfo, '\0', sizeof (di_disk_info_t) * (Size_t) count);

  for (idx = 0; idx < count; idx++) {
    di_ui_t      tblocksz = 0;

    diptr = di_data->diskInfo + idx;
    di_initialize_disk_info (diptr, idx);

    sp = mntbufp + idx;
# if defined (MNT_RDONLY)
    if ( (sp->f_flags & MNT_RDONLY) == MNT_RDONLY) {
      diptr->isReadOnly = true;
    }
# endif
# if defined (MNT_LOCAL)
    if ( (sp->f_flags & MNT_LOCAL) != MNT_LOCAL) {
      diptr->isLocal = false;
    }
# endif
    convertMountOptions ( (unsigned long) sp->f_flags, diptr);
# if _mem_struct_statfs_f_type
#  if defined (MOUNT_NFS3)
    if (sp->f_type == MOUNT_NFS3) {
      strncat (diptr->strdata [DI_DISP_MOUNTOPT], "v3,",
          DI_MOUNTOPT_LEN - strlen (diptr->strdata [DI_DISP_MOUNTOPT]) - 1);
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
    ) {
      struct nfs_args *na;
      na = &sp->mount_info.nfs_args;
      convertNFSMountOptions (na->flags, na->wsize, na->rsize, diptr);
    }
# endif
    di_trimchar (diptr->strdata [DI_DISP_MOUNTOPT], ',');

    stpecpy (diptr->strdata [DI_DISP_FILESYSTEM],
        diptr->strdata [DI_DISP_FILESYSTEM] + DI_FILESYSTEM_LEN,
        sp->f_mntfromname);
    stpecpy (diptr->strdata [DI_DISP_MOUNTPT],
        diptr->strdata [DI_DISP_MOUNTPT] + DI_MOUNTPT_LEN,
        sp->f_mntonname);

# if _mem_struct_statfs_f_fsize
    tblocksz = (di_ui_t) sp->f_fsize;
# endif
# if _mem_struct_statfs_f_bsize && ! _mem_struct_statfs_f_fsize
    tblocksz = (di_ui_t) sp->f_bsize;
# endif
# if ! _mem_struct_statfs_f_bsize && ! _mem_struct_statfs_f_fsize
#  error "struct statfs location failure"
# endif
    di_save_block_sizes (diptr, tblocksz, (di_ui_t) sp->f_blocks,
        (di_ui_t) sp->f_bfree, (di_ui_t) sp->f_bavail);
    di_save_inode_sizes (diptr, (di_ui_t) sp->f_files,
        (di_ui_t) sp->f_ffree, (di_ui_t) sp->f_ffree);
# if _mem_struct_statfs_f_fstypename
    stpecpy (diptr->strdata [DI_DISP_FSTYPE],
        diptr->strdata [DI_DISP_FSTYPE] + DI_FSTYPE_LEN,
        sp->f_fstypename);
# else
#  if _lib_sysfs && _mem_struct_statfs_f_type
     sysfs (GETFSTYP, sp->f_type, diptr->strdata [DI_DISP_FSTYPE]);
# else
#  if _dcl_mnt_names && _mem_struct_statfs_f_type
#    define DI_UNKNOWN_FSTYPE       " (%.2d)?"
    fstype = sp->f_type;
    if ( (fstype >= 0) && (fstype < MNT_NUMTYPES)) {
      stpecpy (diptr->strdata [DI_DISP_FSTYPE],
          diptr->strdata [DI_DISP_FSTYPE] + DI_FSTYPE_LEN,
          mnt_names [fstype]);
    } else {
      Snprintf1 (diptr->strdata [DI_DISP_FSTYPE], DI_FSTYPE_LEN,
          DI_UNKNOWN_FSTYPE, fstype);
    }
#   endif
#  endif
# endif
  }

  free ( (char *) mntbufp);
  return 0;
}

#endif /* _lib_getfsstat */

#if _lib_getmntinfo \
    && ! _lib_getfsstat \
    && ! _lib_getvfsstat

/*
 * di_get_disk_entries
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
di_get_disk_entries (di_data_t *di_data, int *diCount)
{
  di_disk_info_t     *diptr;
  int             count;
  int             idx;
  short           fstype;
  struct statfs   *mntbufp;
  di_opt_t        *diopts;

  diopts = (di_opt_t *) di_data->options;

  if (diopts->optval [DI_OPT_DEBUG] > 0) { printf ("# di_get_disk_entries: getmntinfo\n"); }
  count = getmntinfo (&mntbufp, MNT_WAIT);
  if (count < 1) {
    fprintf (stderr, "Unable to do getmntinfo () errno %d\n", errno);
    return -1;
  }

  *diCount = count;
  di_data->diskInfo = (di_disk_info_t *) malloc (sizeof (di_disk_info_t) * (Size_t) (count + 1));
  if (di_data->diskInfo == (di_disk_info_t *) NULL) {
    fprintf (stderr, "malloc failed for diskInfo. errno %d\n", errno);
    return -1;
  }
  memset ((char *) di_data->diskInfo, '\0', sizeof (di_disk_info_t) * (Size_t) count);

  if (diopts->optval [DI_OPT_DEBUG] > 1) {
    printf ("type_len %d name_len %d spec_name_len %d\n", DI_FSTYPE_LEN,
        DI_MOUNTPT_LEN, DI_FILESYSTEM_LEN);
  }

  for (idx = 0; idx < count; idx++) {
    di_ui_t    tblocksz;

    diptr = di_data->diskInfo + idx;
    di_initialize_disk_info (diptr, idx);
# if defined (MNT_LOCAL)
    if ( (mntbufp [idx].f_flags & MNT_LOCAL) != MNT_LOCAL) {
      diptr->isLocal = false;
    }
# endif

    stpecpy (diptr->strdata [DI_DISP_FILESYSTEM],
        diptr->strdata [DI_DISP_FILESYSTEM] + DI_FILESYSTEM_LEN,
        mntbufp [idx].f_mntfromname);
    stpecpy (diptr->strdata [DI_DISP_MOUNTPT],
        diptr->strdata [DI_DISP_MOUNTPT] + DI_MOUNTPT_LEN,
        mntbufp [idx].f_mntonname);

    tblocksz = 1024;

# if _mem_struct_statfs_f_fsize /* OSF 1.x */
    tblocksz = mntbufp [idx].f_fsize;
# endif
# if _mem_struct_statfs_f_bsize /* OSF 2.x */
    tblocksz = mntbufp [idx].f_bsize;
# endif
    di_save_block_sizes (diptr, tblocksz, (di_ui_t) mntbufp [idx].f_blocks,
        (di_ui_t) mntbufp [idx].f_bfree, (di_ui_t) mntbufp [idx].f_bavail);
    di_save_inode_sizes (diptr, (di_ui_t) mntbufp [idx].f_files,
        (di_ui_t) mntbufp [idx].f_ffree, (di_ui_t) mntbufp [idx].f_ffree);

    fstype = mntbufp [idx].f_type;
# if ! _sys_fs_types && ! defined (INITMOUNTNAMES) \
&& ! _mem_struct_statfs_f_fstypename
    if ( (fstype >= 0) && (fstype <= MOUNT_MAXTYPE)) {
      switch (fstype) {
#  if defined (MOUNT_NONE)
        case MOUNT_NONE: {         /* No Filesystem */
          stpecpy (diptr->strdata [DI_DISP_FSTYPE],
              diptr->strdata [DI_DISP_FSTYPE] + DI_FSTYPE_LEN,
              "none");
          break;
        }
#  endif
#  if defined (MOUNT_UFS)
        case MOUNT_UFS: {         /* UNIX "Fast" Filesystem */
          stpecpy (diptr->strdata [DI_DISP_FSTYPE],
              diptr->strdata [DI_DISP_FSTYPE] + DI_FSTYPE_LEN,
              "ufs");
          break;
        }
#  endif
#  if defined (MOUNT_NFS)
        case MOUNT_NFS: {         /* Network Filesystem */
          stpecpy (diptr->strdata [DI_DISP_FSTYPE],
              diptr->strdata [DI_DISP_FSTYPE] + DI_FSTYPE_LEN,
              "nfs");
          break;
        }
#  endif
#  if defined (MOUNT_MFS)
        case MOUNT_MFS: {         /* Memory Filesystem */
          stpecpy (diptr->strdata [DI_DISP_FSTYPE],
              diptr->strdata [DI_DISP_FSTYPE] + DI_FSTYPE_LEN,
              "mfs");
          break;
        }
#  endif
#  if defined (MOUNT_MSDOS)
        case MOUNT_MSDOS: {       /* MSDOS Filesystem */
          stpecpy (diptr->strdata [DI_DISP_FSTYPE],
              diptr->strdata [DI_DISP_FSTYPE] + DI_FSTYPE_LEN,
              "msdos");
          break;
        }
#  endif
#  if defined (MOUNT_LFS)
        case MOUNT_LFS: {
          stpecpy (diptr->strdata [DI_DISP_FSTYPE],
              diptr->strdata [DI_DISP_FSTYPE] + DI_FSTYPE_LEN,
              "lfs");
          break;
        }
#  endif
#  if defined (MOUNT_LOFS)
        case MOUNT_LOFS: {
          stpecpy (diptr->strdata [DI_DISP_FSTYPE],
              diptr->strdata [DI_DISP_FSTYPE] + DI_FSTYPE_LEN,
              "lofs");
          break;
        }
#  endif
#  if defined (MOUNT_FDESC)
        case MOUNT_FDESC: {
          stpecpy (diptr->strdata [DI_DISP_FSTYPE],
              diptr->strdata [DI_DISP_FSTYPE] + DI_FSTYPE_LEN,
              "fdesc");
          break;
        }
#  endif
#  if defined (MOUNT_PORTAL)
        case MOUNT_PORTAL: {
          stpecpy (diptr->strdata [DI_DISP_FSTYPE],
              diptr->strdata [DI_DISP_FSTYPE] + DI_FSTYPE_LEN,
                "portal");
            break;
        }
#  endif
#  if defined (MOUNT_NULL)
        case MOUNT_NULL: {
          stpecpy (diptr->strdata [DI_DISP_FSTYPE],
              diptr->strdata [DI_DISP_FSTYPE] + DI_FSTYPE_LEN,
              "null");
          break;
        }
#  endif
#  if defined (MOUNT_UMAP)
        case MOUNT_UMAP: {
          stpecpy (diptr->strdata [DI_DISP_FSTYPE],
              diptr->strdata [DI_DISP_FSTYPE] + DI_FSTYPE_LEN,
              "umap");
          break;
        }
#  endif
#  if defined (MOUNT_KERNFS)
        case MOUNT_KERNFS: {
          stpecpy (diptr->strdata [DI_DISP_FSTYPE],
              diptr->strdata [DI_DISP_FSTYPE] + DI_FSTYPE_LEN,
              "kernfs");
          break;
        }
#  endif
#  if defined (MOUNT_PROCFS)
        case MOUNT_PROCFS: {      /* proc filesystem */
          stpecpy (diptr->strdata [DI_DISP_FSTYPE],
              diptr->strdata [DI_DISP_FSTYPE] + DI_FSTYPE_LEN,
              "pfs");
          break;
        }
#  endif
#  if defined (MOUNT_AFS)
        case MOUNT_AFS: {
          stpecpy (diptr->strdata [DI_DISP_FSTYPE],
              diptr->strdata [DI_DISP_FSTYPE] + DI_FSTYPE_LEN,
              "afs");
          break;
        }
#  endif
#  if defined (MOUNT_ISOFS)
        case MOUNT_ISOFS: {       /* iso9660 cdrom */
          stpecpy (diptr->strdata [DI_DISP_FSTYPE],
              diptr->strdata [DI_DISP_FSTYPE] + DI_FSTYPE_LEN,
              "iso9660fs");
          break;
        }
#  endif
#  if defined (MOUNT_ISO9660) && ! defined (MOUNT_CD9660)
        case MOUNT_ISO9660: {       /* iso9660 cdrom */
          stpecpy (diptr->strdata [DI_DISP_FSTYPE],
              diptr->strdata [DI_DISP_FSTYPE] + DI_FSTYPE_LEN,
                "iso9660");
            break;
        }
#  endif
#  if defined (MOUNT_CD9660)
        case MOUNT_CD9660: {       /* iso9660 cdrom */
          stpecpy (diptr->strdata [DI_DISP_FSTYPE],
              diptr->strdata [DI_DISP_FSTYPE] + DI_FSTYPE_LEN,
              "cd9660");
          break;
        }
#  endif
#  if defined (MOUNT_UNION)
        case MOUNT_UNION: {
          stpecpy (diptr->strdata [DI_DISP_FSTYPE],
              diptr->strdata [DI_DISP_FSTYPE] + DI_FSTYPE_LEN,
              "union");
          break;
        }
#  endif
      } /* switch on mount type */
    }
# else
#  if _mem_struct_statfs_f_fstypename
    stpecpy (diptr->strdata [DI_DISP_FSTYPE],
        diptr->strdata [DI_DISP_FSTYPE] + DI_FSTYPE_LEN,
        mntbufp [idx].f_fstypename);
#  else
#   define DI_UNKNOWN_FSTYPE       " (%.2d)?"

        /* could use getvfsbytype here... */
    if ( (fstype >= 0) && (fstype < MNT_NUMTYPES)) {
      stpecpy (diptr->strdata [DI_DISP_FSTYPE],
          diptr->strdata [DI_DISP_FSTYPE] + DI_FSTYPE_LEN,
          mnt_names [fstype]);
    } else {
      Snprintf1 (diptr->strdata [DI_DISP_FSTYPE], DI_FSTYPE_LEN,
          DI_UNKNOWN_FSTYPE, fstype);
    }
#  endif
# endif /* else has sys/fs_types.h */

    diptr->isReadOnly = false;
# if defined (MNT_RDONLY)
    if ( (mntbufp [idx].f_flags & MNT_RDONLY) == MNT_RDONLY) {
      diptr->isReadOnly = true;
    }
# endif
    convertMountOptions ( (unsigned long) mntbufp [idx].f_flags, diptr);
    di_trimchar (diptr->strdata [DI_DISP_MOUNTOPT], ',');

    if (diopts->optval [DI_OPT_DEBUG] > 1) {
      printf ("%s: %s\n", diptr->strdata [DI_DISP_MOUNTPT], diptr->strdata [DI_DISP_FSTYPE]);
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

  free ( (char *) mntbufp);  /* man page says this can't be freed. */
                            /* is it ok to try?                   */
  return 0;
}

#endif /* _lib_getmntinfo */

#if _lib_getvfsstat && _args_getvfsstat == 3

/*
 * di_get_disk_entries
 *
 * NetBSD
 *
 */

int
di_get_disk_entries (di_data_t *di_data, int *diCount)
{
  di_disk_info_t     *diptr;
  int             count;
  int             idx;
  Size_t          bufsize;
  struct statvfs  *mntbufp;
  struct statvfs  *sp;
  di_opt_t        *diopts;

  diopts = (di_opt_t *) di_data->options;

  if (diopts->optval [DI_OPT_DEBUG] > 0) { printf ("# di_get_disk_entries: getvfsstat\n"); }
  count = getvfsstat ( (struct statvfs *) NULL, 0, ST_NOWAIT);
  if (count < 1) {
    fprintf (stderr, "Unable to do getvfsstat () errno %d\n", errno);
    return -1;
  }
  bufsize = sizeof (struct statvfs) * (Size_t) count;
  mntbufp = (struct statvfs *) malloc ( (Size_t) bufsize);
  if (mntbufp == (struct statvfs *) NULL) {
    fprintf (stderr, "malloc failed for mntbufp. errno %d\n", errno);
    return -1;
  }
  memset ( (char *) mntbufp, '\0', sizeof (struct statvfs) * (Size_t) count);
  count = getvfsstat (mntbufp, (Size_t) bufsize, ST_NOWAIT);

  *diCount = count;
  di_data->diskInfo = (di_disk_info_t *) malloc (sizeof (di_disk_info_t) * (Size_t) (count + 1));
  if (di_data->diskInfo == (di_disk_info_t *) NULL) {
    fprintf (stderr, "malloc failed for diskInfo. errno %d\n", errno);
    return -1;
  }
  memset ((char *) di_data->diskInfo, '\0', sizeof (di_disk_info_t) * (Size_t) count);

  for (idx = 0; idx < count; idx++) {
    di_ui_t    tblocksz;

    diptr = di_data->diskInfo + idx;
    di_initialize_disk_info (diptr, idx);

    sp = mntbufp + idx;

# if defined (MNT_RDONLY)
    if ( (sp->f_flag & MNT_RDONLY) == MNT_RDONLY) {
      diptr->isReadOnly = true;
    }
# endif
# if defined (MNT_LOCAL)
    if ( (sp->f_flag & MNT_LOCAL) != MNT_LOCAL) {
      diptr->isLocal = false;
    }
# endif
    convertMountOptions ( (unsigned long) sp->f_flag, diptr);
    di_trimchar (diptr->strdata [DI_DISP_MOUNTOPT], ',');

    if (sp->f_frsize == 0 && sp->f_bsize != 0) {
      tblocksz = sp->f_bsize;
    } else {
      tblocksz = sp->f_frsize;
    }

    di_save_block_sizes (diptr, tblocksz, (di_ui_t) sp->f_blocks,
        (di_ui_t) sp->f_bfree, (di_ui_t) sp->f_bavail);
    di_save_inode_sizes (diptr, (di_ui_t) sp->f_files,
        (di_ui_t) sp->f_ffree, (di_ui_t) sp->f_ffree);

    stpecpy (diptr->strdata [DI_DISP_FILESYSTEM],
        diptr->strdata [DI_DISP_FILESYSTEM] + DI_FILESYSTEM_LEN,
        sp->f_mntfromname);
    stpecpy (diptr->strdata [DI_DISP_MOUNTPT],
        diptr->strdata [DI_DISP_MOUNTPT] + DI_MOUNTPT_LEN,
        sp->f_mntonname);
    stpecpy (diptr->strdata [DI_DISP_FSTYPE],
        diptr->strdata [DI_DISP_FSTYPE] + DI_FSTYPE_LEN,
        sp->f_fstypename);

    if (diopts->optval [DI_OPT_DEBUG] > 1) {
      printf ("%s: %s\n", diptr->strdata [DI_DISP_MOUNTPT], diptr->strdata [DI_DISP_FSTYPE]);
      printf ("\tbsize:%ld  frsize:%ld\n", (long) sp->f_bsize,
          (long) sp->f_frsize);
      printf ("\tblocks: tot:%lu free:%ld avail:%ld\n",
          (unsigned long) sp->f_blocks, (unsigned long) sp->f_bfree,
          (unsigned long) sp->f_bavail);
      printf ("\tinodes: tot:%lu free:%lu avail:%lu\n",
          (unsigned long) sp->f_files, (unsigned long) sp->f_ffree,
          (unsigned long) sp->f_favail);
    }
  }

  free ( (char *) mntbufp);
  return 0;
}

#endif /* _lib_getvfsstat */

#if _lib_getmnt

# if _npt_getmnt
  int getmnt (int *, struct fs_data *, int, int, char *);
# endif

/*
 * di_get_disk_entries
 *
 * ULTRIX does this with a system call.  The system call allows one
 * to retrieve the information in a series of calls, but coding that
 * looks a little tricky; I just allocate a huge buffer and do it in
 * one shot.
 *
 *                  [mogul@wrl.dec.com (Jeffrey Mogul)]
 */

int
di_get_disk_entries (di_data_t *di_data, int *diCount)
{
  di_disk_info_t     *diptr;
  int             count;
  int             bufsize;
  int             idx;
  short           fstype;
  struct fs_data  *fsdbuf;
  int             start;
  int             len;
  di_opt_t        *diopts;

  diopts = (di_opt_t *) di_data->options;

  if (diopts->optval [DI_OPT_DEBUG] > 0) { printf ("# di_get_disk_entries: getmnt\n"); }
  bufsize = NMOUNT * sizeof (struct fs_data);  /* enough for max # mounts */
  fsdbuf = (struct fs_data *) malloc ( (Size_t) bufsize);
  if (fsdbuf == (struct fs_data *) NULL)
  {
    fprintf (stderr, "malloc (%d) for getmnt () failed errno %d\n",
        bufsize, errno);
    return -1;
  }

  start = 0;
  count = getmnt (&start, fsdbuf, bufsize, STAT_MANY, 0);
  if (count < 1) {
    fprintf (stderr, "Unable to do getmnt () [= %d] errno %d\n",
        count, errno);
    free ( (char *) fsdbuf);
    return -1;
  }

  *diCount = count;
  di_data->diskInfo = (di_disk_info_t *) malloc (sizeof (di_disk_info_t) * (Size_t) (count + 1));
  if (di_data->diskInfo == (di_disk_info_t *) NULL) {
    fprintf (stderr, "malloc failed for diskInfo. errno %d\n", errno);
    free ( (char *) fsdbuf);
    return -1;
  }
  memset ((char *) di_data->diskInfo, '\0', sizeof (di_disk_info_t) * count);

  for (idx = 0; idx < count; idx++) {
    diptr = di_data->diskInfo + idx;
    di_initialize_disk_info (diptr, idx);

    if ( (fsdbuf [idx].fd_req.flags & MNT_LOCAL) != MNT_LOCAL) {
      diptr->isLocal = false;
    }

    stpecpy (diptr->strdata [DI_DISP_FILESYSTEM],
        diptr->strdata [DI_DISP_FILESYSTEM] + DI_FILESYSTEM_LEN,
        fsdbuf [idx].fd_filesystem);
    stpecpy (diptr->strdata [DI_DISP_MOUNTPT],
        diptr->strdata [DI_DISP_MOUNTPT] + DI_MOUNTPT_LEN,
        fsdbuf [idx].fd_path);

    /* ULTRIX keeps these fields in units of 1K byte */
    di_save_block_sizes (diptr, 1024, (di_ui_t) fsdbuf [idx].fd_btot,
        (di_ui_t) fsdbuf [idx].fd_bfree, (di_ui_t) fsdbuf [idx].fd_bfreen);
    di_save_inode_sizes (diptr, (di_ui_t) fsdbuf [idx].fd_gtot,
        (di_ui_t) fsdbuf [idx].fd_gfree, (di_ui_t) fsdbuf [idx].fd_gfree);

    fstype = fsdbuf [idx].fd_fstype;
    if (fstype == GT_UNKWN) {
      diptr->printFlag = DI_PRNT_IGNORE;
      if (diopts->optval [DI_OPT_DEBUG] > 2) {
        printf ("mnt: ignore: disk type unknown: %s\n",
            diptr->strdata [DI_DISP_MOUNTPT]);
      }
    }
    else if ( (fstype > 0) && (fstype < GT_NUMTYPES)) {
      stpecpy (diptr->strdata [DI_DISP_FSTYPE],
          diptr->strdata [DI_DISP_FSTYPE] + DI_FSTYPE_LEN,
          gt_names [fstype]);
    } else {
      Snprintf1 (diptr->strdata [DI_DISP_FSTYPE], DI_FSTYPE_LEN,
          "Unknown fstyp %.2d", fstype);
    }

    if ( (fsdbuf [idx].fd_req.flags & MNT_RDONLY) == MNT_RDONLY) {
      diptr->isReadOnly = true;
    } else {
      diptr->isReadOnly = false;
    }
    convertMountOptions ( (unsigned long) fsdbuf [idx].fd_req.flags, diptr);
    di_trimchar (diptr->strdata [DI_DISP_MOUNTOPT], ',');

    if (diopts->optval [DI_OPT_DEBUG] > 1)
    {
      printf ("%s: %s\n", diptr->strdata [DI_DISP_MOUNTPT], diptr->strdata [DI_DISP_FSTYPE]);
      printf ("\tblocks: tot:%ld free:%ld avail:%ld\n",
          fsdbuf [idx].fd_btot, fsdbuf [idx].fd_bfree,
          (int) fsdbuf [idx].fd_bfreen);
      printf ("\tinodes: tot:%ld free:%ld\n",
          fsdbuf [idx].fd_gtot, fsdbuf [idx].fd_gfree);
    }
  }

  free ( (char *) fsdbuf);
  return 0;
}

#endif /* _lib_getmnt */


#if _lib_mntctl

/*
 * di_get_disk_entries
 *
 * AIX uses mntctl to find out about mounted file systems
 * This seems to be better than set/get/end, as we get the
 * remote filesystem flag.
 *
 */

# define DI_FSMAGIC 10    /* base AIX configuration has 5 file systems */
# define NUM_AIX_FSTYPES         6
static char *AIX_fstype [NUM_AIX_FSTYPES] =
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
di_get_disk_entries (di_data_t *di_data, int *diCount)
{
  di_disk_info_t  *diptr = NULL;
  int             num;        /* number of vmount structs returned    */
  char            *vmbuf;     /* buffer for vmount structs returned   */
  Size_t          vmbufsz;    /* size in bytes of vmbuf               */
  int             i;          /* index for looping and stuff          */
  char            *bufp;      /* pointer into vmbuf                   */
  struct vmount   *vmtp;      /* pointer into vmbuf                   */
  struct vfs_ent  *ve;        /* pointer for file system type entry   */
  di_opt_t        *diopts;

  diopts = (di_opt_t *) di_data->options;

  if (diopts->optval [DI_OPT_DEBUG] > 0) { printf ("# di_get_disk_entries: mntctl\n"); }
  i = 0;
  vmbufsz = sizeof (struct vmount) * DI_FSMAGIC; /* initial vmount buffer */

  do {
    if ( (vmbuf = (char *) malloc (vmbufsz)) == (char *) NULL) {
      fprintf (stderr, "malloc (%d) for mntctl () failed errno %d\n",
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
    if (num == 0) {
      memcpy (&vmbufsz, vmbuf, sizeof (vmbufsz)); /* see mntctl (2) */
      if (diopts->optval [DI_OPT_DEBUG] > 0) {
        printf ("vmbufsz too small, new size: %d\n", (int) vmbufsz);
      }
      free ( (char *) vmbuf); /* free this last, it's still being used! */
      ++i;
    }
  } while (num == 0 && i < DI_RETRY_COUNT);

  if (i >= DI_RETRY_COUNT) {
    free ( (char *) vmbuf);
    fprintf (stderr, "unable to allocate adequate buffer for mntctl\n");
    return -1;
  }

  if (num == -1) {
    free ( (char *) vmbuf);
    fprintf (stderr, "%s errno %d\n", strerror (errno), errno);
    return -1;
  }

      /* <num> vmount structs returned in vmbuf */
  *diCount = num;
  di_data->diskInfo = (di_disk_info_t *) malloc (sizeof (di_disk_info_t) *
      (Size_t) (*diCount + 1));
  if (di_data->diskInfo == (di_disk_info_t *) NULL) {
    fprintf (stderr, "malloc failed for diskInfo. %s errno %d\n",
        strerror (errno), errno);
    return -1;
  }

  bufp = vmbuf;
  for (i = 0; i < num; i++) {
    char    *p;
    char    *end;

    diptr = di_data->diskInfo + i;
    di_initialize_disk_info (diptr, i);

    p = diptr->strdata [DI_DISP_FILESYSTEM];
    end = diptr->strdata [DI_DISP_FILESYSTEM] + DI_FILESYSTEM_LEN;

    vmtp = (struct vmount *) bufp;
    if ( (vmtp->vmt_flags & MNT_REMOTE) == MNT_REMOTE) {
      diptr->isLocal = false;
    }
    if ( (vmtp->vmt_flags & MNT_RDONLY) == MNT_RDONLY) {
      diptr->isReadOnly = true;
    }

    *diptr->strdata [DI_DISP_FILESYSTEM] = '\0';
    if (diptr->isLocal == false) {
      p = stpecpy (p, end, (char *) vmt2dataptr (vmtp, VMT_HOSTNAME));
      p = stpecpy (p, end, ":");
    }
    p = stpecpy (p, end, (char *) vmt2dataptr (vmtp, VMT_OBJECT));
    stpecpy (diptr->strdata [DI_DISP_MOUNTPT],
        diptr->strdata [DI_DISP_MOUNTPT] + DI_MOUNTPT_LEN,
        (char *) vmt2dataptr (vmtp, VMT_STUB));

    ve = getvfsbytype (vmtp->vmt_gfstype);
    if (ve == (struct vfs_ent *) NULL || *ve->vfsent_name == '\0') {
      if (vmtp->vmt_gfstype >= 0 &&
          (vmtp->vmt_gfstype < NUM_AIX_FSTYPES)) {
        stpecpy (diptr->strdata [DI_DISP_FSTYPE],
            diptr->strdata [DI_DISP_FSTYPE] + DI_FSTYPE_LEN,
            AIX_fstype [vmtp->vmt_gfstype]);
      }
    } else {
      stpecpy (diptr->strdata [DI_DISP_FSTYPE],
          diptr->strdata [DI_DISP_FSTYPE] + DI_FSTYPE_LEN,
          ve->vfsent_name);
    }

    stpecpy (diptr->strdata [DI_DISP_MOUNTOPT],
        diptr->strdata [DI_DISP_MOUNTOPT] + DI_MOUNTOPT_LEN,
        vmt2dataptr (vmtp, VMT_ARGS));
    di_trimchar (diptr->strdata [DI_DISP_MOUNTOPT], ',');
    bufp += vmtp->vmt_length;

    if (diopts->optval [DI_OPT_DEBUG] > 1) {
      printf ("mnt:%s - %s : %s\n", diptr->strdata [DI_DISP_MOUNTPT],
          diptr->strdata [DI_DISP_FILESYSTEM], diptr->strdata [DI_DISP_FSTYPE]);
      printf ("\t%s\n", (char *) vmt2dataptr (vmtp, VMT_ARGS));
    }
  }
  return 0;
}

#endif  /* _lib_mntctl */


#if _lib_GetDriveType \
    && _lib_GetLogicalDriveStrings

/*
 * di_get_disk_info
 *
 * Windows
 *
 */

# define MSDOS_BUFFER_SIZE          256
# define BYTES_PER_LOGICAL_DRIVE    4

int
di_get_disk_entries (di_data_t *di_data, int *diCount)
{
  di_disk_info_t     *diptr;
  int             i;
  char            diskflag;
  int             rc;
  char            *p;
  char            buff [MSDOS_BUFFER_SIZE];
  di_opt_t        *diopts;

  diopts = (di_opt_t *) di_data->options;

  if (diopts->optval [DI_OPT_DEBUG] > 0) { printf ("# di_get_disk_info: GetLogicalDriveStrings GetDriveType\n"); }
  diskflag = DI_PRNT_SKIP;
  rc = (int) GetLogicalDriveStrings (MSDOS_BUFFER_SIZE, buff);
  *diCount = rc / BYTES_PER_LOGICAL_DRIVE;

  di_data->diskInfo = (di_disk_info_t *)
      malloc (sizeof (di_disk_info_t) * (Size_t) (*diCount + 1));
  if (di_data->diskInfo == (di_disk_info_t *) NULL) {
    fprintf (stderr, "malloc failed for diskInfo. errno %d\n", errno);
    return -1;
  }

  for (i = 0; i < *diCount; ++i) {
    diptr = di_data->diskInfo + i;
    di_initialize_disk_info (diptr, i);

    p = buff + (BYTES_PER_LOGICAL_DRIVE * i);
    stpecpy (diptr->strdata [DI_DISP_MOUNTPT],
        diptr->strdata [DI_DISP_MOUNTPT] + DI_MOUNTPT_LEN,
        p);
    rc = (int) GetDriveType (p);
    diptr->printFlag = DI_PRNT_OK;

    if (rc == DRIVE_NO_ROOT_DIR) {
      diptr->printFlag = DI_PRNT_BAD;
    } else if (rc == DRIVE_REMOVABLE) {
      char    *hnp;
      char    *hnend;

      /* assume that any removable drives before the  */
      /* first non-removable disk are floppies...     */
      DWORD br;
      BOOL bSuccess;
      char handleName [MSDOS_BUFFER_SIZE];

      diptr->printFlag = diskflag;
      bSuccess = 1;
      hnp = handleName;
      hnend = handleName + MSDOS_BUFFER_SIZE;
      hnp = stpecpy (hnp, hnend, "\\\\.\\");
      hnp = stpecpy (hnp, hnend, p);

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

      if (! bSuccess) {
        diptr->printFlag = DI_PRNT_BAD;
      }
    } else {
      diskflag = DI_PRNT_OK;
    }

    if (rc != DRIVE_REMOTE) {
      diptr->isLocal = true;
    }
  } /* for each mounted drive */

  return *diCount;
}

#endif  /* _lib_GetDiskFreeSpace || _lib_GetDiskFreeSpaceEx */

#if _lib_fs_stat_dev \
    && _lib_next_dev

/*
 * di_get_disk_entries
 *
 * For BeOS / Haiku
 *
 */

int
di_get_disk_entries (di_data_t *di_data, int *diCount)
{
  di_disk_info_t  *diptr;
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
  di_opt_t        *diopts;

  diopts = (di_opt_t *) di_data->options;

  if (diopts->optval [DI_OPT_DEBUG] > 0) { printf ("# di_get_disk_entries: fs_stat_dev\n"); }
  count = 0;
  while ( (dev = next_dev (&count)) != B_BAD_VALUE) {
    if ( (stat = fs_stat_dev (dev, &fsinfo)) == B_BAD_VALUE) {
      break;
    }

    idx = *diCount;
    ++*diCount;
    di_data->diskInfo = (di_disk_info_t *) di_realloc (
        (char *) di_data->diskInfo,
        sizeof (di_disk_info_t) * (*diCount + 1));
    if (di_data->diskInfo == (di_disk_info_t *) NULL) {
      fprintf (stderr, "malloc failed for diskInfo. errno %d\n", errno);
      return -1;
    }
    diptr = di_data->diskInfo + idx;
    di_initialize_disk_info (diptr, idx);

    *buff = '\0';
    nref.device = dev;
    nref.node = fsinfo.root;
    dir = new BDirectory (&nref);
    stat = dir->GetEntry (&entry);
    stat = entry.GetPath (&path);
    stpecpy (diptr->strdata [DI_DISP_MOUNTPT],
        diptr->strdata [DI_DISP_MOUNTPT] + DI_MOUNTPT_LEN,
        path.Path ());
    stpecpy (diptr->strdata [DI_DISP_FILESYSTEM],
        diptr->strdata [DI_DISP_FILESYSTEM] + DI_FILESYSTEM_LEN,
        fsinfo.device_name);
    stpecpy (diptr->strdata [DI_DISP_FSTYPE],
        diptr->strdata [DI_DISP_FSTYPE] + DI_FSTYPE_LEN,
        fsinfo.fsh_name);
    di_save_block_sizes (diptr, (di_ui_t) fsinfo.block_size,
        (di_ui_t) fsinfo.total_blocks, (di_ui_t) fsinfo.free_blocks,
        (di_ui_t) fsinfo.free_blocks);
    di_save_inode_sizes (diptr, (di_ui_t) fsinfo.total_nodes,
        (di_ui_t) fsinfo.free_nodes, (di_ui_t) fsinfo.free_nodes);
# if defined (MNT_RDONLY)
    if ( (fsinfo.flags & MNT_RDONLY) == MNT_RDONLY) {
      diptr->isReadOnly = true;
    }
# endif
# if defined (MNT_PERSISTENT)
    if ( (fsinfo.flags & MNT_PERSISTENT) != MNT_PERSISTENT) {
      diptr->printFlag = DI_PRNT_IGNORE;
    }
# endif
    convertMountOptions ( (unsigned long) fsinfo.flags, diptr);
    di_trimchar (diptr->strdata [DI_DISP_MOUNTOPT], ',');

    if (diopts->optval [DI_OPT_DEBUG] > 1) {
      printf ("mnt:%s - %s\n", diptr->strdata [DI_DISP_MOUNTPT], diptr->strdata [DI_DISP_FILESYSTEM]);
      printf ("dev:%d fs:%s\n", dev, diptr->strdata [DI_DISP_FSTYPE]);
      printf ("%s: %s\n", diptr->strdata [DI_DISP_MOUNTPT], diptr->strdata [DI_DISP_FSTYPE]);
      printf ("\tblocks: tot:%ld free:%ld\n",
          fsinfo.total_blocks, fsinfo.free_blocks);
      printf ("\tinodes: tot:%ld free:%ld\n",
          fsinfo.total_nodes, fsinfo.free_nodes);
    }
  }
  return 0;
}

#endif
