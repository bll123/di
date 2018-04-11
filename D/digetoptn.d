// written in the D programming language

module digetoptn;

import std.variant;
import std.string;
import std.conv : to;
debug (1) {
  import std.stdio;
}
version (unittest) {
  import std.stdio;
  import std.math;
}

import config;

enum string
  endArgs       = "--",
  longOptStart  = "--";
enum char
  shortOptStart = '-',
  argAssign     = '=';
// enum getoptType {} doesn't seem to work.
enum ubyte
  GETOPTN_LEGACY = 0,
  GETOPTN_MODERN = 1;
alias ubyte getoptType;
enum ubyte
  GETOPTN_BOOL = 1,
  GETOPTN_STRING = 2,
  GETOPTN_BYTE = 3,
  GETOPTN_SHORT = 4,
  GETOPTN_INT = 5,
  GETOPTN_LONG = 6,
  GETOPTN_DOUBLE = 7,
  GETOPTN_REAL = 8,
  GETOPTN_ALIAS = 9,
  GETOPTN_FUNC_NOARG = 10,
  GETOPTN_FUNC_ARG = 11,
  GETOPTN_IGNORE = 12;

private struct ArgInfo {
  string        arg;
  string        argval;
  int           asidx;      // for short options; current position in arg str
  int           avidx;      // for short options; start of arg val
  size_t        alen;
  bool          longOptFlag;
  bool          attachedArg;
  bool          haveArg;
};

struct OptInfo {
  getoptType    otype;
  void *        optval;
  void delegate (string option, string argval) odg;
};

void
goinit (ref OptInfo[string] oiaa, string option, getoptType otype,
    void *optval, void delegate (string option, string argval) odg)
{
  OptInfo   oi = { otype: otype, optval: optval, odg: odg };
  oiaa[option] = oi;
}

int
getoptn (getoptType gtype, string[] allargs, OptInfo[string] opts)
{
  ArgInfo   ainfo;
  int       rv;

  debug (1) {
    foreach (k, arg; allargs) {
      writefln ("getoptn:args:%d:%s:", k, arg);
    }
  }

  with (ainfo) {

outerfor:
    for (auto aidx = 1; aidx < allargs.length; ++aidx) {
      arg = allargs[aidx];
      argval = null;
      attachedArg = false;
      haveArg = false;
      if (allargs.length > aidx + 1) {
        argval = allargs[aidx + 1];
      }
      asidx = 1;
      avidx = 2;
      if (arg[0] != shortOptStart || arg == endArgs) {
        if (arg == endArgs) { ++aidx; }
        debug (1) {
          writefln ("getoptn:nomore:aidx:%d:", aidx);
        }
        rv = aidx - 1; // end of loop adds one
        break outerfor;
      }

      int rc = 0;
      longOptFlag = false;
      if (arg[0..2] == longOptStart) {
        longOptFlag = true;
      }

      alen = arg.length;
      // check if there's an = assignment...
      auto ival = indexOf (arg, argAssign);
      if (ival >= 0) {
        alen = ival;
      }
      debug (1) {
        writefln ("getoptn:%d:%s:%s:", aidx, arg, arg[0..alen]);
      }

innerwhile:
      while (asidx < arg.length && arg[asidx] != '\0') {
        debug (1) {
          writefln ("getoptn:aidx:%d:asidx:%d:alen:%d:avidx:%d:arg:%s:", aidx, asidx, arg.length, avidx, arg);
        }

        rc = checkOption (gtype, ainfo, arg[0..alen], opts);
        debug (1) {
          writefln ("getoptn:co:rc:%d", rc);
        }
        rv = aidx; // end of loop adds one

        // if the option had an argument, process the next arg
        // otherwise, it was a boolean, and asidx is used.
        if (rc != 0) {
          aidx += rc - 1;  // increment by rc less one, for loop will inc again
          rv = aidx; // end of loop adds one
          if (haveArg) {
            break innerwhile;
          }
        }

        // handles boolean --arg
        if (longOptFlag || gtype == GETOPTN_MODERN) {
          debug (1) {
            writefln ("getoptn:break inner:");
          }
          break innerwhile;
        }

        ++asidx;
        ++avidx;
      } // while
    } // for
  } // with

  debug (1) {
    writefln ("getoptn:exit:%d:rv:%d", allargs.length, rv + 1);
  }
  return rv + 1;
}

private:

version (unittest) {
  mixin template MgetoptTest(T) {
    void
    test (getoptType gtype, string l, bool expok,
        T[] res, T init, T[] expected,
        OptInfo[string] opts, string[] a, int wantrc)
    {
      for (auto i = 0; i < 4; ++i) {
        res[i] = init;
      }
      int  idx;
      int fail = 0;
      ++tcount;

      a = ["dummy"] ~ a;
      try {
        idx = getoptn (gtype, a, opts);
        for (auto i = 0; i < 4; ++i) {
          if (expected[i] != res[i]) {
            fail = 1;
            debug (1) {
              writefln ("  getoptn: %d: exp:%s:res:%s:", i, expected[i], res[i]);
            }
          }
        }
        if (idx != wantrc) {
          fail = 2;
          debug (1) {
            writefln ("  getoptn:idx:%d:want:%d:", idx, wantrc);
          }
        }
        if (! expok) {
          fail = 3;
          debug (1) {
            writefln ("  getoptn: expected failure");
          }
        }
      } catch (Exception e) {
        if (expok) {
          writefln ("  getoptn: expected success; got: %s", e.msg);
          fail = 4;
        }
      }
      if (fail) {
        ++failures;
        writefln ("# %s: fail: %s", testname, l);
      }
    }
  }
}

// for getopt();
unittest {
  int       failures;
  int       tcount;
  bool[4]   rb;
  int[4]    ri;
  long[4]   rl;
  double[4] rd;
  real[4]   rr;

  string    testname = "getopt";

  mixin MgetoptTest!bool tbool;
  mixin MgetoptTest!int  tint;
  mixin MgetoptTest!double  tdouble;

  OptInfo[string] opta;
  goinit (opta, "-a", GETOPTN_BOOL, cast(void *) &rb[0], null);
  goinit (opta, "-b", GETOPTN_BOOL, cast(void *) &rb[1], null);
  goinit (opta, "-c", GETOPTN_BOOL, cast(void *) &rb[2], null);
  goinit (opta, "-d", GETOPTN_BOOL, cast(void *) &rb[3], null);
  tbool.test (GETOPTN_LEGACY, "boolean bundled, no args", true,
      rb, false, [false,false,false,false], opta, [], 1);
  tbool.test (GETOPTN_LEGACY, "boolean bundled, all", true,
      rb, false, [true,true,true,true], opta, ["-abcd"], 2);
  tbool.test (GETOPTN_LEGACY, "boolean separate, all", true,
      rb, false, [true,true,true,true], opta,
      ["-a", "-b", "-c", "-d"], 5);
  tbool.test (GETOPTN_LEGACY, "boolean separate, bc", true,
      rb, false, [false,true,true,false], opta, ["-b", "-c"], 3);
  tbool.test (GETOPTN_LEGACY, "boolean separate, ad", true,
      rb, false, [true,false,false,true], opta, ["-a", "-d"], 3);
  tbool.test (GETOPTN_LEGACY, "boolean unrecognized option: bundled", false,
      rb, false, [true,true,true,true], opta, ["-abxd"], 0);

  OptInfo[string] optb;
  goinit (optb, "--testa", GETOPTN_BOOL, cast(void *) &rb[0], null);
  goinit (optb, "--testb", GETOPTN_BOOL, cast(void *) &rb[1], null);
  goinit (optb, "--testc", GETOPTN_BOOL, cast(void *) &rb[2], null);
  goinit (optb, "--testd", GETOPTN_BOOL, cast(void *) &rb[3], null);
  tbool.test (GETOPTN_LEGACY, "boolean long option", true,
      rb, false, [true,false,false,true], optb, ["--testa", "--testd"], 3);
  tbool.test (GETOPTN_LEGACY, "boolean check return idx", true,
      rb, false, [true,true,true,true], opta, ["-abcd"], 2);

  OptInfo[string] opte;
  goinit (opte, "-a", GETOPTN_INT, cast(void *) &ri[0], null);
  goinit (opte, "-b", GETOPTN_INT, cast(void *) &ri[1], null);
  goinit (opte, "-c", GETOPTN_INT, cast(void *) &ri[2], null);
  goinit (opte, "-d", GETOPTN_INT, cast(void *) &ri[3], null);
  tint.test (GETOPTN_LEGACY, "int separate, ad", true,
      ri, 0, [1,0,0,2], opte, ["-a", "1", "-d", "2"], 5);
  tint.test (GETOPTN_LEGACY, "int attached, ad", true,
      ri, 0, [1,0,0,2], opte, ["-a1", "-d2"], 3);
  tint.test (GETOPTN_LEGACY, "int non option stop arg", true,
      ri, 0, [1,0,0,0], opte, ["-a", "1", "3", "-d", "2"], 3);
  tint.test (GETOPTN_LEGACY, "int -- stop arg", true,
      ri, 0, [1,0,0,0], opte, ["-a", "1", "--", "-d", "2"], 4);
  tint.test (GETOPTN_LEGACY, "int attached non option stop arg", true,
      ri, 0, [1,0,0,0], opte, ["-a1", "3", "-d", "2"], 2);
  tint.test (GETOPTN_LEGACY, "int attached separate", true,
      ri, 0, [1,0,0,2], opte, ["-a1", "-d", "2"], 4);
  tint.test (GETOPTN_LEGACY, "int missing", false,
      ri, 0, [1,0,0,9], opte, ["-a1", "-d"], 0);

  OptInfo[string] optf;
  goinit (optf, "--testa", GETOPTN_INT, cast(void *) &ri[0], null);
  goinit (optf, "--testb", GETOPTN_INT, cast(void *) &ri[1], null);
  goinit (optf, "--testc", GETOPTN_INT, cast(void *) &ri[2], null);
  goinit (optf, "--testd", GETOPTN_INT, cast(void *) &ri[3], null);
  tint.test (GETOPTN_LEGACY, "int long options", true,
      ri, 0, [1,0,0,2], optf, ["--testa", "1", "--testd", "2"], 5);
  tint.test (GETOPTN_LEGACY, "int unrecognized long option", false,
      ri, 0, [999,0,0,999], optf, ["--testz", "1", "--testd", "2"], 0);
  tint.test (GETOPTN_LEGACY, "int unrecognized short option", false,
      ri, 0, [998,0,0,998], optf, ["-x", "1", "--testd", "2"], 0);

  if (failures > 0) {
    write ("unittest: digetopt: getoptn: ");
    writefln ("failed: %d of %d", failures, tcount);
  }
}

int
checkOption (getoptType gtype, ref ArgInfo ainfo,
    string chkarg, OptInfo[string] opts)
{
  string tchk = to!string(shortOptStart) ~ to!string(chkarg[ainfo.asidx]);

  debug (1) {
    writefln ("chk:%s:%s:%d", chkarg, tchk, opts.length);
    foreach (k, v; opts) {
      writefln ("  chk:%s:", k);
    }
  }
  if (chkarg in opts || tchk in opts) {
    OptInfo       opt;

    if (chkarg in opts) {
      opt = opts[chkarg];
      debug (1) {
        writefln ("chk:found:%s:", chkarg);
      }
    }
    if (tchk in opts) {
      debug (1) {
        writefln ("chk:found:%s:", tchk);
      }
      opt = opts[tchk];
    }
    if (opt.otype == GETOPTN_IGNORE) {
      debug (1) {
        writefln ("chk:ignore");
      }
      return 1;
    } else if (opt.otype == GETOPTN_ALIAS) {
      debug (1) {
        writefln ("chk:alias:%s:", *(cast(string*)opt.optval));
      }
      ainfo.asidx = 1;
      return checkOption (gtype, ainfo, *(cast(string*)opt.optval), opts);
    } else {
      debug (1) {
        writefln ("chk:do process");
        writefln ("chk:proc:%s:%s:%d", ainfo.arg, chkarg, ainfo.asidx);
      }
      return processOption (gtype, ainfo, opt);
    }
  }

  string s = "Unrecognized option: " ~ ainfo.arg;
  throw new Exception(s);
  return 0;
}

version (unittest) {
  int
  docoTest (T) (getoptType gtype, string[] a, OptInfo[string] opts, bool expok,
          T init, ref T res, T expres, int expret, ref int rv)
  {
    int       rc;
    int       fail;
    ArgInfo   ainfo;

    res = init;
    with (ainfo) {
      arg = a[0];
      argval = null;
      attachedArg = false;
      haveArg = false;
      if (a.length > 1) {
        argval = a[1];
      }
      asidx = 1;
      avidx = 2;
      alen = arg.length;
      auto ival = indexOf (arg, argAssign);
      if (ival >= 0) {
        alen = ival;
      }
      longOptFlag = false;
      if (arg[0..2] == longOptStart) {
        longOptFlag = true;
      }
    }
    try {
      rc = checkOption (gtype, ainfo, ainfo.arg[0..ainfo.alen], opts);
      rv = rc;
      if (rc != expret) {
        fail = 2;
      }
      if (expres != res) {
        fail = 1;
      }
      if (! expok) {
        fail = 3;
      }
    } catch (Exception e) {
      if (expok) {
        writefln ("  getoptn: expected success; got: %s", e.msg);
        fail = 4;
      }
    }
    return fail;
  }

  mixin template MCOTest (T) {
    void test (T) (getoptType gtype, string label, string[] a,
        OptInfo[string] opts, bool expok, T init, ref T res, T expres, int expret)
    {
      int   rv;

      ++tcount;
      auto fail = docoTest!(T) (gtype, a, opts, expok, init, res, expres, expret, rv);
      if (fail > 0)
      {
        ++failures;
        writefln ("# %s: fail: %s", testname, label);
        writefln ("  expected ok    : %d", expok);
        writefln ("  fail return    : %d", fail);
        writefln ("  expected return: %s got %s", expret, rv);
        writefln ("  expected value : %s got %s", expres, res);
      }
    }
  }
}

unittest {
  string                testname = "chk";
  int                   failures;
  int                   tcount;

  bool                          rb;
  mixin MCOTest!bool            tbool;
  string                        rs;
  mixin MCOTest!string          tstr;

  OptInfo[string] opta;
  goinit (opta, "-a", GETOPTN_BOOL, cast(void *) &rb, null);
  goinit (opta, "-b", GETOPTN_ALIAS, cast(void *) &"-a", null);
  tbool.test (GETOPTN_LEGACY, "co: alias: single", ["-b"], opta, true, false, rb, true, 1);

  OptInfo[string] optb;
  goinit (optb, "-a", GETOPTN_BOOL, cast(void *) &rb, null);
  goinit (optb, "-b", GETOPTN_ALIAS, cast(void *) &"-a", null);
  goinit (optb, "-c", GETOPTN_ALIAS, cast(void *) &"-b", null);
  tbool.test (GETOPTN_LEGACY, "co: alias: double", ["-c"], optb, true, false, rb, true, 1);

  OptInfo[string] optc;
  goinit (optc, "-a", GETOPTN_BOOL, cast(void *) &rb, null);
  goinit (optc, "-b", GETOPTN_ALIAS, cast(void *) &"-a", null);
  goinit (optc, "--clong", GETOPTN_ALIAS, cast(void *) &"-b", null);
  tbool.test (GETOPTN_LEGACY, "co: alias: long", ["--clong"], optc, true, false, rb, true, 1);

  OptInfo[string] optd;
  goinit (optd, "-a", GETOPTN_STRING, cast(void *) &rs, null);
  goinit (optd, "-b", GETOPTN_ALIAS, cast(void *) &"-a", null);
  tstr.test (GETOPTN_LEGACY, "co: alias: arg,=", ["-b=def"], optd, true, "abc", rs, "def", 1);

  OptInfo[string] opte;
  goinit (opte, "-a", GETOPTN_STRING, cast(void *) &rs, null);
  goinit (opte, "--blong", GETOPTN_ALIAS, cast(void *) &"-a", null);
  tstr.test (GETOPTN_LEGACY, "co: alias: long,arg,=", ["--blong=def"], opte, true, "abc", rs, "def", 1);

  OptInfo[string] optf;
  goinit (optf, "--along", GETOPTN_STRING, cast(void *) &rs, null);
  goinit (optf, "-b", GETOPTN_ALIAS, cast(void *) &"--along", null);
  tstr.test (GETOPTN_LEGACY, "co: alias: short,arg,=", ["-b=def"], optf, true, "abc", rs, "def", 1);

  OptInfo[string] optg;
  goinit (optg, "-a", GETOPTN_STRING, cast(void *) &rs, null);
  goinit (optg, "-b", GETOPTN_ALIAS, cast(void *) &"-a", null);
  tstr.test (GETOPTN_LEGACY, "co: alias: arg,sep", ["-b", "def"], optg, true, "abc", rs, "def", 2);

  OptInfo[string] opth;
  goinit (opth, "-a", GETOPTN_STRING, cast(void *) &rs, null);
  goinit (opth, "--blong", GETOPTN_ALIAS, cast(void *) &"-a", null);
  tstr.test (GETOPTN_LEGACY, "co: alias: long,arg,sep", ["--blong", "def"], opth, true, "abc", rs, "def", 2);

  OptInfo[string] opti;
  goinit (opti, "--along", GETOPTN_STRING, cast(void *) &rs, null);
  goinit (opti, "-b", GETOPTN_ALIAS, cast(void *) &"--along", null);
  tstr.test (GETOPTN_LEGACY, "co: alias: short,arg,sep", ["-b", "def"], opti, true, "abc", rs, "def", 2);

  if (failures > 0) {
    write ("unittest: digetopt: checkOption: ");
    writefln ("failed: %d of %d", failures, tcount);
  }
}


void
checkHaveArg (in bool haveArg, in string oa)
{
  if (! haveArg) {
    string s = "Missing argument for -" ~ oa;
    debug (1) {
      writefln ("** throwing %s", s);
    }
    throw new Exception(s);
  }
}

int
processOption (getoptType gtype, ref ArgInfo ainfo, OptInfo opt)
{
  int rc = 0;

  with (ainfo) {
    debug (1) {
      writefln ("## proc:arg:%s:", arg);
    }

    // if length != alen there is a =argval
    // if LEGACY && there's more string and not a boolean, then is argval
    debug (1) {
      writefln ("       :otype:%d:alen:%d:len:%d:asidx:%d:avidx:%d:arg:%s:",
            opt.otype, alen, arg.length, asidx, avidx, arg);
    }
    if (opt.otype != GETOPTN_BOOL && arg.length != alen) {
      argval = arg[alen + 1..$];    // -...=val
      attachedArg = true;
      haveArg = true;
      debug (1) {
        writefln ("       :att:=:%s:", argval);
      }
    } else if (gtype == GETOPTN_LEGACY && opt.otype != GETOPTN_BOOL &&
        ! longOptFlag && arg.length > avidx) {
      argval = arg[avidx..$];       // short option -fval
      attachedArg = true;
      haveArg = true;
      debug (1) {
        writefln ("       :att:legacy:%s:", argval);
      }
    } else if (opt.otype != GETOPTN_BOOL && argval != null) {
      haveArg = true;
      debug (1) {
        writefln ("       :sep:%s:", argval);
      }
    }
    debug (1) {
      writefln ("       :arg:%s:asidx:%d:avidx:%d:att:%d:have:%d:argval:%s:",
          arg, asidx, avidx, attachedArg, haveArg, argval);
    }

    if (opt.otype == GETOPTN_IGNORE) {
      ++rc;
    } else if (opt.otype == GETOPTN_BOOL) {
      *(cast(bool*)opt.optval) = 1 - *(cast(bool*)opt.optval);
      ++rc;
    } else if (opt.otype == GETOPTN_ALIAS) {
      return 1;
    } else if (opt.otype == GETOPTN_BYTE) {
      checkHaveArg (haveArg, arg);
      *(cast(byte*)opt.optval) = to!(byte)(argval);
      if (! attachedArg) { ++rc; }
      ++rc;
    } else if (opt.otype == GETOPTN_SHORT) {
      checkHaveArg (haveArg, arg);
      *(cast(short*)opt.optval) = to!(short)(argval);
      if (! attachedArg) { ++rc; }
      ++rc;
    } else if (opt.otype == GETOPTN_INT) {
      checkHaveArg (haveArg, arg);
      *(cast(int*)opt.optval) = to!(int)(argval);
      if (! attachedArg) { ++rc; }
      ++rc;
    } else if (opt.otype == GETOPTN_LONG) {
      checkHaveArg (haveArg, arg);
      *(cast(long*)opt.optval) = to!(long)(argval);
      if (! attachedArg) { ++rc; }
      ++rc;
    } else if (opt.otype == GETOPTN_DOUBLE) {
      checkHaveArg (haveArg, arg);
      *(cast(double*)opt.optval) = to!(double)(argval);
      if (! attachedArg) { ++rc; }
      ++rc;
    } else if (opt.otype == GETOPTN_REAL) {
      checkHaveArg (haveArg, arg);
      *(cast(real*)opt.optval) = to!(real)(argval);
      if (! attachedArg) { ++rc; }
      ++rc;
    } else if (opt.otype == GETOPTN_STRING) {
      checkHaveArg (haveArg, arg);
      *(cast(string*)opt.optval) = cast(string) argval;
      if (! attachedArg) { ++rc; }
      ++rc;
    } else if (opt.otype == GETOPTN_FUNC_ARG) {
      opt.odg (arg, argval);
      if (! attachedArg) { ++rc; }
      ++rc;
    } else if (opt.otype == GETOPTN_FUNC_NOARG) {
      opt.odg (arg, argval);
      ++rc;
    } else {
      throw new Exception("Unhandled type passed to getopt()");
    }
  }

  debug (1) {
    writefln ("       :rc:%d:", rc);
  }
  return rc;
}

version (unittest) {
  int
  dopoTest (T) (getoptType gtype, string[] a, OptInfo opt, bool expok,
          T init, ref T res, T expres, int expret, ref int rv)
  {
    int       rc;
    int       fail;
    ArgInfo   ainfo;

    res = init;
    with (ainfo) {
      arg = a[0];
      argval = null;
      attachedArg = false;
      haveArg = false;
      if (a.length > 1) {
        argval = a[1];
      }
      asidx = 1;
      avidx = 2;
      alen = arg.length;
      auto ival = indexOf (arg, argAssign);
      if (ival >= 0) {
        alen = ival;
      }
      longOptFlag = false;
      if (arg[0..2] == longOptStart) {
        longOptFlag = true;
      }
    }
    try {
      rc = processOption (gtype, ainfo, opt);
      rv = rc;
      if (rc != expret) {
        fail = 2;
      }
      if (expres != res) {
        fail = 1;
      }
      if (! expok) {
        fail = 3;
      }
    } catch (Exception e) {
      if (expok) {
        writefln ("  getoptn: expected success; got: %s", e.msg);
        fail = 4;
      }
    }
    return fail;
  }

  mixin template MPOTest (T) {
    void test (T) (getoptType gtype, string label, string[] a,
        OptInfo opt, bool expok, T init, ref T res, T expres, int expret)
    {
      int   rv;

      ++tcount;
      auto fail = dopoTest!(T) (gtype, a, opt, expok, init, res, expres, expret, rv);
      if (fail > 0)
      {
        ++failures;
        writefln ("# %s: fail: %s", testname, label);
        writefln ("  expected ok    : %d", expok);
        writefln ("  fail return    : %d", fail);
        writefln ("  expected return: %s got %s", expret, rv);
        writefln ("  expected value : %s got %s", expres, res);
      }
    }
  }

  mixin template MPOIntegerTest (T) {
    void test (string lab, getoptType otype) {
      T v;

      void tester (T) (getoptType gtype, string label, string[] a, OptInfo opt,
            bool expok, T init, ref T res, T expres, int expret) {
        int   rv;

        ++tcount;
        auto fail = dopoTest!(T) (gtype, a, opt, expok, init, res, expres, expret, rv);
        if (fail > 0)
        {
          ++failures;
          writefln ("# %s: fail: %s", testname, label);
          writefln ("  expected ok    : %d", expok);
          writefln ("  fail return    : %d", fail);
          writefln ("  expected return: %s got %s", expret, rv);
          writefln ("  expected value : %s got %s", expres, res);
        }
      }

      OptInfo opt1 = { otype, cast(void *) &v, null };
      tester (GETOPTN_LEGACY, lab ~ " leg,sopt,sep", ["-a", "1"], opt1, true,
          cast(T) 0, v, cast(T) 1, 2);
      tester (GETOPTN_LEGACY, lab ~ " leg,sopt,att", ["-a1"], opt1, true,
          cast(T) 0, v, cast(T) 1, 1);
      tester (GETOPTN_LEGACY, lab ~ " leg,sopt,=", ["-a=1"], opt1, true,
          cast(T) 0, v, cast(T) 1, 1);
      tester (GETOPTN_LEGACY, lab ~ " leg,sopt,miss", ["-a"], opt1, false,
          cast(T) 0, v, cast(T) 0, 0);
      tester (GETOPTN_MODERN, lab ~ " mod,sopt,att", ["-a1"], opt1, false,
          cast(T) 0, v, cast(T) 0, 1);
      tester (GETOPTN_MODERN, lab ~ " mod,sopt,=", ["-a=1"], opt1, true,
          cast(T) 0, v, cast(T) 1, 1);

      OptInfo opt2 = { otype, cast(void *) &v, null };
      tester (GETOPTN_MODERN, lab ~ " mod,lopt,=", ["--a=1"], opt2, true,
          cast(T) 0, v, cast(T) 1, 1);
      tester (GETOPTN_MODERN, lab ~ " mod,lopt,sep", ["--a", "1"], opt2, true,
          cast(T) 0, v, cast(T) 1, 2);
      tester (GETOPTN_MODERN, lab ~ " mod,lopt,miss", ["--a"], opt2, false,
          cast(T) 0, v, cast(T) 0, 1);
      tester (GETOPTN_MODERN, lab ~ " mod,lopt,att", ["--a1"], opt2, false,
          cast(T) 0, v, cast(T) 0, 1);
      tester (GETOPTN_LEGACY, lab ~ " mod,lopt,att", ["--a1"], opt2, false,
          cast(T) 0, v, cast(T) 0, 1);
    }
  }

  mixin template MPORealTest (T) {
    void test (string lab, getoptType otype) {
      T v;

      void tester (T) (getoptType gtype, string label, string[] a, OptInfo opt,
            bool expok, T init, ref T res, T expres, int expret) {
        int   rv;

        ++tcount;
        auto fail = dopoTest!(T) (gtype, a, opt, expok, init, res, expres, expret, rv);
        if (fail > 0)
        {
          if (fail == 1 && fabs(expres - res) > 0.000001) {
            ++failures;
            writefln ("# %s: fail: %s", testname, label);
            writefln ("  expected ok    : %d", expok);
            writefln ("  fail return    : %d", fail);
            writefln ("  expected return: %s got %s", expret, rv);
          }
        }
      }

      OptInfo opt1 = { otype, cast(void *) &v, null };
      tester (GETOPTN_LEGACY, lab ~ " leg,sopt,sep", ["-a", "1.25"], opt1, true,
          cast(T) 0, v, cast(T) 1.25, 2);
      tester (GETOPTN_LEGACY, lab ~ " leg,sopt,att", ["-a1.25"], opt1, true,
          cast(T) 0, v, cast(T) 1.25, 1);
      tester (GETOPTN_LEGACY, lab ~ " leg,sopt,=", ["-a=1.25"], opt1, true,
          cast(T) 0, v, cast(T) 1.25, 1);
      tester (GETOPTN_LEGACY, lab ~ " leg,sopt,miss", ["-a"], opt1, false,
          cast(T) 0, v, cast(T) 0, 0);
      tester (GETOPTN_MODERN, lab ~ " mod,sopt,att", ["-a1.25"], opt1, false,
          cast(T) 0, v, cast(T) 0, 1);
      tester (GETOPTN_MODERN, lab ~ " mod,sopt,=", ["-a=1.25"], opt1, true,
          cast(T) 0, v, cast(T) 1.25, 1);

      OptInfo opt2 = { otype, cast(void *) &v, null };
      tester (GETOPTN_MODERN, lab ~ " mod,lopt,=", ["--a=1.25"], opt2, true,
          cast(T) 0, v, cast(T) 1.25, 1);
      tester (GETOPTN_MODERN, lab ~ " mod,lopt,sep", ["--a", "1.25"], opt2, true,
          cast(T) 0, v, cast(T) 1.25, 2);
      tester (GETOPTN_MODERN, lab ~ " mod,lopt,miss", ["--a"], opt2, false,
          cast(T) 0, v, cast(T) 0, 1);
      tester (GETOPTN_MODERN, lab ~ " mod,lopt,att", ["--a1.25"], opt2, false,
          cast(T) 0, v, cast(T) 0, 1);
      tester (GETOPTN_LEGACY, lab ~ " mod,lopt,att", ["--a1.25"], opt2, false,
          cast(T) 0, v, cast(T) 0, 1);
    }
  }

  mixin template MPOStringTest (T) {
    void test (string lab, getoptType otype) {
      T v;

      void tester (T) (getoptType gtype, string label, string[] a, OptInfo opt,
            bool expok, T init, ref T res, T expres, int expret) {
        int   rv;

        ++tcount;
        auto fail = dopoTest!(T) (gtype, a, opt, expok, init, res, expres, expret, rv);
        if (fail > 0)
        {
          ++failures;
          writefln ("# %s: fail: %s", testname, label);
          writefln ("  expected ok    : %d", expok);
          writefln ("  fail return    : %d", fail);
          writefln ("  expected return: %s got %s", expret, rv);
          writefln ("  expected value : %s got %s", expres, res);
        }
      }

      OptInfo opt1 = { otype, cast(void *) &v, null };
      tester (GETOPTN_LEGACY, lab ~ " leg,sopt,sep", ["-a", "abc"], opt1, true,
          cast(T) "def", v, cast(T) "abc", 2);
      tester (GETOPTN_LEGACY, lab ~ " leg,sopt,att", ["-aabc"], opt1, true,
          cast(T) "def", v, cast(T) "abc", 1);
      tester (GETOPTN_LEGACY, lab ~ " leg,sopt,=", ["-a=abc"], opt1, true,
          cast(T) "def", v, cast(T) "abc", 1);
      tester (GETOPTN_LEGACY, lab ~ " leg,sopt,miss", ["-a"], opt1, false,
          cast(T) "def", v, cast(T) "def", 0);
      tester (GETOPTN_MODERN, lab ~ " mod,sopt,att", ["-aabc"], opt1, false,
          cast(T) "def", v, cast(T) "def", 1);
      tester (GETOPTN_MODERN, lab ~ " mod,sopt,=", ["-a=abc"], opt1, true,
          cast(T) "def", v, cast(T) "abc", 1);

      OptInfo opt2 = { otype, cast(void *) &v, null };
      tester (GETOPTN_MODERN, lab ~ " mod,lopt,=", ["--a=abc"], opt2, true,
          cast(T) "def", v, cast(T) "abc", 1);
      tester (GETOPTN_MODERN, lab ~ " mod,lopt,sep", ["--a", "abc"], opt2, true,
          cast(T) "def", v, cast(T) "abc", 2);
      tester (GETOPTN_MODERN, lab ~ " mod,lopt,miss", ["--a"], opt2, false,
          cast(T) "def", v, cast(T) "def", 1);
      tester (GETOPTN_MODERN, lab ~ " mod,lopt,att", ["--aabc"], opt2, false,
          cast(T) "def", v, cast(T) "def", 1);
      tester (GETOPTN_LEGACY, lab ~ " mod,lopt,att", ["--aabc"], opt2, false,
          cast(T) "def", v, cast(T) "def", 1);
    }
  }

  mixin template MPOFuncTest (T) {
    void test (getoptType gtype, string label, string[] a,
        OptInfo opt, bool expok, ref string res,
        string expres, int expret)
    {
      int   rv;

      ++tcount;
      auto fail = dopoTest!string (gtype, a, opt, expok, null, res, expres, expret, rv);
      if (fail > 0)
      {
        ++failures;
        writefln ("# %s: fail: %s", testname, label);
        writefln ("  expected ok    : %d", expok);
        writefln ("  fail return    : %d", fail);
        writefln ("  expected return: %s got %s", expret, rv);
        writefln ("  expected value : %s got %s", expres, res);
      }
    }
  }
}

// for processOption()
unittest {
  string                testname = "proc";
  int                   failures;
  int                   tcount;

  bool                          rb;
  mixin MPOTest!bool            tbool;
  string                        rs;
  mixin MPOFuncTest!string      tfunc;
  mixin MPOIntegerTest!byte     tibyte;
  mixin MPOIntegerTest!short    tishort;
  mixin MPOIntegerTest!int      tiint;
  mixin MPOIntegerTest!long     tilong;
  mixin MPORealTest!double      trdouble;
  mixin MPORealTest!real        trreal;
  mixin MPOStringTest!string    tsstring;
  mixin MPOStringTest!(char[])  tschar;


  OptInfo opta = { GETOPTN_BOOL, cast(void *) &rb, null };
  tbool.test (GETOPTN_LEGACY, "bool: leg,sopt,f->t", ["-a"], opta, true, false, rb, true, 1);
  tbool.test (GETOPTN_LEGACY, "bool: leg,sopt,t->f", ["-a"], opta, true, true, rb, false, 1);

  OptInfo optb = { GETOPTN_BOOL, cast(void *) &rb, null };
  tbool.test (GETOPTN_MODERN, "bool: mod,lopt,t->f", ["--a"], optb, true, false, rb, true, 1);
  tbool.test (GETOPTN_MODERN, "bool: mod,lopt,f->t", ["--a"], optb, true, true, rb, false, 1);

  OptInfo optc = { GETOPTN_IGNORE, cast(void *) null, null  };
  tbool.test (GETOPTN_MODERN, "ingore: mod,lopt,f->t", ["--a"], optc, true, true, rb, true, 1);

  tibyte.test ("byte", GETOPTN_BYTE);
  tishort.test ("short", GETOPTN_SHORT);
  tiint.test ("int", GETOPTN_INT);
  tilong.test ("long", GETOPTN_LONG);
  trdouble.test ("double", GETOPTN_DOUBLE);
  trreal.test ("real", GETOPTN_REAL);
  tsstring.test ("string", GETOPTN_STRING);
  tschar.test ("char", GETOPTN_STRING);

  auto d1 = delegate (string arg, string val) { rs = arg; };
  OptInfo optd = { GETOPTN_FUNC_NOARG, cast(void *) rs, d1 };
  tfunc.test (GETOPTN_MODERN, "func: mod,noarg", ["-a", "def"], optd, true, rs, "-a", 1);

  auto d2 = delegate (string arg, string val) { rs = val; };
  OptInfo opte = { GETOPTN_FUNC_ARG, cast(void *) rs, d2 };
  tfunc.test (GETOPTN_MODERN, "func: mod,arg", ["-a", "def"] , opte, true, rs, "def", 2);

  if (failures > 0) {
    write ("unittest: digetopt: processOption: ");
    writefln ("failed: %d of %d", failures, tcount);
  }
}
