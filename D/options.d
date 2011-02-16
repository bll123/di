// written in the D programming language

module options;

import std.string;
import std.ctype : isdigit;
import std.utf;
import std.conv : to;
private import std.process : getenv, setenv, unsetenv;
debug (1) {
  import std.stdio;
}
version (unittest) {
  import std.stdio;
}

import config;
import dispopts;
import di_getopt;

private immutable string DI_ALL_FORMAT = "MTS\n\tIO\n\tbuf13\n\tbcvpa\n\tBuv2\n\tiUFP";
private immutable string DI_POSIX_FORMAT = "SbuvpM";
private immutable string DI_DEFAULT_FORMAT = "smbuvpT";
private immutable string DI_DEFAULT_SORT = "m";

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
  short             debugLevel = 0;
  string            formatString = DI_DEFAULT_FORMAT;
  string            sortType = DI_DEFAULT_SORT;
  string            zoneDisplay;
}

int
getDIOptions (string[] args, ref Options opts, ref DisplayOpts dispOpts) {
  int       idx;
  string    s;

  if ((s = getenv ("DIFMT")) != null)
  {
    opts.formatString = s;
  }

    // gnu df
  if ((s = getenv ("POSIXLY_CORRECT")) != null)
  {
    dispOpts.dbsstr = "512";
    opts.formatString = DI_POSIX_FORMAT;
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
    processOpts (s, opts, dispOpts);
  }

  idx = processOpts (args, opts, dispOpts);

  return idx;
}

unittest {
  bool          fail;
  int           failures;
  int           tcount;
  Options       opts;
  DisplayOpts   dispOpts;

  ++tcount;
  fail = false;
  opts = opts.init;
  setenv ("DIFMT", "SMT", true);
  getDIOptions ([""], opts, dispOpts);
  if (opts.formatString != "SMT") { fail = true; }
  if (fail)
  {
    ++failures;
    writefln ("# %s: fail: %s", "getDIOptions:", "Env: DIFMT");
    writefln ("  expected: %s got %s", "SMT", opts.formatString);
  }
  unsetenv ("DIFMT");

  ++tcount;
  fail = false;
  opts = opts.init;
  setenv ("POSIXLY_CORRECT", " ", true);
  getDIOptions ([""], opts, dispOpts);
  if (dispOpts.dbsstr != "512") { fail = true; }
  if (opts.formatString != DI_POSIX_FORMAT) { fail = true; }
  if (dispOpts.posixCompat != true) { fail = true; }
  if (fail)
  {
    ++failures;
    writefln ("# %s: fail: %s", "getDIOptions:", "Env: POSIXLY_CORRECT");
    writefln ("  expected: %s got %s", "512", dispOpts.dbsstr);
    writefln ("  expected: %s got %s", DI_POSIX_FORMAT, opts.formatString);
    writefln ("  expected: %s got %s", true, dispOpts.posixCompat);
  }
  unsetenv ("POSIXLY_CORRECT");

  ++tcount;
  fail = false;
  opts = opts.init;
  setenv ("DF_BLOCK_SIZE", "kB", true);
  getDIOptions ([""], opts, dispOpts);
  if (dispOpts.dbsstr != "kB") { fail = true; }
  if (fail)
  {
    ++failures;
    writefln ("# %s: fail: %s", "getDIOptions:", "Env: DF_BLOCK_SIZE");
    writefln ("  expected: %s got %s", "kB", dispOpts.dbsstr);
  }
  unsetenv ("DF_BLOCK_SIZE");

  ++tcount;
  fail = false;
  opts = opts.init;
  setenv ("BLOCKSIZE", "kB", true);
  getDIOptions ([""], opts, dispOpts);
  if (dispOpts.dbsstr != "kB") { fail = true; }
  if (fail)
  {
    ++failures;
    writefln ("# %s: fail: %s", "getDIOptions:", "Env: BLOCKSIZE");
    writefln ("  expected: %s got %s", "kB", dispOpts.dbsstr);
  }
  unsetenv ("BLOCKSIZE");

  write ("unittest: options: getDIOptions: ");
  if (failures > 0) {
    writefln ("failed: %d of %d", failures, tcount);
  } else {
    writeln ("passed");
  }
}

private:

int
processOpts (string str, ref Options opts, ref DisplayOpts dispOpts)
{
  return processOpts (["dummy"] ~ split(strip(str)), opts, dispOpts);
}

int
processOpts (string[] args, ref Options opts, ref DisplayOpts dispOpts)
{
  int       idx;

  with (opts) {
    idx = getopt (args,
      "A", { formatString = DI_ALL_FORMAT; },
      "a", &displayAll,
      "b", (string arg) { processBaseSize (arg, dispOpts); },
      "B", (string arg) { processBaseSize (arg, dispOpts); },
      "d", (string arg) { dispOpts.dbsstr = arg; if (dispOpts.dbsstr == "k") { hasdashk = true; } },
      "f", &formatString,
         // solaris df compatibility
      "F", (string arg) { processIncludeList (arg, opts); },
      "g", { dispOpts.dbsstr = "g"; },
      "h", { dispOpts.dbsstr = "h"; },
      "H", { dispOpts.dbsstr = "H"; },
      "I", (string arg) { processIncludeList (arg, opts); },
      "k", { dispOpts.dbsstr = "k"; hasdashk = true; },
      "l", &localOnly,
      "L", &includeLoopback,
      "m", { dispOpts.dbsstr = "m"; },
      "n", &noHeader,
      "P", { if (! hasdashk) { dispOpts.dbsstr = "512"; }
             formatString = DI_POSIX_FORMAT;
             dispOpts.posixCompat = true; },
      "q", &noQuotaCheck,
      "s", (string arg) { sortType = processSort (arg); },
      "t", &displayTotal,
      "w", &dispOpts.width,
      "W", &dispOpts.inodeWidth,
      "x", (string arg) { processIgnoreList (arg, opts); },
      "X", &debugLevel,
      "z", &zoneDisplay,
      "Z", { zoneDisplay = "all"; }
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
      i = processOpts (o, opts, dispOpts);
//writeln ("### OA:i:", i, ":ri:", ri);
//writeln ("### OA:var:", var, ":rv:", rv);
      if (var != rv) { fail = true; }
      if (i != ri) { fail = true; }
      if (fail)
      {
        ++failures;
        writefln ("# %s: fail: %s", "processOpts:", l);
        writefln ("  expected: %s got %s", rv, var);
        writefln ("  expected: %s got %s", ri, i);
      }
    }
  }
}

unittest {
  int           failures;
  int           tcount;
  Options       opts;
  DisplayOpts   dispOpts;

string s;
  mixin MoptionsTest!bool tbool;
  mixin MoptionsTest!string tstr;
  mixin MoptionsTest!real treal;
  mixin MoptionsTest!short tshort;
  mixin MoptionsTest!(bool[string]) tbaa;

  with (opts) {
    tbool.test ("-a: displayAll", true, "-a", displayAll, true, 1);
    tstr.test ("-d k: dispOpts.dbsstr", true, "-d k", dispOpts.dbsstr, "k", 2);
    tbool.test ("-d k: hasdashk", true, "-d k", hasdashk, true, 2);
    tstr.test ("-k: dispOpts.dbsstr", true, "-k", dispOpts.dbsstr, "k", 1);
    tbool.test ("-k: hasdashk", true, "-k", hasdashk, true, 1);
    tstr.test ("-d g: dispOpts.dbsstr", true, "-d g", dispOpts.dbsstr, "g", 2);
    tbool.test ("-d g: hasdashk", true, "-d g", hasdashk, false, 2);
    tstr.test ("-g: dispOpts.dbsstr", true, "-g", dispOpts.dbsstr, "g", 1);
    tbool.test ("-g: hasdashk", true, "-g", hasdashk, false, 1);
    tstr.test ("-d m: dispOpts.dbsstr", true, "-d m", dispOpts.dbsstr, "m", 2);
    tbool.test ("-d m: hasdashk", true, "-d m", hasdashk, false, 2);
    tstr.test ("-m: dispOpts.dbsstr", true, "-m", dispOpts.dbsstr, "m", 1);
    tbool.test ("-m: hasdashk", true, "-m", hasdashk, false, 1);
    tstr.test ("-d h: dispOpts.dbsstr", true, "-d h", dispOpts.dbsstr, "h", 2);
    tbool.test ("-d h: hasdashk", true, "-d h", hasdashk, false, 2);
    tstr.test ("-h: dispOpts.dbsstr", true, "-h", dispOpts.dbsstr, "h", 1);
    tbool.test ("-h: hasdashk", true, "-h", hasdashk, false, 1);
    tstr.test ("-d H: dispOpts.dbsstr", true, "-d H", dispOpts.dbsstr, "H", 2);
    tbool.test ("-d H: hasdashk", true, "-d H", hasdashk, false, 2);
    tstr.test ("-H: dispOpts.dbsstr", true, "-H", dispOpts.dbsstr, "H", 1);
    tbool.test ("-H: hasdashk", true, "-H", hasdashk, false, 1);
    tstr.test ("-A: formatString", true, "-A", formatString, DI_ALL_FORMAT, 1);
    treal.test ("-b 1000: dispOpts.baseDispSize", true, "-b 1000", dispOpts.baseDispSize, 1000.0, 2);
    treal.test ("-b 1024: dispOpts.baseDispSize", true, "-b 1024", dispOpts.baseDispSize, 1024.0, 2);
    treal.test ("-b d: dispOpts.baseDispSize", true, "-b d", dispOpts.baseDispSize, 1000.0, 2);
    treal.test ("-b k: dispOpts.baseDispSize", true, "-b k", dispOpts.baseDispSize, 1024.0, 2);
    tstr.test ("-f: formatString", true, "-f SMbuvpT", formatString, "SMbuvpT", 2);
    tbaa.test ("-I nfs: includeList", true, "-I nfs", includeList, ["nfs":true], 2);
    tbaa.test ("-I nfs,jfs: includeList", true, "-I nfs,jfs", includeList, ["nfs":true,"jfs":true], 2);
    tbaa.test ("-I nfs -I jfs: includeList", true, "-I nfs -I jfs", includeList, ["nfs":true,"jfs":true], 4);
    tbool.test ("-l: localOnly", true, "-l", localOnly, true, 1);
    tbool.test ("-L: includeLoopback", true, "-L", includeLoopback, true, 1);
    tbool.test ("-n: noHeader", true, "-n", noHeader, true, 1);
    tstr.test ("-P: dispOpts.dbsstr", true, "-P", dispOpts.dbsstr, "512", 1);
    tbool.test ("-P: dispOpts.posixCompat", true, "-P", dispOpts.posixCompat, true, 1);
    tstr.test ("-P: formatString", true, "-P", formatString, DI_POSIX_FORMAT, 1);
    tstr.test ("-P -k: dispOpts.dbsstr", true, "-P -k", dispOpts.dbsstr, "k", 2);
    tstr.test ("-k -P: dispOpts.dbsstr", true, "-k -P", dispOpts.dbsstr, "k", 2);
    tbool.test ("-P -k: hasdashk", true, "-P -k", hasdashk, true, 2);
    tbool.test ("-k -P: hasdashk", true, "-k -P", hasdashk, true, 2);
    tbool.test ("-q: noQuotaCheck", true, "-q", noQuotaCheck, true, 1);
    tstr.test ("-s r: sortType", true, "-s r", sortType, "rm", 2);
    tstr.test ("-s t: sortType", true, "-s t", sortType, "tm", 2);
    tstr.test ("-s trsrm: sortType", true, "-s trsrm", sortType, "trsrm", 2);
    tbool.test ("-t: displayTotal", true, "-q", displayTotal, true, 1);
    tshort.test ("-w 12: dispOpts.width", true, "-w 12", dispOpts.width, 12, 2);
    tshort.test ("-W 12: dispOpts.inodeWidth", true, "-W 12", dispOpts.width, 12, 2);
    tbaa.test ("-x nfs: ignoreList", true, "-x nfs", ignoreList, ["nfs":true], 2);
    tbaa.test ("-x nfs,jfs: ignoreList", true, "-x nfs,jfs", ignoreList, ["nfs":true,"jfs":true], 2);
    tbaa.test ("-x nfs -x jfs: ignoreList", true, "-x nfs -x jfs", ignoreList, ["nfs":true,"jfs":true], 4);
    tstr.test ("-z some: zoneDisplay", true, "-z some", zoneDisplay, "some", 2);
    tstr.test ("-Z: zoneDisplay", true, "-Z", zoneDisplay, "all", 1);
    tshort.test ("-X 4: debugLevel", true, "-X 4", debugLevel, 4, 2);
  }

  write ("unittest: options: processOpts: ");
  if (failures > 0) {
    writefln ("failed: %d of %d", failures, tcount);
  } else {
    writeln ("passed");
  }
}

void
processBaseSize (string arg, ref DisplayOpts dispOpts)
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

string
processSort (string arg)
{
  string sortType;

  sortType = arg;
  if (arg == "r") {   // backwards compatibility
    sortType = "rm";
  }
  if (arg == "t") {   // add some additional sorting...
    sortType = "tm";
  }
  return sortType;
}

void
processIncludeList (string arg, ref Options opts)
{
  foreach (str; split (arg, ",")) {
    opts.includeList[str] = true;
  }
}

void
processIgnoreList (string arg, ref Options opts)
{
  foreach (str; split (arg, ",")) {
    opts.ignoreList[str] = true;
  }
}

