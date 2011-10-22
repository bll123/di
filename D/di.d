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

  initLocale ();
  getDIOptions (args, opts, dispOpts);

  auto dpList = new DiskPartitions (opts.debugLevel);
  dpList.getEntries ();
  preCheckDiskPartitions (dpList, opts);
  dpList.getPartitionInfo ();
  checkDiskQuotas (dpList, opts);
  checkDiskPartitions (dpList, opts);
  displayAll (opts, dispOpts, dpList);
}

void
preCheckDiskPartitions (ref DiskPartitions dpList, Options opts)
{
  if (opts.localOnly == true ||
    opts.includeList.length > 0 || opts.ignoreList.length > 0)
  {
    foreach (ref dp; dpList.diskPartitions)
    {
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
    }  // for each disk partition
  } // if there's processing to be done
}

void
checkIncludeList (ref DiskPartition dp, Options opts)
{
  if (opts.includeList.length > 0)
  {
    dp.setPrintFlag = dp.DI_PRINT_EXCLUDE;
    if (opts.includeList.get (dp.fsType, false) ||
        (dp.fsType[0..3] == FUSE_FS &&
        opts.includeList.get (FUSE_FS, false)))
    {
      dp.setPrintFlag = dp.DI_PRINT_OK;
    }
  }
}


void
checkDiskPartitions (ref DiskPartitions dpList, Options opts)
{
  foreach (ref dp; dpList.diskPartitions)
  {
    if (dp.printFlag == dp.DI_PRINT_EXCLUDE ||
        dp.printFlag == dp.DI_PRINT_BAD ||
        dp.printFlag == dp.DI_PRINT_OUTOFZONE) {
      dp.setDoPrint (FALSE);
      continue;
    }

    if (dp.printFlag == dp.DI_PRINT_IGNORE ||
        dp.printFlag == dp.DI_PRINT_SKIP) {
      dp.setDoPrint (opts.displayAll);
      continue;
    }

    dp.checkPartSizes;

    if (dp.printFlag == dp.DI_PRINT_OK) {
      if (dp.totalBlocks <= 0.0) {
        dp.setPrintFlag (dp.DI_PRINT_IGNORE);
        dp.setDoPrint (opts.displayAll);
      }
    }

    checkIncludeList (dp, opts);
  }
}

