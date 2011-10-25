// written in the D programming language

module dihelp;

import std.stdio;
import config;
import dilocale;

void
dispVersion ()
{
  writefln (DI_GT("di version %s    Default Format: %s"), DI_VERSION, DI_DEFAULT_FORMAT);
}

void
usage ()
{
    dispVersion ();
                /*  12345678901234567890123456789012345678901234567890123456789012345678901234567890 */
    writeln (DI_GT("Usage: di [-ant] [-d display-size] [-f format] [-x exclude-fstyp-list]"));
    writeln (DI_GT("       [-I include-fstyp-list] [file [...]]"));
    writeln (DI_GT("   -a   : print all mounted devices"));
    writeln (DI_GT("   -d x : size to print blocks in (512 - POSIX, k - kbytes,"));
    writeln (DI_GT("          m - megabytes, g - gigabytes, t - terabytes, h - human readable)."));
    writeln (DI_GT("   -f x : use format string <x>"));
    writeln (DI_GT("   -I x : include only file system types in <x>"));
    writeln (DI_GT("   -x x : exclude file system types in <x>"));
    writeln (DI_GT("   -l   : display local filesystems only"));
    writeln (DI_GT("   -n   : don't print header"));
    writeln (DI_GT("   -t   : print totals"));
    writeln (DI_GT(" Format string values:"));
    writeln (DI_GT("    m - mount point                     M - mount point, full length"));
    writeln (DI_GT("    b - total kbytes                    B - kbytes available for use"));
    writeln (DI_GT("    u - used kbytes                     c - calculated kbytes in use"));
    writeln (DI_GT("    f - kbytes free                     v - kbytes available"));
    writeln (DI_GT("    p - percentage not avail. for use   1 - percentage used"));
    writeln (DI_GT("    2 - percentage of user-available space in use."));
    writeln (DI_GT("    i - total file slots (i-nodes)      U - used file slots"));
    writeln (DI_GT("    F - free file slots                 P - percentage file slots used"));
    writeln (DI_GT("    s - filesystem name                 S - filesystem name, full length"));
    writeln (DI_GT("    t - disk partition type             T - partition type, full length"));
    writeln (DI_GT("See manual page for more options."));
}
