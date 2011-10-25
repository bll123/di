// written in the D programming language

module didiskpart;

import std.stdio;
import std.string;
import std.conv : to;
private import core.memory : GC;
private import core.stdc.stdio;
private import core.stdc.errno;

import config;

struct DiskPartition {

public:
  bool          isRemote;
  bool          isReadOnly;
  bool          doPrint;
  byte          printFlag;
  uint          st_dev;         // disk device number
  uint          sp_dev;         // special device number
  uint          sp_rdev;        // special rdev #
  real          totalBlocks;
  real          freeBlocks;
  real          availBlocks;
  real          blockSize;
  real          totalInodes;
  real          freeInodes;
  real          availInodes;
  string        name;           // mount point
  string        special;        // special device name
  string        fsType;         // type of file system
  string        mountOptions;
  string        mountTime;

  // for printFlag
  enum byte
    DI_PRINT_OK = 0,
    DI_PRINT_IGNORE = 1,
    DI_PRINT_BAD = 2,
    DI_PRINT_OUTOFZONE = 3,
    DI_PRINT_EXCLUDE = 4,
    DI_PRINT_FORCE = 5,
    DI_PRINT_SKIP = 6;

  @property void
  setPrintFlag (byte pFlag)
  {
    printFlag = pFlag;
  }

  @property void
  setDoPrint (bool v)
  {
    doPrint = v;
  }

  void
  checkPartSizes () {
    if (this.freeBlocks < 0.0) {
      this.freeBlocks = 0.0;
    }
    if (this.availBlocks < 0.0) {
      this.availBlocks = 0.0;
    }
    if (this.totalInodes < 0.0) {
      this.freeInodes = 0.0;
      this.availInodes = 0.0;
    }
  }
}; // struct DiskPartition

class DiskPartitions {

private:
  int       debugLevel;

  static if (_cdefine__PATH_MOUNTED) {
    alias _PATH_MOUNTED DI_MOUNT_FILE;
  } else static if (_cdefine__PATH_MNTTAB) {
    alias _PATH_MNTTAB DI_MOUNT_FILE;
  } else static if (_cdefine_MOUNTED) {
    alias MOUNTED DI_MOUNT_FILE;
  } else static if (_cdefine_MNTTAB) {
    alias MNTTAB DI_MOUNT_FILE;
  } else {
    enum string DI_MOUNT_FILE = "/etc/mnttab";
  }

  static if (! _cdefine_MNTTYPE_IGNORE) {
    enum string MNTTYPE_IGNORE = "ignore";
  }

public:

  DiskPartition[]      diskPartitions;

  this () {}

  this (int dbg)
  {
    this.debugLevel = dbg;
  }

  void
  setDebugLevel (int dbg) {
    debugLevel = dbg;
  }

  void
  getEntries () {
    static if (_clib_getmntent && _clib_setmntent && _clib_endmntent &&
        ! _clib_getfsstat)
    {
      FILE *            f;
      C_ST_mntent *     mntEntry;

      static if (_c_args_setmntent == 1)
      {
        f = setmntent (toStringz(DI_MOUNT_FILE));
      } else static if (_c_args_setmntent == 2) {
        f = setmntent (toStringz(DI_MOUNT_FILE), toStringz("r"));
      }
      if (f == cast(FILE *) null)
      {
        string s = format ("Unable to open %s errno %d", DI_MOUNT_FILE, getErrno());
        throw new Exception (s);
      }
      scope (exit) {
        endmntent (f);
      }

      while ((mntEntry = getmntent (f)) != cast (C_ST_mntent *) null)
      {
        DiskPartition dp;

        dp.special = to!(typeof(dp.special))(mntEntry.mnt_fsname);
        dp.name = to!(typeof(dp.name))(mntEntry.mnt_dir);
        dp.fsType = to!(typeof(dp.fsType))(mntEntry.mnt_type);

        if (dp.special == "none") {
          dp.printFlag = dp.DI_PRINT_IGNORE;
        }
        if (dp.fsType == MNTTYPE_IGNORE) {
          dp.printFlag = dp.DI_PRINT_IGNORE;
        }
        if (dp.fsType[0..2] == "nfs") {
          dp.isRemote = true;
        }

    /+
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
  +/
        dp.mountOptions = to!(typeof(dp.mountOptions))(mntEntry.mnt_opts);

        if (debugLevel > 5)
        {
          writefln ("mnt:%s - %s:%s: %d %d",
              dp.name, dp.special, dp.fsType,
              dp.printFlag, dp.isReadOnly);
          writefln ("    %s", dp.mountOptions);
        }

        diskPartitions ~= dp;
      } // while there are mount entries
    } // _clib_get/set/endmntent

    static if (_clib_getfsstat) {
      C_ST_statfs *mntbufp;

//      if (debug > 0) { printf ("# getDiskEntries: getfsstat\n"); }
      auto count = getfsstat (cast(C_ST_statfs *) null,
          cast(_c_arg_2_getfsstat_alias) 0, MNT_NOWAIT);
      if (count < 1)
      {
//          fprintf (stderr, "Unable to do getfsstat () errno %d\n", errno);
          return;
      }
      auto bufsize = C_ST_statfs.sizeof * cast(size_t)count;
      mntbufp = cast(C_ST_statfs*) GC.calloc (bufsize);
      count = getfsstat (mntbufp, cast(_c_arg_2_getfsstat_alias)bufsize,
            MNT_NOWAIT);

/+
# if _dcl_mnt_names
#  if ! defined (MNT_NUMTYPES)
#   define MNT_NUMTYPES (sizeof(mnt_names)/sizeof(char *))
#  endif
# endif
# define DI_UNKNOWN_FSTYPE       "(%.2d)?"

    diDiskInfo_t     *diptr;
    int             count;
    int             idx;
# if _dcl_mnt_names && _mem_struct_statfs_f_type
    short           fstype;
# endif
    _c_arg_2_getfsstat bufsize;
    struct statfs   *mntbufp;
    struct statfs   *sp;

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
        convertMountOptions ((long) sp->f_flags, diptr);
# if _mem_struct_statfs_f_type
#  if defined (MOUNT_NFS3)
        if (sp->f_type == MOUNT_NFS3)
        {
            strncat (diptr->options, "v3,",
                    DI_OPT_LEN - strlen (diptr->options) - 1);
        }
#  endif
# endif
# if _mem_struct_statfs_mount_info && \
        defined (MOUNT_NFS) && \
        (_mem_struct_statfs_f_type || _mem_struct_statfs_f_fstypename)
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
+/

    }

    return;
  }

  void
  getPartitionInfo ()
  {
    static if (_clib_statvfs && ! _clib_getfsstat) {
      C_ST_statvfs        statBuf;

      foreach (ref dp; diskPartitions)
      {
        if (dp.printFlag == dp.DI_PRINT_OK ||
            dp.printFlag == dp.DI_PRINT_SKIP ||
            dp.printFlag == dp.DI_PRINT_FORCE)
        {
          if (statvfs (toStringz(dp.name), &statBuf) != 0)
          {
            string s = format ("statvfs: %s errno %d", dp.name, getErrno());
            throw new Exception (s);
          }

          if (statBuf.f_frsize == 0 && statBuf.f_bsize != 0)
          {
            dp.blockSize = statBuf.f_bsize;
          } else {
            dp.blockSize = statBuf.f_frsize;
          }
          /* Linux! statvfs() returns values in f_bsize rather f_frsize. Bleah.*/
          /* Non-POSIX!  Linux manual pages are incorrect.                     */
          static if (SYSTYPE == "Linux") {
            dp.blockSize = statBuf.f_bsize;
          }
          dp.totalBlocks = cast(typeof(dp.totalBlocks)) statBuf.f_blocks *
              dp.blockSize;
          dp.freeBlocks = cast(typeof(dp.freeBlocks)) statBuf.f_bfree *
              dp.blockSize;
          dp.availBlocks = cast(typeof(dp.availBlocks)) statBuf.f_bavail *
              dp.blockSize;
          dp.totalInodes = statBuf.f_files;
          dp.freeInodes = statBuf.f_ffree;
          dp.availInodes = statBuf.f_favail;
          static if (_cmem_statvfs_f_basetype) {
            if (dp.fsType.length == 0) {
              dp.fsType = to!(typeof(dp.fsType))(statBuf.f_basetype);
            }
          }

          if (debugLevel > 5)
          {
            writefln ("part:%s: %.0f : %.0f %.0f %.0f", dp.name,
                dp.blockSize, dp.totalBlocks, dp.freeBlocks, dp.availBlocks);
          }
        }
      }
    }

    return;
  }

}; // class DiskPartitions
