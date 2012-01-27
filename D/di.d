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

  auto dps = new DiskPartitions (opts.debugLevel);
  dps.getEntries ();
  auto hasPooled = preCheckDiskPartitions (dps, opts);
  if (optidx < args.length || opts.includeLoopback == false) {
    getDiskStatInfo (dps);
    hasLoopback = getDiskSpecialInfo (dps);
  }
  if (optidx < args.length) {
    auto rc = checkFileInfo (dps, opts, hasPooled, args, optidx);
  }
  dps.getPartitionInfo ();
  checkDiskPartitions (dps, opts, hasLoopback);
  if (opts.quotaCheck) {
    checkDiskQuotas (dps, opts);
  }
  doDisplay (opts, dispOpts, dps, hasPooled);
}

bool
preCheckDiskPartitions (ref DiskPartitions dps, Options opts)
{
  bool      hasPooled;

  foreach (ref dp; dps.diskPartitions)
  {
    if (dp.fsType == "zfs" || dp.fsType == "advfs")
    {
      dp.isPooledFS = true;
      hasPooled = true;
    }

    if (! dp.isRemote && dp.fsType[0..2] == "nfs") {
      dp.isRemote = true;
    }

    if (opts.localOnly && dp.isRemote)
    {
      dp.printFlag = dp.DI_PRINT_IGNORE;
    }

    checkIncludeList (dp, opts);

    if (opts.excludeList.length > 0)
    {
      if (opts.excludeList.get (dp.fsType, false) ||
          (dp.fsType[0..3] == FUSE_FS &&
           opts.excludeList.get (FUSE_FS, false)))
      {
        dp.printFlag = dp.DI_PRINT_EXCLUDE;
      }
    }
  }

  return hasPooled;
}

void
checkIncludeList (ref DiskPartitions.DiskPartition dp, Options opts)
{
  if (opts.includeList.length > 0)
  {
    if (dp.printFlag != dp.DI_PRINT_BAD &&
        dp.printFlag != dp.DI_PRINT_OUTOFZONE &&
        opts.includeList.get (dp.fsType, false) ||
        (dp.fsType[0..3] == FUSE_FS &&
         opts.includeList.get (FUSE_FS, false))) {
      dp.doPrint = true;
    }
    else {
      dp.doPrint = false;
    }
  }
}

void
checkDiskPartitions (ref DiskPartitions dps, Options opts,
        bool hasLoopback)
{
  foreach (ref dp; dps.diskPartitions)
  {
    dp.doPrint = true;

    if (dp.printFlag == dp.DI_PRINT_EXCLUDE ||
        dp.printFlag == dp.DI_PRINT_BAD ||
        dp.printFlag == dp.DI_PRINT_OUTOFZONE) {
      dp.doPrint = false;
      /* -a flag does not affect these */
      continue;
    }

    if (dp.printFlag == dp.DI_PRINT_IGNORE ||
        dp.printFlag == dp.DI_PRINT_SKIP) {
      dp.doPrint = opts.displayAll;
      continue;
    }

    dp.checkPartSizes;

    if (dp.printFlag == dp.DI_PRINT_OK) {
      if (dp.totalBlocks <= 0.0) {
        dp.printFlag = dp.DI_PRINT_IGNORE;
        dp.doPrint = opts.displayAll;
      }
    }

    if (hasLoopback && opts.includeLoopback == false) {
      if (dp.isLoopback) {
        dp.printFlag = dp.DI_PRINT_IGNORE;
        dp.doPrint = opts.displayAll;
      }
    }

    checkIncludeList (dp, opts);
  }
}

void
getDiskStatInfo (ref DiskPartitions dps)
{
  C_ST_stat statBuf;

  foreach (ref dp; dps.diskPartitions) {
    if (dp.printFlag == dp.DI_PRINT_EXCLUDE ||
        dp.printFlag == dp.DI_PRINT_BAD ||
        dp.printFlag == dp.DI_PRINT_OUTOFZONE)
    {
      continue;
    }

    dp.st_dev = dp.DI_UNKNOWN_DEV;
    if (stat (toStringz (dp.name), &statBuf) == 0) {
      dp.st_dev = cast(uint) statBuf.st_dev;
    }
  }
}

bool
getDiskSpecialInfo (ref DiskPartitions dps)
{
  C_ST_stat statBuf;
  bool      hasLoopback;

  foreach (ref dp; dps.diskPartitions) {
    if (dp.name[0] == '/' &&
        stat (toStringz(dp.special), &statBuf) == 0) {
      dp.sp_dev = cast(uint) statBuf.st_dev;
      dp.sp_rdev = cast(uint) statBuf.st_rdev;
        /* Solaris's loopback device is "lofs"            */
        /* linux loopback device is "none"                */
        /* linux has rdev = 0                             */
        /* DragonFlyBSD's loopback device is "null"       */
        /* DragonFlyBSD has rdev = -1                     */
        /* solaris is more consistent; rdev != 0 for lofs */
        /* solaris makes sense.                           */
      if ((dp.fsType == "lofs" && dp.sp_rdev != 0) ||
          dp.fsType == "null" || dp.fsType == "none") {
        dp.isLoopback = true;
        hasLoopback = true;
      } else {
        dp.sp_dev = 0;
        dp.sp_rdev = 0;
      }
    }
  }

  return hasLoopback;
}


bool
checkFileInfo (ref DiskPartitions dps, Options opts,
    bool hasPooled, string[] args, int optidx)
{
  C_ST_stat statBuf;

  foreach (ref dp; dps.diskPartitions) {
    if (dp.printFlag == dp.DI_PRINT_OK) {
      dp.printFlag = dp.DI_PRINT_IGNORE;
    }
  }

  if (hasPooled) {
    size_t[]          sortIndex;

    sortIndex.length = dps.diskPartitions.length;
    for (int i = 0; i < sortIndex.length; ++i) {
      sortIndex[i] = i;
    }
    dps.sortPartitions (sortIndex, "s");
  }

  for (auto i = optidx; i < args.length; ++i) {
    auto fd = open (toStringz(args[i]), O_RDONLY | O_NOCTTY);
    C_NATIVE_int rc;
    if (fd < 0) {
      rc = stat (toStringz(args[i]), &statBuf);
    } else {
      rc = fstat (fd, &statBuf);
      close (fd);
    }

    if (rc == 0) {
      foreach (ref dp; dps.diskPartitions) {
        if (opts.debugLevel > 5)
        {
          writefln ("check %s against %s: %d %d", args[i], dp.name,
              cast(uint) statBuf.st_dev, dp.st_dev);
        }

        if (dp.st_dev != dp.DI_UNKNOWN_DEV &&
            cast(uint) statBuf.st_dev == dp.st_dev &&
            ! dp.isLoopback) {
          dp.printFlag = dp.DI_PRINT_FORCE;
          break;
        }
      } // for each partition
    } // if stat of file worked
  } // for each command line argument

  return true;
}
