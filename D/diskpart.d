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
  bool          isPooledFS;
  bool          isLoopback;
  byte          printFlag;
  ushort        index;
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

  // for .st_dev
  enum DI_UNKNOWN_DEV = 0xFFFFFFFF;

  // for .printFlag
  enum byte
    DI_PRINT_OK = 0,
    DI_PRINT_IGNORE = 1,
    DI_PRINT_BAD = 2,
    DI_PRINT_OUTOFZONE = 3,
    DI_PRINT_EXCLUDE = 4,
    DI_PRINT_FORCE = 5,
    DI_PRINT_SKIP = 6;

  @property void
  setPrintFlag (byte pFlag) {
    printFlag = pFlag;
  }

  @property void
  setDoPrint (bool v) {
    doPrint = v;
  }

  @property void
  setPooledFS (bool v) {
    isPooledFS = v;
  }

  @property void
  setRemote (bool v) {
    isRemote = v;
  }

  @property void
  setLoopback (bool v) {
    isLoopback = v;
  }

  @property void
  setSt_dev (uint v) {
    st_dev = v;
  }

  @property void
  setSp_dev (uint v) {
    sp_dev = v;
  }

  @property void
  setSp_rdev (uint v) {
    sp_rdev = v;
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

  void
  initDiskPartition () {
    this.totalBlocks = 0.0;
    this.freeBlocks = 0.0;
    this.availBlocks = 0.0;
    this.totalInodes = 0.0;
    this.freeInodes = 0.0;
    this.availInodes = 0.0;
  }

}; // struct DiskPartition


class DiskPartitions {

private:
  int       debugLevel;
  short     indexCount;

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

  void
  copyCstring (ref string a, char[] b)
  {
    auto l = strlen(b.ptr);
    a = to!(string)(b[0..l]);
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


        dp.initDiskPartition;
        dp.special = to!(typeof(dp.special))(mntEntry.mnt_fsname);
        dp.name = to!(typeof(dp.special))(mntEntry.mnt_dir);
        dp.fsType = to!(typeof(dp.special))(mntEntry.mnt_type);

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

        dp.index = indexCount++;
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

      for (auto idx = 0; idx < count; idx++)
      {
        DiskPartition dp;

        dp.initDiskPartition;
        auto sp = mntbufp + idx;
        copyCstring (dp.special, sp.f_mntfromname);
        copyCstring (dp.name, sp.f_mntonname);
        static if (_cmem_statfs_f_fstypename) {
          copyCstring (dp.fsType, sp.f_fstypename);
//        } else static if (_lib_sysfs && _cmem_statfs_f_type) {
//          char fbuff [DI_TYPE_LEN];
//          sysfs (GETFSTYP, sp.f_type, fbuff);
//          copyCstring (dp.fsType, fbuff);
//        } else static if (_dcl_mnt_names && _cmem_statfs_f_type) {
//          ;
        }

        if (dp.fsType.length > 2 && dp.fsType[0..2] == "nfs") {
          dp.isRemote = true;
        }
        static if (_cdefine_MNT_RDONLY) {
          if ((sp.f_flags & MNT_RDONLY) == MNT_RDONLY)
          {
            dp.isReadOnly = TRUE;
          }
        }
        static if (_cdefine_MNT_LOCAL) {
          if ((sp.f_flags & MNT_LOCAL) != MNT_LOCAL)
          {
            dp.isRemote = TRUE;
          }
        }

        static if (_cmem_statfs_f_fsize) {
          dp.blockSize = sp.f_fsize;
        } else static if (_cmem_statfs_f_bsize) {
          dp.blockSize = sp.f_bsize;
        }
        dp.totalBlocks = cast(typeof(dp.totalBlocks)) sp.f_blocks *
            dp.blockSize;
        dp.freeBlocks = cast(typeof(dp.freeBlocks)) sp.f_bfree *
            dp.blockSize;
        dp.availBlocks = cast(typeof(dp.availBlocks)) sp.f_bavail *
            dp.blockSize;
        dp.totalInodes = sp.f_files;
        dp.freeInodes = sp.f_ffree;
        dp.availInodes = sp.f_ffree;

        dp.index = indexCount++;
        diskPartitions ~= dp;
      }
      GC.free (mntbufp);
      return;
    }

/+
# if _dcl_mnt_names
#  if ! defined (MNT_NUMTYPES)
#   define MNT_NUMTYPES (sizeof(mnt_names)/sizeof(char *))
#  endif
# endif
# define DI_UNKNOWN_FSTYPE       "(%.2d)?"

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
    }
+/
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
              copyCstring (dp.fsType, statBuf.f_basetype);
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
