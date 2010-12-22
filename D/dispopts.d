// written in the D programming language

module di_dispopts;

import std.stdio;
import std.ctype;
import std.conv;

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
  short             maxMountString = 15;
  short             maxSpecialString = 18;
  short             maxTypeString = 7;
  short             maxOptString = 0;
  short             maxMntTimeString = 0;
  string            dbsstr = "H";
  string            dispBlockLabel;
  string            blockFormat[];
  string            blockFormatNR[];   /* no radix */
  string            blockLabelFormat[];
  string            inodeFormat[];
  string            inodeLabelFormat[];
  string            mountFormat[];
  string            mTimeFormat[];
  string            optFormat[];
  string            specialFormat[];
  string            typeFormat[];
}

unittest {

}
