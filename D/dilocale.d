// written in the D programming language

import std.string;
import std.conv : to;
private import std.process : getenv;

import config;

enum string DI_LOCALE_DIR = DI_PREFIX ~ "/share/locale";

static if (_enable_nls) {
  auto DI_GT (string txt) { return (to!string(gettext(toStringz(txt)))); }
} else {
  auto DI_GT (string txt) { return (txt); }
}

void
initLocale ()
{
  string        localeptr = null;

  static if (_clib_setlocale) {
    setlocale (LC_ALL, "");
  }
  static if (_enable_nls) {
    if ((localeptr = getenv ("DI_LOCALE_DIR")) == cast(string) null) {
      localeptr = DI_LOCALE_DIR;
    }
    bindtextdomain (toStringz(DI_PROG), toStringz(localeptr));
    textdomain (toStringz(DI_PROG));
  }
}
