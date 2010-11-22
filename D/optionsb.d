module options;

import std.string;
import std.stdio;
import std.getopt;

private string DI_ALL_FORMAT = "MTS\n\tIO\n\tbuf13\n\tbcvpa\n\tBuv2\n\tiUFP";
private string DI_POSIX_FORMAT = "SbuvpM";
// ### move these two
const double DI_VAL_1000 = 1000;
const double DI_VAL_1024 = 1024;

struct Options {
  string              formatString;
  double              baseDispSize;
  short               baseDispIdx;
  string              dbsstr;
  int                 debugLevel;
  string              sortType;
  string              zoneDisplay;
  short               width;
  short               inodeWidth;
  bool                displayAll;
  bool                hasdashk;
  bool                localOnly;
  bool                includeLoopback;
  bool                noHeader;
  bool                posixCompat;
  bool                noQuotaCheck;
  bool                displayTotal;
  bool                debugHeader;
  bool                unknown;
}

Options opts;

// ignoreList
// includeList

void processArgs (char[] str)
{
  processArgs (cast(string) str);
}

void processArgs (string str)
{
  // split string
}

void processArgs (string[] args, size_t offset)
{
  void processBaseSize (string opt, string arg) {
  }

  void processIncludeList (string opt, string arg) {
  }

  void processIgnoreList (string opt, string arg) {
  }

  void processSort (string opt, string arg) {
  }

  getopt (args,
    std.getopt.config.bundling,
    std.getopt.config.caseSensitive,
//    std.getopt.config.stopOnFirstNonOption,
    "A", { opts.formatString = DI_ALL_FORMAT; },
    "a", &opts.displayAll,
    "b", &processBaseSize,
    "B", &processBaseSize,
    "d", &opts.dbsstr,
    "f", &opts.formatString,
    "F", &processIncludeList,
    "g", &opts.dbsstr,
    "h", &opts.dbsstr,
    "H", &opts.dbsstr,
    "i", &processIgnoreList,
    "I", &processIncludeList,
    "k", { opts.dbsstr = "k"; opts.hasdashk = true; },
    "l", &opts.localOnly,
    "L", &opts.includeLoopback,
    "m", &opts.dbsstr,
    "n", &opts.noHeader,
    "P", { if (!opts.hasdashk) { opts.dbsstr = "512"; }
           opts.formatString = DI_POSIX_FORMAT;
           opts.posixCompat = true; },
    "q", &opts.noQuotaCheck,
    "s", &processSort,
    "t", &opts.displayTotal,
    "w", &opts.width,
    "W", &opts.inodeWidth,
    "x", &processIgnoreList,
    "X", { },
    "z", &opts.zoneDisplay,
    "Z", { opts.zoneDisplay = "all"; }
    );
writef ("formatString: %s\n", opts.formatString);
writef ("displayAll: %d\n", opts.displayAll);
}
