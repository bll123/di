/*
$Id$
$Source$
Copyright 1994-2004 Brad Lanam, Walnut Creek, CA
*/

#include "config.h"
#include "di.h"    /* for macro conversions */

#if ! defined (_lib_getopt)

/*
 *
 * $Id$
 *
 * getopt - get option letter from argv
 *
 * from Cnews by Henry Spencer
 *
 */

#include <stdio.h>
#if _hdr_string
# include <string.h>
#endif
#if _hdr_strings && ((! defined (_hdr_string)) || (_include_string))
# include <strings.h>
#endif

char    *optarg;    /* Global argument pointer. */
int     optind = 0; /* Global argv index. */

static char *scan = NULL;   /* Private scan pointer. */

int
#if _proto_stdc
getopt (int argc, char *argv [], char *optstring)
#else
getopt (argc, argv, optstring)
    int argc;
    char *argv[];
    char *optstring;
#endif
{
    char c;
    char *place;


    optarg = NULL;

    if (scan == NULL || *scan == '\0')
    {
        if (optind == 0)
        {
            optind++;
        }

        if (optind >= argc || argv[optind][0] != '-' ||
                argv[optind][1] == '\0')
        {
            return(EOF);
        }

        if (strcmp(argv[optind], "--")==0)
        {
            optind++;
            return(EOF);
        }

        scan = argv[optind]+1;
        optind++;
    }

    c = *scan++;
    place = strchr(optstring, c);

    if (place == NULL || c == ':')
    {
        fprintf(stderr, "%s: unknown option -%c\n", argv[0], c);
        return('?');
    }

    place++;
    if (*place == ':')
    {
        if (*scan != '\0')
        {
            optarg = scan;
            scan = NULL;
        }
        else if (optind < argc)
        {
            optarg = argv[optind];
            optind++;
        }
        else
        {
            fprintf(stderr, "%s: -%c argument missing\n", argv[0], c);
            return('?');
        }
    }

    return(c);
}

#else

extern int di_lib_debug;

#endif
