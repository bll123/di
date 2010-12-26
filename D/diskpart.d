// written in the D programming language

module di_diskpart;

import std.string;
import std.conv;
import std.stdio;
private import core.stdc.stdio;
private import core.stdc.errno;

import config;

class DiskPartitions {

private:
  int       debugLevel;

  static if (_cdefstr__PATH_MOUNTED) {
    alias _PATH_MOUNTED DI_MOUNT_FILE;
  } else static if (_cdefstr__PATH_MNTTAB) {
    alias _PATH_MNTTAB DI_MOUNT_FILE;
  } else static if (_cdefstr_MOUNTED) {
    alias MOUNTED DI_MOUNT_FILE;
  } else static if (_cdefstr_MNTTAB) {
    alias MNTTAB DI_MOUNT_FILE;
  } else {
    enum string DI_MOUNT_FILE = "/etc/mnttab";
  }

  static if (! _cdefstr_MNTTYPE_IGNORE) {
    enum string MNTTYPE_IGNORE = "ignore";
  }

public:

  struct DiskPartitions {
    real          totalBlocks;
    real          freeBlocks;
    real          availBlocks;
    real          blockSize;
    real          totalInodes;
    real          freeInodes;
    real          availInodes;
    uint          st_dev;         // disk device number
    uint          sp_dev;         // special device number
    uint          sp_rdev;        // special rdev #
    byte          printFlag;
    bool          isReadOnly;
    string        name;           // mount point
    string        special;        // special device name
    string        fsType;         // type of file system
    string        mountOptions;
    string        mountTime;
  };

  DiskPartitions[]      diskPartitions;

  // for printFlag
  enum byte
    DI_PRNT_OK = 0,
    DI_PRNT_IGNORE = 1,
    DI_PRNT_BAD = 2,
    DI_PRNT_OUTOFZONE = 3,
    DI_PRNT_EXCLUDE = 4,
    DI_PRNT_FORCE = 5,
    DI_PRNT_SKIP = 6;

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
  FILE *            f;
  C_ST_mntent *     mntEntry;

  static if (_clib_getmntent && _clib_setmntent && _clib_endmntent)
  {
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
      diskPartitions.length += 1;
      auto dp = &diskPartitions[$-1];
      dp.special = to!(typeof(dp.special))(mntEntry.mnt_fsname);
      dp.name = to!(typeof(dp.name))(mntEntry.mnt_dir);
      dp.fsType = to!(typeof(dp.fsType))(mntEntry.mnt_type);

      if (to!string(mntEntry.mnt_fsname) == "none") {
        dp.printFlag = DI_PRNT_IGNORE;
      }
      if (to!string(mntEntry.mnt_type) == MNTTYPE_IGNORE) {
        dp.printFlag = DI_PRNT_IGNORE;
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
    } // while there are mount entries
  } // _clib_get/set/endmntent

  return;
}

void
getPartitionInfo ()
{
  static if (_clib_statvfs) {
    C_ST_statvfs        statBuf;

    foreach (ref dp; diskPartitions)
    {
      if (dp.printFlag == DI_PRNT_OK ||
          dp.printFlag == DI_PRNT_SKIP ||
          dp.printFlag == DI_PRNT_FORCE)
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
        dp.totalBlocks = cast(typeof(dp.totalBlocks)) statBuf.f_blocks;
        dp.freeBlocks = cast(typeof(dp.freeBlocks)) statBuf.f_bfree;
        dp.availBlocks = cast(typeof(dp.availBlocks)) statBuf.f_bavail;
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

} // class DiskPartitions
