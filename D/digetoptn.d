// written in the D programming language
/*
$Id$
$Source$
*/

module getopt;

import std.string;
import std.conv;
debug (1) {
  import std.stdio;
}
version (unittest) {
  import std.stdio;
}

enum string
  endArgs       = "--",
  longOptStart  = "--";
enum char
  shortOptStart = '-',
  argAssign     = '=';

private struct ArgInfo {
  string[]      args;
  int           aidx;
  int           asidx;
  int           alen;
  bool          longOptFlag;
};

int
getopts (OVL...) (string[] allargs, OVL opts)
{
  assert (opts.length % 2 == 0);
  ArgInfo   ainfo;

  debug (1) {
    foreach (k, arg; allargs) {
      writefln ("getopts:args:%d:%s:", k, arg);
    }
  }

  with (ainfo) {
    args = allargs;

outerfor:
    for (aidx = 0; aidx < args.length; ++aidx) {
      asidx = 1;
      if (args[aidx][0] != shortOptStart || args[aidx] == endArgs) {
        if (args[aidx] == longOptStart) {
          ++aidx;  // point to next argument.
        }
        debug (1) {
          writefln ("getopts:nomore:aidx:%d:", aidx);
        }
        break outerfor;
      }
      debug (1) {
        writefln ("getopts:%d:%s:%s:", aidx, args[aidx], args[aidx][asidx]);
      }

      int rc = 0;
      longOptFlag = false;
      if (args[aidx][0..2] == longOptStart) {
        asidx = 2;
        longOptFlag = true;
      }

      alen = args[aidx].length;
      if (longOptFlag) {
        // check if there's an = assignment...
        auto ival = indexOf (args[aidx], argAssign);
        if (ival >= 0) {
          alen = ival;
        }
      }

innerwhile:
      while (asidx < args[aidx].length && args[aidx][asidx] != '\0') {
        debug (1) {
          writefln ("getopts:aidx:%d:asidx:%d:alen:%d:a[]:%s:%s:", aidx, asidx, args[aidx].length, args[aidx][asidx], args[aidx]);
        }

        rc = checkOption (ainfo, opts);
        if (rc != 0) {
          aidx += rc - 1;  // increment by rc less one, for loop will inc again
          debug (1) {
            writefln ("getopts:rc:%d:aidx:%d:", rc, aidx);
          }
          break innerwhile;
        }

        // handles boolean --arg
        if (longOptFlag) {
          break innerwhile;
        }

        ++asidx;
      } // while
    } // for
  } // with

  debug (1) {
    writefln ("getopts:exit:aidx:%d", ainfo.aidx);
  }
  return ainfo.aidx;
}

private:

version (unittest) {
  mixin template MgetoptsTest(T) {
    void test (string l, bool expected,
        string[] o, string[] a, T[] r, int i) {
      auto v = (T[4]).init;
      int  idx;
      bool fail = false;
      ++tcount;
      try {
        idx = getopts (a, o[2], &v[2], o[1], &v[1], o[0], &v[0], o[3], &v[3]);
        if (! expected) {
          idx = i - 1;
        }
      } catch {
        v = r;
        if (expected) {     // don't pass a failure
          v = v.init;
        }
      }
      if (v != r) { fail = true; }
      if (idx != i) { fail = true; }
      if (fail) {
        ++failures;
      }
      writefln ("# %s: %s: %d: %s",
        testname, fail ? "fail" : "pass", tcount, l);
    }
  }
}

// for getopts();
unittest {
  int       failures;
  int       tcount;

  string    testname = "getopts";

  bool      cobool[4];
  int       coint[4];
  int       idx;

  mixin MgetoptsTest!bool tbool;
  mixin MgetoptsTest!int  tint;
  mixin MgetoptsTest!int  tdouble;

  tbool.test ("boolean bundled, all", true, ["a", "b", "c", "d"],
      ["-abcd"], [true,true,true,true], 1);
  tbool.test ("boolean separate, all", true, ["a", "b", "c", "d"],
      ["-a", "-b", "-c", "-d"], [true,true,true,true], 4);
  tbool.test ("boolean separate, bc", true, ["a", "b", "c", "d"],
      ["-b", "-c"], [false,true,true,false], 2);
  tbool.test ("boolean separate, ad", true, ["a", "b", "c", "d"],
      ["-a", "-d"], [true,false,false,true], 2);
  tint.test ("int separate, ad", true, ["a", "b", "c", "d"],
      ["-a", "1", "-d", "2"], [1,0,0,2], 4);
  tint.test ("int attached, ad", true, ["a", "b", "c", "d"],
      ["-a1", "-d2"], [1,0,0,2], 2);
  tint.test ("int non option stop arg", true, ["a", "b", "c", "d"],
      ["-a", "1", "3", "-d", "2"], [1,0,0,0], 2);
  tint.test ("int -- stop arg", true, ["a", "b", "c", "d"],
      ["-a", "1", "--", "-d", "2"], [1,0,0,0], 3);
  tint.test ("int attached non option stop arg", true, ["a", "b", "c", "d"],
      ["-a1", "3", "-d", "2"], [1,0,0,0], 1);
  tint.test ("int attached/separate", true, ["a", "b", "c", "d"],
      ["-a1", "-d", "2"], [1,0,0,2], 3);
  tint.test ("int missing argument", false, ["a", "b", "c", "d"],
      ["-a1", "-d"], [1,0,0,9], 0);
  tint.test ("int long options", true, ["testa", "testb", "testc", "testd"],
      ["--testa", "1", "--testd", "2"], [1,0,0,2], 4);
  tint.test ("int unrecognized long option", false, ["testa", "testb", "testc", "testd"],
      ["--testz", "1", "--testd", "2"], [999,0,0,999], 0);
  tint.test ("int unrecognized short option", false, ["testa", "testb", "testc", "testd"],
      ["-x", "1", "--testd", "2"], [998,0,0,998], 0);
  tbool.test ("boolean unrecognized option: bundled", false, ["a", "b", "c", "d"],
      ["-abxd"], [true,true,true,true], 0);
  tbool.test ("boolean long option", true, ["testa", "testb", "testc", "testd"],
      ["--testa", "--testd"], [true,false,false,true], 2);
  tbool.test ("boolean check return idx", true, ["a","b","c","d"],
      ["-abcd"], [true,true,true,true], 1);

  write ("unittest: getopt: getopts: ");
  if (failures > 0) {
    writefln ("failed: %d", failures);
  } else {
    writeln ("passed");
  }
}

int
checkOption (OVL...)
    (ref ArgInfo ainfo, OVL opts)
{
  with (ainfo) {
    static if (opts.length > 0) {
      auto opt = to!string(opts[0]);
      if ((opt.length == 1 && opt[0] == args[aidx][asidx]) ||
          (opt.length > 1 && opt == args[aidx][2..alen])) {
        debug (1) {
          writefln ("chk:found:%s:", opt);
        }
        return processOption (ainfo, opt, opts[1]);
      } else {
        return checkOption (ainfo, opts[2..$]);
      }
    } else {
      string s;
      if (args[aidx][0..2] == "--") {
        s = format ("Unrecognized option %s", args[aidx]);
      } else {
        s = format ("Unrecognized option -%s", args[aidx][asidx]);
      }
      debug (1) {
        writefln ("** throwing %s", s);
      }
      throw new Exception (s);
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

// can't pass ov as a ref parameter -- doesn't handle delegates.
int
processOption (OV) (ref ArgInfo ainfo, in string oa, OV ov)
{
  int rc = 0;

  with (ainfo) {
    debug (1) {
      writefln ("proc:args[%d]:%s:", aidx, args[aidx]);
    }

    string    arg;
    bool      attachedArg = false;
    bool      haveArg = false;
    auto      i = asidx + 1;

    // if there is a --arg (asidx > 1 and prior char is -
    //    and the length == alen), there is no arg.
    // if there's no more string, then there is no arg.
    // if there's a null byte, there there is no arg.
    debug (1) {
      writefln ("proc:aidx:%d:asidx:%d:alen:%d:i:%d:args:%s:", aidx, asidx, alen, i, args[aidx]);
    }
    if (! (asidx > 1 && args[aidx][asidx - 1] == longOptStart[1] &&
            args[aidx].length == alen) &&
          (alen > i && args[aidx][i] != '\0')) {
      if (args[aidx].length != alen) {
        arg = args[aidx][alen + 1..$];    // long option --arg=val
      } else {
        arg = args[aidx][i..$];       // short option -fval
      }
      attachedArg = true;
      haveArg = true;
    } else if (args.length > aidx + 1) {
      arg = args[aidx + 1];
      haveArg = true;
    }
    debug (1) {
      writefln ("proc:arg:%s:aidx:%d:asidx:%d:i:%d:att:%d:have:%d",
          arg, aidx, asidx, i, attachedArg, haveArg);
    }

    static if (is(typeof(*ov) == bool)) {
      *ov = true;
    } else static if (is(typeof(ov) == return)) {
      static if (is(typeof(ov()) : void)) {
        ov ();
      } else static if (is(typeof(ov("")) : void)) {
        checkHaveArg (haveArg, oa);
        ov (arg);
        if (! attachedArg) { ++rc; }
        ++rc;
      } else static if (is(typeof(ov("","")) : void)) {
        checkHaveArg (haveArg, oa);
        ov (oa,arg);
        if (! attachedArg) { ++rc; }
        ++rc;
      }
    } else static if (is(typeof(*ov) == return)) {
      static if (is(typeof((*ov)()) : void)) {
        (*ov)();
      } else static if (is(typeof((*ov)("")) : void)) {
        checkHaveArg (haveArg, oa);
        (*ov)(arg);
        if (! attachedArg) { ++rc; }
        ++rc;
      } else static if (is(typeof((*ov)("","")) : void)) {
        checkHaveArg (haveArg, oa);
        (*ov)(oa,arg);
        if (! attachedArg) { ++rc; }
        ++rc;
      }
    } else static if (is(typeof(*ov) == string) ||
          is(typeof(*ov) == char[])) {
      checkHaveArg (haveArg, oa);
      *ov = cast(typeof(*ov)) arg;
      if (! attachedArg) { ++rc; }
      ++rc;
    } else static if (is(typeof(*ov) : real)) {
      checkHaveArg (haveArg, oa);
      *ov = to!(typeof(*ov))(arg);
      if (! attachedArg) { ++rc; }
      ++rc;
    } else {
      debug (1) {
        writefln ("type: %s", typeof(ov).stringof);
      }
      throw new Exception("Unhandled type passed to getopts()");
    }
  }

  debug (1) {
    writefln ("proc:rc:%d:", rc);
  }
  return rc;
}

version (unittest) {
  static string ovstring2;

  int
  dopoTest (T,TV) (bool expected,
      string[] a, int tai, int tasi, string o, T rv, int r, TV v)
  {
    int       rc;
    ArgInfo   ainfo;

    with (ainfo) {
      args = a;
      aidx = tai;
      asidx = tasi;
      alen = args[aidx].length;
      auto ival = indexOf (args[aidx], argAssign);
      if (ival >= 0) {
        alen = ival;
      }
      longOptFlag = false;
      if (args[aidx][0..2] == longOptStart) {
        longOptFlag = true;
      }
    }
    try {
      rc = processOption (ainfo, o, v);
//writeln ("### A:rc:", rc, ":");
//writeln ("### A:v:", *v, ":");
      if (! expected) {
        rc = r - 1;
//writeln ("### B:rc:", rc, ":");
      }
    } catch {
      rc = r;
//writeln ("### C:rc:", rc, ":");
      static if (! is(TV == return)) {
        *v = rv;
      } else {
        ovstring2 = rv;
      }
      if (expected) {
        rc = r - 1;  // fail
//writeln ("### D:rc:", rc, ":");
      }
    }
    return rc;
  }

  mixin template MPOTest (T) {
    void test (T) (string l, bool expected,
        string[] a, int tai, int tasi, string o, T rv, int r)
    {
      T     v;
      bool  fail = false;
      int   rc;

      ++tcount;
      rc = dopoTest!(T,T*) (expected, a, tai, tasi, o, rv, r, &v);
//writeln ("### rc:", rc, ":r:", r, ":");
//writeln ("### v:", v, ":rv:", rv, ":");
      if (rc != r) { fail = true; }
      if (v != rv) { fail = true; }
      if (fail)
      {
        ++failures;
      }
      writefln ("# %s: %d: %s: %s",
        testname, tcount, fail ? "fail" : "pass", l);
    }
  }

  private void
  POTestD (D) (ref int tcount, ref int failures, string testname,
      string l, bool expected,
      string[] a, int tai, int tasi, string o, string rv, int r, D dg)
  {
    bool  fail = false;
    int   rc;

    ++tcount;
    ovstring2 = ovstring2.init;
    rc = dopoTest!(string,D) (expected, a, tai, tasi, o, rv, r, dg);
//writeln ("### rc:", rc, ":r:", r, ":");
//writeln ("### ovs2:", ovstring2, ":rv:", rv, ":");
    if (rc != r) { fail = true; }
    if (ovstring2 != rv) { fail = true; }
    if (fail)
    {
      ++failures;
    }
    writefln ("# %s: %d: %s: %s",
      testname, tcount, fail ? "fail" : "pass", l);
  }
}

// for processOption()
unittest {
  string    testname = "proc";
  int       failures;
  int       tcount;

  mixin MPOTest!bool    tbool;
  mixin MPOTest!int     tint;
  mixin MPOTest!double  tdbl;
  mixin MPOTest!string  tstring;
  mixin MPOTest!(char[]) tchar;
  alias POTestD         tdelg;

  tbool.test ("bool: single, first", true, ["-a"], 0, 1, "a", true, 0);
  tbool.test ("bool: combined, first", true, ["-ab"], 0, 1, "a", true, 0);
  tbool.test ("bool: combined, second", true, ["-ab"], 0, 2, "b", true, 0);
  tint.test ("int: attached", true, ["-a5"], 0, 1, "a", 5, 1);
  tint.test ("int: separate", true, ["-a", "6"], 0, 1, "a", 6, 2);
  tint.test ("int: attached, second", true, ["-ab7"], 0, 2, "b", 7, 1);
  tint.test ("int: separate, second", true, ["-ab", "8"], 0, 2, "b", 8, 2);
  tint.test ("int: no arg", false, ["-a"], 0, 1, "a", 9, 0);
  tdbl.test ("double: attached", true, ["-a5.5"], 0, 1, "a", 5.5, 1);
  tdbl.test ("double: separate", true, ["-a", "6.5"], 0, 1, "a", 6.5, 2);
  tdbl.test ("double: attached, second", true, ["-ab7.5"], 0, 2, "b", 7.5, 1);
  tdbl.test ("double: separate, second", true, ["-ab", "8.5"], 0, 2, "b", 8.5, 2);
  tdbl.test ("double: no arg", false, ["-a"], 0, 1, "a", 9.5, 0);
  tstring.test ("string: attached", true, ["-a5.5x"], 0, 1, "a", "5.5x", 1);
  tstring.test ("string: separate", true, ["-a", "6.5x"], 0, 1, "a", "6.5x", 2);
  tstring.test ("string: attached, second", true, ["-ab7.5x"], 0, 2, "b", "7.5x", 1);
  tstring.test ("string: separate, second", true, ["-ab", "8.5x"], 0, 2, "b", "8.5x", 2);
  tstring.test ("string: no arg", false, ["-a"], 0, 1, "a", "9.5x", 0);
  tchar.test ("char: attached", true, ["-a5.5y"], 0, 1, "a", "5.5y", 1);
  tchar.test ("char: separate", true, ["-a", "6.5y"], 0, 1, "a", "6.5y", 2);
  tchar.test ("char: attached, second", true, ["-ab7.5y"], 0, 2, "b", "7.5y", 1);
  tchar.test ("char: separate, second", true, ["-ab", "8.5y"], 0, 2, "b", "8.5y", 2);
  tchar.test ("char: no arg", false, ["-a"], 0, 1, "a", "9.5y", 0);
  tstring.test ("string: long arg", true, ["--testa", "40"], 0, 2, "testa", "40", 2);
  tbool.test ("bool: long arg", true, ["--testa"], 0, 2, "testa", true, 0);
  tstring.test ("string: long arg=val", true, ["--testa=42"], 0, 2, "testa", "42", 1);

  auto d = delegate() { ovstring2 = "abc"; };

  tdelg (tcount, failures, testname,
    "delegate: 0: ", true, ["-a"], 0, 1, "a", "abc", 0, d);

  auto d2 = delegate(string arg) { ovstring2 = arg; };

  tdelg (tcount, failures, testname,
    "delegate: 1: attached", true, ["-a5.5z"], 0, 1, "a", "5.5z", 1, d2);
  tdelg (tcount, failures, testname,
    "delegate: 1: separated", true, ["-a", "6.5z"], 0, 1, "a", "6.5z", 2, d2);
  tdelg (tcount, failures, testname,
    "delegate: 1: attached, second", true, ["-ab7.5z"], 0, 2, "b", "7.5z", 1, d2);
  tdelg (tcount, failures, testname,
    "delegate: 1: separated, second", true, ["-ab", "8.5z"], 0, 2, "b", "8.5z", 2, d2);
  tdelg (tcount, failures, testname,
    "delegate: 1: no arg", false, ["-a"], 0, 1, "a", "9.5z", 0, d2);

  auto d3 = delegate(string opt, string arg) { ovstring2 = opt ~ arg; };

  tdelg (tcount, failures, testname,
    "delegate: 2: attached", true, ["-a5.5z"], 0, 1, "a", "a5.5z", 1, d3);
  tdelg (tcount, failures, testname,
    "delegate: 2: separated", true, ["-a", "6.5z"], 0, 1, "a", "a6.5z", 2, d3);
  tdelg (tcount, failures, testname,
    "delegate: 2: attached, second", true, ["-ab7.5z"], 0, 2, "b", "b7.5z", 1, d3);
  tdelg (tcount, failures, testname,
    "delegate: 2: separated, second", true, ["-ab", "8.5z"], 0, 2, "b", "b8.5z", 2, d3);
  tdelg (tcount, failures, testname,
    "delegate: 2: no arg", false, ["-a"], 0, 1, "a", "9.5z", 0, d3);

  auto d4 = function (string opt, string arg) { ovstring2 = opt ~ arg; };

/+
  // 35: function: two arg, catenated
  ovstring2 = ovstring2.init;
  rc = doPOTest (tcount, ["-a5.5z"], 0, 1, "a", &d4);
  if (ovstring2 != "a5.5z") { failures ~= tcount; }
  if (rc != 1) { failures ~= tcount; }

  // 36: function: two arg, separated
  ovstring2 = ovstring2.init;
  rc = doPOTest (tcount, ["-a", "6.5z"], 0, 1, "a", &d4);
  if (ovstring2 != "a6.5z") { failures ~= tcount; }
  if (rc != 2) { failures ~= tcount; }

  // 37: function: two arg, catenated, second
  ovstring2 = ovstring2.init;
  rc = doPOTest (tcount, ["-ab7.5z"], 0, 2, "b", &d4);
  if (ovstring2 != "b7.5z") { failures ~= tcount; }
  if (rc != 1) { failures ~= tcount; }

  // 38: function: two arg, separated, second
  ovstring2 = ovstring2.init;
  rc = doPOTest (tcount, ["-ab", "8.5z"], 0, 2, "b", &d4);
  if (ovstring2 != "b8.5z") { failures ~= tcount; }
  if (rc != 2) { failures ~= tcount; }

  // 39: function two arg, no arg
  ovstring2 = ovstring2.init;
  try {
    rc = doPOTest (tcount, ["-a"], 0, 1, "a", &d4);
  } catch {
    ovstring2 = "9.5z";
  }
  if (ovstring2 != "9.5z") { failures ~= tcount; }

+/

  write ("unittest: getopt: processOption: ");
  if (failures > 0) {
    writefln ("failed: %d", failures);
  } else {
    writeln ("passed");
  }
}
