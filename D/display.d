// written in the D programming language

module di_display;

import std.stdio;
import std.string;
import std.ctype;
import std.conv;

import config;
import dispopts;

struct DisplayTable {
  real          size;
  real          low;
  real          high;
  char          key;
  string        suffix;
  string        disp[2];
};

DisplayTable[] displayTable = [
  // size,low,high,key,suffix,disp[2]
  { 0, 0, 0, 'b', " ", [ "Bytes", "Bytes" ] },
  { 0, 0, 0, 'k', "K", [ "KBytes", "KBytes" ] },
  { 0, 0, 0, 'm', "M", [ "Megs", "Mebis" ] },
  { 0, 0, 0, 'g', "G", [ "Gigs", "Gibis" ] },
  { 0, 0, 0, 't', "T", [ "Teras", "Tebis" ] },
  { 0, 0, 0, 'p', "P", [ "Petas", "Pebis" ] },
  { 0, 0, 0, 'e', "E", [ "Exas", "Exbis" ] },
  { 0, 0, 0, 'z', "Z", [ "Zettas", "Zebis" ] },
  { 0, 0, 0, 'y', "Y", [ "Yottas", "Yobis" ] },
  { 0, 0, 0, 'h', "h", [ "Size", "Size" ] },
  { 0, 0, 0, 'H', "H", [ "Size", "Size" ] }
  ];

size_t[char]    displayIdxs;
bool            dispIdxsInitialized;

void
dumpDispTable ()
{
  foreach (disp; displayTable) {
    writefln ("%s %s %-6s \n    sz:%40.30g \n   low:%40.30g \n  high:%40.30g", disp.key, disp.suffix, disp.disp[0], disp.size, disp.low, disp.high);
  }
}

void
setDispBlockSize (ref DisplayOpts dispOpts)
{
  size_t        i;
  real          val;
  char          c;

  initializeDisplayIdxs ();

  c = dispOpts.dbsstr[0];
  if (isdigit (c)) {
    val = to!(typeof(val))(dispOpts.dbsstr);
    foreach (disp; displayTable) {
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
    auto idx = displayIdxs [c];
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
  displayTable [0].low = 1; // temporary, reset below
  displayTable [0].high = dispOpts.baseDispSize;
  for (i = 1; i < displayTable.length; ++i)
  {
    if (displayTable [i].key == 'h') { continue; }
//    displayTable [i].format = dispOpts.blockFormat;
    displayTable [i].size = displayTable [i - 1].size * dispOpts.baseDispSize;
    displayTable [i].low = displayTable [i - 1].low * dispOpts.baseDispSize;
    displayTable [i].high = displayTable [i - 1].high * dispOpts.baseDispSize;
  }
  displayTable [0].low = 0;
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

unittest {

}
