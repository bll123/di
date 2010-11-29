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

struct ArgInfo {
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

version (unittest) {
  mixin template MgetoptsTest(T) {
    void test (string l, bool expected,
        string[] o, string[] a, T[] r, int i) {
      auto v = (T[4]).init;
      int  idx;
      ++tcount;
      writefln ("# %s: %d: %s", testname, tcount, l);
      try {
        idx = getopts (a, o[2], &v[2], o[1], &v[1], o[0], &v[0], o[3], &v[3]);
      } catch {
        v = r;
        if (expected) {     // don't pass a failure
          v = v.init;
        }
      }
      if (v != r) { failures ~= tcount; }
      if (idx != i) { failures ~= tcount; }
    }
  }
}

// for getopts();
unittest {
  int[]     failures;
  int       tcount;

  string    testname = "getopts";

  bool      cobool[4];
  int       coint[4];
  int       idx;

  mixin MgetoptsTest!bool tbool;
  mixin MgetoptsTest!int  tint;
  mixin MgetoptsTest!int  tdouble;

  // 1
  tbool.test ("boolean bundled, all", true, ["a", "b", "c", "d"],
      ["-abcd"], [true,true,true,true], 1);

  // 2
  tbool.test ("boolean separate, all", true, ["a", "b", "c", "d"],
      ["-a", "-b", "-c", "-d"], [true,true,true,true], 4);

  // 3
  tbool.test ("boolean separate, bc", true, ["a", "b", "c", "d"],
      ["-b", "-c"], [false,true,true,false], 2);

  // 4
  tbool.test ("boolean separate, ad", true, ["a", "b", "c", "d"],
      ["-a", "-d"], [true,false,false,true], 2);

  // 5
  tint.test ("int separate, ad", true, ["a", "b", "c", "d"],
      ["-a", "1", "-d", "2"], [1,0,0,2], 4);

  // 6
  tint.test ("int attached, ad", true, ["a", "b", "c", "d"],
      ["-a1", "-d2"], [1,0,0,2], 2);

  // 7
  tint.test ("int non option stop arg", true, ["a", "b", "c", "d"],
      ["-a", "1", "3", "-d", "2"], [1,0,0,0], 2);

  // 8 :
  tint.test ("int -- stop arg", true, ["a", "b", "c", "d"],
      ["-a", "1", "--", "-d", "2"], [1,0,0,0], 3);

  // 9
  tint.test ("int attached non option stop arg", true, ["a", "b", "c", "d"],
      ["-a1", "3", "-d", "2"], [1,0,0,0], 1);

  // 10
  tint.test ("int attached/separate", true, ["a", "b", "c", "d"],
      ["-a1", "-d", "2"], [1,0,0,2], 3);

  // 11 : missing argument
  tint.test ("int missing argument", false, ["a", "b", "c", "d"],
      ["-a1", "-d"], [1,0,0,9], 0);

  // 12
  tint.test ("int long options", true, ["testa", "testb", "testc", "testd"],
      ["--testa", "1", "--testd", "2"], [1,0,0,2], 4);

  // 13 : unrecognized option --arg should throw error
  tint.test ("int unrecognized long option", false, ["testa", "testb", "testc", "testd"],
      ["--testz", "1", "--testd", "2"], [999,0,0,999], 0);

  // 14 : unrecognized option -x should throw error
  tint.test ("int unrecognized short option", false, ["testa", "testb", "testc", "testd"],
      ["-x", "1", "--testd", "2"], [998,0,0,998], 0);

  // 15 : unrecognized option -x should throw error
  tbool.test ("boolean unrecognized option: bundled", false, ["a", "b", "c", "d"],
      ["-abxd"], [true,true,true,true], 0);

  // 16 : --arg boolean
  tbool.test ("boolean long option", true, ["testa", "testb", "testc", "testd"],
      ["--testa", "--testd"], [true,false,false,true], 2);

  // 17 : check returned index
  tbool.test ("boolean check return idx", true, ["a","b","c","d"],
      ["-abcd"], [true,true,true,true], 1);

  write ("unittest: getopt: getopts: ");
  if (failures.length > 0) {
    writeln ("failed:", failures);
  } else {
    writeln ("passed");
  }
}

private int
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

private void
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
private int
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

  int doPOTest (T) (ref int tcount, string[] allargs,
        int taidx, int tasidx, string opt, T var) {
    ++tcount; writefln ("# %s test %d", "proc", tcount);
    static if (is(typeof(*T) == string) ||
        is(typeof(*T) == char[]) ||
        is(typeof(*T) == bool) ||
        is(typeof(*T) : real)) {
      *var = typeof(*T).init;
    }

    ArgInfo ainfo;

    with (ainfo) {
      args = allargs;
      aidx = taidx;
      asidx = tasidx;
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
    auto rc = processOption (ainfo, opt, var);
    return rc;
  }
}

// for processOption()
unittest {
  bool      ovbool;
  int       ovint;
  double    ovdbl;
  string    ovstring;
  char[]    ovchar;

  int       rc;
  int[]  failures;
  int    tcount;
  int    aidx = 0;
  int    asidx = 1;

  // 1 : bool: single argument in first position.
  rc = doPOTest (tcount, ["-a"], 0, 1, "a", &ovbool);
  if (ovbool != true) { failures ~= tcount; }
  if (rc != 0) { failures ~= tcount; }

  // 2: bool: two args, combined, first position
  rc = doPOTest (tcount, ["-ab"], 0, 1, "a", &ovbool);
  if (ovbool != true) { failures ~= tcount; }
  if (rc != 0) { failures ~= tcount; }

  // 3: bool: two args, combined, second position
  rc = doPOTest (tcount, ["-ab"], 0, 2, "b", &ovbool);
  if (ovbool != true) { failures ~= tcount; }
  if (rc != 0) { failures ~= tcount; }

  // 4: int: catenated
  rc = doPOTest (tcount, ["-a5"], 0, 1, "a", &ovint);
  if (ovint != 5) { failures ~= tcount; }
  if (rc != 1) { failures ~= tcount; }

  // 5: int: separated
  rc = doPOTest (tcount, ["-a", "6"], 0, 1, "a", &ovint);
  if (ovint != 6) { failures ~= tcount; }
  if (rc != 2) { failures ~= tcount; }

  // 6: int: catenated, second position
  rc = doPOTest (tcount, ["-ab7"], 0, 2, "b", &ovint);
  if (ovint != 7) { failures ~= tcount; }
  if (rc != 1) { failures ~= tcount; }

  // 7: int: separate, second position
  rc = doPOTest (tcount, ["-ab", "8"], 0, 2, "b", &ovint);
  if (ovint != 8) { failures ~= tcount; }
  if (rc != 2) { failures ~= tcount; }

  // 8: int: no arg
  try {
    rc = doPOTest (tcount, ["-a"], 0, 1, "a", &ovint);
  } catch {
    ovint = 9;
  }
  if (ovint != 9) { failures ~= tcount; }

  // 9: dbl: catenated
  rc = doPOTest (tcount, ["-a5.5"], 0, 1, "a", &ovdbl);
  if (ovdbl != 5.5) { failures ~= tcount; }
  if (rc != 1) { failures ~= tcount; }

  // 10: dbl: separated
  rc = doPOTest (tcount, ["-a", "6.5"], 0, 1, "a", &ovdbl);
  if (ovdbl != 6.5) { failures ~= tcount; }
  if (rc != 2) { failures ~= tcount; }

  // 11: dbl: catenated, second position
  rc = doPOTest (tcount, ["-ab7.5"], 0, 2, "b", &ovdbl);
  if (ovdbl != 7.5) { failures ~= tcount; }
  if (rc != 1) { failures ~= tcount; }

  // 12: dbl: separate, second position
  rc = doPOTest (tcount, ["-ab", "8.5"], 0, 2, "b", &ovdbl);
  if (ovdbl != 8.5) { failures ~= tcount; }
  if (rc != 2) { failures ~= tcount; }

  // 13: dbl: no arg
  try {
    rc = doPOTest (tcount, ["-a"], 0, 1, "a", &ovdbl);
  } catch {
    ovdbl = 9.5;
  }
  if (ovdbl != 9.5) { failures ~= tcount; }

  // 14: string: catenated
  rc = doPOTest (tcount, ["-a5.5x"], 0, 1, "a", &ovstring);
  if (ovstring != "5.5x") { failures ~= tcount; }
  if (rc != 1) { failures ~= tcount; }

  // 15: string: separated
  rc = doPOTest (tcount, ["-a", "6.5x"], 0, 1, "a", &ovstring);
  if (ovstring != "6.5x") { failures ~= tcount; }
  if (rc != 2) { failures ~= tcount; }

  // 16: string: catenated, second position
  rc = doPOTest (tcount, ["-ab7.5x"], 0, 2, "b", &ovstring);
  if (ovstring != "7.5x") { failures ~= tcount; }
  if (rc != 1) { failures ~= tcount; }

  // 17: string: separate, second position
  rc = doPOTest (tcount, ["-ab", "8.5x"], 0, 2, "b", &ovstring);
  if (ovstring != "8.5x") { failures ~= tcount; }
  if (rc != 2) { failures ~= tcount; }

  // 18: string: no arg
  try {
    rc = doPOTest (tcount, ["-a"], 0, 1, "a", &ovstring);
  } catch {
    ovstring = "9.5x";
  }
  if (ovstring != "9.5x") { failures ~= tcount; }

  // 19: char: catenated
  rc = doPOTest (tcount, ["-a5.5y"], 0, 1, "a", &ovchar);
  if (ovchar != "5.5y") { failures ~= tcount; }
  if (rc != 1) { failures ~= tcount; }

  // 20: char: separated
  rc = doPOTest (tcount, ["-a", "6.5y"], 0, 1, "a", &ovchar);
  if (ovchar != "6.5y") { failures ~= tcount; }
  if (rc != 2) { failures ~= tcount; }

  // 21: char: catenated, second position
  rc = doPOTest (tcount, ["-ab7.5y"], 0, 2, "b", &ovchar);
  if (ovchar != "7.5y") { failures ~= tcount; }
  if (rc != 1) { failures ~= tcount; }

  // 22: char: separate, second position
  rc = doPOTest (tcount, ["-ab", "8.5y"], 0, 2, "b", &ovchar);
  if (ovchar != "8.5y") { failures ~= tcount; }
  if (rc != 2) { failures ~= tcount; }

  // 23: char: no arg
  try {
    rc = doPOTest (tcount, ["-a"], 0, 1, "a", &ovchar);
  } catch {
    ovchar = cast(char[]) "9.5y";
  }
  if (ovchar != "9.5y") { failures ~= tcount; }

  auto d = delegate() { ovint = 1; };

  // 24: delegate: no arg
  ovint = ovint.init;
  rc = doPOTest (tcount, ["-a"], 0, 1, "a", d);
  if (ovint != 1) { failures ~= tcount; }
  if (rc != 0) { failures ~= tcount; }

  auto d2 = delegate(string arg) { ovstring = arg; };

  // 25: delegate: one arg, catenated
  ovstring = ovstring.init;
  rc = doPOTest (tcount, ["-a5.5z"], 0, 1, "a", d2);
  if (ovstring != "5.5z") { failures ~= tcount; }
  if (rc != 1) { failures ~= tcount; }

  // 26: delegate: one arg, separated
  ovstring = ovstring.init;
  rc = doPOTest (tcount, ["-a", "6.5z"], 0, 1, "a", d2);
  if (ovstring != "6.5z") { failures ~= tcount; }
  if (rc != 2) { failures ~= tcount; }

  // 27: delegate: one arg, catenated, second
  ovstring = ovstring.init;
  rc = doPOTest (tcount, ["-ab7.5z"], 0, 2, "b", d2);
  if (ovstring != "7.5z") { failures ~= tcount; }
  if (rc != 1) { failures ~= tcount; }

  // 28: delegate: one arg, separated, second
  ovstring = ovstring.init;
  rc = doPOTest (tcount, ["-ab", "8.5z"], 0, 2, "b", d2);
  if (ovstring != "8.5z") { failures ~= tcount; }
  if (rc != 2) { failures ~= tcount; }

  // 29: delegate: one arg, no arg
  ovstring = ovstring.init;
  try {
    rc = doPOTest (tcount, ["-a"], 0, 1, "a", d2);
  } catch {
    ovstring = "9.5z";
  }
  if (ovstring != "9.5z") { failures ~= tcount; }

  auto d3 = delegate(string opt, string arg) { ovstring = opt ~ arg; };

  // 30: delegate: two arg, catenated
  ovstring = ovstring.init;
  rc = doPOTest (tcount, ["-a5.5z"], 0, 1, "a", d3);
  if (ovstring != "a5.5z") { failures ~= tcount; }
  if (rc != 1) { failures ~= tcount; }

  // 31: delegate: two arg, separated
  ovstring = ovstring.init;
  rc = doPOTest (tcount, ["-a", "6.5z"], 0, 1, "a", d3);
  if (ovstring != "a6.5z") { failures ~= tcount; }
  if (rc != 2) { failures ~= tcount; }

  // 32: delegate: two arg, catenated, second
  ovstring = ovstring.init;
  rc = doPOTest (tcount, ["-ab7.5z"], 0, 2, "b", d3);
  if (ovstring != "b7.5z") { failures ~= tcount; }
  if (rc != 1) { failures ~= tcount; }

  // 33: delegate: two arg, separated, second
  ovstring = ovstring.init;
  rc = doPOTest (tcount, ["-ab", "8.5z"], 0, 2, "b", d3);
  if (ovstring != "b8.5z") { failures ~= tcount; }
  if (rc != 2) { failures ~= tcount; }

  // 34: delegate: two arg, no arg
  ovstring = ovstring.init;
  try {
    rc = doPOTest (tcount, ["-a"], 0, 1, "a", d3);
  } catch {
    ovstring = "9.5z";
  }
  if (ovstring != "9.5z") { failures ~= tcount; }

  auto d4 = function (string opt, string arg) { ovstring2 = opt ~ arg; };

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

  // 40: --arg, string
  ovstring = ovstring.init;
  rc = doPOTest (tcount, ["--testa", "40"], 0, 2, "testa", &ovstring);
  if (ovstring != "40") { failures ~= tcount; }
  if (rc != 2) { failures ~= tcount; }

  // 41: --arg, bool
  ovbool = ovbool.init;
  rc = doPOTest (tcount, ["--testa"], 0, 2, "testa", &ovbool);
  if (ovbool != true) { failures ~= tcount; }
  if (rc != 0) { failures ~= tcount; }
  // boolean long option handled by getopts().

  // 42: --arg, string
  ovstring = ovstring.init;
  rc = doPOTest (tcount, ["--testa=42"], 0, 2, "testa", &ovstring);
  if (ovstring != "42") { failures ~= tcount; }
  if (rc != 1) { failures ~= tcount; }

  write ("unittest: getopt: processOption: ");
  if (failures.length > 0) {
    writeln ("failed:", failures);
  } else {
    writeln ("passed");
  }
}
