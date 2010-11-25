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
  bool      coboola, coboolb, coboolc, coboold;
  int       cointa, cointb, cointc, cointd;

  int[]     failures;
  int       tcount;

  // 1
  coboola = bool.init;
  coboolb = bool.init;
  coboolc = bool.init;
  coboold = bool.init;
  ++tcount;
  getopts (["-abcd"], "c", &coboolc, "b", &coboolb, "a", &coboola, "d", &coboold);
  if (coboola != true) { failures ~= tcount; }
  if (coboolb != true) { failures ~= tcount; }
  if (coboolc != true) { failures ~= tcount; }
  if (coboold != true) { failures ~= tcount; }

  // 2
  coboola = bool.init;
  coboolb = bool.init;
  coboolc = bool.init;
  coboold = bool.init;
  ++tcount;
  getopts (["-a", "-b", "-c", "-d"], "c", &coboolc, "b", &coboolb, "a", &coboola, "d", &coboold);
  if (coboola != true) { failures ~= tcount; }
  if (coboolb != true) { failures ~= tcount; }
  if (coboolc != true) { failures ~= tcount; }
  if (coboold != true) { failures ~= tcount; }

  // 3
  coboola = bool.init;
  coboolb = bool.init;
  coboolc = bool.init;
  coboold = bool.init;
  ++tcount;
  getopts (["-b", "-c"], "c", &coboolc, "b", &coboolb, "a", &coboola, "d", &coboold);
  if (coboola != false) { failures ~= tcount; }
  if (coboolb != true) { failures ~= tcount; }
  if (coboolc != true) { failures ~= tcount; }
  if (coboold != false) { failures ~= tcount; }

  // 4
  coboola = bool.init;
  coboolb = bool.init;
  coboolc = bool.init;
  coboold = bool.init;
  ++tcount;
  getopts (["-a", "-d"], "c", &coboolc, "b", &coboolb, "a", &coboola, "d", &coboold);
  if (coboola != true) { failures ~= tcount; }
  if (coboolb != false) { failures ~= tcount; }
  if (coboolc != false) { failures ~= tcount; }
  if (coboold != true) { failures ~= tcount; }

  // 5
  cointa = int.init;
  cointb = int.init;
  cointc = int.init;
  cointd = int.init;
  ++tcount;
  getopts (["-a", "1", "-d", "2"], "c", &cointc, "b", &cointb, "a", &cointa, "d", &cointd);
  if (cointa != 1) { failures ~= tcount; }
  if (cointb != 0) { failures ~= tcount; }
  if (cointc != 0) { failures ~= tcount; }
  if (cointd != 2) { failures ~= tcount; }
  debug (1) {
    writefln ("5:%d:%d:%d:%d", cointa, cointb, cointc, cointd);
  }

  // 6
  cointa = int.init;
  cointb = int.init;
  cointc = int.init;
  cointd = int.init;
  ++tcount;
  getopts (["-a1", "-d2"], "c", &cointc, "b", &cointb, "a", &cointa, "d", &cointd);
  if (cointa != 1) { failures ~= tcount; }
  if (cointb != 0) { failures ~= tcount; }
  if (cointc != 0) { failures ~= tcount; }
  if (cointd != 2) { failures ~= tcount; }
  debug (1) {
    writefln ("6:%d:%d:%d:%d", cointa, cointb, cointc, cointd);
  }

  // 7
  cointa = int.init;
  cointb = int.init;
  cointc = int.init;
  cointd = int.init;
  ++tcount;
  getopts (["-a", "1", "3", "-d", "2"], "c", &cointc, "b", &cointb, "a", &cointa, "d", &cointd);
  if (cointa != 1) { failures ~= tcount; }
  if (cointb != 0) { failures ~= tcount; }
  if (cointc != 0) { failures ~= tcount; }
  if (cointd != 0) { failures ~= tcount; }
  debug (1) {
    writefln ("7:%d:%d:%d:%d", cointa, cointb, cointc, cointd);
  }

  // 8
  cointa = int.init;
  cointb = int.init;
  cointc = int.init;
  cointd = int.init;
  ++tcount;
  getopts (["-a", "1", "--", "-d", "2"], "c", &cointc, "b", &cointb, "a", &cointa, "d", &cointd);
  if (cointa != 1) { failures ~= tcount; }
  if (cointb != 0) { failures ~= tcount; }
  if (cointc != 0) { failures ~= tcount; }
  if (cointd != 0) { failures ~= tcount; }
  debug (1) {
    writefln ("8:%d:%d:%d:%d", cointa, cointb, cointc, cointd);
  }

  write ("unittest: getopt: getopts: ");
  if (failures.length > 0) {
    writeln ("failed:", failures);
  } else {
    writeln ("passed");
  }
}

private int checkOption (OVL...)
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


private int processOption (OV)
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
    writefln ("proc:arg:%s:att:%d:have:%d", arg, attachedArg, haveArg);
  }

  static if (is(typeof(*ov) == bool)) {
    *ov = true;
  } else static if (is(typeof(ov) == delegate) ||
        is(typeof(*ov) == function)) {
    static if (is(typeof(ov()) : void)) {
      ov ();
    } else static if (is(typeof(ov("")) : void)) {
      if (! haveArg) { throw new Exception("Argument not specified for " ~ args[aidx]); }
      ov (arg);
      if (! attachedArg) { ++rc; }
      ++rc;
    } else static if (is(typeof(ov("","")) : void)) {
      if (! haveArg) { throw new Exception("Argument not specified for " ~ args[aidx]); }
      ov (oa,arg);
      if (! attachedArg) { ++rc; }
      ++rc;
    }
  } else static if (is(typeof(*ov) == string) ||
        is(typeof(*ov) == char[])) {
    if (! haveArg) { throw new Exception("Argument not specified for " ~ args[aidx]); }
    *ov = cast(typeof(*ov)) arg;
    if (! attachedArg) { ++rc; }
    ++rc;
  } else static if (is(typeof(*ov) : real)) {
    if (! haveArg) { throw new Exception("Argument not specified for " ~ args[aidx]); }
    *ov = to!(typeof(*ov))(arg);
    if (! attachedArg) { ++rc; }
    ++rc;
  } else {
    throw new Exception("Unhandled type passed to getopts()");
  }

  debug (1) {
    writefln ("proc:rc:%d:", rc);
  }
  return rc;
}

// for processOption()
unittest {
  bool   ovbool;
  int    ovint;
  double ovdbl;
  string ovstring;
  char[] ovchar;

  int[]  failures;
  int    tcount;
  int    aidx = 0;
  int    aidx2 = 1;

  // bool: single argument in first position.
  ovbool = bool.init;
  aidx = 0;
  aidx2 = 1;
  ++tcount;
  processOption (["-a"], aidx, aidx2, "a", &ovbool);
  if (ovbool != true) { failures ~= tcount; }
  // bool: two args, combined, first position
  ovbool = bool.init;
  aidx = 0;
  aidx2 = 1;
  ++tcount;
  processOption (["-ab"], aidx, aidx2, "a", &ovbool);
  if (ovbool != true) { failures ~= tcount; }
  // bool: two args, combined, second position
  ovbool = bool.init;
  aidx = 0;
  aidx2 = 2;
  ++tcount;
  processOption (["-ab"], aidx, aidx2, "b", &ovbool);
  if (ovbool != true) { failures ~= tcount; }

  // int: catenated
  ovint = int.init;
  aidx = 0;
  aidx2 = 1;
  ++tcount;
  processOption (["-a5"], aidx, aidx2, "a", &ovint);
  if (ovint != 5) { failures ~= tcount; }
  // int: separated
  ovint = int.init;
  aidx = 0;
  aidx2 = 1;
  ++tcount;
  processOption (["-a", "6"], aidx, aidx2, "a", &ovint);
  if (ovint != 6) { failures ~= tcount; }
  // int: catenated, second position
  ovint = int.init;
  aidx = 0;
  aidx2 = 2;
  ++tcount;
  processOption (["-ab7"], aidx, aidx2, "b", &ovint);
  if (ovint != 7) { failures ~= tcount; }
  // int: separate, second position
  ovint = int.init;
  aidx = 0;
  aidx2 = 2;
  ++tcount;
  processOption (["-ab", "8"], aidx, aidx2, "b", &ovint);
  if (ovint != 8) { failures ~= tcount; }
  // int: no arg
  ovint = int.init;
  aidx = 0;
  aidx2 = 1;
  ++tcount;
  try {
    processOption (["-a"], aidx, aidx2, "a", &ovint);
  } catch {
    ovint = 9;
  }
  if (ovint != 9) { failures ~= tcount; }

  // dbl: catenated
  ovdbl = double.init;
  aidx = 0;
  aidx2 = 1;
  ++tcount;
  processOption (["-a5.5"], aidx, aidx2, "a", &ovdbl);
  if (ovdbl != 5.5) { failures ~= tcount; }
  // dbl: separated
  ovdbl = double.init;
  aidx = 0;
  aidx2 = 1;
  ++tcount;
  processOption (["-a", "6.5"], aidx, aidx2, "a", &ovdbl);
  if (ovdbl != 6.5) { failures ~= tcount; }
  // dbl: catenated, second position
  ovdbl = double.init;
  aidx = 0;
  aidx2 = 2;
  ++tcount;
  processOption (["-ab7.5"], aidx, aidx2, "b", &ovdbl);
  if (ovdbl != 7.5) { failures ~= tcount; }
  // dbl: separate, second position
  ovdbl = double.init;
  aidx = 0;
  aidx2 = 2;
  ++tcount;
  processOption (["-ab", "8.5"], aidx, aidx2, "b", &ovdbl);
  if (ovdbl != 8.5) { failures ~= tcount; }
  // dbl: no arg
  ovdbl = double.init;
  aidx = 0;
  aidx2 = 1;
  ++tcount;
  try {
    processOption (["-a"], aidx, aidx2, "a", &ovdbl);
  } catch {
    ovdbl = 9.5;
  }
  if (ovdbl != 9.5) { failures ~= tcount; }

  // string: catenated
  ovstring = string.init;
  aidx = 0;
  aidx2 = 1;
  ++tcount;
  processOption (["-a5.5x"], aidx, aidx2, "a", &ovstring);
  if (ovstring != "5.5x") { failures ~= tcount; }
  // string: separated
  ovstring = string.init;
  aidx = 0;
  aidx2 = 1;
  ++tcount;
  processOption (["-a", "6.5x"], aidx, aidx2, "a", &ovstring);
  if (ovstring != "6.5x") { failures ~= tcount; }
  // string: catenated, second position
  ovstring = string.init;
  aidx = 0;
  aidx2 = 2;
  ++tcount;
  processOption (["-ab7.5x"], aidx, aidx2, "b", &ovstring);
  if (ovstring != "7.5x") { failures ~= tcount; }
  // string: separate, second position
  ovstring = string.init;
  aidx = 0;
  aidx2 = 2;
  ++tcount;
  processOption (["-ab", "8.5x"], aidx, aidx2, "b", &ovstring);
  if (ovstring != "8.5x") { failures ~= tcount; }
  // string: no arg
  ovstring = string.init;
  aidx = 0;
  aidx2 = 1;
  ++tcount;
  try {
    processOption (["-a"], aidx, aidx2, "a", &ovstring);
  } catch {
    ovstring = "9.5x";
  }
  if (ovstring != "9.5x") { failures ~= tcount; }

  // char: catenated
  ovchar = (char[]).init;
  aidx = 0;
  aidx2 = 1;
  ++tcount;
  processOption (["-a5.5y"], aidx, aidx2, "a", &ovchar);
  if (ovchar != "5.5y") { failures ~= tcount; }
  // char: separated
  ovchar = (char[]).init;
  aidx = 0;
  aidx2 = 1;
  ++tcount;
  processOption (["-a", "6.5y"], aidx, aidx2, "a", &ovchar);
  if (ovchar != "6.5y") { failures ~= tcount; }
  // char: catenated, second position
  ovchar = (char[]).init;
  aidx = 0;
  aidx2 = 2;
  ++tcount;
  processOption (["-ab7.5y"], aidx, aidx2, "b", &ovchar);
  if (ovchar != "7.5y") { failures ~= tcount; }
  // char: separate, second position
  ovchar = (char[]).init;
  aidx = 0;
  aidx2 = 2;
  ++tcount;
  processOption (["-ab", "8.5y"], aidx, aidx2, "b", &ovchar);
  if (ovchar != "8.5y") { failures ~= tcount; }
  // char: no arg
  ovchar = (char[]).init;
  aidx = 0;
  aidx2 = 1;
  ++tcount;
  try {
    processOption (["-a"], aidx, aidx2, "a", &ovchar);
  } catch {
    ovchar = cast(char[]) "9.5y";
  }
  if (ovchar != "9.5y") { failures ~= tcount; }

  auto d = delegate() { ovint = 1; };
  // delegate: no arg
  ++tcount;
  processOption (["-a5.5x"], aidx, aidx2, "a", d);
  if (ovint != 1) { failures ~= tcount; }

  auto d2 = delegate(string arg) { ovstring = arg; };
  // delegate: one arg, catenated
  ++tcount;
  processOption (["-a5.5z"], aidx, aidx2, "a", d2);
  if (ovstring != "5.5z") { failures ~= tcount; }
  // delegate: one arg, separated
  ovstring = string.init;
  aidx = 0;
  aidx2 = 1;
  ++tcount;
  processOption (["-a", "6.5z"], aidx, aidx2, "a", d2);
  if (ovstring != "6.5z") { failures ~= tcount; }
  // delegate: one arg, catenated, second
  ovstring = string.init;
  aidx = 0;
  aidx2 = 2;
  ++tcount;
  processOption (["-ab7.5z"], aidx, aidx2, "b", d2);
  if (ovstring != "7.5z") { failures ~= tcount; }
  // delegate: one arg, separated, second
  ovstring = string.init;
  aidx = 0;
  aidx2 = 2;
  ++tcount;
  processOption (["-ab", "8.5z"], aidx, aidx2, "b", d2);
  if (ovstring != "8.5z") { failures ~= tcount; }
  // delegate: one arg, no arg
  ovstring = string.init;
  aidx = 0;
  aidx2 = 1;
  ++tcount;
  try {
    processOption (["-a"], aidx, aidx2, "a", d2);
  } catch {
    ovstring = "9.5z";
  }
  if (ovstring != "9.5z") { failures ~= tcount; }

  auto d3 = delegate(string opt, string arg) { ovstring = opt ~ arg; };
  // delegate: two arg, catenated
  ++tcount;
  processOption (["-a5.5z"], aidx, aidx2, "a", d3);
  if (ovstring != "a5.5z") { failures ~= tcount; }
  // delegate: two arg, separated
  ovstring = string.init;
  aidx = 0;
  aidx2 = 1;
  ++tcount;
  processOption (["-a", "6.5z"], aidx, aidx2, "a", d3);
  if (ovstring != "a6.5z") { failures ~= tcount; }
  // delegate: two arg, catenated, second
  ovstring = string.init;
  aidx = 0;
  aidx2 = 2;
  ++tcount;
  processOption (["-ab7.5z"], aidx, aidx2, "b", d3);
  if (ovstring != "b7.5z") { failures ~= tcount; }
  // delegate: two arg, separated, second
  ovstring = string.init;
  aidx = 0;
  aidx2 = 2;
  ++tcount;
  processOption (["-ab", "8.5z"], aidx, aidx2, "b", d3);
  if (ovstring != "b8.5z") { failures ~= tcount; }
  // delegate: two arg, no arg
  ovstring = string.init;
  aidx = 0;
  aidx2 = 1;
  ++tcount;
  try {
    processOption (["-a"], aidx, aidx2, "a", d3);
  } catch {
    ovstring = "9.5z";
  }
  if (ovstring != "9.5z") { failures ~= tcount; }

  // functions

  write ("unittest: getopt: processOption: ");
  if (failures.length > 0) {
    writeln ("failed:", failures);
  } else {
    writeln ("passed");
  }
}

