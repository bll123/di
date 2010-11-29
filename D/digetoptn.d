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

int
getopts (OVL...) (string[] args, OVL opts)
{
  assert (opts.length % 2 == 0);
  int       i;
  int       j;

  debug (1) {
    foreach (k, arg; args) {
      writefln ("getopts:args:%d:%s:", k, arg);
    }
  }

outerfor:
  for (i = 0; i < args.length; ++i) {
    j = 1;
    if (args[i][0] != '-' || args[i] == "--") {
      if (args[i] == "--") {
        ++i;  // point to next argument.
      }
      debug (1) {
        writefln ("getopts:nomore:i:%d:", i);
      }
      break outerfor;
    }
    debug (1) {
      writefln ("getopts:%d:%s:%s:", i, args[i], args[i][j]);
    }

    int rc = 0;
innerwhile:
    while (j < args[i].length && args[i][j] != '\0') {
      if (args[i][0..2] == "--") {
        j = 2;
      }
      debug (1) {
        writefln ("getopts:i:%d:j:%d:len:%d:a[ij]%s:%s:", i, j, args[i].length, args[i][j], args[i]);
      }
      rc = checkOption (args, i, j, opts);
      if (rc < 0) {
        string s;
        if (args[i][0..2] == "--") {
          s = format ("Unrecognized option %s", args[i]);
        } else {
          s = format ("Unrecognized option -%s", args[i][j]);
        }
        debug (1) {
          writefln ("** throwing %s", s);
        }
        throw new Exception (s);
      }
      if (rc != 0) {
        i += rc - 1;  // increment by rc less one, for loop will inc again
        debug (1) {
          writefln ("getopts:rc:%d:i:%d:", rc, i);
        }
        break innerwhile;
      }
      // handles boolean --arg
      if (args[i][0..2] == "--") {
        break innerwhile;
      }
      ++j;
    }
  }

  debug (1) {
    writefln ("getopts:exit:i:%d", i);
  }
  return i;
}

// for getopts();
unittest {
  string    testname = "getopts";
  bool      cobool[4];
  int       coint[4];

  int[]     failures;
  int       tcount;
  int       idx;

  // 1
  cobool = (bool[4]).init;
  ++tcount; writefln ("# %s test %d", testname, tcount);
  idx = getopts (["-abcd"], "c", &cobool[2], "b", &cobool[1], "a", &cobool[0], "d", &cobool[3]);
  if (cobool != [true,true,true,true]) { failures ~= tcount; }
  if (idx != 1) { failures ~= tcount; }

  // 2
  cobool = (bool[4]).init;
  ++tcount; writefln ("# %s test %d", testname, tcount);
  idx = getopts (["-a", "-b", "-c", "-d"], "c", &cobool[2], "b", &cobool[1], "a", &cobool[0], "d", &cobool[3]);
  if (cobool != [true,true,true,true]) { failures ~= tcount; }
  if (idx != 4) { failures ~= tcount; }

  // 3
  cobool = (bool[4]).init;
  ++tcount; writefln ("# %s test %d", testname, tcount);
  idx = getopts (["-b", "-c"], "c", &cobool[2], "b", &cobool[1], "a", &cobool[0], "d", &cobool[3]);
  if (cobool != [false,true,true,false]) { failures ~= tcount; }
  if (idx != 2) { failures ~= tcount; }

  // 4
  cobool = (bool[4]).init;
  ++tcount; writefln ("# %s test %d", testname, tcount);
  idx = getopts (["-a", "-d"], "c", &cobool[2], "b", &cobool[1], "a", &cobool[0], "d", &cobool[3]);
  if (cobool != [true,false,false,true]) { failures ~= tcount; }
  if (idx != 2) { failures ~= tcount; }

  // 5
  coint = (int[4]).init;
  ++tcount; writefln ("# %s test %d", testname, tcount);
  idx = getopts (["-a", "1", "-d", "2"], "c", &coint[2], "b", &coint[1], "a", &coint[0], "d", &coint[3]);
  if (coint != [1,0,0,2]) { failures ~= tcount; }
  if (idx != 4) { failures ~= tcount; }
  debug (1) {
    writefln ("5:%d:%d:%d:%d", coint[0], coint[1], coint[2], coint[3]);
  }

  // 6
  coint = (int[4]).init;
  ++tcount; writefln ("# %s test %d", testname, tcount);
  idx = getopts (["-a1", "-d2"], "c", &coint[2], "b", &coint[1], "a", &coint[0], "d", &coint[3]);
  if (coint != [1,0,0,2]) { failures ~= tcount; }
  if (idx != 2) { failures ~= tcount; }
  debug (1) {
    writefln ("6:%d:%d:%d:%d", coint[0], coint[1], coint[2], coint[3]);
  }

  // 7
  coint = (int[4]).init;
  ++tcount; writefln ("# %s test %d", testname, tcount);
  idx = getopts (["-a", "1", "3", "-d", "2"], "c", &coint[2], "b", &coint[1], "a", &coint[0], "d", &coint[3]);
  if (coint != [1,0,0,0]) { failures ~= tcount; }
  if (idx != 2) { failures ~= tcount; }
  debug (1) {
    writefln ("7:%d:%d:%d:%d:idx:%d", coint[0], coint[1], coint[2], coint[3], idx);
  }

  // 8 :
  coint = (int[4]).init;
  ++tcount; writefln ("# %s test %d", testname, tcount);
  idx = getopts (["-a", "1", "--", "-d", "2"], "c", &coint[2], "b", &coint[1], "a", &coint[0], "d", &coint[3]);
  if (coint != [1,0,0,0]) { failures ~= tcount; }
  if (idx != 3) { failures ~= tcount; }
  debug (1) {
    writefln ("8:%d:%d:%d:%d:idx:%d", coint[0], coint[1], coint[2], coint[3], idx);
  }

  // 9
  coint = (int[4]).init;
  ++tcount; writefln ("# %s test %d", testname, tcount);
  idx = getopts (["-a1", "3", "-d", "2"], "c", &coint[2], "b", &coint[1], "a", &coint[0], "d", &coint[3]);
  if (coint != [1,0,0,0]) { failures ~= tcount; }
  if (idx != 1) { failures ~= tcount; }
  debug (1) {
    writefln ("9:%d:%d:%d:%d", coint[0], coint[1], coint[2], coint[3]);
  }

  // 10
  coint = (int[4]).init;
  ++tcount; writefln ("# %s test %d", testname, tcount);
  idx = getopts (["-a1", "-d", "2"], "c", &coint[2], "b", &coint[1], "a", &coint[0], "d", &coint[3]);
  if (coint != [1,0,0,2]) { failures ~= tcount; }
  if (idx != 3) { failures ~= tcount; }
  debug (1) {
    writefln ("10:%d:%d:%d:%d", coint[0], coint[1], coint[2], coint[3]);
  }

  // 11 : missing argument
  coint = (int[4]).init;
  ++tcount; writefln ("# %s test %d", testname, tcount);
  try {
    idx = getopts (["-a1", "-d"], "c", &coint[2], "b", &coint[1], "a", &coint[0], "d", &coint[3]);
  } catch {
    coint[3] = 9;
  }
  if (coint != [1,0,0,9]) { failures ~= tcount; }
  debug (1) {
    writefln ("11:%d:%d:%d:%d", coint[0], coint[1], coint[2], coint[3]);
  }

  // 12
  coint = (int[4]).init;
  ++tcount; writefln ("# %s test %d", testname, tcount);
  idx = getopts (["--testa", "1", "--testd", "2"], "testc", &coint[2], "testb", &coint[1], "testa", &coint[0], "testd", &coint[3]);
  if (coint != [1,0,0,2]) { failures ~= tcount; }
  if (idx != 4) { failures ~= tcount; }
  debug (1) {
    writefln ("10:%d:%d:%d:%d", coint[0], coint[1], coint[2], coint[3]);
  }

  // 13 : unrecognized option --arg should throw error
  coint = (int[4]).init;
  ++tcount; writefln ("# %s test %d", testname, tcount);
  try {
    idx = getopts (["--testz", "1", "--testd", "2"], "testc", &coint[2], "testb", &coint[1], "testa", &coint[0], "testd", &coint[3]);
  } catch {
    coint[0] = 999;
    coint[3] = 999;
  }
  if (coint != [999,0,0,999]) { failures ~= tcount; }

  // 14 : unrecognized option -x should throw error
  coint = (int[4]).init;
  ++tcount; writefln ("# %s test %d", testname, tcount);
  try {
    idx = getopts (["-x", "1", "--testd", "2"], "testc", &coint[2], "testb", &coint[1], "testa", &coint[0], "testd", &coint[3]);
  } catch {
    coint[0] = 998;
    coint[3] = 998;
  }
  if (coint != [998,0,0,998]) { failures ~= tcount; }

  // 15 : unrecognized option -x should throw error
  coint = (int[4]).init;
  cobool = (int[4]).init;
  ++tcount; writefln ("# %s test %d", testname, tcount);
  try {
    idx = getopts (["-abxd"], "c", &cobool[2], "b", &cobool[1], "a", &cobool[0], "d", &cobool[3]);
  } catch {
    coint[0] = 997;
    coint[3] = 997;
  }
  if (coint != [997,0,0,997]) { failures ~= tcount; }

  // 16 : --arg boolean
  cobool = (bool[4]).init;
  ++tcount; writefln ("# %s test %d", testname, tcount);
  try {
    idx = getopts (["--testa", "--testd", ], "testc", &cobool[2], "testb", &cobool[1], "testa", &cobool[0], "testd", &cobool[3]);
    if (cobool != [true,0,0,true]) { failures ~= tcount; }
    if (idx != 2) { failures ~= tcount; }
  } catch {
    failures ~= tcount;
  }

  // 17 : check returned index
  cobool = (bool[4]).init;
  ++tcount; writefln ("# %s test %d", testname, tcount);
  idx = getopts (["-abcd"], "c", &cobool[2], "b", &cobool[1], "a", &cobool[0], "d", &cobool[3]);
  if (cobool != [true,true,true,true]) { failures ~= tcount; }
  if (idx != 1) { failures ~= tcount; }

  write ("unittest: getopt: getopts: ");
  if (failures.length > 0) {
    writeln ("failed:", failures);
  } else {
    writeln ("passed");
  }
}

private int
checkOption (OVL...)
    (string[] args, int aidx, int aidx2, OVL opts)
{
  static if (opts.length > 0) {
    auto opt = to!string(opts[0]);
    if ((opt.length == 1 && opt[0] == args[aidx][aidx2]) ||
        (opt.length > 1 && opt == args[aidx][2..$])) {
      debug (1) {
        writefln ("chk:found:%s:", opt);
      }
      return processOption (args, aidx, aidx2, opt, opts[1]);
    } else {
      return checkOption (args, aidx, aidx2, opts[2..$]);
    }
  } else {
    return -1;
  }

  return 0;
}

// can't pass ov as a ref parameter -- doesn't handle delegates.
private int
processOption (OV)
    (string[] args, int aidx, int aidx2, string oa, OV ov)
{
  int rc = 0;

  debug (1) {
    writefln ("proc:args[%d]:%s:", aidx, args[aidx]);
  }

  string    arg;
  bool      attachedArg = false;
  bool      haveArg = false;
  auto      i = aidx2 + 1;
  // if there is a --arg (aidx > 2 and prior char is -), there is no arg.
  // if there's no more string, then there is no arg.
  // if there's a null byte, there there is no arg.
  if (! (aidx2 > 1 && args[aidx][aidx2 - 1] == '-') &&
      args[aidx].length > i && args[aidx][i] != '\0') {
    arg = args[aidx][i..$];
    attachedArg = true;
    haveArg = true;
  } else if (args.length > aidx + 1) {
    arg = args[aidx + 1];
    haveArg = true;
  }
  debug (1) {
    writefln ("proc:arg:%s:aidx:%d:aidx2:%d:i:%d:att:%d:have:%d",
        arg, aidx, aidx2, i, attachedArg, haveArg);
  }

  static if (is(typeof(*ov) == bool)) {
    *ov = true;
  } else static if (is(typeof(ov) == return)) {
    static if (is(typeof(ov()) : void)) {
      ov ();
    } else static if (is(typeof(ov("")) : void)) {
      if (! haveArg) {
        string s = "Missing argument for -" ~ oa;
        debug (1) {
          writefln ("** throwing %s", s);
        }
        throw new Exception(s);
      }
      ov (arg);
      if (! attachedArg) { ++rc; }
      ++rc;
    } else static if (is(typeof(ov("","")) : void)) {
      if (! haveArg) {
        string s = "Missing argument for -" ~ oa;
        debug (1) {
          writefln ("** throwing %s", s);
        }
        throw new Exception(s);
      }
      ov (oa,arg);
      if (! attachedArg) { ++rc; }
      ++rc;
    }
  } else static if (is(typeof(*ov) == return)) {
    static if (is(typeof((*ov)()) : void)) {
      (*ov)();
    } else static if (is(typeof((*ov)("")) : void)) {
      if (! haveArg) {
        string s = "Missing argument for -" ~ oa;
        debug (1) {
          writefln ("** throwing %s", s);
        }
        throw new Exception(s);
      }
      (*ov)(arg);
      if (! attachedArg) { ++rc; }
      ++rc;
    } else static if (is(typeof((*ov)("","")) : void)) {
      if (! haveArg) {
        string s = "Missing argument for -" ~ oa;
        debug (1) {
          writefln ("** throwing %s", s);
        }
        throw new Exception(s);
      }
      (*ov)(oa,arg);
      if (! attachedArg) { ++rc; }
      ++rc;
    }
  } else static if (is(typeof(*ov) == string) ||
        is(typeof(*ov) == char[])) {
    if (! haveArg) {
      string s = "Missing argument for -" ~ oa;
      debug (1) {
        writefln ("** throwing %s", s);
      }
      throw new Exception(s);
    }
    *ov = cast(typeof(*ov)) arg;
    if (! attachedArg) { ++rc; }
    ++rc;
  } else static if (is(typeof(*ov) : real)) {
    if (! haveArg) {
      string s = "Missing argument for -" ~ oa;
      debug (1) {
        writefln ("** throwing %s", s);
      }
      throw new Exception(s);
    }
    *ov = to!(typeof(*ov))(arg);
    if (! attachedArg) { ++rc; }
    ++rc;
  } else {
    debug (1) {
      writefln ("type: %s", typeof(ov).stringof);
    }
    throw new Exception("Unhandled type passed to getopts()");
  }

  debug (1) {
    writefln ("proc:rc:%d:", rc);
  }
  return rc;
}

version (unittest) {
  static string ovstring2;

  int doPOTest (T) (ref int tcount, string[] args,
        int aidx, int aidx2, string opt, T var) {
    ++tcount; writefln ("# %s test %d", "proc", tcount);
    static if (is(typeof(*T) == string) ||
        is(typeof(*T) == char[]) ||
        is(typeof(*T) == bool) ||
        is(typeof(*T) : real)) {
      *var = typeof(*T).init;
    }
    auto rc = processOption (args, aidx, aidx2, opt, var);
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
  int    aidx2 = 1;

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

  write ("unittest: getopt: processOption: ");
  if (failures.length > 0) {
    writeln ("failed:", failures);
  } else {
    writeln ("passed");
  }
}
