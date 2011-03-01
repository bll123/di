// written in the D programming language

module didispopts;

import std.stdio;
import std.string;

import config;

enum real
  size1000 = 1000.0,
  size1024 = 1024.0;
enum int
  idx1000 = 0,
  idx1024 = 1;

struct DisplayOpts {
  bool              posixCompat = false;
  real              dispBlockSize = 0;
  real              baseDispSize = size1024;
  int               baseDispIdx = idx1024;
  short             width = 8;
  short             inodeWidth = 7;
  string            dbsstr = "H";
  string            dispBlockLabel;
}

