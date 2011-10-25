// written in the D programming language

module didisplay;

import std.stdio;
import std.string;
import std.ascii : isDigit;
import std.uni : toLower;
import std.conv : to, chop;
private import std.math : floor;

import config;
import options;
import dispopts;
import diskpart;
import dilocale;

private:

enum char
  sortNone = 'n',
  sortMount = 'm',
  sortSpecial = 's',
  sortAvail = 'a',
  sortReverse = 'r',
  sortType = 't',
  sortAscending = 'A',
  sortDescending = 'D';

enum dchar
  fmtMount = 'm',
  fmtMountFull = 'M',
  fmtSpecial = 's',
  fmtSpecialFull = 'S',
  fmtType = 't',
  fmtTypeFull = 'T',
  fmtBlocksTot = 'b',
  fmtBlocksTotAvail = 'B',
  fmtBlocksUsed = 'u',
  fmtBlocksCalcUsed = 'c',
  fmtBlocksFree = 'f',
  fmtBlocksAvail = 'v',
  fmtBlocksPercNotAvail = 'p',
  fmtBlocksPercUsed = '1',
  fmtBlocksPercBSD = '2',
  fmtBlocksPercAvail = 'a',
  fmtBlocksPercFree = '3',
  fmtInodesTot = 'i',
  fmtInodesUsed = 'U',
  fmtInodesFree = 'F',
  fmtInodesPerc = 'P',
  fmtMountTime = 'I',
  fmtMountOptions = 'O';

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
  { fmtMount,               FTYPE_STRING,    15, "Mount" },
  { fmtMountFull,           FTYPE_STRING,    15, "Mount" },
  { fmtSpecial,             FTYPE_STRING,    15, "Filesystem" },
  { fmtSpecialFull,         FTYPE_STRING,    15, "Filesystem" },
  { fmtType,                FTYPE_STRING,    15, "fsType" },
  { fmtTypeFull,            FTYPE_STRING,    15, "fs Type" },
  { fmtBlocksTot,           FTYPE_SPACE,      8, "" },
  { fmtBlocksTotAvail,      FTYPE_SPACE,      8, "" },
  { fmtBlocksUsed,          FTYPE_SPACE,      8, "Used" },
  { fmtBlocksCalcUsed,      FTYPE_SPACE,      8, "Used" },
  { fmtBlocksFree,          FTYPE_SPACE,      8, "Free" },
  { fmtBlocksAvail,         FTYPE_SPACE,      8, "Avail" },
  { fmtBlocksPercNotAvail,  FTYPE_PERC_SPACE, 3, "%Used" },
  { fmtBlocksPercUsed,      FTYPE_PERC_SPACE, 3, "%Used" },
  { fmtBlocksPercBSD,       FTYPE_PERC_SPACE, 3, "%Used" },
  { fmtBlocksPercAvail,     FTYPE_PERC_SPACE, 3, "%Free" },
  { fmtBlocksPercFree,      FTYPE_PERC_SPACE, 3, "%Free" },
  { fmtInodesTot,           FTYPE_INODE,      7, "Inodes" },
  { fmtInodesUsed,          FTYPE_INODE,      7, "IUsed" },
  { fmtInodesFree,          FTYPE_INODE,      7, "IFree" },
  { fmtInodesPerc,          FTYPE_PERC_INODE, 3, "%IUsed" },
  { fmtMountTime,           FTYPE_STRING,    15, "Mount Time" },
  { fmtMountOptions,        FTYPE_STRING,    15, "Options" }
];

size_t[dchar]   formatTypesIdxs;
bool            fmtInfoIdxsInitialized;

struct DisplayData
{
  Options           *opts;
  DisplayOpts       *dispOpts;
  DiskPartitions    *dpList;
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
  // size,low,high,key,suffix,disp[2]
  { 0, 0, 0, 0, 'b', " ", [ "Bytes", "Bytes" ] },
  { 0, 0, 0, 0, 'k', "K", [ "KBytes", "KBytes" ] },
  { 0, 0, 0, 1, 'm', "M", [ "Megs", "Mebis" ] },
  { 0, 0, 0, 1, 'g', "G", [ "Gigs", "Gibis" ] },
  { 0, 0, 0, 1, 't', "T", [ "Teras", "Tebis" ] },
  { 0, 0, 0, 1, 'p', "P", [ "Petas", "Pebis" ] },
  { 0, 0, 0, 1, 'e', "E", [ "Exas", "Exbis" ] },
  { 0, 0, 0, 1, 'z', "Z", [ "Zettas", "Zebis" ] },
  { 0, 0, 0, 1, 'y', "Y", [ "Yottas", "Yobis" ] },
  { 0, 0, 0, 1, 'h', "h", [ "Size", "Size" ] },
  { 0, 0, 0, 1, 'H', "H", [ "Size", "Size" ] }
  ];

size_t[char]    displayIdxs;
bool            dispIdxsInitialized;

public:

void
doDisplay (ref Options opts, ref DisplayOpts dispOpts,
    ref DiskPartitions dpList)
{
  DisplayData       dispData;

  dispData.opts = &opts;
  dispData.dispOpts = &dispOpts;
  dispData.dpList = &dpList;

  initializeIdxs ();
  initializeTitles (dispOpts);
  initializeDisplayTable (dispOpts);
  setDispBlockSize (dispOpts);
  if (opts.debugLevel > 30)
  {
    dumpDispTable ();
  }

  buildDisplayList (dispData);
  displayTitle (dispData);
  displayPartitions (dispData);
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
  int[dchar] dpMax;

  foreach (dchar c; dispData.opts.formatString)
  {
    if (c in formatTypesIdxs &&
        formatTypes[formatTypesIdxs[c]].ftype == FTYPE_STRING)
    {
      dpMax[c] = 0;
    }
  }

  foreach (dp; dispData.dpList.diskPartitions)
  {
    if (dp.printFlag != dp.DI_PRINT_OK)
    {
      continue;
    }

    if (fmtSpecialFull in dpMax) {
      setMaxLen (dp.special, dpMax[fmtSpecialFull]);
    }
    if (fmtMountFull in dpMax) {
      setMaxLen (dp.name, dpMax[fmtMountFull]);
    }
    if (fmtTypeFull in dpMax) {
      setMaxLen (dp.fsType, dpMax[fmtTypeFull]);
    }
    if (fmtMountOptions in dpMax) {
//    setMaxLen (dp.mountOptions, dpMax[fmtMountOptions]);
    }
    if (fmtMountTime in dpMax) {
      setMaxLen (dp.mountTime, dpMax[fmtMountTime]);
    }
  }

  foreach (dchar c; dispData.opts.formatString)
  {
    if (c in formatTypesIdxs) {
      auto title = DI_GT (formatTypes[formatTypesIdxs[c]].title);
      auto ftype = formatTypes[formatTypesIdxs[c]].ftype;
      auto width = formatTypes[formatTypesIdxs[c]].width;

      switch (c)
      {
        case fmtMountFull:
        case fmtSpecialFull:
        case fmtTypeFull:
        case fmtMountTime:
        case fmtMountOptions:
        {
          width = dpMax[c];
          break;
        }

        case fmtBlocksTot:
        case fmtBlocksTotAvail:
        {
          title = DI_GT (dispData.dispOpts.dispBlockLabel);
          break;
        }

        default:
        {
          break;
        }
      }

      switch (ftype)
      {
        case FTYPE_STRING:
        {
          if (title.length > width)
          {
            width = title.length;
          }
          auto fmt = format ("%%-%ds ", width);
          dispData.dispFmtString[c] = fmt;
          dispData.titleString ~= format (fmt, title);
          break;
        }

        case FTYPE_SPACE:
        {
          if (title.length > width)
          {
            width = title.length;
          }
          if (dispData.dispOpts.width != 0) {
            width = dispData.dispOpts.width;
          }
          auto twidth = width;
          if (dispData.dispOpts.dbsstr == "h" || dispData.dispOpts.dbsstr == "H")
          {
            --twidth;
          }
          auto fmt = format ("%%%d.%df", twidth, dispData.dispOpts.precision);
          dispData.dispFmtString[c] = fmt;
          fmt = format ("%%%ds ", width);
          dispData.titleString ~= format (fmt, title);
          break;
        }

        case FTYPE_PERC_SPACE:
        {
          auto fmt = "";
          if (title.length > width)
          {
            width = title.length;
          }
          auto twidth = width - 2;
          if (twidth < 1) { twidth = 1; }
          fmt ~= format ("%%%d.0f%%%%  ", twidth);
          dispData.dispFmtString[c] = fmt;
          fmt = format ("%%%ds ", width);
          dispData.titleString ~= format (fmt, title);
          break;
        }

        case FTYPE_INODE:
        {
          if (title.length > width)
          {
            width = title.length;
          }
          if (dispData.dispOpts.inodeWidth != 0) {
            width = dispData.dispOpts.inodeWidth;
          }
          auto fmt = format ("%%%d.0f ", width);
          dispData.dispFmtString[c] = fmt;
          fmt = format ("%%%ds ", width);
          dispData.titleString ~= format (fmt, title);
          break;
        }

        case FTYPE_PERC_INODE:
        {
          auto fmt = format ("%%%d.0f%%%% ", width);
          dispData.dispFmtString[c] = fmt;
          fmt = format ("%%%ds ", width);
          dispData.titleString ~= format (fmt, title);
          break;
        }

        default:
        {
          break;
        }
      } // switch on format type
    } else {
      dispData.titleString ~= format ("%s", c);
    }
  } // for each format string char
}

void
displayTitle (ref DisplayData dispData)
{
  if (! dispData.opts.printHeader) { return; }
  writeln (dispData.titleString);
}


void
displayPartitions (ref DisplayData dispData)
{
  foreach (dp; dispData.dpList.diskPartitions)
  {
    if (dp.printFlag != dp.DI_PRINT_OK)
    {
      continue;
    }

    string      outString;
    string      sval;
    real        rval;
    real        uval;
    size_t      dHidx;

    if (dispData.dispOpts.dbsstr == "H") {
      real mval = 0.0;

      foreach (dchar c; dispData.opts.formatString)
      {
        if (c in formatTypesIdxs) {
          auto ftype = formatTypes[formatTypesIdxs[c]].ftype;

          if (ftype == FTYPE_SPACE) {
            switch (c)
            {
              case fmtBlocksTot:
              {
                rval = dp.totalBlocks;
                break;
              }

              case fmtBlocksTotAvail:
              {
                rval = dp.totalBlocks - (dp.freeBlocks - dp.availBlocks);
                break;
              }

              case fmtBlocksUsed:
              {
                rval = dp.totalBlocks - dp.freeBlocks;
                break;
              }

              case fmtBlocksCalcUsed:
              {
                rval = dp.totalBlocks - dp.availBlocks;
                break;
              }

              case fmtBlocksFree:
              {
                rval = dp.freeBlocks;
                break;
              }

              case fmtBlocksAvail:
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
          case fmtMount:
          case fmtMountFull:
          {
            sval = dp.name;
            break;
          }

          case fmtSpecial:
          case fmtSpecialFull:
          {
            sval = dp.special;
            break;
          }

          case fmtType:
          case fmtTypeFull:
          {
            sval = dp.fsType;
            break;
          }

          case fmtBlocksTot:
          {
            rval = dp.totalBlocks;
            break;
          }

          case fmtBlocksTotAvail:
          {
            rval = dp.totalBlocks - (dp.freeBlocks - dp.availBlocks);
            break;
          }

          case fmtBlocksUsed:
          {
            rval = dp.totalBlocks - dp.freeBlocks;
            break;
          }

          case fmtBlocksCalcUsed:
          {
            rval = dp.totalBlocks - dp.availBlocks;
            break;
          }

          case fmtBlocksFree:
          {
            rval = dp.freeBlocks;
            break;
          }

          case fmtBlocksAvail:
          {
            rval = dp.availBlocks;
            break;
          }

          case fmtBlocksPercNotAvail:
          {
            rval = dp.totalBlocks;
            uval = dp.totalBlocks - dp.availBlocks;
            break;
          }

          case fmtBlocksPercUsed:
          {
            rval = dp.totalBlocks;
            uval = dp.totalBlocks - dp.freeBlocks;
            break;
          }

          case fmtBlocksPercBSD:
          {
            rval = dp.totalBlocks - (dp.freeBlocks - dp.availBlocks);
            uval = dp.totalBlocks - dp.freeBlocks;
            break;
          }

          case fmtBlocksPercAvail:
          {
            rval = dp.totalBlocks;
            uval = dp.availBlocks;
            break;
          }

          case fmtBlocksPercFree:
          {
            rval = dp.totalBlocks;
            uval = dp.freeBlocks;
            break;
          }

          case fmtInodesTot:
          {
            rval = dp.totalInodes;
            break;
          }

          case fmtInodesUsed:
          {
            rval = dp.totalInodes - dp.freeInodes;
            break;
          }

          case fmtInodesFree:
          {
            rval = dp.freeInodes;
            break;
          }

          case fmtInodesPerc:
          {
            rval = dp.totalInodes;
            uval = dp.totalInodes - dp.availInodes;
            break;
          }

          case fmtMountTime:
          {
            sval = dp.mountTime;
            break;
          }

          case fmtMountOptions:
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
            outString ~= format (dispData.dispFmtString[c], sval);
            break;
          }

          case FTYPE_SPACE:
          {
            outString ~= blockDisplay (dispData, c, rval, dHidx);
            break;
          }

          case FTYPE_PERC_SPACE:
          case FTYPE_PERC_INODE:
          {
            outString ~= percDisplay (dispData, c, rval, uval);
            break;
          }

          case FTYPE_INODE:
          {
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
    } // for each format character

    writeln (outString);
  } // for each disk partition
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
        const real val, const size_t dHidx)
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
  auto fmt = dispData.dispFmtString[c] ~ suffix ~ " ";
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
//writefln ("found:%s", c);
    auto idx = displayIdxs [c];
    dispOpts.dispBlockLabel = displayTable [idx].disp [dispOpts.baseDispIdx];
    val = displayTable [idx].size;
    dispOpts.precision = displayTable [idx].precision;
  } else {
//writefln ("not found:%s", c);
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
      writefln ("# %s: %s: %s",
        "setDispBlockSize:", "fail", l);
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

  write ("unittest: display: setDispBlockSize: ");
  if (failures > 0) {
    writefln ("failed: %d of %d", failures, tcount);
  } else {
    writeln ("passed");
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
    formatTypes[formatTypesIdxs[fmtMountFull]].title = "Mounted On";
    formatTypes[formatTypesIdxs[fmtBlocksAvail]].title = "Available";
    formatTypes[formatTypesIdxs[fmtBlocksPercNotAvail]].title = "Capacity";
    formatTypes[formatTypesIdxs[fmtBlocksPercUsed]].title = "Capacity";
    formatTypes[formatTypesIdxs[fmtBlocksPercBSD]].title = "Capacity";
  }
}
