/*
$Id$
$Source$
Copyright 2005 Brad Lanam, Walnut Creek, CA
*/

#include "config.h"
#include <stdio.h>
#include <stdlib.h>

extern int di_lib_debug;

int
#if _proto_stdc
main (int argc, char *argv [])
#else
main (argc, argv)
    int                 argc;
    char                *argv [];
#endif
{
#if _enable_nls
    exit (0);
#else
    exit (1);
#endif
}
