/*
$Id$
$Source$
Copyright 2006 Brad Lanam, Walnut Creek, CA
*/

#include "config.h"
#include <stdio.h>
#if _hdr_stdlib
# include <stdlib.h>
#endif

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
