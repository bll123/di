// written in the D programming language

import std.stdio: writef;
import std.string;

import config;
import options;
import display;

void main (string[] args)
{
  getDIOptions (args);
  initializeDisp ();
  dumpDispTable ();
}
