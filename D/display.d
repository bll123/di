// written in the D programming language

module display;

import std.stdio;
import std.string;
import std.ascii : isDigit;
import std.uni : toLower;
import std.conv : to;
private import std.utf : count;
private import std.math : floor;

import config;
import options;
import dispopts;
import diskpart;
import dilocale;

private:

enum dchar
  FMT_MOUNT = 'm',
  FMT_MOUNT_FULL = 'M',
  FMT_SPECIAL = 's',
  FMT_SPECIAL_FULL = 'S',
  FMT_TYPE = 't',
  FMT_TYPE_FULL = 'T',
  FMT_BLOCKS_TOT = 'b',
  FMT_BLOCKS_TOT_AVAIL = 'B',
  FMT_BLOCKS_USED = 'u',
  FMT_BLOCKS_CALC_USED = 'c',
  FMT_BLOCKS_FREE = 'f',
  FMT_BLOCKS_AVAIL = 'v',
  FMT_BLOCKS_PERC_NOT_AVAIL = 'p',
  FMT_BLOCKS_PERC_USED = '1',
  FMT_BLOCKS_PERC_USED_AVAIL = '2',
  FMT_BLOCKS_PERC_AVAIL = 'a',
  FMT_BLOCKS_PERC_FREE = '3',
  FMT_INODES_TOT = 'i',
  FMT_INODES_USED = 'U',
  FMT_INODES_FREE = 'F',
  FMT_INODES_PERC = 'P',
  FMT_MOUNT_TIME = 'I',
  FMT_MOUNT_OPTIONS = 'O';

enum int
  FTYPE_STRING = 1,
  FTYPE_SPACE = 2,
  FTYPE_PERC_SPACE = 3,
  FTYPE_INODE = 4,
  FTYPE_PERC_INODE = 5;

struct FormatInfo {
  dchar         key;
  int           ftype;
  long          width;
  string        title;
};

FormatInfo[] formatTypes = [
  { key:   FMT_MOUNT,
    ftype: FTYPE_STRING,
    width: 15,
    title: "Mount" },
  { key:   FMT_MOUNT_FULL,
    ftype: FTYPE_STRING,
    width: 15,
    title: "Mount" },
  { key:   FMT_SPECIAL,
    ftype: FTYPE_STRING,
    width: 18,
    title: "Filesystem" },
  { key:   FMT_SPECIAL_FULL,
    ftype: FTYPE_STRING,
    width: 18,
    title: "Filesystem" },
  { key:   FMT_TYPE,
    ftype: FTYPE_STRING,
    width: 7,
    title: "fsType" },
  { key:   FMT_TYPE_FULL,
    ftype: FTYPE_STRING,
    width: 7,
    title: "fs Type" },
  { key:   FMT_BLOCKS_TOT,
    ftype: FTYPE_SPACE,
    width: 8,
    title: "" },
  { key:   FMT_BLOCKS_TOT_AVAIL,
    ftype: FTYPE_SPACE,
    width: 8,
    title: "" },
  { key:   FMT_BLOCKS_USED,
    ftype: FTYPE_SPACE,
    width: 8,
    title: "Used" },
  { key:   FMT_BLOCKS_CALC_USED,
    ftype: FTYPE_SPACE,
    width: 8,
    title: "Used" },
  { key:   FMT_BLOCKS_FREE,
    ftype: FTYPE_SPACE,
    width: 8,
    title: "Free" },
  { key:   FMT_BLOCKS_AVAIL,
    ftype: FTYPE_SPACE,
    width: 8,
    title: "Avail" },
  { key:   FMT_BLOCKS_PERC_NOT_AVAIL,
    ftype: FTYPE_PERC_SPACE,
    width: 6,
    title: "%Used" },
  { key:   FMT_BLOCKS_PERC_USED,
    ftype: FTYPE_PERC_SPACE,
    width: 6,
    title: "%Used" },
  { key:   FMT_BLOCKS_PERC_USED_AVAIL,
    ftype: FTYPE_PERC_SPACE,
    width: 6,
    title: "%Used" },
  { key:   FMT_BLOCKS_PERC_AVAIL,
    ftype: FTYPE_PERC_SPACE,
    width: 5,
    title: "%Free" },
  { key:   FMT_BLOCKS_PERC_FREE,
    ftype: FTYPE_PERC_SPACE,
    width: 5,
    title: "%Free" },
  { key:   FMT_INODES_TOT,
    ftype: FTYPE_INODE,
    width: 9,
    title: "Inodes" },
  { key:   FMT_INODES_USED,
    ftype: FTYPE_INODE,
    width: 9,
    title: "IUsed" },
  { key:   FMT_INODES_FREE,
    ftype: FTYPE_INODE,
    width: 9,
    title: "IFree" },
  { key:   FMT_INODES_PERC,
    ftype: FTYPE_PERC_INODE,
    width: 6,
    title: "%IUsed" },
  { key:   FMT_MOUNT_TIME,
    ftype: FTYPE_STRING,
    width: 0,
    title: "" },
  { key:   FMT_MOUNT_OPTIONS,
    ftype: FTYPE_STRING,
    width: 15,
    title: "Options" }
];

size_t[dchar]   formatTypesIdxs;
bool            fmtInfoIdxsInitialized;

struct DisplayData
{
  Options           opts;
  DisplayOpts       dispOpts;
  DiskPartitions    dps;
  string            titleString;
  string[dchar]     dispFmtString;
};

struct DisplayTable {
  real              size;
  real              low;
  real              high;
  short             precision;
  char              key;
  string            suffix;
  string            disp[2];
};

DisplayTable[] displayTable = [
  // size == low except for bytes
  { size: 0, low: 0, high: 0, precision: 0, key: 'b',
    suffix: " ", disp: [ "Bytes", "Bytes" ] },
  { size: 0, low: 0, high: 0, precision: 0, key: 'k',
    suffix: "K", disp: [ "KBytes", "KBytes" ] },
  { size: 0, low: 0, high: 0, precision: 1, key: 'm',
    suffix: "M", disp: [ "Megs", "Mebis" ] },
  { size: 0, low: 0, high: 0, precision: 1, key: 'g',
    suffix: "G", disp: [ "Gigs", "Gibis" ] },
  { size: 0, low: 0, high: 0, precision: 1, key: 't',
    suffix: "T", disp: [ "Teras", "Tebis" ] },
  { size: 0, low: 0, high: 0, precision: 1, key: 'p',
    suffix: "P", disp: [ "Petas", "Pebis" ] },
  { size: 0, low: 0, high: 0, precision: 1, key: 'e',
    suffix: "E", disp: [ "Exas", "Exbis" ] },
  { size: 0, low: 0, high: 0, precision: 1, key: 'z',
    suffix: "Z", disp: [ "Zettas", "Zebis" ] },
  { size: 0, low: 0, high: 0, precision: 1, key: 'y',
    suffix: "Y", disp: [ "Yottas", "Yobis" ] },
  { size: 0, low: 0, high: 0, precision: 1, key: 'h',
    suffix: "h", disp: [ "Size", "Size" ] },
  { size: 0, low: 0, high: 0, precision: 1, key: 'H',
    suffix: "H", disp: [ "Size", "Size" ] }
  ];

size_t[char]    displayIdxs;
bool            dispIdxsInitialized;

public:

void
doDisplay (Options opts, ref DisplayOpts dispOpts,
    DiskPartitions dps, bool hasPooled)
{
  DisplayData       dispData;

  initializeIdxs ();
  initializeTitles (dispOpts);
  initializeDisplayTable (dispOpts);
  setDispBlockSize (dispOpts);
  if (opts.debugLevel > 30)
  {
    dumpDispTable ();
  }

  dispData.opts = opts;
  dispData.dispOpts = dispOpts;
  dispData.dps = dps;

  buildDisplayList (dispData);
  displayTitle (dispData);
  displayPartitions (dispData, hasPooled);
}

private:

void
dumpDispTable ()
{
  foreach (disp; displayTable)
  {
    writefln ("%s %s %-6s sz:%25.0f\n  low:%25.0f high:%25.0f",
        disp.key, disp.suffix, disp.disp[0], disp.size, disp.low, disp.high);
  }
}

void
buildDisplayList (ref DisplayData dispData)
{
  int[dchar]    dpMax;
  bool          first = true;

  foreach (dchar c; dispData.opts.formatString)
  {
    if (c in formatTypesIdxs &&
        formatTypes[formatTypesIdxs[c]].ftype == FTYPE_STRING)
    {
      dpMax[c] = 0;
    }
  }

  foreach (dp; dispData.dps.diskPartitions)
  {
    if (! dp.doPrint) {
      continue;
    }

    if (FMT_SPECIAL_FULL in dpMax) {
      setMaxLen (dp.special, dpMax[FMT_SPECIAL_FULL]);
    }
    if (FMT_MOUNT_FULL in dpMax) {
      setMaxLen (dp.name, dpMax[FMT_MOUNT_FULL]);
    }
    if (FMT_TYPE_FULL in dpMax) {
      setMaxLen (dp.fsType, dpMax[FMT_TYPE_FULL]);
    }
    if (FMT_MOUNT_OPTIONS in dpMax) {
      setMaxLen (dp.mountOptions, dpMax[FMT_MOUNT_OPTIONS]);
    }
  }

  foreach (dchar c; dispData.opts.formatString)
  {
    if (c in formatTypesIdxs) {
      string fmt;
      auto title = DI_GT (formatTypes[formatTypesIdxs[c]].title);
      auto ftype = formatTypes[formatTypesIdxs[c]].ftype;
      auto width = formatTypes[formatTypesIdxs[c]].width;

      switch (c)
      {
        case FMT_MOUNT_FULL:
        case FMT_SPECIAL_FULL:
        case FMT_TYPE_FULL:
        case FMT_MOUNT_TIME:
        case FMT_MOUNT_OPTIONS:
        {
          width = dpMax[c];
          break;
        }

        case FMT_BLOCKS_TOT:
        case FMT_BLOCKS_TOT_AVAIL:
        {
          title = DI_GT (dispData.dispOpts.dispBlockLabel);
          break;
        }

        default:
        {
          break;
        }
      }

      if (dispData.opts.csvOutput) {
        title = to!string(c);
      }

      auto utflen = std.utf.count (title);
      if (utflen > width)
      {
        width = utflen;
      }

      switch (ftype)
      {
        case FTYPE_STRING:
        {
          fmt = format ("%%-%d.%ds ", width, width);
          if (dispData.opts.csvOutput) {
            fmt = "\"%s\"";
          }
          dispData.dispFmtString[c] = fmt;
          break;
        }

        case FTYPE_SPACE:
        {
          if (dispData.dispOpts.width != 0) {
            width = dispData.dispOpts.width;
          }
          auto twidth = width;
          if (dispData.dispOpts.dbsstr == "h" || dispData.dispOpts.dbsstr == "H")
          {
            --twidth;
          }
          fmt = format ("%%%d.%df", twidth, dispData.dispOpts.precision);
          if (dispData.opts.csvOutput) {
            fmt = format ("%%.%df", dispData.dispOpts.precision);
          }
          dispData.dispFmtString[c] = fmt;
          break;
        }

        case FTYPE_PERC_SPACE:
        {
          auto twidth = width - 2;
          if (twidth < 1) { twidth = 1; }
          fmt = format ("%%%d.0f%%%%  ", twidth);
          if (dispData.opts.csvOutput) {
            fmt = "%.0f%%";
          }
          dispData.dispFmtString[c] = fmt;
          break;
        }

        case FTYPE_INODE:
        {
          if (dispData.dispOpts.inodeWidth != 0) {
            width = dispData.dispOpts.inodeWidth;
          }
          fmt = format ("%%%d.0f ", width);
          if (dispData.opts.csvOutput) {
            fmt = "%.0f";
          }
          dispData.dispFmtString[c] = fmt;
          break;
        }

        case FTYPE_PERC_INODE:
        {
          fmt = format ("%%%d.0f%%%% ", width);
          if (dispData.opts.csvOutput) {
            fmt = "%.0f%%";
          }
          dispData.dispFmtString[c] = fmt;
          break;
        }

        default:
        {
          break;
        }
      } // switch on format type

      auto twid = width + title.length - utflen;
      if (dispData.opts.csvOutput) {
        fmt = "%s";
      } else {
        if (ftype == FTYPE_STRING) {
          fmt = format ("%%-%d.%ds ", twid, twid);
        } else {
          fmt = format ("%%%d.%ds ", twid, twid);
        }
      }
      if (dispData.opts.csvOutput && ! first) {
        dispData.titleString ~= ",";
      }
      dispData.titleString ~= format (fmt, title);

    } else {
      dispData.titleString ~= format ("%s", c);
    }
    first = false;
  } // for each format string char
}

void
displayTitle (ref DisplayData dispData)
{
  if (! dispData.opts.printHeader) { return; }
  writeln (dispData.titleString);
}


void
displayPartitions (ref DisplayData dispData, bool hasPooled)
{
  size_t[]      sortIndex;

  sortIndex.length = dispData.dps.diskPartitions.length;
  for (int i = 0; i < sortIndex.length; ++i) {
    sortIndex[i] = i;
  }

  if (dispData.opts.displayTotal) {
    bool      inpool;
    string    lastpool;

    dispData.dps.tot.initDiskPartition;
    dispData.dps.tot.name = DI_GT("Total");

    if (hasPooled) {
      dispData.dps.sortPartitions (sortIndex, "s");
    }

    foreach (size_t i, size_t v; sortIndex)
    {
      bool      startpool;
      auto      dp = dispData.dps.diskPartitions[v];

      startpool = false;

      if (hasPooled && dp.isPooledFS) {
        if (lastpool.length == 0 ||
            (dp.special.length >= lastpool.length &&
             lastpool != dp.special[0..lastpool.length])) {
          lastpool = dp.special;
          auto si = indexOf (lastpool, "#");  /* for advfs */
          if (si >= 0) {
            lastpool.length = si;
          }
          si = indexOf (lastpool, "/");  /* for zfs */
          if (si >= 0) {
            lastpool.length = si;
          }
          inpool = false;
          startpool = true;
        }
      } else {
        inpool = false;
      }

      if (dp.doPrint) {
        /* only add in a pooled filesystem if it's the first in the list    */
        /* belonging to that pool                                           */
        if (! inpool) {
          dispData.dps.tot.totalBlocks += dp.totalBlocks;
          dispData.dps.tot.freeBlocks += dp.freeBlocks;
          dispData.dps.tot.availBlocks += dp.availBlocks;
          dispData.dps.tot.totalInodes += dp.totalInodes;
          dispData.dps.tot.freeInodes += dp.freeInodes;
          dispData.dps.tot.availInodes += dp.availInodes;
        } else {
          /* if in a pool of disks, add the total used to the totals */
          dispData.dps.tot.totalBlocks += dp.totalBlocks - dp.freeBlocks;
          dispData.dps.tot.totalInodes += dp.totalInodes - dp.freeInodes;
        }
      }

      if (startpool) {
        inpool = true;
      }
    }
  }

  dispData.dps.sortPartitions (sortIndex, dispData.opts.sortType);

  foreach (size_t i, size_t v; sortIndex)
  {
    auto dp = dispData.dps.diskPartitions[v];

    if (! dp.doPrint) {
      continue;
    }

    printPartition (dispData, dp);
  } // for each disk partition

  if (dispData.opts.displayTotal) {
    printPartition (dispData, dispData.dps.tot);
  }
}

void
printPartition (ref DisplayData dispData, DiskPartitions.DiskPartition dp)
{
  string      outString;
  string      sval;
  real        rval;
  real        uval;
  size_t      dHidx;
  bool        first = true;

  if (dispData.dispOpts.dbsstr == "H") {
    real mval = 0.0;

    foreach (dchar c; dispData.opts.formatString)
    {
      if (c in formatTypesIdxs) {
        auto ftype = formatTypes[formatTypesIdxs[c]].ftype;

        if (ftype == FTYPE_SPACE) {
          switch (c)
          {
            case FMT_BLOCKS_TOT:
            {
              rval = dp.totalBlocks;
              break;
            }

            case FMT_BLOCKS_TOT_AVAIL:
            {
              rval = dp.totalBlocks - (dp.freeBlocks - dp.availBlocks);
              break;
            }

            case FMT_BLOCKS_USED:
            {
              rval = dp.totalBlocks - dp.freeBlocks;
              break;
            }

            case FMT_BLOCKS_CALC_USED:
            {
              rval = dp.totalBlocks - dp.availBlocks;
              break;
            }

            case FMT_BLOCKS_FREE:
            {
              rval = dp.freeBlocks;
              break;
            }

            case FMT_BLOCKS_AVAIL:
            {
              rval = dp.availBlocks;
              break;
            }

            default:
            {
              break;
            }
          }  // switch on format character

          if (rval > mval) {
            mval = rval;
          }
        }  // is FTYPE_SPACE
      }  // valid format character
    } // for each character in format string

    dHidx = findDispSize (mval);
  } // if -d H

  foreach (dchar c; dispData.opts.formatString)
  {
    if (c in formatTypesIdxs) {
      auto ftype = formatTypes[formatTypesIdxs[c]].ftype;

      switch (c)
      {
        case FMT_MOUNT:
        case FMT_MOUNT_FULL:
        {
          sval = dp.name;
          break;
        }

        case FMT_SPECIAL:
        case FMT_SPECIAL_FULL:
        {
          sval = dp.special;
          break;
        }

        case FMT_TYPE:
        case FMT_TYPE_FULL:
        {
          sval = dp.fsType;
          break;
        }

        case FMT_BLOCKS_TOT:
        {
          rval = dp.totalBlocks;
          break;
        }

        case FMT_BLOCKS_TOT_AVAIL:
        {
          rval = dp.totalBlocks - (dp.freeBlocks - dp.availBlocks);
          break;
        }

        case FMT_BLOCKS_USED:
        {
          rval = dp.totalBlocks - dp.freeBlocks;
          break;
        }

        case FMT_BLOCKS_CALC_USED:
        {
          rval = dp.totalBlocks - dp.availBlocks;
          break;
        }

        case FMT_BLOCKS_FREE:
        {
          rval = dp.freeBlocks;
          break;
        }

        case FMT_BLOCKS_AVAIL:
        {
          rval = dp.availBlocks;
          break;
        }

        case FMT_BLOCKS_PERC_NOT_AVAIL:
        {
          rval = dp.totalBlocks;
          uval = dp.totalBlocks - dp.availBlocks;
          break;
        }

        case FMT_BLOCKS_PERC_USED:
        {
          rval = dp.totalBlocks;
          uval = dp.totalBlocks - dp.freeBlocks;
          break;
        }

        case FMT_BLOCKS_PERC_USED_AVAIL:
        {
          rval = dp.totalBlocks - (dp.freeBlocks - dp.availBlocks);
          uval = dp.totalBlocks - dp.freeBlocks;
          break;
        }

        case FMT_BLOCKS_PERC_AVAIL:
        {
          rval = dp.totalBlocks;
          uval = dp.availBlocks;
          break;
        }

        case FMT_BLOCKS_PERC_FREE:
        {
          rval = dp.totalBlocks;
          uval = dp.freeBlocks;
          break;
        }

        case FMT_INODES_TOT:
        {
          rval = dp.totalInodes;
          break;
        }

        case FMT_INODES_USED:
        {
          rval = dp.totalInodes - dp.freeInodes;
          break;
        }

        case FMT_INODES_FREE:
        {
          rval = dp.freeInodes;
          break;
        }

        case FMT_INODES_PERC:
        {
          rval = dp.totalInodes;
          uval = dp.totalInodes - dp.availInodes;
          break;
        }

        case FMT_MOUNT_TIME:
        {
          break;
        }

        case FMT_MOUNT_OPTIONS:
        {
          sval = dp.mountOptions;
          break;
        }

        default:
        {
          break;
        }
      }  // switch on format character

      switch (ftype)
      {
        case FTYPE_STRING:
        {
          if (dispData.opts.csvOutput && ! first) {
            outString ~= ",";
          }
          outString ~= format (dispData.dispFmtString[c], sval);
          break;
        }

        case FTYPE_SPACE:
        {
          if (dispData.opts.csvOutput && ! first) {
            outString ~= ",";
          }
          outString ~= blockDisplay (dispData, c, rval, dHidx,
                dispData.opts.csvOutput);
          break;
        }

        case FTYPE_PERC_SPACE:
        case FTYPE_PERC_INODE:
        {
          if (dispData.opts.csvOutput && ! first) {
            outString ~= ",";
          }
          outString ~= percDisplay (dispData, c, rval, uval);
          break;
        }

        case FTYPE_INODE:
        {
          if (dispData.opts.csvOutput && ! first) {
            outString ~= ",";
          }
          outString ~= inodeDisplay (dispData, c, rval);
          break;
        }

        default:
        {
          break;
        }
      }
    } else {
      outString ~= c;
    }
    first = false;
  } // for each format character

  writeln (outString);
}

auto
findDispSize (real val)
{
  foreach (i, dt; displayTable)
  {
    if (val >= dt.low && val < dt.high)
    {
      return i;
    }
  }

  return displayIdxs['m'];
}

string
blockDisplay (const DisplayData dispData, const dchar c,
        const real val, const size_t dHidx, bool csvOutput)
{
  size_t        idx;
  real          dbs = dispData.dispOpts.dispBlockSize;
  string        suffix = "";

  if (dispData.dispOpts.dbsstr == "h") {
    idx = findDispSize (val);
  }
  if (dispData.dispOpts.dbsstr == "H") {
    idx = dHidx;
  }

  if (dispData.dispOpts.dbsstr == "h" || dispData.dispOpts.dbsstr == "H")
  {
    dbs = displayTable[idx].size;
    suffix = displayTable[idx].suffix;
  }

  auto nval = val / dbs;
  auto fmt = dispData.dispFmtString[c] ~ suffix;
  if (! csvOutput) {
    fmt ~= " ";
  } else {
    fmt = "\"" ~ fmt ~ "\"";
  }
  return format (fmt, nval);
}

string
inodeDisplay (const DisplayData dispData, const dchar c, const real val)
{
  size_t        idx;
  string        suffix = "";

  auto fmt = dispData.dispFmtString[c];
  return format (fmt, val);
}

string
percDisplay (const DisplayData dispData, const dchar c,
    const real tval, const real uval)
{
  real          perc;

  if (tval > 0.0)
  {
    perc = uval / tval;
    perc *= 100;
  }
  else
  {
    perc = 0.0;
  }

  auto fmt = dispData.dispFmtString[c];
  return format (fmt, perc);
}

void
setDispBlockSize (ref DisplayOpts dispOpts)
{
  real          val;
  char          c;

  val = 1024.0 * 1024.0;

  c = dispOpts.dbsstr[0];
  if (isDigit (c)) {
    val = to!(typeof(val))(dispOpts.dbsstr);
    val = floor (val);  // make sure it's an integer value
//writefln ("digit:%s:val=%25.0f", dispOpts.dbsstr, val);
    foreach (disp; displayTable) {
      if (val == disp.size) {
        c = disp.key;
//writefln ("digit:%s", c);
        break;
      }
    }
  } else {
    if (c != 'H') {
      c = cast(char) toLower (c);
    }
    if (dispOpts.dbsstr == "HUMAN") {
      c = 'h';
    }
//writefln ("non-digit:%s:%s", dispOpts.dbsstr, c);
    if (dispOpts.dbsstr.length > 1 && dispOpts.dbsstr[1] == 'B' &&
        dispOpts.baseDispSize != size1000) {
//writefln ("non-digit:1000:%s", c);
      dispOpts.baseDispSize = size1000;
      dispOpts.baseDispIdx = idx1000;
      initializeDisplayTable (dispOpts);
    }
    if (dispOpts.dbsstr.length > 1 && dispOpts.dbsstr[1] == 'b' &&
        dispOpts.baseDispSize != size1024) {
//writefln ("non-digit:1024:%s", c);
      dispOpts.baseDispSize = size1024;
      dispOpts.baseDispIdx = idx1024;
      initializeDisplayTable (dispOpts);
    }
  }

  if (c in displayIdxs) {
    auto idx = displayIdxs [c];
    dispOpts.dispBlockLabel = displayTable [idx].disp [dispOpts.baseDispIdx];
    val = displayTable [idx].size;
    dispOpts.precision = displayTable [idx].precision;
  } else {
    foreach (idx, disp; displayTable)
    {
      if (disp.key == 'h') { break; }
      auto tval = val / disp.size;
      if (floor (tval) == tval)
      {
        auto s = disp.suffix;
        if (idx == 0) { s = "b"; }
        dispOpts.dispBlockLabel = format ("%.0f%s", tval, s);
      }
    }
  }

  if (dispOpts.posixCompat && val == 512.0)
  {
    dispOpts.dispBlockLabel = "512-blocks";
  }
  if (dispOpts.posixCompat && val == 1024.0)
  {
    dispOpts.dispBlockLabel = "1024-blocks";
  }

//writefln ("block label:%s", dispOpts.dispBlockLabel);

  dispOpts.dispBlockSize = val;
//writefln ("block size:%.0f", val);
}

unittest {
  DisplayOpts       dispOpts;
  int               tcount;
  int               failures;

  void
  test (string l, bool posix,
      string dbsstr, real bds, int bdi,
      string rdbl, real rdbs)
  {
    bool      fail;

    ++tcount;
    dispOpts = dispOpts.init;
    dispOpts.dbsstr = dbsstr;
    dispOpts.baseDispSize = bds;
    dispOpts.baseDispIdx = bdi;
    dispOpts.posixCompat = posix;
    setDispBlockSize (dispOpts);
    if (dispOpts.dispBlockLabel != rdbl) { fail = true; }
    if (dispOpts.dispBlockSize != rdbs) { fail = true; }
    if (fail)
    {
      ++failures;
      writefln ("# setDispBlockSize-fail: %s", l);
      writefln ("  using:dbsstr:%s: bds:%.0f: bdi:%d: rdbl:%s: rdbs:%.0f:",
         dbsstr, bds, bdi, rdbl, rdbs);
      writefln ("  expected: %s got %s", rdbl, dispOpts.dispBlockLabel);
      writefln ("  expected: %.0f got %.0f", rdbs, dispOpts.dispBlockSize);
    }
  }

  initializeIdxs ();

  dispOpts.baseDispSize = size1024;
  dispOpts.baseDispIdx = idx1024;
  initializeDisplayTable (dispOpts);
  for (auto i = 0; i < displayTable.length; ++i)
  {
    test (format("%.0f", displayTable[i].size), false,
          format ("%.0f", displayTable[i].size), size1024, idx1024,
          displayTable[i].disp[1], displayTable[i].size);
    test (to!string(displayTable[i].key), false,
          to!string(displayTable[i].key), size1024, idx1024,
          displayTable[i].disp[1], displayTable[i].size);
  }
  test ("HUMAN", false, "HUMAN", size1024, idx1024, "Size", 0);

  dispOpts.baseDispSize = size1000;
  dispOpts.baseDispIdx = idx1000;
  initializeDisplayTable (dispOpts);
  for (auto i = 0; i < displayTable.length; ++i)
  {
    test (format("%.0f", displayTable[i].size), false,
          format ("%.0f", displayTable[i].size), size1000, idx1000,
          displayTable[i].disp[0], displayTable[i].size);
    test (to!string(displayTable[i].key), false,
          to!string(displayTable[i].key), size1000, idx1000,
          displayTable[i].disp[0], displayTable[i].size);
  }
  test ("HUMAN", false, "HUMAN", size1000, idx1000, "Size", 0);

  dispOpts.baseDispSize = size1000;
  dispOpts.baseDispIdx = idx1000;
  initializeDisplayTable (dispOpts);
  for (auto i = 0; i < displayTable.length; ++i)
  {
    test (format ("%sB", displayTable[i].key), false,
          format ("%sB", displayTable[i].key), size1024, idx1024,
          displayTable[i].disp[0], displayTable[i].size);
  }

  test ("2.5", false, "2.5", size1000, idx1000, "2b", 2);
  test ("2", false, "2", size1000, idx1000, "2b", 2);
  test ("2048", false, "2048", size1000, idx1000, "2048b", 2048);
  test ("2000", false, "2000", size1000, idx1000, "2K", 2000);

  dispOpts.baseDispSize = size1024;
  dispOpts.baseDispIdx = idx1024;
  initializeDisplayTable (dispOpts);
  for (auto i = 0; i < displayTable.length; ++i)
  {
    test (format ("%sb", displayTable[i].key), false,
          format ("%sb", displayTable[i].key), size1000, idx1000,
          displayTable[i].disp[1], displayTable[i].size);
  }

  test ("2.5", false, "2.5", size1024, idx1024, "2b", 2.0);
  test ("2", false, "2", size1024, idx1024, "2b", 2.0);
  test ("2048", false, "2048", size1024, idx1024, "2K", 2048.0);
  test ("2000", false, "2000", size1024, idx1024, "2000b", 2000.0);

  test ("Posix-1024", true, "k", size1024, idx1024, "1024-blocks", 1024.0);
  test ("Posix-512", true, "512", size1024, idx1024, "512-blocks", 512.0);

  if (failures > 0) {
    write ("unittest: display: setDispBlockSize: ");
    writefln ("failed: %d of %d", failures, tcount);
  }
}

void
initializeDisplayTable (ref DisplayOpts dispOpts)
{
  displayTable [0].size = 1;
  displayTable [0].low = 1;     // temporary, reset below
  displayTable [0].high = dispOpts.baseDispSize;
  for (auto i = 1; i < displayTable.length; ++i)
  {
    if (displayTable [i].key == 'h') { break; }
    if (displayTable [i].key == 'H') { break; }
    displayTable [i].size = displayTable [i - 1].size * dispOpts.baseDispSize;
    displayTable [i].low = displayTable [i - 1].low * dispOpts.baseDispSize;
    displayTable [i].high = displayTable [i - 1].high * dispOpts.baseDispSize;
  }
  displayTable [0].low = 0;
}

void
initializeIdxs ()
{
  if (! dispIdxsInitialized) {
    foreach (idx, dt; displayTable) {
      displayIdxs[dt.key] = idx;
    }
    dispIdxsInitialized = true;
  }
  if (! fmtInfoIdxsInitialized) {
    foreach (idx, fi; formatTypes) {
      formatTypesIdxs[fi.key] = idx;
    }
    fmtInfoIdxsInitialized = true;
  }
}

void
setMaxLen (string val, ref int max)
{
  int len = cast(int) val.length;
  if (len > max)
  {
    max = len;
  }
}

void
initializeTitles (DisplayOpts dispOpts)
{
  if (dispOpts.posixCompat)
  {
    formatTypes[formatTypesIdxs[FMT_MOUNT_FULL]].title = "Mounted On";
    formatTypes[formatTypesIdxs[FMT_BLOCKS_AVAIL]].title = "Available";
    formatTypes[formatTypesIdxs[FMT_BLOCKS_PERC_NOT_AVAIL]].title = "Capacity";
    formatTypes[formatTypesIdxs[FMT_BLOCKS_PERC_USED]].title = "Capacity";
    formatTypes[formatTypesIdxs[FMT_BLOCKS_PERC_USED_AVAIL]].title = "Capacity";
  }
}

