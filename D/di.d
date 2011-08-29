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
  displayAll (opts, dispOpts, dpList);
}

void
preCheckDiskPartitions (ref DiskPartitions dpList, Options opts)
{
/+
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
+/
}

void
checkIncludeList (ref DiskPartition dp, Options opts)
{
/+
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
+/
}


void
usage ()
{
    writefln (DI_GT("di version %s    Default Format: %s"), DI_VERSION, DI_DEFAULT_FORMAT);
                /*  12345678901234567890123456789012345678901234567890123456789012345678901234567890 */
    writeln (DI_GT("Usage: di [-ant] [-d display-size] [-f format] [-x exclude-fstyp-list]"));
    writeln (DI_GT("       [-I include-fstyp-list] [file [...]]"));
    writeln (DI_GT("   -a   : print all mounted devices"));
    writeln (DI_GT("   -d x : size to print blocks in (512 - POSIX, k - kbytes,"));
    writeln (DI_GT("          m - megabytes, g - gigabytes, t - terabytes, h - human readable)."));
    writeln (DI_GT("   -f x : use format string <x>"));
    writeln (DI_GT("   -I x : include only file system types in <x>"));
    writeln (DI_GT("   -x x : exclude file system types in <x>"));
    writeln (DI_GT("   -l   : display local filesystems only"));
    writeln (DI_GT("   -n   : don't print header"));
    writeln (DI_GT("   -t   : print totals"));
    writeln (DI_GT(" Format string values:"));
    writeln (DI_GT("    m - mount point                     M - mount point, full length"));
    writeln (DI_GT("    b - total kbytes                    B - kbytes available for use"));
    writeln (DI_GT("    u - used kbytes                     c - calculated kbytes in use"));
    writeln (DI_GT("    f - kbytes free                     v - kbytes available"));
    writeln (DI_GT("    p - percentage not avail. for use   1 - percentage used"));
    writeln (DI_GT("    2 - percentage of user-available space in use."));
    writeln (DI_GT("    i - total file slots (i-nodes)      U - used file slots"));
    writeln (DI_GT("    F - free file slots                 P - percentage file slots used"));
    writeln (DI_GT("    s - filesystem name                 S - filesystem name, full length"));
    writeln (DI_GT("    t - disk partition type             T - partition type, full length"));
    writeln (DI_GT("See manual page for more options."));
}
