// written in the D programming language

import std.stdio: writef;
import std.string;

import config;
import options;
import dispopts;
import diskpart;
import display;

void main (string[] args)
{
  Options       opts;
  DisplayOpts   dispOpts;

  getDIOptions (args, opts, dispOpts);

  auto dpList = new DiskPartitions (opts.debugLevel);
  dpList.getEntries ();
  dpList.getPartitionInfo ();
  displayAll (opts, dispOpts, dpList);
}
