// written in the D programming language

module di_display;

import std.stdio;
import std.string;
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

DisplayOpts dispOpts;

struct DisplayTable {
  char          key;
  string        suffix;
  real          size;
  real          low;
  real          high;
  string        disp[2];
};

DisplayTable[] displayTable = [
  // key,suffix,size,low,high,disp[2]
  { 'b', " ", 0, 0, 0, [ "Bytes", "Bytes" ] },
  { 'k', "K", 0, 0, 0, [ "KBytes", "KBytes" ] },
  { 'm', "M", 0, 0, 0, [ "Megs", "Mebis" ] },
  { 'g', "G", 0, 0, 0, [ "Gigs", "Gibis" ] },
  { 't', "T", 0, 0, 0, [ "Teras", "Tebis" ] },
  { 'p', "P", 0, 0, 0, [ "Petas", "Pebis" ] },
  { 'e', "E", 0, 0, 0, [ "Exas", "Exbis" ] },
  { 'z', "Z", 0, 0, 0, [ "Zettas", "Zebis" ] },
  { 'y', "Y", 0, 0, 0, [ "Yottas", "Yobis" ] },
  { 'h', "h", 0, 0, 0, [ "Size", "Size" ] },
  { 'H', "H", 0, 0, 0, [ "Size", "Size" ]}
  ];

int[char] displayIdxs;
bool      dispIdxsInitialized;

void
initializeDisp ()
{
  initializeDisplayIdxs ();
  setDispBlockSize ();
}

void
dumpDispTable ()
{
  foreach (disp; displayTable) {
    writefln ("%s %s %-6s \n    sz:%40.30g \n   low:%40.30g \n  high:%40.30g", disp.key, disp.suffix, disp.disp[0], disp.size, disp.low, disp.high);
  }
}

private:

void
initializeDisplayIdxs ()
{
  if (! dispIdxsInitialized) {
    foreach (idx, dt; displayTable) {
      displayIdxs[dt.key] = idx;
    }
    dispIdxsInitialized = true;
  }
}

void
setDispBlockSize ()
{
  size_t        i;
  real          val;
  char          c;

  initializeDisplayIdxs ();
  i = 0;
  c = dispOpts.dbsstr[0];
  if (isdigit (c)) {
    val = to!(typeof(val))(dispOpts.dbsstr);
    foreach (key, disp; displayTable) {
      if (val == disp.size) {
        c = disp.key;
      }
    }
  } else {
    if (c != 'H') {
      c = cast(char) tolower (c);
    }
    val = 1.0;
    if (dispOpts.dbsstr == "HUMAN") {
      c = 'h';
    }
    if (dispOpts.dbsstr.length == 2 && dispOpts.dbsstr[1] == 'B') {
      dispOpts.baseDispSize = size1000;
      dispOpts.baseDispIdx = idx1000;
    }
  }

  if (c in displayIdxs) {
    int idx = displayIdxs[c];
    val *= displayTable [idx].size;
    dispOpts.dispBlockLabel = displayTable [idx].disp [dispOpts.baseDispIdx];
  } else {
    dispOpts.dispBlockLabel = ""; //sprintf ("%.0fb", val);
  }

  if (dispOpts.posixCompat && val == 512.0)
  {
    dispOpts.dispBlockLabel = "512-blocks";
  }
  if (dispOpts.posixCompat && val == 1024.0)
  {
    dispOpts.dispBlockLabel = "1024-blocks";
  }

  dispOpts.dispBlockSize = val;

//  displayTable [0].format = dispOpts.blockFormatNR;
//  displayTable [1].format = dispOpts.blockFormat;

  displayTable [0].size = 1;
  displayTable [0].low = 1;
  displayTable [0].high = dispOpts.baseDispSize;
  for (i = 1; i < displayTable.length; ++i)
  {
    if (displayTable [i].key == 'h') { break; }
//    displayTable [i].format = dispOpts.blockFormat;
    displayTable [i].size = displayTable [i - 1].size * dispOpts.baseDispSize;
    displayTable [i].low = displayTable [i - 1].low * dispOpts.baseDispSize;
    displayTable [i].high = displayTable [i - 1].high * dispOpts.baseDispSize;
  }
  displayTable [0].low = 0;
}

unittest {

}
