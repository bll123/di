// written in the D programming language
/*
$Id$
$Source$
*/

module digetopt;

import std.variant;
import std.string;
import std.conv : to;
debug (1) {
  import std.stdio;
}
version (unittest) {
  import std.stdio;
}

import config;

enum string
  endArgs       = "--",
  longOptStart  = "--";
enum char
  shortOptStart = '-',
  argAssign     = '=';
enum byte
  GETOPTN_LEGACY = 0,
  GETOPTN_MODERN = 1;
alias byte getoptType;
enum byte
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
  int           asidx;
  size_t        alen;
  bool          longOptFlag;
};

struct OptInfo {
  string        option;
  byte          otype;
  void *        optval;
  void delegate (string option, string argval) odg;
};

int
getoptn (getoptType gtype, string[] allargs, OptInfo[] opts)
{
  ArgInfo   ainfo;
  int       rv;

  debug (1) {
    foreach (k, arg; allargs) {
      writefln ("getopt:args:%d:%s:", k, arg);
    }
  }

  with (ainfo) {

outerfor:
    for (auto aidx = 1; aidx < allargs.length; ++aidx) {
      arg = allargs[aidx];
      argval = null;
      if (allargs.length > aidx + 1) {
        argval = allargs[aidx + 1];
      }
      asidx = 1;
      if (arg[0] != shortOptStart || arg == endArgs) {
        debug (1) {
          writefln ("getopt:nomore:aidx:%d:", aidx);
        }
        break outerfor;
      }

      int rc = 0;
      longOptFlag = false;
      if (arg[0..2] == longOptStart) {
        asidx = 2;
        longOptFlag = true;
      }

      alen = arg.length;
      // check if there's an = assignment...
      auto ival = indexOf (arg, argAssign);
      if (ival >= 0) {
        alen = ival;
      }
      debug (1) {
        writefln ("getopt:%d:%s:%s:", aidx, arg, arg[0..alen]);
      }

innerwhile:
      while (asidx < arg.length && arg[asidx] != '\0') {
        debug (1) {
          writefln ("getopt:aidx:%d:asidx:%d:alen:%d:a[]:%s:%s:", aidx, asidx, arg.length, arg[asidx], arg);
        }

        rc = checkOption (gtype, ainfo, arg, opts);
        if (rc != 0) {
          aidx += rc - 1;  // increment by rc less one, for loop will inc again
          debug (1) {
            writefln ("getopt:rc:%d:aidx:%d:", rc, aidx);
          }
        }
        rv = aidx;

        // handles boolean --arg
        if (longOptFlag || gtype == GETOPTN_MODERN) {
          debug (1) {
            writefln ("getopt:break inner:");
          }
          break innerwhile;
        }

        ++asidx;
      } // while
    } // for
  } // with

  debug (1) {
    writefln ("getopt:exit:");
  }
  return rv;
}

private:

version (unittest) {
  mixin template MgetoptTest(T) {
    void
    test (getoptType gtype, string l, bool expok,
        T[] res, T init, T[] expected,
        OptInfo[] opts, string[] a, int count)
    {
      for (auto i = 0; i < 4; ++i) {
        res[i] = init;
      }
      int  idx;
      bool fail = false;
      ++tcount;

      a = ["dummy"] ~ a;
      try {
        idx = getoptn (gtype, a, opts);
        for (auto i = 0; i < 4; ++i) {
          if (expected[i] != res[i]) {
            fail = true;
            debug (1) {
              writefln ("  getoptn: %d: exp:%s:res:%s:", i, expected[i], res[i]);
            }
          }
        }
        if (! expok) {
          fail = true;
          debug (1) {
            writefln ("  getoptn: expected failure");
          }
        }
      } catch {
        if (expok) {
          debug (1) {
            writefln ("  getoptn: expected success");
          }
          fail = true;
        }
      }
      if (idx != count) {
        fail = true;
        debug (1) {
          writefln ("  getoptn: idx:%d:count:%d:", idx, count);
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
  mixin MgetoptTest!int  tdouble;

  OptInfo[] opta = [
    { "-a", GETOPTN_BOOL, cast(void *) &rb[0], null },
    { "-b", GETOPTN_BOOL, cast(void *) &rb[1], null },
    { "-c", GETOPTN_BOOL, cast(void *) &rb[2], null },
    { "-d", GETOPTN_BOOL, cast(void *) &rb[3], null }
    ];
  tbool.test (GETOPTN_LEGACY, "boolean bundled, all", true,
      rb, false, [true,true,true,true], opta, ["-abcd"], 1);
  OptInfo[] optb = [
    { "-a", GETOPTN_BOOL, cast(void *) &rb[0], null },
    { "-b", GETOPTN_BOOL, cast(void *) &rb[1], null },
    { "-c", GETOPTN_BOOL, cast(void *) &rb[2], null },
    { "-d", GETOPTN_BOOL, cast(void *) &rb[3], null }
    ];
  tbool.test (GETOPTN_LEGACY, "boolean separate, all", true,
      rb, false, [true,true,true,true], optb,
      ["-a", "-b", "-c", "-d"], 4);
  OptInfo[] optc = [
    { "-a", GETOPTN_BOOL, cast(void *) &rb[0], null },
    { "-b", GETOPTN_BOOL, cast(void *) &rb[1], null },
    { "-c", GETOPTN_BOOL, cast(void *) &rb[2], null },
    { "-d", GETOPTN_BOOL, cast(void *) &rb[3], null }
    ];
  tbool.test (GETOPTN_LEGACY, "boolean separate, bc", true,
      rb, false, [false,true,true,false], optc, ["-b", "-c"], 2);
  OptInfo[] optd = [
    { "-a", GETOPTN_BOOL, cast(void *) &rb[0], null },
    { "-b", GETOPTN_BOOL, cast(void *) &rb[1], null },
    { "-c", GETOPTN_BOOL, cast(void *) &rb[2], null },
    { "-d", GETOPTN_BOOL, cast(void *) &rb[3], null }
    ];
  tbool.test (GETOPTN_LEGACY, "boolean separate, ad", true,
      rb, false, [true,false,false,true], optd, ["-a", "-d"], 2);

/+
  tint.test ("int separate, ad", true,
      [GETOPTN_NUMERIC, GETOPTN_NUMERIC, GETOPTN_NUMERIC, GETOPTN_NUMERIC],
      ["a", "b", "c", "d"],
      ["-a", "1", "-d", "2"], [1,0,0,2], 4);
  tint.test ("int attached, ad", true,
      [GETOPTN_NUMERIC, GETOPTN_NUMERIC, GETOPTN_NUMERIC, GETOPTN_NUMERIC],
      ["a", "b", "c", "d"],
      ["-a1", "-d2"], [1,0,0,2], 2);
  tint.test ("int non option stop arg", true,
      [GETOPTN_NUMERIC, GETOPTN_NUMERIC, GETOPTN_NUMERIC, GETOPTN_NUMERIC],
      ["a", "b", "c", "d"],
      ["-a", "1", "3", "-d", "2"], [1,0,0,0], 2);
  tint.test ("int -- stop arg", true,
      [GETOPTN_NUMERIC,GETOPTN_NUMERIC,GETOPTN_NUMERIC,GETOPTN_NUMERIC],
      ["a", "b", "c", "d"],
      ["-a", "1", "--", "-d", "2"], [1,0,0,0], 3);
  tint.test ("int attached non option stop arg", true,
      [GETOPTN_NUMERIC,GETOPTN_NUMERIC,GETOPTN_NUMERIC,GETOPTN_NUMERIC],
      ["a", "b", "c", "d"],
      ["-a1", "3", "-d", "2"], [1,0,0,0], 1);
  tint.test ("int attached/separate", true,
      [GETOPTN_NUMERIC,GETOPTN_NUMERIC,GETOPTN_NUMERIC,GETOPTN_NUMERIC],
      ["a", "b", "c", "d"],
      ["-a1", "-d", "2"], [1,0,0,2], 3);
  tint.test ("int missing argument", false,
      [GETOPTN_NUMERIC,GETOPTN_NUMERIC,GETOPTN_NUMERIC,GETOPTN_NUMERIC],
      ["a", "b", "c", "d"],
      ["-a1", "-d"], [1,0,0,9], 0);
  tint.test ("int long options", true,
      [GETOPTN_NUMERIC,GETOPTN_NUMERIC,GETOPTN_NUMERIC,GETOPTN_NUMERIC],
      ["testa", "testb", "testc", "testd"],
      ["--testa", "1", "--testd", "2"], [1,0,0,2], 4);
  tint.test ("int unrecognized long option", false,
      [GETOPTN_NUMERIC,GETOPTN_NUMERIC,GETOPTN_NUMERIC,GETOPTN_NUMERIC],
      ["testa", "testb", "testc", "testd"],
      ["--testz", "1", "--testd", "2"], [999,0,0,999], 0);
  tint.test ("int unrecognized short option", false,
      [GETOPTN_NUMERIC,GETOPTN_NUMERIC,GETOPTN_NUMERIC,GETOPTN_NUMERIC],
      ["testa", "testb", "testc", "testd"],
      ["-x", "1", "--testd", "2"], [998,0,0,998], 0);
  tbool.test ("boolean unrecognized option: bundled", false,
      [GETOPTN_BOOL,GETOPTN_BOOL,GETOPTN_BOOL,GETOPTN_BOOL],
      ["a", "b", "c", "d"],
      ["-abxd"], [true,true,true,true], 0);
  tbool.test ("boolean long option", true,
      [GETOPTN_BOOL,GETOPTN_BOOL,GETOPTN_BOOL,GETOPTN_BOOL],
      ["testa", "testb", "testc", "testd"],
      ["--testa", "--testd"], [true,false,false,true], 2);
  tbool.test ("boolean check return idx", true,
      [GETOPTN_BOOL,GETOPTN_BOOL,GETOPTN_BOOL,GETOPTN_BOOL],
      ["a","b","c","d"],
      ["-abcd"], [true,true,true,true], 1);
+/

  write ("unittest: digetopt: getoptn: ");
  if (failures > 0) {
    writefln ("failed: %d of %d", failures, tcount);
  } else {
    writeln ("passed");
  }
}

int
checkOption (getoptType gtype, ref ArgInfo ainfo, string chkarg, OptInfo[] opts)
{
  with (ainfo) {
    for (auto i = 0; i < opts.length; ++i) {
      auto opt = opts[i];
      debug (1) {
        writefln ("chk:%s:%s:%d", chkarg, opt.option, asidx);
      }
      if ((opt.option.length == 2 && opt.option[1] == chkarg[asidx]) ||
          (opt.option == chkarg[0..alen])) {
        debug (1) {
          writefln ("chk:found:%s:", opt.option);
        }
        if (opt.otype == GETOPTN_IGNORE) {
          debug (1) {
            writefln ("chk:ignore");
          }
          return 1;
        } else if (opt.otype == GETOPTN_ALIAS) {
          debug (1) {
            writefln ("chk:alias:%s:", opt.otype);
          }
          asidx = 1;
          return checkOption (gtype, ainfo, *(cast(string*)opt.optval), opts);
        } else {
          debug (1) {
            writefln ("chk:do process");
          }
          return processOption (gtype, ainfo, opt);
        }
      }
    }
  }

  return 0;
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

    bool      attachedArg = false;
    bool      haveArg = false;

    // if length != alen there is a =argval
    // if LEGACY && there's more string and not a boolean, then is argval
    debug (1) {
      writefln ("       :otype:%d:alen:%d:len:%d:asidx:%d:arg:%s:",
            opt.otype, alen, arg.length, asidx, arg);
    }
    if (opt.otype != GETOPTN_BOOL && arg.length != alen) {
      argval = arg[alen + 1..$];    // -...=val
      attachedArg = true;
      haveArg = true;
      debug (1) {
        writefln ("       :att:=:%s:", argval);
      }
    } else if (gtype == GETOPTN_LEGACY && opt.otype != GETOPTN_BOOL &&
        ! longOptFlag && arg.length > asidx + 1) {
      argval = arg[asidx + 1..$];       // short option -fval
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
      writefln ("       :arg:%s:asidx:%d:att:%d:have:%d:argval:%s:",
          arg, asidx, attachedArg, haveArg, argval);
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
  static string ovstring2;

  bool
  dopoTest (T) (getoptType gtype, string[] a, OptInfo opt, bool expok,
          T init, ref T res, T expres, int expret, ref int rv)
  {
    int       rc;
    bool      fail;
    ArgInfo   ainfo;

    res = init;
    with (ainfo) {
      arg = a[0];
      argval = null;
      if (a.length > 1) {
        argval = a[1];
      }
      asidx = 1;
      alen = arg.length;
      auto ival = indexOf (arg, argAssign);
      if (ival >= 0) {
        alen = ival;
      }
      longOptFlag = false;
      if (arg[0..2] == longOptStart) {
        asidx = 2;
        longOptFlag = true;
      }
    }
    try {
      rc = processOption (gtype, ainfo, opt);
      rv = rc;
      if (rc != expret) {
        fail = true;
      }
      if (expres != res) {
        fail = true;
      }
      if (! expok) {
        fail = true;
      }
    } catch {
      if (expok) {
        fail = true;
      }
    }
    return fail;
  }

  mixin template MPOTest (T) {
    void test (T) (getoptType gtype, string label, string[] a,
        OptInfo opt, bool expok, T init, ref T res, T expres, int expret)
    {
      bool  fail;
      int   rv;

      ++tcount;
      fail = dopoTest!(T) (gtype, a, opt, expok, init, res, expres, expret, rv);
      if (fail)
      {
        ++failures;
        writefln ("# %s: fail: %s", testname, label);
        writefln ("  expected ok    : %d got %d", expok, fail);
        writefln ("  expected return: %s got %s", expret, rv);
        writefln ("  expected value : %s got %s", expres, res);
      }
    }
  }

  mixin template MPOIntegerTest (T) {
    void test (string lab, byte otype) {
      T v;

      void tester (T) (getoptType gtype, string label, string[] a, OptInfo opt,
            bool expok, T init, ref T res, T expres, int expret) {
        bool  fail;
        int   rv;

        ++tcount;
        fail = dopoTest!(T) (gtype, a, opt, expok, init, res, expres, expret, rv);
        if (fail)
        {
          ++failures;
          writefln ("# %s: fail: %s", testname, label);
          writefln ("  expected ok    : %d got %d", expok, fail);
          writefln ("  expected return: %s got %s", expret, rv);
          writefln ("  expected value : %s got %s", expres, res);
        }
      }

      OptInfo opt1 = { "-a", otype, cast(void *) &v, null };
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
      OptInfo opt2 = { "--a", otype, cast(void *) &v, null };
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
    void test (string lab, byte otype) {
      T v;

      void tester (T) (getoptType gtype, string label, string[] a, OptInfo opt,
            bool expok, T init, ref T res, T expres, int expret) {
        bool  fail;
        int   rv;

        ++tcount;
        fail = dopoTest!(T) (gtype, a, opt, expok, init, res, expres, expret, rv);
        if (fail)
        {
          ++failures;
          writefln ("# %s: fail: %s", testname, label);
          writefln ("  expected ok    : %d got %d", expok, fail);
          writefln ("  expected return: %s got %s", expret, rv);
          writefln ("  expected value : %s got %s", expres, res);
        }
      }

      OptInfo opt1 = { "-a", otype, cast(void *) &v, null };
      tester (GETOPTN_LEGACY, lab ~ " leg,sopt,sep", ["-a", "1.5"], opt1, true,
          cast(T) 0, v, cast(T) 1.5, 2);
      tester (GETOPTN_LEGACY, lab ~ " leg,sopt,att", ["-a1.5"], opt1, true,
          cast(T) 0, v, cast(T) 1.5, 1);
      tester (GETOPTN_LEGACY, lab ~ " leg,sopt,=", ["-a=1.5"], opt1, true,
          cast(T) 0, v, cast(T) 1.5, 1);
      tester (GETOPTN_LEGACY, lab ~ " leg,sopt,miss", ["-a"], opt1, false,
          cast(T) 0, v, cast(T) 0, 0);
      tester (GETOPTN_MODERN, lab ~ " mod,sopt,att", ["-a1.5"], opt1, false,
          cast(T) 0, v, cast(T) 0, 1);
      tester (GETOPTN_MODERN, lab ~ " mod,sopt,=", ["-a=1.5"], opt1, true,
          cast(T) 0, v, cast(T) 1.5, 1);
      OptInfo opt2 = { "--a", otype, cast(void *) &v, null };
      tester (GETOPTN_MODERN, lab ~ " mod,lopt,=", ["--a=1.5"], opt2, true,
          cast(T) 0, v, cast(T) 1.5, 1);
      tester (GETOPTN_MODERN, lab ~ " mod,lopt,sep", ["--a", "1.5"], opt2, true,
          cast(T) 0, v, cast(T) 1.5, 2);
      tester (GETOPTN_MODERN, lab ~ " mod,lopt,miss", ["--a"], opt2, false,
          cast(T) 0, v, cast(T) 0, 1);
      tester (GETOPTN_MODERN, lab ~ " mod,lopt,att", ["--a1.5"], opt2, false,
          cast(T) 0, v, cast(T) 0, 1);
      tester (GETOPTN_LEGACY, lab ~ " mod,lopt,att", ["--a1.5"], opt2, false,
          cast(T) 0, v, cast(T) 0, 1);
    }
  }

  mixin template MPOStringTest (T) {
    void test (string lab, byte otype) {
      T v;

      void tester (T) (getoptType gtype, string label, string[] a, OptInfo opt,
            bool expok, T init, ref T res, T expres, int expret) {
        bool  fail;
        int   rv;

        ++tcount;
        fail = dopoTest!(T) (gtype, a, opt, expok, init, res, expres, expret, rv);
        if (fail)
        {
          ++failures;
          writefln ("# %s: fail: %s", testname, label);
          writefln ("  expected ok    : %d got %d", expok, fail);
          writefln ("  expected return: %s got %s", expret, rv);
          writefln ("  expected value : %s got %s", expres, res);
        }
      }

      OptInfo opt1 = { "-a", otype, cast(void *) &v, null };
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
      OptInfo opt2 = { "--a", otype, cast(void *) &v, null };
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
      bool  fail;
      int   rv;

      ++tcount;
      fail = dopoTest!string (gtype, a, opt, expok, null, res, expres, expret, rv);
      if (fail)
      {
        ++failures;
        writefln ("# %s: fail: %s", testname, label);
        writefln ("  expected ok    : %d got %d", expok, fail);
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


  OptInfo opta = { "-a", GETOPTN_BOOL, cast(void *) &rb, null };
  tbool.test (GETOPTN_LEGACY, "bool: leg,sopt,f->t", ["-a"], opta, true, false, rb, true, 1);
  tbool.test (GETOPTN_LEGACY, "bool: leg,sopt,t->f", ["-a"], opta, true, true, rb, false, 1);
  OptInfo optb = { "--a", GETOPTN_BOOL, cast(void *) &rb, null };
  tbool.test (GETOPTN_MODERN, "bool: mod,lopt,t->f", ["--a"], optb, true, false, rb, true, 1);
  tbool.test (GETOPTN_MODERN, "bool: mod,lopt,f->t", ["--a"], optb, true, true, rb, false, 1);
  OptInfo optc = { "--a", GETOPTN_IGNORE, cast(void *) null, null  };
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
  OptInfo optd = { "-a", GETOPTN_FUNC_NOARG, cast(void *) rs, d1 };
  tfunc.test (GETOPTN_MODERN, "func: mod,noarg", ["-a", "def"], optd, true, rs, "-a", 1);

  auto d2 = delegate (string arg, string val) { rs = val; };
  OptInfo opte = { "-a", GETOPTN_FUNC_ARG, cast(void *) rs, d2 };
  tfunc.test (GETOPTN_MODERN, "func: mod,arg", ["-a", "def"] , opte, true, rs, "def", 2);

  write ("unittest: digetopt: processOption: ");
  if (failures > 0) {
    writefln ("failed: %d of %d", failures, tcount);
  } else {
    writeln ("passed");
  }
}
