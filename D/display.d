// written in the D programming language

module di_display;

import std.stdio;
import std.string;
import std.ctype : isdigit;
import std.conv : tolower, to;
private import std.math : floor;

import config;
import options;
import dispopts;
import diskpart;

struct DisplayData
{
  Options           *opts;
  DisplayOpts       *dispOpts;
  DiskPartitions    *dpList;
  string            titleString;
  string[dchar]     dispFmtString;
};

void
displayAll (ref Options opts, ref DisplayOpts dispOpts,
    ref DiskPartitions dpList)
{
  DisplayData       dispData;

  initializeDisplayIdxs ();
  initializeDisplayTable (dispOpts);
  setDispBlockSize (dispOpts);
  if (opts.debugLevel > 10) {
    dumpDispTable ();
  }

  // why isn't there a more better way to do this?
  dispData.opts = &opts;
  dispData.dispOpts = &dispOpts;
  dispData.dpList = &dpList;

  findMaximums (dispData);
  buildFormat (dispData);
  displayTitle (dispData);
  displayPartitions (dispData);
}

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

enum char
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
  fmtInodesTot = 'i',
  fmtInodesUsed = 'U',
  fmtInodesFree = 'F',
  fmtInodesPerc = 'P',
  fmtMountTime = 'I',
  fmtMountOptions = 'O';

struct DisplayTable {
  real          size;
  real          low;
  real          high;
  char          key;
  string        suffix;
  string        disp[2];
};

DisplayTable[] displayTable = [
  // size == low except for bytes
  // size,low,high,key,suffix,disp[2]
  { 0, 0, 0, 'b', " ", [ "Bytes", "Bytes" ] },
  { 0, 0, 0, 'k', "K", [ "KBytes", "KBytes" ] },
  { 0, 0, 0, 'm', "M", [ "Megs", "Mebis" ] },
  { 0, 0, 0, 'g', "G", [ "Gigs", "Gibis" ] },
  { 0, 0, 0, 't', "T", [ "Teras", "Tebis" ] },
  { 0, 0, 0, 'p', "P", [ "Petas", "Pebis" ] },
  { 0, 0, 0, 'e', "E", [ "Exas", "Exbis" ] },
  { 0, 0, 0, 'z', "Z", [ "Zettas", "Zebis" ] },
  { 0, 0, 0, 'y', "Y", [ "Yottas", "Yobis" ] },
  { 0, 0, 0, 'h', "h", [ "Size", "Size" ] },
  { 0, 0, 0, 'H', "H", [ "Size", "Size" ] }
  ];

size_t[char]    displayIdxs;
bool            dispIdxsInitialized;

void
dumpDispTable ()
{
  foreach (disp; displayTable) {
    writefln ("%s %s %-6s sz:%25.0f\n  low:%25.0f high:%25.0f",
        disp.key, disp.suffix, disp.disp[0], disp.size, disp.low, disp.high);
  }
}

void
findMaximums (ref DisplayData dispData)
{
  foreach (dp; dispData.dpList.diskPartitions)
  {
  }
}

void
buildFormat (ref DisplayData dispData)
{
  foreach (dchar c; dispData.opts.formatString)
  {
    switch (c)
    {
      case fmtMount:
      {
        dispData.dispFmtString[c] = "%s ";
        dispData.titleString ~= format ("%s ", "Mount");
        break;
      }

      case fmtMountFull:
      {
        dispData.dispFmtString[c] = "%s ";
        dispData.titleString ~= format ("%s ", "MountF");
        break;
      }

      case fmtSpecial:
      {
        dispData.dispFmtString[c] = "%s ";
        dispData.titleString ~= format ("%s ", "Special");
        break;
      }

      case fmtSpecialFull:
      {
        dispData.dispFmtString[c] = "%s ";
        dispData.titleString ~= format ("%s ", "SpecialF");
        break;
      }

      case fmtType:
      {
        dispData.dispFmtString[c] = "%s ";
        dispData.titleString ~= format ("%s ", "Type");
        break;
      }

      case fmtTypeFull:
      {
        dispData.dispFmtString[c] = "%s ";
        dispData.titleString ~= format ("%s ", "TypeF");
        break;
      }

      case fmtBlocksTot:
      {
        dispData.dispFmtString[c] = "%s ";
        dispData.titleString ~= format ("%s ", "BlocksT");
        break;
      }

      case fmtBlocksTotAvail:
      {
        dispData.dispFmtString[c] = "%s ";
        dispData.titleString ~= format ("%s ", "BlocksTA");
        break;
      }

      case fmtBlocksUsed:
      {
        dispData.dispFmtString[c] = "%s ";
        dispData.titleString ~= format ("%s ", "BlocksU");
        break;
      }

      case fmtBlocksCalcUsed:
      {
        dispData.dispFmtString[c] = "%s ";
        dispData.titleString ~= format ("%s ", "BlocksC");
        break;
      }

      case fmtBlocksFree:
      {
        dispData.dispFmtString[c] = "%s ";
        dispData.titleString ~= format ("%s ", "BlocksF");
        break;
      }

      case fmtBlocksAvail:
      {
        dispData.dispFmtString[c] = "%s ";
        dispData.titleString ~= format ("%s ", "BlocksA");
        break;
      }

      case fmtBlocksPercNotAvail:
      {
        dispData.dispFmtString[c] = "%s%% ";
        dispData.titleString ~= format ("%s ", "Blocks%NA");
        break;
      }

      case fmtBlocksPercUsed:
      {
        dispData.dispFmtString[c] = "%s%% ";
        dispData.titleString ~= format ("%s ", "Blocks%U");
        break;
      }

      case fmtBlocksPercBSD:
      {
        dispData.dispFmtString[c] = "%s%% ";
        dispData.titleString ~= format ("%s ", "Blocks%B");
        break;
      }

      case fmtInodesTot:
      {
        dispData.dispFmtString[c] = "%s ";
        dispData.titleString ~= format ("%s ", "InodeT");
        break;
      }

      case fmtInodesUsed:
      {
        dispData.dispFmtString[c] = "%s ";
        dispData.titleString ~= format ("%s ", "InodeU");
        break;
      }

      case fmtInodesFree:
      {
        dispData.dispFmtString[c] = "%s ";
        dispData.titleString ~= format ("%s ", "InodeF");
        break;
      }

      case fmtInodesPerc:
      {
        dispData.dispFmtString[c] = "%s%% ";
        dispData.titleString ~= format ("%s ", "Inode%");
        break;
      }

      case fmtMountTime:
      {
        dispData.dispFmtString[c] = "%s ";
        dispData.titleString ~= format ("%s ", "MT");
        break;
      }

      case fmtMountOptions:
      {
        dispData.dispFmtString[c] = "%s ";
        dispData.titleString ~= format ("%s ", "Opt");
        break;
      }

      default:
      {
        dispData.titleString ~= " ";
        break;
      }
    }
  }
}

void
displayTitle (ref DisplayData dispData)
{
  if (dispData.opts.noHeader) { return; }
  writeln (dispData.titleString);
}

void
displayPartitions (ref DisplayData dispData)
{
  foreach (dp; dispData.dpList.diskPartitions)
  {
    string        temp;

    if (dp.printFlag != dispData.dpList.DI_PRNT_OK)
    {
      continue;
    }

    foreach (dchar c; dispData.opts.formatString)
    {
      switch (c)
      {
        case fmtMount:
        {
          temp ~= format (dispData.dispFmtString[c], dp.name);
          break;
        }

        case fmtMountFull:
        {
          temp ~= format (dispData.dispFmtString[c], dp.name);
          break;
        }

        case fmtSpecial:
        {
          temp ~= format (dispData.dispFmtString[c], dp.special);
          break;
        }

        case fmtSpecialFull:
        {
          temp ~= format (dispData.dispFmtString[c], dp.special);
          break;
        }

        case fmtType:
        {
          temp ~= format (dispData.dispFmtString[c], dp.fsType);
          break;
        }

        case fmtTypeFull:
        {
          temp ~= format (dispData.dispFmtString[c], dp.fsType);
          break;
        }

        case fmtBlocksTot:
        {
          temp ~= format (dispData.dispFmtString[c],
              dp.totalBlocks * dp.blockSize);
          break;
        }

        case fmtBlocksTotAvail:
        {
          temp ~= format (dispData.dispFmtString[c],
              dp.availBlocks * dp.blockSize);
          break;
        }

        case fmtBlocksUsed:
        {
          break;
        }

        case fmtBlocksCalcUsed:
        {
          break;
        }

        case fmtBlocksFree:
        {
          break;
        }

        case fmtBlocksAvail:
        {
          break;
        }

        case fmtBlocksPercNotAvail:
        {
          break;
        }

        case fmtBlocksPercUsed:
        {
          break;
        }

        case fmtBlocksPercBSD:
        {
          break;
        }

        case fmtInodesTot:
        {
          break;
        }

        case fmtInodesUsed:
        {
          break;
        }

        case fmtInodesFree:
        {
          break;
        }

        case fmtInodesPerc:
        {
          break;
        }

        case fmtMountTime:
        {
          break;
        }

        case fmtMountOptions:
        {
          break;
        }

        default:
        {
          temp ~= c;
          break;
        }
      }
    }

    writeln (temp);
//    writefln ("%s %s %.1fG", dp.name, dp.special,
//        dp.totalBlocks * dp.blockSize / 1024.0 / 1024.0 / 1024.0);
  }
}

void
setDispBlockSize (ref DisplayOpts dispOpts)
{
  real          val;
  char          c;

  val = 1024.0 * 1024.0;

  c = dispOpts.dbsstr[0];
  if (isdigit (c)) {
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
      c = cast(char) tolower (c);
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

  initializeDisplayIdxs ();

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
initializeDisplayIdxs ()
{
  if (! dispIdxsInitialized) {
    foreach (idx, dt; displayTable) {
      displayIdxs[dt.key] = idx;
    }
    dispIdxsInitialized = true;
  }
}
