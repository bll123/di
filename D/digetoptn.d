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
  int   i;
  int   j;

outerfor:
  for (i = 0; i < args.length; ++i) {
    j = 1;
    if (args[i][0] != '-' || args[i] == "--") {
      debug (1) {
        writefln ("getopts:nomore");
      }
      break outerfor;
    }
    debug (1) {
      writefln ("getopts:%d:%s:%s:", i, args[i], args[i][j]);
    }

    int rc = 0;
innerwhile:
    while (j < args[i].length && args[i][j] != '\0') {
      debug (1) {
        writefln ("getopts:i:%d:j:%d:len:%d:a[ij]%s:", i, j, args[i].length, args[i][j]);
      }
      rc = checkOption (args, i, j, opts);
      if (rc != 0) {
        i += rc - 1;  // increment by rc less one, for loop will inc again
        debug (1) {
          writefln ("getopts:rc:%d:i:%d:", rc, i);
        }
        break innerwhile;
      }
      ++j;
    }
  }
  return i;
}

// for getopts();
unittest {
  bool      cobool[4];
  int       coint[4];

  int[]     failures;
  int       tcount;

  // 1
  cobool = (bool[4]).init;
  ++tcount;
  getopts (["-abcd"], "c", &cobool[2], "b", &cobool[1], "a", &cobool[0], "d", &cobool[3]);
  if (cobool != [true,true,true,true]) { failures ~= tcount; }

  // 2
  cobool = (bool[4]).init;
  ++tcount;
  getopts (["-a", "-b", "-c", "-d"], "c", &cobool[2], "b", &cobool[1], "a", &cobool[0], "d", &cobool[3]);
  if (cobool != [true,true,true,true]) { failures ~= tcount; }

  // 3
  cobool = (bool[4]).init;
  ++tcount;
  getopts (["-b", "-c"], "c", &cobool[2], "b", &cobool[1], "a", &cobool[0], "d", &cobool[3]);
  if (cobool != [false,true,true,false]) { failures ~= tcount; }

  // 4
  cobool = (bool[4]).init;
  ++tcount;
  getopts (["-a", "-d"], "c", &cobool[2], "b", &cobool[1], "a", &cobool[0], "d", &cobool[3]);
  if (cobool != [true,false,false,true]) { failures ~= tcount; }

  // 5
  coint = (int[4]).init;
  ++tcount;
  getopts (["-a", "1", "-d", "2"], "c", &coint[2], "b", &coint[1], "a", &coint[0], "d", &coint[3]);
  if (coint != [1,0,0,2]) { failures ~= tcount; }
  debug (1) {
    writefln ("5:%d:%d:%d:%d", coint[0], coint[1], coint[2], coint[3]);
  }

  // 6
  coint = (int[4]).init;
  ++tcount;
  getopts (["-a1", "-d2"], "c", &coint[2], "b", &coint[1], "a", &coint[0], "d", &coint[3]);
  if (coint != [1,0,0,2]) { failures ~= tcount; }
  debug (1) {
    writefln ("6:%d:%d:%d:%d", coint[0], coint[1], coint[2], coint[3]);
  }

  // 7
  coint = (int[4]).init;
  ++tcount;
  getopts (["-a", "1", "3", "-d", "2"], "c", &coint[2], "b", &coint[1], "a", &coint[0], "d", &coint[3]);
  if (coint != [1,0,0,0]) { failures ~= tcount; }
  debug (1) {
    writefln ("7:%d:%d:%d:%d", coint[0], coint[1], coint[2], coint[3]);
  }

  // 8
  coint = (int[4]).init;
  ++tcount;
  getopts (["-a", "1", "--", "-d", "2"], "c", &coint[2], "b", &coint[1], "a", &coint[0], "d", &coint[3]);
  if (coint != [1,0,0,0]) { failures ~= tcount; }
  debug (1) {
    writefln ("8:%d:%d:%d:%d", coint[0], coint[1], coint[2], coint[3]);
  }

  // 9
  coint = (int[4]).init;
  ++tcount;
  getopts (["-a1", "3", "-d", "2"], "c", &coint[2], "b", &coint[1], "a", &coint[0], "d", &coint[3]);
  if (coint != [1,0,0,0]) { failures ~= tcount; }
  debug (1) {
    writefln ("9:%d:%d:%d:%d", coint[0], coint[1], coint[2], coint[3]);
  }

  // 10
  coint = (int[4]).init;
  ++tcount;
  getopts (["-a1", "-d", "2"], "c", &coint[2], "b", &coint[1], "a", &coint[0], "d", &coint[3]);
  if (coint != [1,0,0,2]) { failures ~= tcount; }
  debug (1) {
    writefln ("10:%d:%d:%d:%d", coint[0], coint[1], coint[2], coint[3]);
  }

  // 11
  coint = (int[4]).init;
  ++tcount;
  try {
    getopts (["-a1", "-d"], "c", &coint[2], "b", &coint[1], "a", &coint[0], "d", &coint[3]);
  } catch {
    coint[3] = 9;
  }
  if (coint != [1,0,0,9]) { failures ~= tcount; }
  debug (1) {
    writefln ("11:%d:%d:%d:%d", coint[0], coint[1], coint[2], coint[3]);
  }

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
    if (opt[0] == args[aidx][aidx2]) {
      debug (1) {
        writefln ("chk:found:%d:%d:%s:", aidx, aidx2, args[aidx][aidx2]);
        writefln ("  chk:%s:", typeof(opts[0]).stringof);
        writefln ("  chk:%s:", typeof(opts[1]).stringof);
      }
      return processOption (args, aidx, aidx2, opts[0], opts[1]);
    } else {
      return checkOption (args, aidx, aidx2, opts[2..$]);
    }
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
    foreach (i, arg; args) {
      writefln ("proc:args:%d:%s:", i, arg);
    }
  }

  string    arg;
  bool      attachedArg = false;
  bool      haveArg = false;
  auto      i = aidx2 + 1;
  if (args[aidx].length > i && args[aidx][i] != '\0') {
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
      if (! haveArg) { throw new Exception("Missing argument for -" ~ oa); }
      ov (arg);
      if (! attachedArg) { ++rc; }
      ++rc;
    } else static if (is(typeof(ov("","")) : void)) {
      if (! haveArg) { throw new Exception("Missing argument for -" ~ oa); }
      ov (oa,arg);
      if (! attachedArg) { ++rc; }
      ++rc;
    }
  } else static if (is(typeof(*ov) == return)) {
    static if (is(typeof((*ov)()) : void)) {
      (*ov)();
    } else static if (is(typeof((*ov)("")) : void)) {
      if (! haveArg) { throw new Exception("Missing argument for -" ~ oa); }
      (*ov)(arg);
      if (! attachedArg) { ++rc; }
      ++rc;
    } else static if (is(typeof((*ov)("","")) : void)) {
      if (! haveArg) { throw new Exception("Missing argument for -" ~ oa); }
      (*ov)(oa,arg);
      if (! attachedArg) { ++rc; }
      ++rc;
    }
  } else static if (is(typeof(*ov) == string) ||
        is(typeof(*ov) == char[])) {
    if (! haveArg) { throw new Exception("Missing argument for -" ~ oa); }
    *ov = cast(typeof(*ov)) arg;
    if (! attachedArg) { ++rc; }
    ++rc;
  } else static if (is(typeof(*ov) : real)) {
    if (! haveArg) { throw new Exception("Missing argument for -" ~ oa); }
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

  void doPOTest (T) (ref int tcount, string[] args,
        int aidx, int aidx2, string opt, T var) {
    ++tcount;
    static if (is(typeof(*T) == string) ||
        is(typeof(*T) == char[]) ||
        is(typeof(*T) == bool) ||
        is(typeof(*T) : real)) {
      *var = typeof(*T).init;
    }
    processOption (args, aidx, aidx2, opt, var);
  }
}

// for processOption()
unittest {
  bool      ovbool;
  int       ovint;
  double    ovdbl;
  string    ovstring;
  char[]    ovchar;

  int[]  failures;
  int    tcount;
  int    aidx = 0;
  int    aidx2 = 1;

  // 1 : bool: single argument in first position.
  doPOTest (tcount, ["-a"], 0, 1, "a", &ovbool);
  if (ovbool != true) { failures ~= tcount; }

  // 2: bool: two args, combined, first position
  doPOTest (tcount, ["-ab"], 0, 1, "a", &ovbool);
  if (ovbool != true) { failures ~= tcount; }

  // 3: bool: two args, combined, second position
  doPOTest (tcount, ["-ab"], 0, 2, "b", &ovbool);
  if (ovbool != true) { failures ~= tcount; }

  // 4: int: catenated
  doPOTest (tcount, ["-a5"], 0, 1, "a", &ovint);
  if (ovint != 5) { failures ~= tcount; }

  // 5: int: separated
  doPOTest (tcount, ["-a", "6"], 0, 1, "a", &ovint);
  if (ovint != 6) { failures ~= tcount; }

  // 6: int: catenated, second position
  doPOTest (tcount, ["-ab7"], 0, 2, "b", &ovint);
  if (ovint != 7) { failures ~= tcount; }

  // 7: int: separate, second position
  doPOTest (tcount, ["-ab", "8"], 0, 2, "b", &ovint);
  if (ovint != 8) { failures ~= tcount; }

  // 8: int: no arg
  try {
    doPOTest (tcount, ["-a"], 0, 1, "a", &ovint);
  } catch {
    ovint = 9;
  }
  if (ovint != 9) { failures ~= tcount; }

  // 9: dbl: catenated
  doPOTest (tcount, ["-a5.5"], 0, 1, "a", &ovdbl);
  if (ovdbl != 5.5) { failures ~= tcount; }

  // 10: dbl: separated
  doPOTest (tcount, ["-a", "6.5"], 0, 1, "a", &ovdbl);
  if (ovdbl != 6.5) { failures ~= tcount; }

  // 11: dbl: catenated, second position
  doPOTest (tcount, ["-ab7.5"], 0, 2, "b", &ovdbl);
  if (ovdbl != 7.5) { failures ~= tcount; }

  // 12: dbl: separate, second position
  doPOTest (tcount, ["-ab", "8.5"], 0, 2, "b", &ovdbl);
  if (ovdbl != 8.5) { failures ~= tcount; }

  // 13: dbl: no arg
  try {
    doPOTest (tcount, ["-a"], 0, 1, "a", &ovdbl);
  } catch {
    ovdbl = 9.5;
  }
  if (ovdbl != 9.5) { failures ~= tcount; }

  // 14: string: catenated
  doPOTest (tcount, ["-a5.5x"], 0, 1, "a", &ovstring);
  if (ovstring != "5.5x") { failures ~= tcount; }

  // 15: string: separated
  doPOTest (tcount, ["-a", "6.5x"], 0, 1, "a", &ovstring);
  if (ovstring != "6.5x") { failures ~= tcount; }

  // 16: string: catenated, second position
  doPOTest (tcount, ["-ab7.5x"], 0, 2, "b", &ovstring);
  if (ovstring != "7.5x") { failures ~= tcount; }

  // 17: string: separate, second position
  doPOTest (tcount, ["-ab", "8.5x"], 0, 2, "b", &ovstring);
  if (ovstring != "8.5x") { failures ~= tcount; }

  // 18: string: no arg
  try {
    doPOTest (tcount, ["-a"], 0, 1, "a", &ovstring);
  } catch {
    ovstring = "9.5x";
  }
  if (ovstring != "9.5x") { failures ~= tcount; }

  // 19: char: catenated
  doPOTest (tcount, ["-a5.5y"], 0, 1, "a", &ovchar);
  if (ovchar != "5.5y") { failures ~= tcount; }

  // 20: char: separated
  doPOTest (tcount, ["-a", "6.5y"], 0, 1, "a", &ovchar);
  if (ovchar != "6.5y") { failures ~= tcount; }

  // 21: char: catenated, second position
  doPOTest (tcount, ["-ab7.5y"], 0, 2, "b", &ovchar);
  if (ovchar != "7.5y") { failures ~= tcount; }

  // 22: char: separate, second position
  doPOTest (tcount, ["-ab", "8.5y"], 0, 2, "b", &ovchar);
  if (ovchar != "8.5y") { failures ~= tcount; }

  // 23: char: no arg
  try {
    doPOTest (tcount, ["-a"], 0, 1, "a", &ovchar);
  } catch {
    ovchar = cast(char[]) "9.5y";
  }
  if (ovchar != "9.5y") { failures ~= tcount; }

  auto d = delegate() { ovint = 1; };

  // 24: delegate: no arg
  ovint = ovint.init;
  doPOTest (tcount, ["-a5.5x"], 0, 1, "a", d);
  if (ovint != 1) { failures ~= tcount; }

  auto d2 = delegate(string arg) { ovstring = arg; };

  // 25: delegate: one arg, catenated
  ovstring = ovstring.init;
  doPOTest (tcount, ["-a5.5z"], 0, 1, "a", d2);
  if (ovstring != "5.5z") { failures ~= tcount; }

  // 26: delegate: one arg, separated
  ovstring = ovstring.init;
  doPOTest (tcount, ["-a", "6.5z"], 0, 1, "a", d2);
  if (ovstring != "6.5z") { failures ~= tcount; }

  // 27: delegate: one arg, catenated, second
  ovstring = ovstring.init;
  doPOTest (tcount, ["-ab7.5z"], 0, 2, "b", d2);
  if (ovstring != "7.5z") { failures ~= tcount; }

  // 28: delegate: one arg, separated, second
  ovstring = ovstring.init;
  doPOTest (tcount, ["-ab", "8.5z"], 0, 2, "b", d2);
  if (ovstring != "8.5z") { failures ~= tcount; }

  // 29: delegate: one arg, no arg
  ovstring = ovstring.init;
  try {
    doPOTest (tcount, ["-a"], 0, 1, "a", d2);
  } catch {
    ovstring = "9.5z";
  }
  if (ovstring != "9.5z") { failures ~= tcount; }

  auto d3 = delegate(string opt, string arg) { ovstring = opt ~ arg; };

  // 30: delegate: two arg, catenated
  ovstring = ovstring.init;
  doPOTest (tcount, ["-a5.5z"], 0, 1, "a", d3);
  if (ovstring != "a5.5z") { failures ~= tcount; }

  // 31: delegate: two arg, separated
  ovstring = ovstring.init;
  doPOTest (tcount, ["-a", "6.5z"], 0, 1, "a", d3);
  if (ovstring != "a6.5z") { failures ~= tcount; }

  // 32: delegate: two arg, catenated, second
  ovstring = ovstring.init;
  doPOTest (tcount, ["-ab7.5z"], 0, 2, "b", d3);
  if (ovstring != "b7.5z") { failures ~= tcount; }

  // 33: delegate: two arg, separated, second
  ovstring = ovstring.init;
  doPOTest (tcount, ["-ab", "8.5z"], 0, 2, "b", d3);
  if (ovstring != "b8.5z") { failures ~= tcount; }

  // 34: delegate: two arg, no arg
  ovstring = ovstring.init;
  try {
    doPOTest (tcount, ["-a"], 0, 1, "a", d3);
  } catch {
    ovstring = "9.5z";
  }
  if (ovstring != "9.5z") { failures ~= tcount; }

  auto d4 = function (string opt, string arg) { ovstring2 = opt ~ arg; };

  // 35: function: two arg, catenated
  ovstring2 = ovstring2.init;
  doPOTest (tcount, ["-a5.5z"], 0, 1, "a", &d4);
  if (ovstring2 != "a5.5z") { failures ~= tcount; }

  // 36: function: two arg, separated
  ovstring2 = ovstring2.init;
  doPOTest (tcount, ["-a", "6.5z"], 0, 1, "a", &d4);
  if (ovstring2 != "a6.5z") { failures ~= tcount; }

  // 37: function: two arg, catenated, second
  ovstring2 = ovstring2.init;
  doPOTest (tcount, ["-ab7.5z"], 0, 2, "b", &d4);
  if (ovstring2 != "b7.5z") { failures ~= tcount; }

  // 38: function: two arg, separated, second
  ovstring2 = ovstring2.init;
  doPOTest (tcount, ["-ab", "8.5z"], 0, 2, "b", &d4);
  if (ovstring2 != "b8.5z") { failures ~= tcount; }

  // 39: function two arg, no arg
  ovstring2 = ovstring2.init;
  try {
    doPOTest (tcount, ["-a"], 0, 1, "a", &d4);
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
