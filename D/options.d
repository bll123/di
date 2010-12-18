// written in the D programming language

module options;

import std.string;
import std.ctype;
import std.utf;
import std.conv;
import std.process;
debug (1) {
  import std.stdio;
}
version (unittest) {
  import std.stdio;
}

import config;
import display;
import di_getopt;

private immutable string DI_ALL_FORMAT = "MTS\n\tIO\n\tbuf13\n\tbcvpa\n\tBuv2\n\tiUFP";
private immutable string DI_POSIX_FORMAT = "SbuvpM";
private immutable string DI_DEFAULT_FORMAT = "smbuvpT";

enum char
  sortNone = 'n',
  sortMount = 'm',
  sortSpecial = 's',
  sortAvail = 'a',
  sortReverse = 'r',
  sortType = 't',
  sortAscending = 'A',
  sortDescending = 'D';

struct Options {
  bool              displayAll = false;
  bool              hasdashk = false;
  bool              localOnly = false;
  bool              includeLoopback = false;
  bool              noHeader = false;
  bool              noQuotaCheck = false;
  bool              displayTotal = true;
  bool              debugHeader = false;
  bool              unknown = false;
  bool[string]      includeList;
  bool[string]      ignoreList;
  int               debugLevel = 0;
  string            formatString = DI_DEFAULT_FORMAT;
  string            sortType = "m";
  string            zoneDisplay;
}

Options opts;

int
getDIOptions (string[] args) {
  int       idx;
  string    s;

  with (opts) {
    if ((s = getenv ("DIFMT")) != null)
    {
      formatString = s;
    }

      // gnu df
    if ((s = getenv ("POSIXLY_CORRECT")) != null)
    {
      dispOpts.dbsstr = "512";
      formatString = DI_POSIX_FORMAT;
      dispOpts.posixCompat = true;
    }

      // bsd df
    if ((s = getenv ("BLOCKSIZE")) != null)
    {
      dispOpts.dbsstr = s;
    }

        // gnu df
    if ((s = getenv ("DF_BLOCK_SIZE")) != null)
    {
      dispOpts.dbsstr = s;
    }

    if ((s = getenv ("DI_ARGS")) != null)
    {
      processOpts (s);
    }

    idx = processOpts (args);
  }

  return idx;
}

private:

int
processOpts (string str) {
  return processOpts (["dummy"] ~ split(strip(str)));
}

int
processOpts (string[] args)
{
  int       idx;

  with (opts) {
    idx = getopt (args,
      "A", { formatString = DI_ALL_FORMAT; return 1; },
      "a", &displayAll,
      "b", &processBaseSize,
      "B", &processBaseSize,
      "d", (string arg) { dispOpts.dbsstr = arg; if (dispOpts.dbsstr == "k") { hasdashk = true; } },
      "f", &formatString,
      "F", &processIncludeList,     // solaris df compatibility
      "g", { dispOpts.dbsstr = "g"; return 1; },
      "h", { dispOpts.dbsstr = "h"; return 1; },
      "H", { dispOpts.dbsstr = "H"; return 1; },
      "I", &processIncludeList,
      "k", { dispOpts.dbsstr = "k"; hasdashk = true; return 1; },
      "l", &localOnly,
      "L", &includeLoopback,
      "m", { dispOpts.dbsstr = "m"; return 1; },
      "n", &noHeader,
      "P", { if (!hasdashk) { dispOpts.dbsstr = "512"; }
             formatString = DI_POSIX_FORMAT;
             dispOpts.posixCompat = true;
             return 1; },
      "q", &noQuotaCheck,
      "s", &processSort,
      "t", &displayTotal,
      "w", &dispOpts.width,
      "W", &dispOpts.inodeWidth,
      "x", &processIgnoreList,
      "X", &debugLevel,
      "z", &zoneDisplay,
      "Z", { zoneDisplay = "all"; return 1; }
      );
  }

  return idx;
}

version (unittest) {
  mixin template MoptionsTest(T) {
    void
    test (string l, bool expected,
        string o, ref T var, T rv, int ri)
    {
      int       i;
      bool      fail;

      ++tcount;
      opts = opts.init;
      i = processOpts (o);
writeln ("### OA:i:", i, ":ri:", ri);
writeln ("### OA:var:", var, ":rv:", rv);
      if (var != rv) { fail = true; }
      if (i != ri) { fail = true; }
      if (fail)
      {
        ++failures;
      }
      writefln ("# %s: %s: %d: %s",
        "processOpts:", fail ? "fail" : "pass", tcount, l);
    }
  }
}

unittest {
  int       i;

  int       failures;
  int       tcount;
  string    sarr[];

  mixin MoptionsTest!bool tbool;
  mixin MoptionsTest!string tstr;

  with (opts) {
    tbool.test ("displayAll", true, "-a", displayAll, true, 1);
    tstr.test ("dispOpts.dbsstr", true, "-d k", dispOpts.dbsstr, "k", 2);
    if (hasdashk != true) { ++failures; }
    tstr.test ("dispOpts.dbsstr", true, "k", dispOpts.dbsstr, "k", 1);
    if (hasdashk != true) { ++failures; }

    // 6
    ++tcount;
    opts = opts.init;
    auto s = "-k";
    i = processOpts (s);
    if (dispOpts.dbsstr != "k") { ++failures; }
    if (hasdashk != true) { ++failures; }

    // 7
    ++tcount;
    opts = opts.init;
    s = "-d g";
    i = processOpts (s);
    if (dispOpts.dbsstr != "g") { ++failures; }
    if (hasdashk != false) { ++failures; }

    // 8
    ++tcount;
    opts = opts.init;
    s = "-g";
    i = processOpts (s);
    if (dispOpts.dbsstr != "g") { ++failures; }
    if (hasdashk != false) { ++failures; }

    // 9
    ++tcount;
    opts = opts.init;
    s = "-d m";
    i = processOpts (s);
    if (dispOpts.dbsstr != "m") { ++failures; }
    if (hasdashk != false) { ++failures; }

    // 10
    ++tcount;
    opts = opts.init;
    s = "-m";
    i = processOpts (s);
    if (dispOpts.dbsstr != "m") { ++failures; }
    if (hasdashk != false) { ++failures; }

    // 11
    ++tcount;
    opts = opts.init;
    s = "-d h";
    i = processOpts (s);
    if (dispOpts.dbsstr != "h") { ++failures; }
    if (hasdashk != false) { ++failures; }

    // 12
    ++tcount;
    opts = opts.init;
    s = "-h";
    i = processOpts (s);
    if (dispOpts.dbsstr != "h") { ++failures; }
    if (hasdashk != false) { ++failures; }

    // 13
    ++tcount;
    opts = opts.init;
    s = "-d H";
    i = processOpts (s);
    if (dispOpts.dbsstr != "H") { ++failures; }
    if (hasdashk != false) { ++failures; }

    // 14
    ++tcount;
    opts = opts.init;
    s = "-H";
    i = processOpts (s);
    if (dispOpts.dbsstr != "H") { ++failures; }
    if (hasdashk != false) { ++failures; }

    // 15
    ++tcount;
    opts = opts.init;
    s = "-A";
    i = processOpts (s);
    if (formatString != DI_ALL_FORMAT) { ++failures; }

    // 16
    ++tcount;
    opts = opts.init;
    s = "-b 1000";
    i = processOpts (s);
    if (dispOpts.baseDispSize != 1000.0) { ++failures; }

    // 17
    ++tcount;
    opts = opts.init;
    s = "-b d";
    i = processOpts (s);
    if (dispOpts.baseDispSize != 1000.0) { ++failures; }

    // 18
    ++tcount;
    opts = opts.init;
    s = "-b k";
    i = processOpts (s);
    if (dispOpts.baseDispSize != 1024.0) { ++failures; }

    // 19
    ++tcount;
    opts = opts.init;
    s = "-f smbuvpT";
    i = processOpts (s);
    if (formatString != "smbuvpT") { ++failures; }

    // 20
    ++tcount;
    opts = opts.init;
    s = "-I nfs";
    i = processOpts (s);
    if (includeList.keys != ["nfs"]) { ++failures; }

    // 21
    ++tcount;
    opts = opts.init;
    s = "-I nfs,jfs";
    i = processOpts (s);
    sarr = includeList.keys;
    if (sarr.sort != ["jfs", "nfs"]) { ++failures; }

    // 22
    ++tcount;
    opts = opts.init;
    s = "-I nfs -I jfs";
    i = processOpts (s);
    sarr = includeList.keys;
    if (sarr.sort != ["jfs", "nfs"]) { ++failures; }

    // 23
    ++tcount;
    opts = opts.init;
    s = "-l";
    i = processOpts (s);
    if (localOnly != true) { ++failures; }

    // 24
    ++tcount;
    opts = opts.init;
    s = "-L";
    i = processOpts (s);
    if (includeLoopback != true) { ++failures; }

    // 25
    ++tcount;
    opts = opts.init;
    s = "-n";
    i = processOpts (s);
    if (noHeader != true) { ++failures; }

    // 26
    ++tcount;
    opts = opts.init;
    s = "-P";
    i = processOpts (s);
    if (hasdashk && dispOpts.dbsstr != "k") { ++failures; }
    if (! hasdashk && dispOpts.dbsstr != "512") { ++failures; }
    if (dispOpts.posixCompat != true) { ++failures; }
    if (formatString != DI_POSIX_FORMAT) { ++failures; }

    // 27
    ++tcount;
    opts = opts.init;
    s = "-q";
    i = processOpts (s);
    if (noQuotaCheck != true) { ++failures; }

    // 28
    ++tcount;
    opts = opts.init;
    s = "-s r";
    i = processOpts (s);
    if (sortType != "rm") { ++failures; }

    // 29
    ++tcount;
    opts = opts.init;
    s = "-s t";
    i = processOpts (s);
    if (sortType != "tm") { ++failures; }

    // 30
    ++tcount;
    opts = opts.init;
    s = "-s trsrm";
    i = processOpts (s);
    if (sortType != "trsrm") { ++failures; }

    // 31
    ++tcount;
    opts = opts.init;
    s = "-t";
    i = processOpts (s);
    if (displayTotal != true) { ++failures; }

    // 32
    ++tcount;
    opts = opts.init;
    s = "-w 12";
    i = processOpts (s);
    if (dispOpts.width != 12) { ++failures; }

    // 33
    ++tcount;
    opts = opts.init;
    s = "-W 12";
    i = processOpts (s);
    if (dispOpts.inodeWidth != 12) { ++failures; }

    // 34
    ++tcount;
    opts = opts.init;
    s = "-x nfs";
    i = processOpts (s);
    if (ignoreList.keys != ["nfs"]) { ++failures; }

    // 35
    ++tcount;
    opts = opts.init;
    s = "-x nfs,jfs";
    i = processOpts (s);
    sarr = ignoreList.keys;
    if (sarr.sort != ["jfs", "nfs"]) { ++failures; }

    // 36
    ++tcount;
    opts = opts.init;
    s = "-x nfs -x jfs";
    i = processOpts (s);
    sarr = ignoreList.keys;
    if (sarr.sort != ["jfs", "nfs"]) { ++failures; }

    // 37
    ++tcount;
    opts = opts.init;
    s = "-z some";
    i = processOpts (s);
    if (zoneDisplay != "some") { ++failures; }

    // 38
    ++tcount;
    opts = opts.init;
    s = "-Z";
    i = processOpts (s);
    if (zoneDisplay != "all") { ++failures; }

    // 39
    ++tcount;
    opts = opts.init;
    s = "-X 4";
    i = processOpts (s);
    if (debugLevel != 4) { ++failures; }
  }

  write ("unittest: options: processOpts: ");
  if (failures > 0) {
    writefln ("failed: %d of %d", failures, tcount);
  } else {
    writeln ("passed");
  }
}

void
processBaseSize (string opt, string arg)
{
  size_t    i = 0;
  dchar     c;

  with (dispOpts) {
    c = decode (arg, i);
    if (isdigit (c)) {
      baseDispSize = to!(typeof(baseDispSize))(arg);
      baseDispIdx = idx1000; // unknown, really
      if (baseDispSize == 1024.0) {
        baseDispIdx = idx1024;
      }
    }
    else if (arg == "k") {
      baseDispSize = size1024;
      baseDispIdx = idx1024;
    }
    else if (arg == "d" || arg == "si") {
      baseDispSize = size1000;
      baseDispIdx = idx1000;
    }
  }
}

void
processSort (string arg)
{
  with (opts) {
    sortType = arg;
    if (arg == "r") {   // backwards compatibility
      sortType = "rm";
    }
    if (arg == "t") {   // add some additional sorting...
      sortType = "tm";
    }
  }
}

void
processIncludeList (string arg)
{
  foreach (str; split (arg, ",")) {
    opts.includeList[str] = true;
  }
}

void
processIgnoreList (string arg)
{
  foreach (str; split (arg, ",")) {
    opts.ignoreList[str] = true;
  }
}

