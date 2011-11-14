// written in the D programming language

import std.stdio;
import std.string;

import config;
import options;
import dispopts;
import diskpart;
import display;
import dilocale;
import diquota;

enum string FUSE_FS = "fuse";

void main (string[] args)
{
  Options       opts;
  DisplayOpts   dispOpts;
  bool          hasLoopback;

  initLocale ();
  auto optidx = getDIOptions (args, opts, dispOpts);

  auto dpList = new DiskPartitions (opts.debugLevel);
  dpList.getEntries ();
  auto hasPooled = preCheckDiskPartitions (dpList, opts);
  if (optidx < args.length || opts.includeLoopback == false) {
    getDiskStatInfo (dpList);
    hasLoopback = getDiskSpecialInfo (dpList);
  }
  if (optidx < args.length) {
    auto rc = checkFileInfo (dpList, opts, hasPooled, args, optidx);
  }
  dpList.getPartitionInfo ();
  checkDiskPartitions (dpList, opts, hasLoopback);
  if (opts.quotaCheck) {
    checkDiskQuotas (dpList, opts);
  }
  doDisplay (opts, dispOpts, dpList, hasPooled);
}

bool
preCheckDiskPartitions (ref DiskPartitions dpList, Options opts)
{
  bool      hasPooled;

  foreach (ref dp; dpList.diskPartitions)
  {
    if (dp.fsType == "zfs" || dp.fsType == "advfs")
    {
      dp.isPooledFS = true;
      hasPooled = true;
    }

    if (! dp.isRemote && dp.fsType[0..2] == "nfs") {
      dp.setRemote = true;
    }

    if (opts.localOnly && dp.isRemote)
    {
      dp.setPrintFlag = dp.DI_PRINT_IGNORE;
    }

    checkIncludeList (dp, opts);

    if (opts.ignoreList.length > 0)
    {
      if (opts.ignoreList.get (dp.fsType, false) ||
          (dp.fsType[0..3] == FUSE_FS &&
           opts.ignoreList.get (FUSE_FS, false)))
      {
        dp.setPrintFlag = dp.DI_PRINT_EXCLUDE;
      }
    }
  }

  return hasPooled;
}

void
checkIncludeList (ref DiskPartition dp, Options opts)
{
  if (opts.includeList.length > 0)
  {
    if (dp.printFlag != dp.DI_PRINT_BAD &&
        dp.printFlag != dp.DI_PRINT_OUTOFZONE &&
        opts.includeList.get (dp.fsType, false) ||
        (dp.fsType[0..3] == FUSE_FS &&
         opts.includeList.get (FUSE_FS, false))) {
      dp.setDoPrint = true;
    }
    else {
      dp.setDoPrint = false;
    }
  }
}

void
checkDiskPartitions (ref DiskPartitions dpList, Options opts,
        bool hasLoopback)
{
  foreach (ref dp; dpList.diskPartitions)
  {
    dp.setDoPrint = true;

    if (dp.printFlag == dp.DI_PRINT_EXCLUDE ||
        dp.printFlag == dp.DI_PRINT_BAD ||
        dp.printFlag == dp.DI_PRINT_OUTOFZONE) {
      dp.setDoPrint = false;
      /* -a flag does not affect these */
      continue;
    }

    if (dp.printFlag == dp.DI_PRINT_IGNORE ||
        dp.printFlag == dp.DI_PRINT_SKIP) {
      dp.setDoPrint = opts.displayAll;
      continue;
    }

    dp.checkPartSizes;

    if (dp.printFlag == dp.DI_PRINT_OK) {
      if (dp.totalBlocks <= 0.0) {
        dp.setPrintFlag = dp.DI_PRINT_IGNORE;
        dp.setDoPrint = opts.displayAll;
      }
    }

    if (hasLoopback && opts.includeLoopback == false) {
      if (dp.isLoopback) {
        dp.setPrintFlag = dp.DI_PRINT_IGNORE;
        dp.setDoPrint = opts.displayAll;
      }
    }

    checkIncludeList (dp, opts);
  }
}

void
getDiskStatInfo (ref DiskPartitions dpList)
{
  C_ST_stat statBuf;

  foreach (ref dp; dpList.diskPartitions) {
    if (dp.printFlag == dp.DI_PRINT_EXCLUDE ||
        dp.printFlag == dp.DI_PRINT_BAD ||
        dp.printFlag == dp.DI_PRINT_OUTOFZONE)
    {
      continue;
    }

    dp.st_dev = dp.DI_UNKNOWN_DEV;
    if (stat (toStringz (dp.name), &statBuf) == 0) {
      dp.setSt_dev = cast(uint) statBuf.st_dev;
    }
  }
}

bool
getDiskSpecialInfo (ref DiskPartitions dpList)
{
  C_ST_stat statBuf;
  bool      hasLoopback;

  foreach (ref dp; dpList.diskPartitions) {
    if (dp.name[0] == '/' &&
        stat (toStringz(dp.special), &statBuf) == 0) {
      dp.setSp_dev = cast(uint) statBuf.st_dev;
      dp.setSp_rdev = cast(uint) statBuf.st_rdev;
        /* Solaris's loopback device is "lofs"            */
        /* linux loopback device is "none"                */
        /* linux has rdev = 0                             */
        /* DragonFlyBSD's loopback device is "null"       */
        /* DragonFlyBSD has rdev = -1                     */
        /* solaris is more consistent; rdev != 0 for lofs */
        /* solaris makes sense.                           */
      if ((dp.fsType == "lofs" && dp.sp_rdev != 0) ||
          dp.fsType == "null" || dp.fsType == "none") {
        dp.setLoopback = true;
        hasLoopback = true;
      } else {
        dp.setSp_dev = 0;
        dp.setSp_rdev = 0;
      }
    }
  }

  return hasLoopback;
}


bool
checkFileInfo (ref DiskPartitions dpList, Options opts,
    bool hasPooled, string[] args, int optidx)
{
  C_ST_stat statBuf;

  foreach (ref dp; dpList.diskPartitions) {
    if (dp.printFlag == dp.DI_PRINT_OK) {
      dp.setPrintFlag = dp.DI_PRINT_IGNORE;
    }
  }

  if (hasPooled) {
    size_t[]          sortIndex;

    sortIndex.length = dpList.diskPartitions.length;
    for (int i = 0; i < sortIndex.length; ++i) {
      sortIndex[i] = i;
    }
    sortPartitions (dpList.diskPartitions, sortIndex, "s");
  }

  for (auto i = optidx; i < args.length; ++i) {
    auto fd = open (toStringz(args[i]), O_RDONLY | O_NOCTTY);
    C_TYP_int rc;
    if (fd < 0) {
      rc = stat (toStringz(args[i]), &statBuf);
    } else {
      rc = fstat (fd, &statBuf);
      close (fd);
    }

    if (rc == 0) {
      foreach (ref dp; dpList.diskPartitions) {
        if (opts.debugLevel > 5)
        {
          writefln ("check %s against %s: %d %d", args[i], dp.name,
              cast(uint) statBuf.st_dev, dp.st_dev);
        }

        if (dp.st_dev != dp.DI_UNKNOWN_DEV &&
            cast(uint) statBuf.st_dev == dp.st_dev &&
            ! dp.isLoopback) {
          dp.setPrintFlag = dp.DI_PRINT_FORCE;
          break;
        }
      } // for each partition
    } // if stat of file worked
  } // for each command line argument

  return true;
}
