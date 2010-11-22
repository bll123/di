/*
$Id$
$Source$
*/

module getopt;

import std.string;
import std.conv;
import std.stdio;

int
getopts (OVL...) (string[] args, OVL opts)
{
  assert (opts.length % 2 == 0);
  int   i;
  int   j;

  for (i = 0; i < args.length; ++i) {
    j = 1;
    if (args[i][0] != '-' || args[i] == "--") {
      break;
    }
writef ("getopts: %d: %s\n", i, args[i][j]);
    int rc = 0;
    while (j < args[i].length && args[i][j] != '\0') {
writef ("getopts: i: %d j: %d len:%d %s\n", i, j, args[i].length, args[i][j]);
      rc = checkOption (args, i, j, opts);
      if (rc != 0) {
writef ("getopts: rc: %d\n", rc);
        i += rc;
        break;
      }
      ++j;
    }
  }
  return i;
}

private int checkOption (OVL...)
    (string[] args, int aidx, int aidx2, OVL opts)
{
  static if (opts.length > 0) {
    auto opt = to!string(opts[0]);
    if (opt[0] == args[aidx][aidx2]) {
writef ("found: %d %d: %s\n", aidx, aidx2, args[aidx][aidx2]);
writef ("  chk: %s\n", typeof(opts[0]).stringof);
writef ("  chk: %s\n", typeof(opts[1]).stringof);
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

  string    arg;
  bool      attachedArg = false;
  if (args[aidx].length > aidx2) {
    auto i = aidx2 + 1;
    arg = args[aidx][i..$];
    attachedArg = true;
  } else {
    arg = args[aidx + 1];
  }

  static if (is(typeof(*ov) == bool)) {
writef ("A:proc: %s: %s %s\n", oa, arg, typeof(*ov).stringof);
    *ov = true;
  } else static if (is(typeof(ov) == delegate)) {
writef ("B:proc: %s: %s %s\n", oa, arg, typeof(ov()).stringof);
    ov ();
  } else static if (is(typeof(*ov) == function)) {
writef ("FG: %s: function\n", oa);
    static if (is(typeof(ov("")) : void)) {
writef ("F:proc: %s: %s %s\n", oa, arg, typeof(ov("")).stringof);
      ov (arg);
    }
    static if (is(typeof(ov("","")) : void)) {
writef ("G:proc: %s: %s %s\n", oa, arg, typeof(ov("","")).stringof);
      ov (oa,arg);
    }
    if (! attachedArg) { rc += 1; }
    rc += 1;
  } else static if (is(typeof(*ov) == string)) {
writef ("C:proc: %s: %s %s\n", oa, arg, typeof(*ov).stringof);
    *ov = arg;
    if (! attachedArg) { rc += 1; }
    rc += 1;
  } else static if (is(typeof(*ov) : real)) {
writef ("D:proc: %s: %s %s\n", oa, arg, typeof(*ov).stringof);
    *ov = to!(typeof(*ov))(arg);
    if (! attachedArg) { rc += 1; }
    rc += 1;
  } else {
writef ("E:proc: %s: %s %s\n", oa, arg, typeof(ov).stringof);
  }

  return rc;
}

