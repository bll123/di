// written in the D programming language

module options;

import std.string;
import std.ascii : isDigit;
import std.utf;
import std.conv : to;
private import std.process : getenv;
debug (1) {
  import std.stdio;
}
version (unittest) {
  import std.stdio;
  private import std.process : setenv, unsetenv;
}

import config;
import dihelp;
import digetoptn;
import dispopts;

private immutable string DI_ALL_FORMAT = "MTS\n\tO\n\tbuf13\n\tbcvpa\n\tBuv2\n\tiUFP";
private immutable string DI_POSIX_FORMAT = "SbuvpM";
private immutable string DI_DEFAULT_SORT = "m";

struct Options {
  bool              csvOutput = false;
  bool              displayAll = false;
  bool              localOnly = false;
  bool              includeLoopback = false;
  bool              printHeader = true;
  bool              quotaCheck = true;
  bool              displayTotal = false;
  bool              debugHeader = false;
  bool              unknown = false;
  short             debugLevel = 0;
  string            formatString = DI_DEFAULT_FORMAT;
  string            sortType = DI_DEFAULT_SORT;
  string            zoneDisplay;
  bool[string]      includeList;
  bool[string]      excludeList;
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

  unsetenv ("DI_ARGS");
  unsetenv ("BLOCKSIZE");
  unsetenv ("DF_BLOCK_SIZE");
  unsetenv ("POSIXLY_CORRECT");

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

  if (failures > 0) {
    write ("unittest: options: getDIOptions: ");
    writefln ("failed: %d of %d", failures, tcount);
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
  int               idx;
  OptInfo[string]   options;

  with (opts) {
    auto delA = delegate (string arg, string val)
        { formatString = DI_ALL_FORMAT; };
    auto delB = delegate (string arg, string val)
        { processBaseSize (val, dispOpts); };
    auto delg = delegate (string arg, string val)
        { dispOpts.dbsstr = "g"; };
    auto delh = delegate (string arg, string val)
        { dispOpts.dbsstr = "h"; };
    auto delH = delegate (string arg, string val)
        { dispOpts.dbsstr = "H"; };
    auto delhelp = delegate (string arg, string val)
        { usage(); exit (0); };
    auto delI = delegate (string arg, string val)
        { processIncludeList (val, opts); };
    auto delk = delegate (string arg, string val)
        { dispOpts.dbsstr = "k"; };
    auto delm = delegate (string arg, string val)
        { dispOpts.dbsstr = "m"; };
    auto delP = delegate (string arg, string val)
        { if (dispOpts.dbsstr != "k") { dispOpts.dbsstr = "512"; }
          formatString = DI_POSIX_FORMAT;
          dispOpts.posixCompat = true; };
    auto dels = delegate (string arg, string val)
        { sortType = processSort (val); };
    auto delsi = delegate (string arg, string val)
        { dispOpts.baseDispSize = size1000;
          dispOpts.baseDispIdx = idx1000;
          dispOpts.dbsstr = "H"; };
    auto delversion = delegate (string arg, string val)
        { dispVersion(); exit (0); };
    auto delx = delegate (string arg, string val)
        { processExcludeList (val, opts); };
    auto delZ = delegate (string arg, string val)
        { zoneDisplay = "all";  };

    goinit (options, "-A", GETOPTN_FUNC_NOARG,   cast(void *) null, delA);
    goinit (options, "-a", GETOPTN_BOOL,     cast(void *) &displayAll, null);
    goinit (options, "--all", GETOPTN_ALIAS,    cast(void *) &"-a", null);
    goinit (options, "-b",   GETOPTN_ALIAS,    cast(void *) &"-B", null );
    goinit (options, "--block-size", GETOPTN_ALIAS, cast(void *) &"-B", null );
    goinit (options, "-B",   GETOPTN_FUNC_ARG,   cast(void *) null, delB );
    goinit (options, "-c",   GETOPTN_BOOL,     cast(void *) &csvOutput, null );
    goinit (options, "--csv-output", GETOPTN_ALIAS, cast(void *) &"-c", null );
    goinit (options, "-d",   GETOPTN_STRING,    cast(void *) &dispOpts.dbsstr, null );
    goinit (options, "-f",   GETOPTN_STRING,    cast(void *) &formatString, null );
    goinit (options, "--format-string", GETOPTN_ALIAS, cast(void *) &"-f", null );
    goinit (options, "-F",   GETOPTN_ALIAS,    cast(void *) &"-I", null );
    goinit (options, "-g",   GETOPTN_FUNC_NOARG,  cast(void *) null, delg );
    goinit (options, "-h",   GETOPTN_FUNC_NOARG,  cast(void *) null, delh );
    goinit (options, "-H",   GETOPTN_FUNC_NOARG,  cast(void *) null, delH );
    goinit (options, "--help", GETOPTN_FUNC_NOARG,    cast(void *) null, delhelp );
    goinit (options, "--human-readable", GETOPTN_ALIAS, cast(void *) &"-H", null );
    goinit (options, "-?",   GETOPTN_ALIAS,    cast(void *) &"--help", null );
    goinit (options, "-i",   GETOPTN_ALIAS,     cast(void *) &"-x", null );
    goinit (options, "-I",   GETOPTN_FUNC_ARG,   cast(void *) null, delI );
    goinit (options, "--inodes", GETOPTN_IGNORE,   cast(void *) null, null );
    goinit (options, "-k",   GETOPTN_FUNC_NOARG,  cast(void *) null, delk );
    goinit (options, "-l",   GETOPTN_BOOL,     cast(void *) &localOnly, null );
    goinit (options, "--local", GETOPTN_ALIAS,    cast(void *) &"-l", null );
    goinit (options, "-L",   GETOPTN_BOOL,     cast(void *) &includeLoopback, null );
    goinit (options, "-m",   GETOPTN_FUNC_NOARG,  cast(void *) null, delm );
    goinit (options, "-n",   GETOPTN_BOOL,     cast(void *) &printHeader, null );
    goinit (options, "--no-sync", GETOPTN_IGNORE,  cast(void *) null, null );
    goinit (options, "-P",   GETOPTN_FUNC_NOARG,  cast(void *) null, delP );
    goinit (options, "--portability", GETOPTN_ALIAS, cast(void *) &"-P", null );
    goinit (options, "--print-type", GETOPTN_IGNORE, cast(void *) null, null );
    goinit (options, "-q",   GETOPTN_BOOL,     cast(void *) &quotaCheck, null );
    goinit (options, "-s",   GETOPTN_FUNC_ARG,   cast(void *) null, dels );
    goinit (options, "--si",  GETOPTN_FUNC_NOARG,  cast(void *) null, delsi );
    goinit (options, "--sync", GETOPTN_IGNORE,    cast(void *) null, null );
    goinit (options, "-t",   GETOPTN_BOOL,     cast(void *) &displayTotal, null );
    goinit (options, "--total", GETOPTN_ALIAS,    cast(void *) &"-t", null );
    goinit (options, "--type", GETOPTN_ALIAS,    cast(void *) &"-I", null );
    goinit (options, "-v",   GETOPTN_IGNORE,    cast(void *) null, null );
    goinit (options, "--version", GETOPTN_FUNC_NOARG, cast(void *) null, delversion );
    goinit (options, "-w",   GETOPTN_SHORT,    cast(void *) &dispOpts.width, null );
    goinit (options, "-W",   GETOPTN_SHORT,    cast(void *) &dispOpts.inodeWidth, null );
    goinit (options, "-x",   GETOPTN_FUNC_ARG,   cast(void *) null, delx );
    goinit (options, "--exclude-type", GETOPTN_ALIAS, cast(void *) &"-x", null );
    goinit (options, "-X",   GETOPTN_SHORT,    cast(void *) &debugLevel, null );
    goinit (options, "-z",   GETOPTN_STRING,    cast(void *) &zoneDisplay, null );
    goinit (options, "-Z",   GETOPTN_FUNC_NOARG,  cast(void *) null, delZ );

    idx = getoptn (GETOPTN_LEGACY, args, options);
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
    tbool.test ("-a: displayAll", true, "-a", displayAll, true, 2);
    tstr.test ("-d k: dispOpts.dbsstr", true, "-d k", dispOpts.dbsstr, "k", 3);
    tstr.test ("-k: dispOpts.dbsstr", true, "-k", dispOpts.dbsstr, "k", 2);
    tstr.test ("-d g: dispOpts.dbsstr", true, "-d g", dispOpts.dbsstr, "g", 3);
    tstr.test ("-g: dispOpts.dbsstr", true, "-g", dispOpts.dbsstr, "g", 2);
    tstr.test ("-d m: dispOpts.dbsstr", true, "-d m", dispOpts.dbsstr, "m", 3);
    tstr.test ("-m: dispOpts.dbsstr", true, "-m", dispOpts.dbsstr, "m", 2);
    tstr.test ("-d h: dispOpts.dbsstr", true, "-d h", dispOpts.dbsstr, "h", 3);
    tstr.test ("-h: dispOpts.dbsstr", true, "-h", dispOpts.dbsstr, "h", 2);
    tstr.test ("-d H: dispOpts.dbsstr", true, "-d H", dispOpts.dbsstr, "H", 3);
    tstr.test ("-H: dispOpts.dbsstr", true, "-H", dispOpts.dbsstr, "H", 2);
    tstr.test ("-A: formatString", true, "-A", formatString, DI_ALL_FORMAT, 2);
    treal.test ("-b 1000: dispOpts.baseDispSize", true, "-b 1000", dispOpts.baseDispSize, 1000.0, 3);
    treal.test ("-b 1024: dispOpts.baseDispSize", true, "-b 1024", dispOpts.baseDispSize, 1024.0, 3);
    treal.test ("-b d: dispOpts.baseDispSize", true, "-b d", dispOpts.baseDispSize, 1000.0, 3);
    treal.test ("-b k: dispOpts.baseDispSize", true, "-b k", dispOpts.baseDispSize, 1024.0, 3);
    tstr.test ("-f: formatString", true, "-f SMbuvpT", formatString, "SMbuvpT", 3);
    tbaa.test ("-I nfs: includeList", true, "-I nfs", includeList, ["nfs":true], 3);
    tbaa.test ("-I nfs,jfs: includeList", true, "-I nfs,jfs", includeList, ["nfs":true,"jfs":true], 3);
    tbaa.test ("-I nfs -I jfs: includeList", true, "-I nfs -I jfs", includeList, ["nfs":true,"jfs":true], 5);
    tbool.test ("-l: localOnly", true, "-l", localOnly, true, 2);
    tbool.test ("-L: includeLoopback", true, "-L", includeLoopback, true, 2);
    tbool.test ("-n: printHeader", true, "-n", printHeader, false, 2);
    tstr.test ("-P: dispOpts.dbsstr", true, "-P", dispOpts.dbsstr, "512", 2);
    tbool.test ("-P: dispOpts.posixCompat", true, "-P", dispOpts.posixCompat, true, 2);
    tstr.test ("-P: formatString", true, "-P", formatString, DI_POSIX_FORMAT, 2);
    tstr.test ("-P -k: dispOpts.dbsstr", true, "-P -k", dispOpts.dbsstr, "k", 3);
    tstr.test ("-k -P: dispOpts.dbsstr", true, "-k -P", dispOpts.dbsstr, "k", 3);
    tbool.test ("-q: quotaCheck", true, "-q", quotaCheck, false, 2);
    tstr.test ("-s r: sortType", true, "-s r", sortType, "rm", 3);
    tstr.test ("-s t: sortType", true, "-s t", sortType, "tm", 3);
    tstr.test ("-s trsrm: sortType", true, "-s trsrm", sortType, "trsrm", 3);
    tbool.test ("-t: displayTotal", true, "-t", displayTotal, true, 2);
    tshort.test ("-w 12: dispOpts.width", true, "-w 12", dispOpts.width, 12, 3);
    tshort.test ("-W 12: dispOpts.inodeWidth", true, "-W 12", dispOpts.width, 12, 3);
    tbaa.test ("-x nfs: excludeList", true, "-x nfs", excludeList, ["nfs":true], 3);
    tbaa.test ("-x nfs,jfs: excludeList", true, "-x nfs,jfs", excludeList, ["nfs":true,"jfs":true], 3);
    tbaa.test ("-x nfs -x jfs: excludeList", true, "-x nfs -x jfs", excludeList, ["nfs":true,"jfs":true], 5);
    tstr.test ("-z some: zoneDisplay", true, "-z some", zoneDisplay, "some", 3);
    tstr.test ("-Z: zoneDisplay", true, "-Z", zoneDisplay, "all", 2);
    tshort.test ("-X 4: debugLevel", true, "-X 4", debugLevel, 4, 3);
  }

  if (failures > 0) {
    write ("unittest: options: processOpts: ");
    writefln ("failed: %d of %d", failures, tcount);
  }
}

void
processBaseSize (string arg, ref DisplayOpts dispOpts)
{
  size_t    i = 0;
  dchar     c;

  with (dispOpts) {
    c = decode (arg, i);
    if (isDigit (c)) {
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
processExcludeList (string arg, ref Options opts)
{
  foreach (str; split (arg, ",")) {
    opts.excludeList[str] = true;
  }
}
